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

#ifndef ICC_USE_BAR
#define ICC_USE_BAR 	2
#endif

#define DO_TEST_EXPAND(VAL)  VAL ## 0
#define TEST_EXPAND(VAL)     DO_TEST_EXPAND(VAL)

#define DO_GET_BAR_ADDRESS(BAR, SUFFIX) EP_BAR ## BAR ## _ ## SUFFIX
#define GET_BAR_ADDRESS(BAR) 	DO_GET_BAR_ADDRESS(BAR, ADDR)
#define GET_BAR_END(BAR) 		DO_GET_BAR_ADDRESS(BAR, END)

/* FIXME remove hardcoding of addresses. */

#define EP_PCIE_BASE_ADDR 0x72000000
#define EP_DDR_ADDR 0x8FF00000

#ifndef ICC_BUILD_FOR_M4

/* define BARs for communication from the RC side
 * TODO: take them from device tree */
#ifndef ICC_PCIE_SHMEM_BLUEBOX
#define EP_BAR0_ADDR	0x72100000
#define EP_BAR0_END 	0x721fffff  /* 1MB */
#define EP_BAR2_ADDR	0x72200000
#define EP_BAR2_END		0x722fffff  /* 1MB */
#define EP_BAR4_ADDR	0x72310000
#define EP_BAR4_END		0x72310fff  /* 4KB */
#define EP_BAR5_ADDR	0x72300000
#define EP_BAR5_END		0x7230ffff  /* 64Kb */
#else
#define EP_BAR0_ADDR	0x1446000000
#define EP_BAR0_END 	0x14460fffff  /* 1MB */
#define EP_BAR2_ADDR	0x1446100000
#define EP_BAR2_END		0x14461fffff  /* 1MB */
#define EP_BAR4_ADDR	0x1446210000
#define EP_BAR4_END		0x1446210fff  /* 4KB */
#define EP_BAR5_ADDR	0x1446200000
#define EP_BAR5_END		0x144620ffff  /* 64Kb */
#endif

#define EP_BAR_ADDR	GET_BAR_ADDRESS(ICC_USE_BAR)

#if (TEST_EXPAND(EP_BAR_ADDR) == 0)
#error "Invalid BAR selected
#else
#define EP_BAR_SIZE	(GET_BAR_END(ICC_USE_BAR) - EP_BAR_ADDR + 1)
#endif

/* Physical memory mapped by the RC CPU.
 * The RC's shared DDR mapping is different in the Bluebox vs S32V EVB case.
 * For the moment, this setting is statically defined in the Makefile.
 */
#ifdef ICC_PCIE_SHMEM_BLUEBOX  /* LS2-S32V */
/* for LS2 kernel started with mem=13568M, unallocated RAM starts at 0x8350000000 */
#define RC_DDR_ADDR     0x8350000000
#else                           /* EVB-PCIE */
#define RC_DDR_ADDR     0x8FF00000
#endif

#endif

#ifdef ICC_BUILD_FOR_M4
    /* M4 stands for the module replacing the RTOS app, that is the one initializing the shared mem.
     * In our case that would be the PCIe EndPoint. */
    #define IRAM_BASE_ADDR EP_DDR_ADDR
    #define IRAM_PING_ADDR EP_PCIE_BASE_ADDR
    #define IRAM_POLL_ADDR EP_DDR_ADDR

#else

    #define IRAM_BASE_ADDR EP_BAR_ADDR
    #define IRAM_PING_ADDR EP_BAR_ADDR
    #define IRAM_POLL_ADDR RC_DDR_ADDR

#endif

#define MAP_DDR_SIZE    1024 * 1024 * 1 /* 1 MB */

#ifndef ICC_BUILD_FOR_M4
ICC_Config_t * ICC_Config_Ptr_M4;
ICC_Config_t * ICC_Config_Ptr_M4_Remote;
#endif

struct s32v_bar {
	uint64_t bar_addr;
	uint32_t bar_size;
};

struct handshake {
	struct s32v_bar rc_bar;
	uint64_t rc_ddr_addr;
};

static struct s32v_bar icc_bar
#ifndef ICC_BUILD_FOR_M4
	/* BAR attributes are initialized on the RC side only */
	= {
			EP_BAR_ADDR,
			EP_BAR_SIZE
	}
#endif
;

/* first 128 bits are for the hand shake */
#define ICC_CONFIG_OFFSET_FROM_BASE (sizeof(struct handshake))

struct ICC_device_data {
    struct cdev cdev;
};

