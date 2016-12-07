/**
*   @file    ICC_lib.c
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
#include "ICC_Sw_Platform.h"

MODULE_DESCRIPTION("ICC device");
MODULE_AUTHOR("Freescale Semiconductor");
MODULE_LICENSE("GPL");

#define LOG_LEVEL       KERN_ALERT

#ifdef ICC_LINUX2LINUX
/* first words are for the hand shake */
#define ICC_CONFIG_OFFSET_FROM_BASE (sizeof(struct handshake))
#else
#define ICC_CONFIG_OFFSET_FROM_BASE (0)
#endif

struct ICC_device_data {
    struct cdev cdev;
};

static struct ICC_device_data devs[NUM_MINORS];
static dev_t dev_no;

static struct ICC_platform_data * icc_plat_data;

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
    file->private_data = NULL;

    printk(LOG_LEVEL "Closed ICC_library device\n");

    return 0;
}

static const struct file_operations ICC_fops = {
    .owner  = THIS_MODULE,
    .open   = ICC_dev_open,
    .release = ICC_dev_release,
};

char * ICC_Shared_Virt_Base_Addr;
#ifndef ICC_BUILD_FOR_M4
ICC_Config_t * ICC_Config_Ptr_M4;
ICC_Config_t * ICC_Config_Ptr_M4_Remote;
#endif

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t ICC_Notify_Peer( void )
{
    int err = 0;

#ifdef ICC_USE_POLLING
    err = poll_notify_peer(icc_plat_data);
#else
    err = intr_notify_peer();
#endif

    return (err == -ETIMEDOUT ? ICC_ERR_TIMEOUT :
            (err ? ICC_ERR_OS_LINUX_REGISTER_IRQ : ICC_SUCCESS));
}

#ifdef ICC_USE_POLLING

#ifndef ICC_BUILD_FOR_M4
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t ICC_Notify_Peer_Alive(void)
{
    int err = poll_notify_peer_alive(icc_plat_data);

    return (err ? ICC_ERR_OS_LINUX_REGISTER_IRQ : ICC_SUCCESS);
}

#else

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t ICC_Wait_For_Peer(void)
{
    int err = poll_wait_for_peer(icc_plat_data);

    return (err == -ETIMEDOUT ? ICC_ERR_TIMEOUT :
            (err ? ICC_ERR_OS_LINUX_REGISTER_IRQ : ICC_SUCCESS));
}

#endif  /* ICC_BUILD_FOR_M4 */

