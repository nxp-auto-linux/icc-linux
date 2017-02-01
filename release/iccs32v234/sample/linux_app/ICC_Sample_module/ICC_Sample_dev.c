/**
*   @file    ICC_Sample_dev.c
*   @version 0.0.1
*
*   @brief   ICC_Sample - Inter Core Communication Sample application
*   @details       Inter Core Communication application
*
*   @addtogroup [ICC_Sample]
*   @{
*/
/*==================================================================================================
*   Project              : ICC_Sample
*   Platform             : ARM
*   Peripheral           :
*   Dependencies         : none
*
*   Build Version        :
*
*   (c) Copyright 2014,2016 Freescale Semiconductor Inc.
*   (c) Copyright 2016 NXP
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
#include <linux/delay.h>

#include "ICC_Config.h"
#include "ICC_Linux_Sample.h"

#define ICC_LOG_SRC
#include "ICC_Log.h"

MODULE_DESCRIPTION("ICC Sample application");
MODULE_AUTHOR("Freescale Semiconductor");
MODULE_LICENSE("GPL");

#define BASEMINOR   0
#define COUNT       1

#define MODULE_NAME     "ICC_Sample"
#define NUM_MINORS      1

#ifdef ICC_USE_CHR_DEVICE

struct ICC_Sample_device_data {
    struct cdev cdev;
};

struct ICC_Sample_device_data devs[NUM_MINORS];
static dev_t dev_no;

static int ICC_Sample_dev_open(struct inode *inode, struct file *file)
{
    struct ICC_Sample_device_data *data = container_of(inode->i_cdev,
                                                struct ICC_Sample_device_data,
                                                cdev);

    file->private_data = data;

    return 0;
}

static int ICC_Sample_dev_release(struct inode *inode, struct file *file)
{
    file->private_data = NULL;

    return 0;
}

static const struct file_operations ICC_Sample_fops = {
    .owner  = THIS_MODULE,
    .open   = ICC_Sample_dev_open,
    .release = ICC_Sample_dev_release,
};

#endif

static void ICC_Sample_dev_exit(void);

static int ICC_Sample_dev_init(void)
{
    int err=0;

#ifdef ICC_USE_CHR_DEVICE
    int i;

    /* register device */
    err = alloc_chrdev_region(&dev_no, BASEMINOR, NUM_MINORS, MODULE_NAME); 

    if (err < 0) {
        ICC_ERR("Major number allocation has failed");
        return err;
    }

    ICC_DEBUG("Major number %d", MAJOR(dev_no));


    /* initialize each device */
    for (i = 0; i < NUM_MINORS; i++) {
        cdev_init(&devs[i].cdev, &ICC_Sample_fops);
        cdev_add(&devs[i].cdev, MKDEV(MAJOR(dev_no), i),1);
    }
#endif

    /* Start the ICC Linux Sample */
    if ( Start_ICC_Sample() != 0 ) err = -1;

    if (!err) {
    	ICC_DEBUG("Finished initialization");
    }
    else {
    	ICC_Sample_dev_exit();
    }

    return err;
}

static void ICC_Sample_dev_exit(void)
{
#ifdef ICC_USE_CHR_DEVICE
    int i;
#endif

    if ( Stop_ICC_Sample() != 0 )
        return;

#ifdef ICC_USE_CHR_DEVICE
    for (i = 0; i < NUM_MINORS; i++)
        cdev_del(&devs[i].cdev);

    unregister_chrdev_region(dev_no, NUM_MINORS);
#endif

    ICC_DEBUG("Finished execution");

    /* wait for any thread activity to finish */
    msleep(100);
}


module_init(ICC_Sample_dev_init);
module_exit(ICC_Sample_dev_exit);
