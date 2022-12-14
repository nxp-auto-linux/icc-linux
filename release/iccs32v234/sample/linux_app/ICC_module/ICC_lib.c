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
*   (c) Copyright 2016,2017 NXP
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

#include <linux/interrupt.h>
#include <linux/of_platform.h>

#include "ICC_Api.h"
#include "ICC_Notification.h"

#include "ICC_Log.h"

MODULE_DESCRIPTION("ICC device");
MODULE_AUTHOR("Freescale Semiconductor");
MODULE_LICENSE("GPL");

#ifdef ICC_USE_CHR_DEVICE

struct ICC_device_data {
    struct cdev cdev;
};

static struct ICC_device_data devs[NUM_MINORS];
static dev_t dev_no;

#endif

static struct ICC_platform_data * icc_plat_data;

#ifdef ICC_USE_CHR_DEVICE

static int ICC_dev_open(struct inode *inode, struct file *file)
{
    struct ICC_device_data *data = container_of(inode->i_cdev,
                                                struct ICC_device_data,
                                                cdev);
    file->private_data = data;

    ICC_DEBUG("Opened ICC_library device");
    return 0;
}

static int ICC_dev_release(struct inode *inode, struct file *file)
{
    file->private_data = NULL;

    ICC_DEBUG("Closed ICC_library device");

    return 0;
}

static const struct file_operations ICC_fops = {
    .owner  = THIS_MODULE,
    .open   = ICC_dev_open,
    .release = ICC_dev_release,
};

#endif

char * ICC_Shared_Virt_Base_Addr;
#ifndef ICC_BUILD_FOR_M4
ICC_Config_t * ICC_Config_Ptr_M4;
ICC_Config_t * ICC_Config_Ptr_M4_Remote;
#endif

ICC_Err_t ICC_Notify_Peer( void )
{
    int err = notify_peer(icc_plat_data);

    return (err == -ETIMEDOUT ? ICC_ERR_TIMEOUT :
            (err ? ICC_ERR_OS_LINUX_REGISTER_IRQ : ICC_SUCCESS));
}

#ifdef ICC_LINUX2LINUX

#ifndef ICC_BUILD_FOR_M4
ICC_Err_t ICC_Notify_Peer_Alive(void)
{
    int err = notify_peer_alive(icc_plat_data);

    return (err ? ICC_ERR_OS_LINUX_REGISTER_IRQ : ICC_SUCCESS);
}

#else

ICC_Err_t ICC_Wait_For_Peer(void)
{
    int err = wait_for_peer(icc_plat_data);

    return (err == -ETIMEDOUT ? ICC_ERR_TIMEOUT :
            (err ? ICC_ERR_OS_LINUX_REGISTER_IRQ : ICC_SUCCESS));
}

#endif  /* ICC_BUILD_FOR_M4 */

#endif  /* ICC_LINUX2LINUX */

