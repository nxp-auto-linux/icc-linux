/**
*   @file    ICC_dev.c
*   @version 0.0.2
*
*   @brief   ICC - Inter Core Communication device driver
*   @details       Inter Core Communication device driver
*
*   @addtogroup [ICC]
*   @{
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : ARM
*   Peripheral           :
*   Dependencies         : none
*
*   Build Version        :
*
*   (c) Copyright 2014,2016 Freescale Semiconductor Inc.
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License
*   as published by the Free Software Foundation; either version 2
*   of the License, or (at your option) any later version
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
==================================================================================================*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>

#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>

#include "ICC_Api.h"

MODULE_DESCRIPTION("ICC device");
MODULE_AUTHOR("Freescale Semiconductor");
MODULE_LICENSE("GPL");

#define MODULE_NAME     "ICC"
#define BASEMINOR       0
#define NUM_MINORS      1

#define LOG_LEVEL       KERN_ALERT
#define PARAM_PERM      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP

//#define USE_INTERRUPTS

/* FIXME remove hardcoding of addresses. */

#define EP_PCIE_BASE_ADDR 0x72000000
#define EP_DDR_ADDR 0x8FF00000
#define EP_PCIE_DBI      0x72ffc000 /* PCIe DBI for S32V, from dtb */
#define EP_PCIE_DBI_SIZE 0x4000 /* PCIe DBI region size from dtb */

/* Physical memory mapped by the RC CPU.
 * The RC's shared DDR mapping is different in the Bluebox vs EVB case.
 * For the moment, this setting is statically defined in the Makefile.
 */
#if defined PCIE_SHMEM_BLUEBOX  /* LS2-S32V */
#define RC_DDR_ADDR     0x8350000000
#define EP_BAR2_ADDR    0x1440100000ll
#else                           /* EVB-PCIE */
#define RC_DDR_ADDR     0x8FF00000
#define EP_BAR2_ADDR    0x72200000ll
#endif

#ifndef ICC_DO_NOT_USE_INTERRUPTS
#define ICC_CONFIG_OFFSET_FROM_BASE 0
#else
/* first 32bits are for the poll variable */
#define ICC_CONFIG_OFFSET_FROM_BASE 4
#endif

#ifdef ICC_BUILD_FOR_M4
    /* M4 stands for the module replacing the RTOS app, that is the one initializing the shared mem.
     * In our case that would be the PCIe EndPoint. */
    #define IRAM_BASE_ADDR EP_DDR_ADDR
    #define IRAM_PING_ADDR EP_PCIE_BASE_ADDR
    #define IRAM_POLL_ADDR EP_DDR_ADDR

#else

    #define IRAM_BASE_ADDR EP_BAR2_ADDR
    #define IRAM_PING_ADDR EP_BAR2_ADDR
    #define IRAM_POLL_ADDR RC_DDR_ADDR

#endif

#define MAP_DDR_SIZE    1024 * 1024 * 1 /* 1 MB */

/* Value to wait for */
#define WAKE_UP_PATTERN     0x42

#ifndef ICC_BUILD_FOR_M4
ICC_Config_t * ICC_Config_Ptr_M4 = NULL;
ICC_Config_t * ICC_Config_Ptr_M4_Remote = NULL;
#endif

struct ICC_device_data {
    struct cdev cdev;
};

struct ICC_device_data devs[NUM_MINORS];
static dev_t dev_no;

#ifndef ICC_DO_NOT_USE_INTERRUPTS

struct ICC_platform_data {
    struct device * dev;
    unsigned int shared_irq;
    unsigned int local_irq;
};

struct ICC_platform_data * icc_data = NULL;

static struct of_device_id ICC_dt_ids[] = {
    {
        .compatible = "fsl,s32v234-icc",
    },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ICC_dt_ids);

static int ICC_probe(struct platform_device *pdev)
{
    icc_data = devm_kzalloc(&pdev->dev, sizeof(*icc_data), GFP_KERNEL);
    if (!icc_data)
        return -ENOMEM;

    icc_data->dev = &pdev->dev;
    icc_data->shared_irq = platform_get_irq(pdev, ICC_CFG_HW_CPU2CPU_IRQ);
#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
    icc_data->local_irq = platform_get_irq(pdev, ICC_CFG_HW_LOCAL_IRQ);
#endif

    platform_set_drvdata(pdev, icc_data);

    return 0;
}

static int ICC_remove(struct platform_device *pdev)
{
    if (icc_data) {
        platform_set_drvdata(pdev, NULL);
        devm_kfree(&pdev->dev, icc_data);
        icc_data = NULL;
    }
    return 0;
}

