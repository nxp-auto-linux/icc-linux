/**
*   @file    ICC_Interrupts.c
*   @version 0.0.2
*
*   @brief   ICC - Inter Core Communication device driver interrupt support
*   @details       Inter Core Communication device driver interrupt support
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
#include "ICC_Platform.h"

#define LOG_LEVEL       KERN_ALERT

#ifndef ICC_USE_POLLING

extern
void ICC_Remote_Event_Handler(void);

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

extern
void ICC_Local_Event_Handler(void);

#endif

#ifndef ICC_USE_POLLING
static
irqreturn_t ICC_Cpu2Cpu_ISR_Handler(int irq, void *dev_id)
{
    ICC_Remote_Event_Handler();

    return IRQ_HANDLED;
}
#endif

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
static
irqreturn_t ICC_Local_ISR_Handler(int irq, void *dev_id)
{
    ICC_Local_Event_Handler();

    return IRQ_HANDLED;
}

#endif

static int ICC_Enable_Peer_Interrupt(struct ICC_platform_data * icc_data)
{
    if (icc_data) {
        struct device * dev = &icc_data->pdev->dev;
        unsigned int shared_irq = icc_data->shared_irq;

        if (!dev)
            return -ENODEV;

        /* request interrupt line for inter-core notifications */
        if (devm_request_irq(dev, shared_irq, ICC_Cpu2Cpu_ISR_Handler, 0, MODULE_NAME, NULL) != 0)
        {
            printk (KERN_ERR "Failed to register interrupt\n");
            return -ENODEV;
        }

        return 0;
    };

    return -EINVAL;
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

int ICC_Enable_Local_Interrupt(struct ICC_platform_data * icc_data)
{
    if (icc_data) {
        struct device * dev = &icc_data->pdev->dev;
        unsigned int local_irq = icc_data->local_irq;

        if (!dev)
            return -ENODEV;

        /* request interrupt line for local core notifications */
        if (devm_request_irq(dev, local_irq, ICC_Local_ISR_Handler, 0, MODULE_NAME, NULL) != 0) {
            printk (KERN_ERR "Failed to register local interrupt\n");
            return -ENODEV;
        }

        return 0;
    }

    return -EINVAL;
}

#endif

int init_interrupt_data(struct ICC_platform_data * icc_data)
{
    if (icc_data) {
        int err = 0;
        struct platform_device * pdev = icc_data->pdev;

        if (!pdev)
            return -ENODEV;

        icc_data->shared_irq = platform_get_irq(pdev, ICC_CFG_HW_CPU2CPU_IRQ);
#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
        icc_data->local_irq = platform_get_irq(pdev, ICC_CFG_HW_LOCAL_IRQ);
#endif

        /* request interrupt line for intercore notifications */
        if ((err = ICC_Enable_Peer_Interrupt(icc_data)) != 0) {
            return err;
        }

        #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
            /* request interrupt line for local core notifications */
            if (err = ICC_Enable_Local_Interrupt(icc_data) != 0) {
                return err;
            }
        #endif

        return 0;
    }

    return -EINVAL;
}

#endif