struct ICC_device_data devs[NUM_MINORS];
static dev_t dev_no;

#ifndef ICC_USE_POLLING

struct ICC_platform_data {
    struct device * dev;
    unsigned int shared_irq;
    unsigned int local_irq;
};

struct ICC_platform_data * icc_data;

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

/* Value for 'incoming data from peer' */
#define WAKE_UP_PATTERN     0x42

/* Waiting value */
#define WAIT_PATTERN        0x0

u32 *poll_addr;
u32 *ping_addr;

#define POLL_TIMEOUT_MS 1

struct task_struct *poll_thread;

ICC_ATTR_SEC_TEXT_CODE
extern void
ICC_Remote_Event_Handler(void);

static int poll_thread_fn(void *arg)
{
    u32 *addr = (u32 *)arg;

    while (!kthread_should_stop()) {
        msleep_interruptible(POLL_TIMEOUT_MS);
        if (*addr == WAKE_UP_PATTERN) {
            /* event detected */
            ICC_Remote_Event_Handler();
            *addr = WAIT_PATTERN;
        }
    }

    return 0;
}

static int pcie_shmem_poll_init(void)
{
    int err = 0;

    poll_addr = (u32 *)ioremap_nocache(IRAM_POLL_ADDR, SZ_4K);
    if (!poll_addr) {
        pr_warn("Could not ioremap %#llx\n", IRAM_POLL_ADDR);
        err = -EIO;
        goto ioremap_error;
    }
    /* Initialize it before we poll for changes */
    *poll_addr = WAIT_PATTERN;

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

    ping_addr = (u32 *)ioremap_nocache(IRAM_PING_ADDR, SZ_4K);
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

#endif  /* ICC_USE_POLLING */

/* following code is calling an api custom exported from the pcie driver */

#ifdef ICC_BUILD_FOR_M4

struct s32v_inbound_region {
    uint32_t  bar_nr;
    uint32_t  target_addr;
    uint32_t  region;
};

struct s32v_outbound_region {
    uint64_t target_addr;
    uint64_t base_addr;
    uint32_t size;
    uint32_t region;
    uint32_t region_type;
};

static struct s32v_inbound_region icc_inb = {
    ICC_USE_BAR,      /* BAR2 */
    EP_DDR_ADDR,    /* locally-mapped DDR on EP */
    0       /* region 0 */
};

/* Outbound region structure */
static struct s32v_outbound_region icc_outb = {
    NULL,    /* target_addr */
    EP_PCIE_BASE_ADDR,    /* base_addr */
    MAP_DDR_SIZE,   /* size >= 64K(min for PCIE on S32V) */
    0,      /* region number */
    0       /* region type = mem */
};

extern int s32v_pcie_setup_outbound(void * data);
extern int s32v_pcie_setup_inbound(void * data);

static int pcie_init_inbound(void)
{
    int ret = 0;

    /* Setup the inbound window for transactions from RC */
    ret = s32v_pcie_setup_inbound(&icc_inb);

    if (ret < 0) {
        printk(KERN_ERR "[pcie_init_inbound] Error while setting inbound region\n");
        goto err;
    } else {
        printk("Inbound region setup successfully\n");
    }

err:
    return ret;
}

int pcie_init_outbound(void)
{
    int ret = 0;

    /* Setup outbound window for accessing RC mem */
    ret = s32v_pcie_setup_outbound(&icc_outb);

    if (ret < 0) {
        printk(KERN_ERR "[pcie_mappings_init] Error while setting outbound region\n");
        goto err;
    } else {
        printk("Outbound region setup successfully\n");
        printk("\tRC %#llx (BAR %d) mapped to EP %#llx, size %d\n", icc_bar.bar_addr, icc_inb.bar_nr, icc_inb.target_addr, icc_bar.bar_size);
        printk("\tEP %#llx mapped to RC %#llx, size %d\n", icc_outb.base_addr, icc_outb.target_addr, icc_outb.size);
    }

err:
    return ret;
}

#endif  /* ICC_BUILD_FOR_M4 */

#ifdef ICC_USE_POLLING

ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Remote( void )
{
    u32 crt_val = WAIT_PATTERN;
    if (ping_addr) {
        /* wait for the peer to become available */
        while ((crt_val = *ping_addr) != WAIT_PATTERN) {
            msleep_interruptible(POLL_TIMEOUT_MS);
        }
        *ping_addr = WAKE_UP_PATTERN;
    }
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Remote_Alive( void )
{
    if (ping_addr) {
#ifdef ICC_BUILD_FOR_M4
    	*ping_addr = WAKE_UP_PATTERN;
#else
    	struct handshake * phshake = (struct handshake *)ping_addr;
    	phshake->rc_bar.bar_addr = IRAM_PING_ADDR;
    	phshake->rc_bar.bar_size = EP_BAR_SIZE;
        phshake->rc_ddr_addr = IRAM_POLL_ADDR;
#endif
    }
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Wait_For_Peer( void )
{
    u32 crt_val;
    while ((crt_val = *poll_addr) == WAIT_PATTERN) {
        msleep_interruptible(POLL_TIMEOUT_MS);
    }
#ifdef ICC_BUILD_FOR_M4
    struct handshake * phshake = (struct handshake *)poll_addr;
    icc_outb.target_addr = phshake->rc_ddr_addr;
    icc_bar.bar_addr = phshake->rc_bar.bar_addr;
    icc_bar.bar_size = phshake->rc_bar.bar_addr;
    pcie_init_outbound();
#endif
    *poll_addr = WAIT_PATTERN;
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

#endif

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

char * ICC_Shared_Virt_Base_Addr;
struct resource * ICC_Mem_Region;

#ifdef ICC_USE_POLLING
struct resource * ICC_Ping_Region;
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

#ifndef ICC_USE_POLLING
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
#ifndef ICC_BUILD_FOR_M4
    uint64_t * shared_start = NULL;
#endif

    printk(LOG_LEVEL "[ICC_dev_init] Freescale ICC linux driver\n");

#ifndef ICC_USE_POLLING
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
    ICC_Shared_Virt_Base_Addr = (char *)ioremap_nocache(IRAM_BASE_ADDR, MAP_DDR_SIZE) + ICC_CONFIG_OFFSET_FROM_BASE;
    printk(LOG_LEVEL "[ICC_dev_init] reserved %#llx size is %d\n", ICC_Shared_Virt_Base_Addr, MAP_DDR_SIZE );
    if( !ICC_Shared_Virt_Base_Addr ){
        printk(KERN_ERR "[ICC_dev_init] ICC_Shared_Virt_Base_Addr virtual mapping has failed for %#x\n", IRAM_BASE_ADDR);
        err = -ENOMEM;
        goto cleanup;
    }

#ifndef ICC_BUILD_FOR_M4

    /* discover location of the configuration
     * TODO: should we use another offset in shm to store the ICC_Config0 offset?
     */
    ICC_Config_Ptr_M4 = (ICC_Config_t *)(ICC_Shared_Virt_Base_Addr);
    shared_start = ICC_Config_Ptr_M4;
    for (i = 0; i < MAP_DDR_SIZE / sizeof(uint64_t); i++, shared_start++) {
        union local_magic * crt_start = (union local_magic *)shared_start;
	    if ((ICC_Local_Magic.raw.m0 == crt_start->raw.m0) &&
		(ICC_Local_Magic.raw.m1 == crt_start->raw.m1)){
		    ICC_Config_Ptr_M4 = (ICC_Config_t *)crt_start;
		    printk(LOG_LEVEL "[ICC_dev_init] ICC Shared Config found at address %#llx\n", ICC_Config_Ptr_M4);
		    break;
	    }
    }

    ICC_Config_Ptr_M4_Remote = ICC_Config_Ptr_M4->This_Ptr;

    printk(LOG_LEVEL "[ICC_dev_init] ICC Shared Config local virtual address: %#llx \n", ICC_Config_Ptr_M4);
    printk(LOG_LEVEL "[ICC_dev_init] ICC Shared Config remote virtual address: %#llx \n", ICC_Config_Ptr_M4_Remote);
#endif

#ifdef ICC_BUILD_FOR_M4
    /* setup PCIE */
    pcie_init_inbound();
#endif

#ifdef ICC_USE_POLLING
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

#ifndef ICC_USE_POLLING

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

#endif  /* ICC_USE_POLLING */

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

#ifdef ICC_LINUX2LINUX
#ifdef ICC_BUILD_FOR_M4
    EXPORT_SYMBOL(ICC_Shared_Virt_Base_Addr);
#else
    EXPORT_SYMBOL(ICC_Config_Ptr_M4);
    EXPORT_SYMBOL(ICC_Config_Ptr_M4_Remote);
#endif
#endif

#ifdef ICC_USE_POLLING
EXPORT_SYMBOL(ICC_Notify_Remote_Alive);
EXPORT_SYMBOL(ICC_Wait_For_Peer);
#endif

module_init(ICC_dev_init);
module_exit(ICC_dev_exit);
