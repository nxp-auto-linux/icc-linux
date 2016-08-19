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
*   (c) Copyright 2014 Freescale Semiconductor Inc.
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

#include "ICC_Api.h"

MODULE_DESCRIPTION("ICC device");
MODULE_AUTHOR("Freescale Semiconductor");
MODULE_LICENSE("Freescale");

#define MODULE_NAME     "ICC"
#define BASEMINOR   0
#define NUM_MINORS      1

#define LOG_LEVEL       KERN_ALERT
#define PARAM_PERM      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP


#define IRAM_BASE_ADDR  0x3F000000

struct ICC_device_data {
    struct cdev cdev;
};

struct ICC_device_data devs[NUM_MINORS];
static dev_t dev_no;

ICC_Config_t * ICC_Config_Ptr_M4;



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

static int ICC_dev_init(void)
{
    int err;
    int i;

    if (!request_mem_region(IRAM_BASE_ADDR, 0x40000, "ICC_shmem")) {
            printk ("Failed to request mem region!\n");
            return -ENOMEM;
    }

    ICC_Shared_Virt_Base_Addr = ioremap_nocache(IRAM_BASE_ADDR, 0x40000);
    
    if( !ICC_Shared_Virt_Base_Addr ){
            printk(LOG_LEVEL "ICC_Shared_Virt_Base_Addr virtual mapping has failed for 0x%08x\n", IRAM_BASE_ADDR);
    }


    ICC_Config_Ptr_M4 = (ICC_Config_t *)(ICC_Shared_Virt_Base_Addr + 0x00001c20 );  /**<  ICC_Config on m4 MAP file = 0x3f041c20 */

    /* register device */
    err = alloc_chrdev_region(&dev_no, BASEMINOR, NUM_MINORS, MODULE_NAME); 

    if (err < 0) {
        printk(LOG_LEVEL "Major number allocation has failed\n");
        return err;
    }

    printk(LOG_LEVEL "[ICC_dev_init] Major number %d\n", MAJOR(dev_no));


    /* initialize each device */
    for (i = 0; i < NUM_MINORS; i++) {
        cdev_init(&devs[i].cdev, &ICC_fops);
        cdev_add(&devs[i].cdev, MKDEV(MAJOR(dev_no), i),1);
    }

    printk(LOG_LEVEL "[ICC_Config] 0x%08x \n", ICC_Config_Ptr_M4);

    return 0;
}

static void ICC_dev_exit(void)
{
    int i;

    for (i = 0; i < NUM_MINORS; i++)
        cdev_del(&devs[i].cdev);

    unregister_chrdev_region(dev_no, NUM_MINORS);

    iounmap(ICC_Config_Ptr_M4);
    release_mem_region(IRAM_BASE_ADDR, 0x40000);

    printk(LOG_LEVEL "[ICC_dev_exit] \n");
}


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
