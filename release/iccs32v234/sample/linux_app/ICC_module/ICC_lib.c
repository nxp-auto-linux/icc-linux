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
#include "ICC_Config.h"

MODULE_DESCRIPTION("ICC device");
MODULE_AUTHOR("Freescale Semiconductor");
MODULE_LICENSE("GPL");

// this macro may come from outside, e.g. from the makefile or bitbake compile flags
#ifndef ICC_CONFIG_OFFSET
#ifdef ICC_CFG_HEARTBEAT_ENABLED
    #define ICC_CONFIG_OFFSET 0x00001ca0
#else
    #define ICC_CONFIG_OFFSET 0x00001c98
#endif
#endif  /* ICC_CONFIG_OFFSET */


#define MODULE_NAME     "ICC"
#define BASEMINOR   0
#define NUM_MINORS      1

#define LOG_LEVEL       KERN_ALERT
#define PARAM_PERM      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP


#define IRAM_BASE_ADDR  0x3E900000

struct ICC_device_data {
    struct cdev cdev;
};

struct ICC_platform_data {
    struct device * dev;
    unsigned int shared_irq;
    unsigned int local_irq;
};

struct ICC_device_data devs[NUM_MINORS];
static dev_t dev_no;

struct ICC_platform_data * icc_data = NULL;

ICC_Config_t * ICC_Config_Ptr_M4 = NULL;

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

static void local_cleanup(void)
{
    if (ICC_Shared_Virt_Base_Addr) {
        iounmap(ICC_Shared_Virt_Base_Addr);
        ICC_Shared_Virt_Base_Addr = NULL;
        ICC_Config_Ptr_M4 = NULL;
    }

    if (ICC_Mem_Region) {
        release_mem_region(IRAM_BASE_ADDR, 0x40000);
    }
    platform_driver_unregister(&ICC_driver);
}

static int __init ICC_dev_init(void)
{
    int err;
    int i;

    printk(LOG_LEVEL "[ICC_dev_init] Freescale ICC linux driver\n");

    err = platform_driver_register(&ICC_driver);
    if (err) {
        return err;
    }

    ICC_Mem_Region = request_mem_region(IRAM_BASE_ADDR, 0x40000, "ICC_shmem");
    if (!ICC_Mem_Region) {
        printk (KERN_ERR "[ICC_dev_init] Failed to request mem region!\n");
        err = -ENOMEM;
        goto cleanup;
    }

    ICC_Shared_Virt_Base_Addr = ioremap_nocache(IRAM_BASE_ADDR, 0x40000);
    printk(LOG_LEVEL "[ICC_dev_init] return %16llx size is %d\n", ICC_Shared_Virt_Base_Addr, sizeof(char * ) );
    if( !ICC_Shared_Virt_Base_Addr ){
        printk(KERN_ERR "[ICC_dev_init] ICC_Shared_Virt_Base_Addr virtual mapping has failed for 0x%08x\n", IRAM_BASE_ADDR);
        err = -ENOMEM;
        goto cleanup;
    }

    ICC_Config_Ptr_M4 = (ICC_Config_t *)(ICC_Shared_Virt_Base_Addr + ICC_CONFIG_OFFSET );

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

    printk(LOG_LEVEL "[ICC_dev_init] 0x%08x \n", ICC_Config_Ptr_M4);

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

module_init(ICC_dev_init);
module_exit(ICC_dev_exit);