static int ICC_suspend(struct device *dev)
{
    return 0;
}

static int ICC_resume(struct device *dev)
{
    return 0;
}

static SIMPLE_DEV_PM_OPS(ICC_pm_ops, ICC_suspend, ICC_resume);

static struct platform_driver ICC_driver = {
	.probe		= ICC_probe,
	.remove		= ICC_remove,
	.driver		= {
		.name	= MODULE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table	= ICC_dt_ids,
		.pm	= &ICC_pm_ops,
	},
};

#else

#include <linux/kthread.h>
#include <linux/delay.h>

u8 *poll_addr = NULL;
u8 *ping_addr = NULL;

#define POLL_TIMEOUT_MS 1

struct task_struct *poll_thread;

ICC_ATTR_SEC_TEXT_CODE
extern void
ICC_Remote_Event_Handler(void);

static int poll_thread_fn(void *arg)
{
    u8 *addr = (u8 *)arg;

    while (!kthread_should_stop()) {
        msleep_interruptible(POLL_TIMEOUT_MS);
        if (*addr == WAKE_UP_PATTERN) {
            /* event detected */
            ICC_Remote_Event_Handler();
            *addr = 0;
        }
    }

    return 0;
}

static int pcie_shmem_poll_init(void)
{
    int err = 0;

    poll_addr = (u8 *)ioremap_nocache(IRAM_POLL_ADDR, SZ_4K);
    if (!poll_addr) {
        pr_warn("Could not ioremap %#x\n", IRAM_POLL_ADDR);
        err = -EIO;
        goto ioremap_error;
    }
    /* Initialize it before we poll for changes */
    *poll_addr = 0;

    /* Start the polling thread */
    poll_thread = kthread_run(poll_thread_fn, poll_addr, "shmem_poll_thread");
    if (IS_ERR(poll_thread)) {
        pr_warn("Error starting poll thread");
        err = -EFAULT;
        goto poll_thread_error;
    }

    return 0;

poll_thread_error:
    iounmap(poll_addr);
    poll_addr = NULL;
ioremap_error:
    return err;
}

static void pcie_shmem_poll_exit(void)
{
    if (poll_thread) {
        kthread_stop(poll_thread);
        poll_thread = NULL;
    }
    iounmap(poll_addr);
    poll_addr = NULL;
}

static int pcie_shmem_ping_init(void)
{
    int err = 0;

    ping_addr = (u8 *)ioremap_nocache(IRAM_PING_ADDR, SZ_4K);
    if (!ping_addr) {
        pr_warn("Could not ioremap %#x\n", IRAM_PING_ADDR);
        return -EIO;
    }

    return 0;
}

