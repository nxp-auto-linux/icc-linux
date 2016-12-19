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

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/platform_device.h>

#include <linux/interrupt.h>

#include "ICC_Api.h"
#include "ICC_Hw.h"
#include "ICC_Notification.h"

#include "ICC_Log.h"
#include "ICC_Of.h"

/* SRAM base address from device tree */
static uint64_t ICC_SRAM_Phys_Base_Addr;

extern char *ICC_Shared_Virt_Base_Addr;

/* first words reserved for other purposes (e.g. a handshake) */
#define ICC_CONFIG_OFFSET_FROM_BASE (0)

/* ICC shared config is not at the beginning of the SRAM,
 * it can be located starting with an offset (1Mb) from SRAM base.
 */
#define ICC_SHARED_PHYS_BASE_ADDR_OFFSET 1024*1024 /* 1Mb */

static const uint64_t get_intr_shmem_base_phys_address(void)
{
    return ICC_SRAM_Phys_Base_Addr + ICC_SHARED_PHYS_BASE_ADDR_OFFSET;
}

void intr_init_shmem(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct platform_device * pdev = icc_data->pdev;
        struct device *dev = &(pdev->dev);
        struct device_node *sram = NULL;

        sram = icc_of_find_sram_node();
        if (sram) {
            uint64_t reg_size = 0;
            uint32_t reg_flags = 0;
            const __be32 * sram_base = of_get_address(sram, 0, &reg_size, &reg_flags);
            ICC_SRAM_Phys_Base_Addr = of_translate_address(sram, sram_base);
            ICC_INFO("Found SRAM block: addr=%#llx size=%d", ICC_SRAM_Phys_Base_Addr, reg_size);

            of_node_put(sram);

            /* Reserve shared memory */
            if (!devm_request_mem_region(dev, get_intr_shmem_base_phys_address(), get_shmem_size(), "ICC_shmem")) {
                ICC_ERR("Failed to request mem region!");
                return;
            }

            /* Map shared memory in local address space */
            ICC_Shared_Virt_Base_Addr = devm_ioremap_nocache(dev, get_intr_shmem_base_phys_address(), get_shmem_size()) + ICC_CONFIG_OFFSET_FROM_BASE;
            ICC_INFO("Reserved ICC_Shared_Virt_Base_Addr=%#llx size=%d", ICC_Shared_Virt_Base_Addr, get_shmem_size() - ICC_CONFIG_OFFSET_FROM_BASE);
            if( !ICC_Shared_Virt_Base_Addr ){
                ICC_ERR("ICC_Shared_Virt_Base_Addr virtual mapping has failed for %#x", get_intr_shmem_base_phys_address());
                return;
            }
        }
    }
}

void intr_cleanup_shmem(struct ICC_platform_data *icc_data)
{}

ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS char * ICC_HW_MSCM_VIRT_BASE;
ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA extern unsigned int (* ICC_Initialized)[2];

int intr_notify_peer(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Trigger_Cpu2Cpu_Interrupt( ICC_CFG_HW_CPU2CPU_IRQ ); /**< trigger remote interrupt */

        return 0;
    }

    ICC_ERR("Interrupts not registered");
    return -EINVAL;
}

void intr_clear_notify_from_peer(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Clear_Cpu2Cpu_Interrupt(ICC_CFG_HW_CPU2CPU_IRQ);
        return;
    }

    ICC_ERR("Interrupts not registered");
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
void intr_notify_local(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Trigger_Local_Interrupt( ICC_CFG_HW_LOCAL_IRQ ); /**< trigger local interrupt */
        return;
    }

    ICC_ERR("Interrupts not registered");
}

void intr_clear_notify_local(void)
{
    if (ICC_HW_MSCM_VIRT_BASE) {
        ICC_HW_Clear_Local_Interrupt( ICC_CFG_HW_LOCAL_IRQ ); /**< trigger local interrupt */
        return;
    }

    ICC_ERR("Interrupts not registered");
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
            ICC_ERR("Failed to register interrupt");
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
            ICC_ERR("Failed to register local interrupt");
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
        struct device *dev = &(pdev->dev);
        struct device_node *mscm = NULL;

        icc_data->shared_irq = -ENXIO;

        mscm = icc_of_find_mscm_node();
        if (mscm) {
            uint64_t reg_size = 0;
            uint32_t reg_flags = 0;
            const __be32 * mscm_base = of_get_address(mscm, 0, &reg_size, &reg_flags);
            uint64_t reg_addr = of_translate_address(mscm, mscm_base);
            ICC_INFO("found MSCM block: addr=%#llx size=%d", reg_addr, reg_size);

            ICC_HW_MSCM_VIRT_BASE = devm_ioremap_nocache(dev, reg_addr, reg_size);
            ICC_INFO("reserved ICC_HW_MSCM_VIRT_BASE=%#llx size=%d", ICC_HW_MSCM_VIRT_BASE, reg_size);

            /* search the mscm node for the interrupts and reserve the appropriate one(s) */
            icc_data->shared_irq = of_irq_get(mscm, ICC_CFG_HW_CPU2CPU_IRQ);
            ICC_INFO("reserved irq=%d from MSCM node", icc_data->shared_irq);

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
            icc_data->local_irq = of_irq_get(mscm, ICC_CFG_HW_LOCAL_IRQ);
#endif

            of_node_put(mscm);

            if (NULL == ICC_HW_MSCM_VIRT_BASE)
            {
                ICC_ERR("MSCM ioremap failed");
                return -ENOMEM;
            }
        }
        else {

            if (icc_data->shared_irq < 0) {
                /* search the icc node itself for the interrupts - backwards compatibility */
                icc_data->shared_irq = platform_get_irq(pdev, ICC_CFG_HW_CPU2CPU_IRQ);
                ICC_INFO("reserved irq=%d", icc_data->shared_irq);

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
                icc_data->local_irq = platform_get_irq(pdev, ICC_CFG_HW_LOCAL_IRQ);
#endif
            }
        }

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

char *ICC_Phys_To_Virt(char *phys_addr)
{
    return (((char*)(((uint64_t)phys_addr) & 0xFFFFFFFFFFFFFFFF) - get_intr_shmem_base_phys_address()) + (uint64_t)ICC_Shared_Virt_Base_Addr);
}