#endif  /* ICC_USE_POLLING */

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_From_Peer(void)
{
#ifdef ICC_USE_POLLING
    poll_clear_notify_from_peer(icc_plat_data);
#else
    intr_clear_notify_from_peer();
#endif
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Local(void)
{
    // not supported for polling

#ifndef ICC_USE_POLLING
    intr_notify_local();
#endif
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_Local(void)
{
    // not supported for polling

#ifndef ICC_USE_POLLING
    intr_clear_notify_local();
#endif
}
#endif

static void local_cleanup(struct ICC_platform_data *icc_data)
{
    if (ICC_Shared_Virt_Base_Addr) {
        ICC_Shared_Virt_Base_Addr = NULL;

#ifndef ICC_BUILD_FOR_M4
        ICC_Config_Ptr_M4 = NULL;
        ICC_Config_Ptr_M4_Remote = NULL;
#endif
    }

#ifdef ICC_USE_POLLING
    shmem_poll_exit(icc_data);
    shmem_ping_exit(icc_data);
#endif
}

union local_magic {
	char str[ICC_CONFIG_MAGIC_SIZE];
	struct {
		u64 m0, m1;
	} raw;
} ICC_Local_Magic = { ICC_CONFIG_MAGIC };

static int local_init(struct ICC_platform_data * icc_data)
{
    if (icc_data) {

        int err;
#ifndef ICC_BUILD_FOR_M4
        int i;
        uint64_t * shared_start = NULL;
#endif
        struct device *dev = &icc_data->pdev->dev;

        /* Initialize shared memory */
        if (!devm_request_mem_region(dev, get_shmem_base_address(), get_shmem_size(), "ICC_shmem")) {
            printk (KERN_ERR "[ICC_dev_init] Failed to request mem region!\n");
            err = -ENOMEM;
            goto cleanup;
        }

        /* ICC Shared mem is mapped differently on RC and EP, but in both cases it physically
         * resides on EP side.
         */
        ICC_Shared_Virt_Base_Addr = devm_ioremap_nocache(dev, get_shmem_base_address(), get_shmem_size()) + ICC_CONFIG_OFFSET_FROM_BASE;
        printk(LOG_LEVEL "[ICC_dev_init] reserved ICC_Shared_Virt_Base_Addr=%#llx size is %d\n", ICC_Shared_Virt_Base_Addr, get_shmem_size() - ICC_CONFIG_OFFSET_FROM_BASE);
        if( !ICC_Shared_Virt_Base_Addr ){
            printk(KERN_ERR "[ICC_dev_init] ICC_Shared_Virt_Base_Addr virtual mapping has failed for %#x\n", get_shmem_base_address());
            err = -ENOMEM;
            goto cleanup;
        }

#ifndef ICC_BUILD_FOR_M4

        /* discover location of the configuration
         */
        ICC_Config_Ptr_M4 = (ICC_Config_t *)(ICC_Shared_Virt_Base_Addr);
        shared_start = (uint64_t *)ICC_Config_Ptr_M4;
        for (i = 0; i < get_shmem_size() / sizeof(uint64_t); i++, shared_start++) {
            union local_magic * crt_start = (union local_magic *)shared_start;
            if ((ICC_Local_Magic.raw.m0 == crt_start->raw.m0) &&
            (ICC_Local_Magic.raw.m1 == crt_start->raw.m1)){
                ICC_Config_Ptr_M4 = (ICC_Config_t *)crt_start;
                printk(LOG_LEVEL "[ICC_dev_init] ICC Shared Config found at address %#llx\n", ICC_Config_Ptr_M4);
                break;
            }
        }

        ICC_Config_Ptr_M4_Remote = (ICC_Config_t *)(ICC_Config_Ptr_M4->This_Ptr);

        printk(LOG_LEVEL "[ICC_dev_init] ICC Shared Config local virtual address: %#llx \n", ICC_Config_Ptr_M4);
        printk(LOG_LEVEL "[ICC_dev_init] ICC Shared Config remote virtual address: %#llx \n", ICC_Config_Ptr_M4_Remote);
#endif

#ifdef ICC_BUILD_FOR_M4
        /* setup PCIE */
        pcie_init_inbound();
#endif

#ifdef ICC_USE_POLLING
        shmem_poll_init(icc_data);
        shmem_ping_init(icc_data);
#endif

        return 0;

cleanup:
        local_cleanup(icc_data);

        return err;
    }

    return -EINVAL;
}

static int ICC_remove(struct platform_device *pdev);

static int ICC_probe(struct platform_device *pdev)
{
    int err = 0;

    icc_plat_data = devm_kzalloc(&pdev->dev, sizeof(struct ICC_platform_data), GFP_KERNEL);
    if (!icc_plat_data)
        return -ENOMEM;

    icc_plat_data->pdev = pdev;

#ifndef ICC_USE_POLLING
    init_interrupt_data(icc_plat_data);
#endif

    platform_set_drvdata(pdev, icc_plat_data);

    err = local_init(icc_plat_data);
    if (err)
        ICC_remove(pdev);

    return err;
}

static int ICC_remove(struct platform_device *pdev)
{
    if (icc_plat_data) {
        platform_set_drvdata(pdev, NULL);
        devm_kfree(&(pdev->dev), icc_plat_data);
        icc_plat_data = NULL;
    }
    return 0;
}

static struct of_device_id ICC_dt_ids[] = {
    {
        .compatible = "fsl,s32v234-icc",
    },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ICC_dt_ids);

static struct platform_driver ICC_driver = {
    .probe      = ICC_probe,
    .remove     = ICC_remove,
    .driver     = {
        .name   = MODULE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = ICC_dt_ids
    },
};

static void __exit ICC_dev_exit(void);

static int __init ICC_dev_init(void)
{
    int i, err = platform_driver_register(&ICC_driver);
    if (err) {
        return err;
    }

    printk(LOG_LEVEL "[ICC_dev_init] Freescale ICC linux driver\n");

    err = alloc_chrdev_region(&dev_no, BASEMINOR, NUM_MINORS, MODULE_NAME);

    if (err) {
        printk(KERN_ERR "[ICC_dev_init] Major number allocation has failed\n");
        return err;
    }

    printk(LOG_LEVEL "[ICC_dev_init] Major number %d\n", MAJOR(dev_no));

    /* initialize each device */
    for (i = 0; i < NUM_MINORS; i++) {
        cdev_init(&devs[i].cdev, &ICC_fops);
        cdev_add(&devs[i].cdev, MKDEV(MAJOR(dev_no), i),1);
    }

    return 0;
}

static void __exit ICC_dev_exit(void)
{
    int i;

    for (i = 0; i < NUM_MINORS; i++)
        cdev_del(&devs[i].cdev);

    unregister_chrdev_region(dev_no, NUM_MINORS);

    platform_driver_unregister(&ICC_driver);

    printk(LOG_LEVEL "[ICC_dev_exit] \n");
}

/**
 * Exporting ICC API functions to be used by other kernel modules
 */
#if (defined(ICC_BUILD_FOR_M4) && defined(ICC_LINUX2LINUX))
EXPORT_SYMBOL(ICC_Relocate_Config);
#endif
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
#ifdef ICC_BUILD_FOR_M4
EXPORT_SYMBOL(ICC_Wait_For_Peer);
#else
EXPORT_SYMBOL(ICC_Notify_Peer_Alive);
#endif
#endif

module_init(ICC_dev_init);
module_exit(ICC_dev_exit);