static void pcie_shmem_ping_exit(void)
{
    iounmap(ping_addr);
    ping_addr = NULL;
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Remote( void )
{
    *ping_addr = WAKE_UP_PATTERN;
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_Remote( void )
{
    // nothing to do, remote peer must clear its flag.
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Local( void )
{
    // not supported
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_Local( void )
{
    // not supported
}
#endif

#endif  /* USE_INTERRUPTS */

/* following code is taken from the pcie driver (ioctl functions are not exported) */

struct s32v_inbound_region {
    unsigned int  bar_nr;
    unsigned int  target_addr;
    unsigned int  region;
};

struct s32v_outbound_region {
    unsigned long long int target_addr;
    unsigned long long int base_addr;
    unsigned int  size;
    unsigned int  region;
    unsigned int  region_type;
};

static struct s32v_inbound_region inb1 = {
    2,      /* BAR2 */
    EP_DDR_ADDR,    /* locally-mapped DDR on EP */
    0       /* region 0 */
};

/* Outbound region structure */
static struct s32v_outbound_region outb1 = {
    RC_DDR_ADDR,    /* target_addr */
    EP_PCIE_BASE_ADDR,    /* base_addr */
    MAP_DDR_SIZE,   /* size >= 64K(min for PCIE on S32V) */
    0,      /* region number */
    0       /* region type = mem */
};

#define NR_REGIONS      4

#define PCIE_ATU_VIEWPORT       0x900
#define PCIE_ATU_REGION_INBOUND     (0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND    (0x0 << 31)
#define PCIE_ATU_REGION_INDEX1      (0x1 << 0)
#define PCIE_ATU_REGION_INDEX0      (0x0 << 0)
#define PCIE_ATU_CR1            0x904
#define PCIE_ATU_TYPE_MEM       (0x0 << 0)
#define PCIE_ATU_TYPE_IO        (0x2 << 0)
#define PCIE_ATU_TYPE_CFG0      (0x4 << 0)
#define PCIE_ATU_TYPE_CFG1      (0x5 << 0)
#define PCIE_ATU_CR2            0x908
#define PCIE_ATU_ENABLE         (0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE    (0x1 << 30)
#define PCIE_ATU_BAR_NUM(bar)   ((bar) << 8)
#define PCIE_ATU_LOWER_BASE     0x90C
#define PCIE_ATU_UPPER_BASE     0x910
#define PCIE_ATU_LIMIT          0x914
#define PCIE_ATU_LOWER_TARGET       0x918
#define PCIE_ATU_BUS(x)         (((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)         (((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)        (((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET       0x91C

char * PCIE_DBI_Virt_Base_Addr = NULL;

static int s32v_pcie_iatu_outbound_set(struct s32v_outbound_region *ptrOutb)
{
    int ret = 0;

    if (!PCIE_DBI_Virt_Base_Addr || !ptrOutb) {
       return -EINVAL;
    }

    if ((ptrOutb->size < (64 * SZ_1K)) ||
        (ptrOutb->region > NR_REGIONS - 1))
        return -EINVAL;

    writel(PCIE_ATU_REGION_OUTBOUND | ptrOutb->region,
        EP_PCIE_DBI + PCIE_ATU_VIEWPORT);
    writel(lower_32_bits(ptrOutb->base_addr),
        PCIE_DBI_Virt_Base_Addr +  PCIE_ATU_LOWER_BASE);
    writel(upper_32_bits(ptrOutb->base_addr),
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_UPPER_BASE);
    writel(lower_32_bits(ptrOutb->base_addr + ptrOutb->size - 1),
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_LIMIT);
    writel(lower_32_bits(ptrOutb->target_addr),
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_LOWER_TARGET);
    writel(upper_32_bits(ptrOutb->target_addr),
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_UPPER_TARGET);
    writel(ptrOutb->region_type, PCIE_DBI_Virt_Base_Addr + PCIE_ATU_CR1);
    writel(PCIE_ATU_ENABLE, PCIE_DBI_Virt_Base_Addr + PCIE_ATU_CR2);

    return ret;
}

static int s32v_pcie_iatu_inbound_set(struct s32v_inbound_region *ptrInb)
{
    int ret = 0;

    if (!PCIE_DBI_Virt_Base_Addr || !ptrInb) {
       return -EINVAL;
    }
    if (ptrInb->region > NR_REGIONS - 1) {
       return -EINVAL;
    }

    writel(PCIE_ATU_REGION_INBOUND | ptrInb->region,
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_VIEWPORT);
    writel(lower_32_bits(ptrInb->target_addr),
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_LOWER_TARGET);
    writel(upper_32_bits(ptrInb->target_addr),
        PCIE_DBI_Virt_Base_Addr + PCIE_ATU_UPPER_TARGET);
    writel(PCIE_ATU_TYPE_MEM, PCIE_DBI_Virt_Base_Addr + PCIE_ATU_CR1);
    writel(PCIE_ATU_ENABLE | PCIE_ATU_BAR_MODE_ENABLE |
        PCIE_ATU_BAR_NUM(ptrInb->bar_nr), PCIE_DBI_Virt_Base_Addr + PCIE_ATU_CR2);

    return ret;
}

/* end of pcie driver code */

static int pcie_mappings_init(void)
{
    int ret = 0;

    PCIE_DBI_Virt_Base_Addr = ioremap_nocache(EP_PCIE_DBI, EP_PCIE_DBI_SIZE);
    printk(LOG_LEVEL "[pcie_mappings_init] PCIE DBI reserved %16llx size is %d\n", PCIE_DBI_Virt_Base_Addr, sizeof(char * ) );
    if( !PCIE_DBI_Virt_Base_Addr ){
        printk(KERN_ERR "[pcie_mappings_init] PCIE DBI virtual mapping has failed for 0x%08x\n", EP_PCIE_DBI);
        ret = -ENOMEM;
        goto err;
    }

    /* Setup outbound window for accessing RC mem */
    ret = s32v_pcie_iatu_outbound_set(&outb1);
    if (ret < 0) {
        printk(KERN_ERR "[pcie_mappings_init] Error while setting outbound1 region\n");
        goto err;
    } else {
        printk("Outbound1 region setup successfully\n");
    }

    /* Same thing for inbound window for transactions from RC */
    ret = s32v_pcie_iatu_inbound_set(&inb1);
    if (ret < 0) {
        printk(KERN_ERR "[pcie_mappings_init] Error while setting inbound1 region\n");
        goto err;
    } else {
        printk("Inbound1 region setup successfully\n");
    }

err:
    return ret;
}

static void pcie_mappings_exit(void)
{
    iounmap(PCIE_DBI_Virt_Base_Addr);
    PCIE_DBI_Virt_Base_Addr = NULL;
}

static int ICC_dev_open(struct inode *inode, struct file *file)
{
    struct ICC_device_data *data = container_of(inode->i_cdev,
                                                struct ICC_device_data,
                                                cdev);
    file->private_data = data;

    printk(LOG_LEVEL "Opened ICC_library device\n");
    return 0;
}

static int ICC_dev_release(struct inode *inode, struct file *file)
{
    /* struct ICC_device_data *data = (struct ICC_device_data *)file->private_data; */

    printk(LOG_LEVEL "Closed ICC_library device\n");

    return 0;
}

static const struct file_operations ICC_fops = {
    .owner  = THIS_MODULE,
    .open   = ICC_dev_open,
    .release = ICC_dev_release,
};

char * ICC_Shared_Virt_Base_Addr = NULL;
struct resource * ICC_Mem_Region = NULL;

#ifdef ICC_DO_NOT_USE_INTERRUPTS
struct resource * ICC_Ping_Region = NULL;
#endif

static void local_cleanup(void)
{
    if (ICC_Shared_Virt_Base_Addr) {
        iounmap(ICC_Shared_Virt_Base_Addr);
        ICC_Shared_Virt_Base_Addr = NULL;
#ifndef ICC_BUILD_FOR_M4
        ICC_Config_Ptr_M4 = NULL;
        ICC_Config_Ptr_M4_Remote = NULL;
#endif
    }

    if (ICC_Mem_Region) {
        release_mem_region(IRAM_BASE_ADDR, MAP_DDR_SIZE);
    }

    pcie_mappings_exit();

#ifndef ICC_DO_NOT_USE_INTERRUPTS
    platform_driver_unregister(&ICC_driver);
#else
    pcie_shmem_poll_exit();
    pcie_shmem_ping_exit();
#endif
}

union local_magic {
	char str[ICC_CONFIG_MAGIC_SIZE];
	struct {
		u64 m0, m1;
	} raw;
} ICC_Local_Magic = { ICC_CONFIG_MAGIC };

static int __init ICC_dev_init(void)
{
    int err;
    int i;
    union local_magic * shared_start = NULL;

    printk(LOG_LEVEL "[ICC_dev_init] Freescale ICC linux driver\n");

#ifndef ICC_DO_NOT_USE_INTERRUPTS
    err = platform_driver_register(&ICC_driver);
    if (err) {
        return err;
    }
#endif

    /* Initialize shared memory */
    ICC_Mem_Region = request_mem_region(IRAM_BASE_ADDR, MAP_DDR_SIZE, "ICC_shmem");
    if (!ICC_Mem_Region) {
        printk (KERN_ERR "[ICC_dev_init] Failed to request mem region!\n");
        err = -ENOMEM;
        goto cleanup;
    }

    /* ICC Shared mem is mapped differently on RC and EP, but in both cases it physically
     * resides on EP side.
     */
    ICC_Shared_Virt_Base_Addr = ioremap_nocache(IRAM_BASE_ADDR + ICC_CONFIG_OFFSET_FROM_BASE, MAP_DDR_SIZE);
    printk(LOG_LEVEL "[ICC_dev_init] reserved %16llx size is %d\n", ICC_Shared_Virt_Base_Addr, sizeof(char * ) );
    if( !ICC_Shared_Virt_Base_Addr ){
        printk(KERN_ERR "[ICC_dev_init] ICC_Shared_Virt_Base_Addr virtual mapping has failed for 0x%08x\n", IRAM_BASE_ADDR);
        err = -ENOMEM;
        goto cleanup;
    }

#ifndef ICC_BUILD_FOR_M4

    /* discover location of the configuration
     * TODO: should we use another offset in shm to store the ICC_Config0 offset?
     */
    ICC_Config_Ptr_M4 = (ICC_Config_t *)(ICC_Shared_Virt_Base_Addr + ICC_CONFIG_OFFSET_FROM_BASE);
    shared_start = (union local_magic *)ICC_Config_Ptr_M4;
    for (i = 0; i < MAP_DDR_SIZE / sizeof(union local_magic); i++, shared_start++) {
	    if ((ICC_Local_Magic.raw.m0 == shared_start->raw.m0) &&
		(ICC_Local_Magic.raw.m1 == shared_start->raw.m1)){
		    ICC_Config_Ptr_M4 = (ICC_Config_t *)shared_start;
		    printk(LOG_LEVEL "[ICC_dev_init] ICC Config found at address %16llx\n", ICC_Config_Ptr_M4);
	    }
    }

    ICC_Config_Ptr_M4_Remote = ICC_Config_Ptr_M4->This_Ptr;

    printk(LOG_LEVEL "[ICC_dev_init] ICC Config local virtual address: 0x%08x \n", ICC_Config_Ptr_M4);
    printk(LOG_LEVEL "[ICC_dev_init] ICC Config remote virtual address: 0x%08x \n", ICC_Config_Ptr_M4_Remote);
#endif

#ifdef ICC_BUILD_FOR_M4
    /* setup PCIE */
    pcie_mappings_init();
#endif

#ifdef ICC_DO_NOT_USE_INTERRUPTS
    pcie_shmem_poll_init();
    pcie_shmem_ping_init();
#endif

    /* register device */
    err = alloc_chrdev_region(&dev_no, BASEMINOR, NUM_MINORS, MODULE_NAME); 

    if (err < 0) {
        printk(KERN_ERR "[ICC_dev_init] Major number allocation has failed\n");
        goto cleanup;
    }

    printk(LOG_LEVEL "[ICC_dev_init] Major number %d\n", MAJOR(dev_no));

    /* initialize each device */
    for (i = 0; i < NUM_MINORS; i++) {
        cdev_init(&devs[i].cdev, &ICC_fops);
        cdev_add(&devs[i].cdev, MKDEV(MAJOR(dev_no), i),1);
    }

    return 0;

cleanup:
    local_cleanup();

    return err;
}

static void __exit ICC_dev_exit(void)
{
    int i;

    for (i = 0; i < NUM_MINORS; i++)
        cdev_del(&devs[i].cdev);

    unregister_chrdev_region(dev_no, NUM_MINORS);

    local_cleanup();

    printk(LOG_LEVEL "[ICC_dev_exit] \n");
}

#ifndef ICC_DO_NOT_USE_INTERRUPTS

struct device * ICC_get_device(void)
{
    if (icc_data) {
        return icc_data->dev;
    }

    printk(KERN_ERR "[ICC_get_device] Invalid (NULL) internal data\n");
    return NULL;
}

char * ICC_get_device_name(void)
{
    return MODULE_NAME;
}

unsigned int ICC_get_shared_irq(void)
{
    if (icc_data) {
        return icc_data->shared_irq;
    }

    printk(KERN_ERR "[ICC_get_shared_irq] Invalid (NULL) internal data\n");
    return (unsigned int)(-1);
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
unsigned int ICC_get_local_irq(void)
{
    if (icc_data) {
        return icc_data->local_irq;
    }

    printk(KERN_ERR "[ICC_get_local_irq] Invalid (NULL) internal data\n");
    return (unsigned int)(-1);
}
#endif

#endif  /* USE_INTERRUPTS */

/**
 * Exporting ICC API functions to be used by other kernel modules
 */
EXPORT_SYMBOL(ICC_Initialize);
EXPORT_SYMBOL(ICC_Finalize);
EXPORT_SYMBOL(ICC_Get_Node_State);
EXPORT_SYMBOL(ICC_Open_Channel);
EXPORT_SYMBOL(ICC_Close_Channel);
EXPORT_SYMBOL(ICC_Get_Channel_State);
EXPORT_SYMBOL(ICC_Get_Channel_Free);
EXPORT_SYMBOL(ICC_Get_Channel_Pending);
EXPORT_SYMBOL(ICC_Get_Next_Msg_Size);
EXPORT_SYMBOL(ICC_Msg_Send);
EXPORT_SYMBOL(ICC_Msg_Recv);
#ifdef ICC_CFG_HEARTBEAT_ENABLED
    EXPORT_SYMBOL(ICC_Heartbeat_Initialize);
    EXPORT_SYMBOL(ICC_Heartbeat_Finalize);
    EXPORT_SYMBOL(ICC_Heartbeat_Runnable);
#endif /* ICC_CFG_HEARTBEAT_ENABLED */
#if (defined(ICC_LINUX2LINUX) && defined(ICC_BUILD_FOR_M4))
    EXPORT_SYMBOL(ICC_Shared_Virt_Base_Addr);
#endif

module_init(ICC_dev_init);
module_exit(ICC_dev_exit);
