/**
*   @file    ICC_Interrupts.c
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication device driver interrupt support
*   @details       Inter Core Communication device driver interrupt support
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

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/interrupt.h>

#include "ICC_Api.h"
#include "ICC_Sw_Platform.h"
#include "ICC_Hw.h"

#define LOG_LEVEL       KERN_ALERT

#define IRAM_BASE_ADDR  0x3E900000
#define IRAM_SIZE       0x40000

const uint64_t get_shmem_base_address(void)
{
    return IRAM_BASE_ADDR;
}

const uint32_t get_shmem_size(void)
{
    return IRAM_SIZE;
}

ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS char * ICC_HW_MSCM_VIRT_BASE;
ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA extern unsigned int (* ICC_Initialized)[2];

int intr_notify_peer(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Trigger_Cpu2Cpu_Interrupt( ICC_CFG_HW_CPU2CPU_IRQ ); /**< trigger remote interrupt */

        return 0;
    }

    printk (KERN_ERR "Interrupts not registered\n");
    return -EINVAL;
}

void intr_clear_notify_from_peer(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Clear_Cpu2Cpu_Interrupt(ICC_CFG_HW_CPU2CPU_IRQ);
        return;
    }

    printk (KERN_ERR "Interrupts not registered\n");
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
void intr_notify_local(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Trigger_Local_Interrupt( ICC_CFG_HW_LOCAL_IRQ ); /**< trigger local interrupt */
        return;
    }

    printk (KERN_ERR "Interrupts not registered\n");
}

void intr_clear_notify_local(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Clear_Local_Interrupt( ICC_CFG_HW_LOCAL_IRQ ); /**< trigger local interrupt */
        return;
    }

    printk (KERN_ERR "Interrupts not registered\n");
}
#endif

extern
void ICC_Remote_Event_Handler(void);

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

extern
void ICC_Local_Event_Handler(void);

#endif

static
irqreturn_t ICC_Cpu2Cpu_ISR_Handler(int irq, void *dev_id)
{
    ICC_Remote_Event_Handler();

    return IRQ_HANDLED;
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
static
irqreturn_t ICC_Local_ISR_Handler(int irq, void *dev_id)
{
    ICC_Local_Event_Handler();

    return IRQ_HANDLED;
}

#endif

static int ICC_Enable_Peer_Interrupt(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct device *dev = &(icc_data->pdev->dev);
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

extern void ICC_Clear_Notify_From_Peer(void);

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
extern void ICC_Clear_Notify_Local(void);
#endif

/*
 * OS specific initialization of interrupts.
 * This function has restricted functionality, since interrupt
 * related code is device specific and should be handled by the
 * device initialization code.
 */
ICC_Err_t ICC_OS_Init_Interrupts( void )
{
    if ( ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_REMOTE_CORE_ID ] )
    {
       ICC_Clear_Notify_From_Peer();
    }

    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
        ICC_Clear_Notify_Local();
    #endif

    return ICC_SUCCESS;
}


int init_interrupt_data(struct ICC_platform_data * icc_data)
{
    if (icc_data) {
        int err = 0;
        struct platform_device * pdev = icc_data->pdev;
        struct device *dev = &pdev->dev;

        if (!pdev)
            return -ENODEV;

        ICC_HW_MSCM_VIRT_BASE = devm_ioremap_nocache(dev, ICC_HW_MSCM_BASE, ICC_HW_MSCM_SIZE);
        printk(LOG_LEVEL "[init_interrupt_data] reserved ICC_HW_MSCM_VIRT_BASE=%#llx size is %d\n", ICC_HW_MSCM_VIRT_BASE, ICC_HW_MSCM_SIZE);

        if (NULL == ICC_HW_MSCM_VIRT_BASE)
        {
            printk (KERN_ALERT "MSCM ioremap failed\n");
            return -ENOMEM;
        }

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

