 /**
*   @file    ICC_Os_Types.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication OS types
*   @details Inter Core Communication OS types
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
/*==================================================================================================
==================================================================================================*/

#ifndef ICC_OS_TYPES_H_
#define ICC_OS_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * APP OS alternatives
 */

/* Linux OS */

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/slab.h>


/*
 * per Fifo OS specific RAM structure
 */
typedef struct {


        /*
         * APP OS alternatives
         */
            /* Linux OS */
            #ifndef ICC_CFG_NO_TIMEOUT
                wait_queue_head_t * wait_queue;
                uint32_t timeout;
                uint32_t event_type;
            #endif /* not ICC_CFG_NO_TIMEOUT */
            struct semaphore fifo_lock;



} ICC_Fifo_Os_Ram_t;

/* placeholder type */
typedef uint32_t ICC_Fifo_Os_Config_t;

#ifndef ICC_CFG_NO_TIMEOUT

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    /*
     * HEARTBEAT OS specific Config structure
     */
    typedef struct {
        uint32_t channel_id;                         /**<  channel id reserved for Heartbeat */
        uint32_t rate_usec;                         /**<  sent messages rate */
        uint32_t txrx_timeout;                       /**<  received message timeout:  txrx_timeout > rate_ticks */

    } ICC_Heartbeat_Os_Config_t;

    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

#endif /* ICC_CFG_NO_TIMEOUT */

#ifdef __cplusplus
}
#endif

#endif /* ICC_OS_TYPES_H_ */
