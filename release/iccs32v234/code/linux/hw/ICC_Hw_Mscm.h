 /**
*   @file    ICC_Hw_Mscm.h
*   @version 0.8.0
*
*   @brief   ICC - MSCM register definitions
*   @details MSCM register definitions
*
*   @addtogroup [ICC]
*   @{
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : Arm Architecture
*   Peripheral           : 
*   Dependencies         : none
*
*   SW Version           : 0.8.0
*   Build Version        : S32V234_ICC_0.8.0
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
/*==================================================================================================
==================================================================================================*/

#ifndef ICC_HW_MSCM_H_
#define ICC_HW_MSCM_H_

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <asm/io.h>

#include "ICC_Hw_Platform.h"

    #define ICC_ASM  asm volatile
    #define ICC_MSYNC() ICC_ASM("dsb sy\n")

    /* Field definitions for IRCPGIR */
    #define ICC_MSCM_IRCPGIR_TLF_SHIFT      (24UL)
    #define ICC_MSCM_IRCPGIR_TLF_MASK       ((0x00000003UL) << (ICC_MSCM_IRCPGIR_TLF_SHIFT))
    #define ICC_MSCM_IRCPGIR_TLF(n)         ((n << ICC_MSCM_IRCPGIR_TLF_SHIFT) & ICC_MSCM_IRCPGIR_TLF_MASK)

    #define ICC_MSCM_IRCPGIR_INTID_SHIFT    (0UL)
    #define ICC_MSCM_IRCPGIR_INTID_MASK     ((0x00000003UL) << (ICC_MSCM_IRCPGIR_INTID_SHIFT))
    #define ICC_MSCM_IRCPGIR_INTID(n)       ((n << ICC_MSCM_IRCPGIR_INTID_SHIFT) & ICC_MSCM_IRCPGIR_INTID_MASK)


    #define ICC_MSCM_IRCPxIR_INTx_MASK(n)   ((0x00000001UL) << (n))


#define ICC_START_SEC_VAR_UNSPECIFIED
#include "ICC_MemMap.h"

    extern ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS char * ICC_HW_MSCM_VIRT_BASE;

#define ICC_STOP_SEC_VAR_UNSPECIFIED
#include "ICC_MemMap.h"


    #define ICC_MSCM_IRCP0IR                (ICC_HW_MSCM_VIRT_BASE + 0x800UL )           /**< Interrupt Router CP0 Interrupt Register */
    #define ICC_MSCM_IRCP1IR                (ICC_HW_MSCM_VIRT_BASE + 0x804UL )           /**< Interrupt Router CP1 Interrupt Register */

    #define ICC_MSCM_IRCPGIR                (ICC_HW_MSCM_VIRT_BASE + 0x820UL )           /**< Interrupt Router CPU Generate Interrupt Register */
    #define ICC_MSCM_IRSPRC(n)              (ICC_HW_MSCM_VIRT_BASE + 0x880UL + 2*(n) )   /**< Interrupt Router Shared Peripheral Routing Control n */

    /*
     * trigger ISR to notify the remote node
     */
    #define ICC_HW_Trigger_Cpu2Cpu_Interrupt(int_id) \
                                                     \
            ICC_MSYNC();                             \
            iowrite32(ICC_MSCM_IRCPGIR_TLF(1) | ICC_MSCM_IRCPGIR_INTID(int_id), ICC_MSCM_IRCPGIR)


    /*
     * clear ISR flag
     */
    #define ICC_HW_Clear_Cpu2Cpu_Interrupt(int_id)                                  \
            if (0 == ICC_GET_CORE_ID) {                                             \
                iowrite32(ICC_MSCM_IRCPxIR_INTx_MASK(int_id), ICC_MSCM_IRCP0IR);    \
                                                                                    \
            } else if (1 == ICC_GET_CORE_ID) {                                      \
                                                                                    \
                iowrite32(ICC_MSCM_IRCPxIR_INTx_MASK(int_id), ICC_MSCM_IRCP1IR);    \
            }


    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

         /*
          * trigger ISR to notify the local node
          */
         #define ICC_HW_Trigger_Local_Interrupt(int_id)  \
                                                         \
                ICC_MSYNC();                             \
                iowrite32(ICC_MSCM_IRCPGIR_TLF(2) | ICC_MSCM_IRCPGIR_INTID(int_id), ICC_MSCM_IRCPGIR)

         /*
          * clear ISR flag for local interrupt
          */
         #define ICC_HW_Clear_Local_Interrupt(int_id) ICC_HW_Clear_Cpu2Cpu_Interrupt(int_id)

    #endif /* ICC_CFG_LOCAL_NOTIFICATIONS */






#endif /* ICC_HW_MSCM_H_ */