void ICC_Clear_Notify_From_Peer(void)
{
    clear_notify_from_peer(icc_plat_data);
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
void ICC_Notify_Local(void)
{
    notify_local();
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_Local(void)
{
    clear_notify_local();
}
#endif

static void local_cleanup(struct ICC_platform_data *icc_data)
{
    cleanup_shmem(icc_data);

    ICC_Shared_Virt_Base_Addr = NULL;

#ifndef ICC_BUILD_FOR_M4
    ICC_Config_Ptr_M4 = NULL;
    ICC_Config_Ptr_M4_Remote = NULL;
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

#ifndef ICC_BUILD_FOR_M4
        int i;
        uint64_t * shared_start = NULL;
#endif

        init_shmem(icc_data);

        if (!ICC_Shared_Virt_Base_Addr) {
            return -ENOMEM;
        }

#ifndef ICC_BUILD_FOR_M4

        /* Discover location of the configuration
         */
        shared_start = (uint64_t *)ICC_Shared_Virt_Base_Addr;
        for (i = 0; i < get_shmem_size() / sizeof(uint64_t); i++, shared_start++) {
            union local_magic * crt_start = (union local_magic *)shared_start;
            if ((ICC_Local_Magic.raw.m0 == crt_start->raw.m0) &&
            (ICC_Local_Magic.raw.m1 == crt_start->raw.m1)){
                ICC_Config_Ptr_M4 = (ICC_Config_t *)crt_start;
                ICC_Config_Ptr_M4_Remote = (ICC_Config_t *)(ICC_Config_Ptr_M4->This_Ptr);
                ICC_INFO("ICC Shared Config found at address %#llx", ICC_Config_Ptr_M4);
                break;
            }
        }

        if (ICC_Config_Ptr_M4) {
            ICC_INFO("ICC Shared Config local virtual address: %#llx", ICC_Config_Ptr_M4);
            ICC_INFO("ICC Shared Config remote virtual address: %#llx", ICC_Config_Ptr_M4_Remote);
        }
        else {
            ICC_ERR("ICC Shared Config not found");
            return -ENOMEM;
        }
#endif

        return 0;
    }

    return -EINVAL;
}

static int ICC_remove(struct platform_device *pdev);

static int ICC_probe(struct platform_device *pdev)
{
    int err = 0;

    ICC_DEBUG("Probing ICC device");

    if (!icc_plat_data) {
        icc_plat_data = devm_kzalloc(&pdev->dev, sizeof(struct ICC_platform_data), GFP_KERNEL);
        if (!icc_plat_data)
            return -ENOMEM;

        icc_plat_data->pdev = pdev;

        init_notifications(icc_plat_data);

        platform_set_drvdata(pdev, icc_plat_data);

        err = local_init(icc_plat_data);
        if (err)
            ICC_remove(pdev);
    }

    return err;
}

static int ICC_remove(struct platform_device *pdev)
{
    ICC_DEBUG("Removing ICC device");

    local_cleanup(icc_plat_data);

    if (icc_plat_data) {
        platform_set_drvdata(pdev, NULL);
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
#ifdef ICC_USE_CHR_DEVICE
    int i;
#endif
    int err = 0;

    ICC_DEBUG("ICC linux driver");

    if (of_find_matching_node(NULL, ICC_dt_ids)) {
        err = platform_driver_register(&ICC_driver);
        if (err) {
            ICC_ERR("Failed to register ICC driver");
            return err;
        }
    }
    else {
 
        /* We don't need the dts node 'icc-linux' for ICC on M4-A53, since all required information
         * already is in the default dts for the platform (nodes mscm1, sram).
         * For ICC over PCIe, we do need that node, to get the shared memory reference.
         */

#ifdef ICC_LINUX2LINUX
        ICC_ERR("Failed to access the shared memory. icc-linux node is missing from dtb.");
        return -ENODEV;
#else
         /* If we have icc-linux node, we're fine. But if not, we'll never get probed by the kernel,
          * using the so we do it manually here.
          */
        if (!platform_create_bundle(&ICC_driver, ICC_probe, NULL, 0, NULL, 0)) {
            ICC_ERR("Failed to probe ICC driver");
            return -ENODEV;
        }
#endif
    }

#ifdef ICC_USE_CHR_DEVICE
    err = alloc_chrdev_region(&dev_no, BASEMINOR, NUM_MINORS, MODULE_NAME);
    if (err) {
        ICC_ERR("Major number allocation has failed");
        return err;
    }

    ICC_DEBUG("Major number %d", MAJOR(dev_no));

    /* initialize each device */
    for (i = 0; i < NUM_MINORS; i++) {
        cdev_init(&devs[i].cdev, &ICC_fops);
        cdev_add(&devs[i].cdev, MKDEV(MAJOR(dev_no), i),1);
    }
#endif

    return 0;
}

static void __exit ICC_dev_exit(void)
{
#ifdef ICC_USE_CHR_DEVICE
    int i;

    for (i = 0; i < NUM_MINORS; i++)
        cdev_del(&devs[i].cdev);

    unregister_chrdev_region(dev_no, NUM_MINORS);
#endif

    platform_driver_unregister(&ICC_driver);

    ICC_DEBUG("ICC linux driver exit");
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
    EXPORT_SYMBOL(ICC_Wait_For_Peer);
#else
    EXPORT_SYMBOL(ICC_Config_Ptr_M4);
    EXPORT_SYMBOL(ICC_Config_Ptr_M4_Remote);
    EXPORT_SYMBOL(ICC_Notify_Peer_Alive);
#endif
#endif

module_init(ICC_dev_init);
module_exit(ICC_dev_exit);
