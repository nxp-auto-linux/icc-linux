 /**
*   @file    ICC_Os.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication OS API
*   @details Inter Core Communication OS API
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

#ifndef ICC_OS_H
#define ICC_OS_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "ICC_Types.h"


/*
 * OS abstraction API declaration
 */


#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"

/*
 * OS specific initialization
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Initialize( ICC_IN const ICC_Config_t * config_ptr );

/*
 * OS specific initialization of interrupts
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Init_Interrupts( void );


/*
 * Finalize OS specific elements
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Finalize( void );

/*
 * Try and get OS semaphore, starts single core critical section
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Get_Semaphore( ICC_IN ICC_Channel_t ch_id,
                                ICC_IN unsigned int fifo_id );


/*
 * Release OS semaphore, starts single core critical section
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Release_Semaphore( ICC_IN ICC_Channel_t ch_id,
                                    ICC_IN unsigned int fifo_id );
#ifndef ICC_CFG_NO_TIMEOUT

/*
 * Wait on channel specific (already configured) event.
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Wait_Event( ICC_IN ICC_Channel_t ch_id,
                             ICC_IN unsigned int fifo_id );


/*
 * Get channel specific (already configured) event.
 * Used to differentiate between different wake-up sources.
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Get_Event( ICC_IN ICC_Channel_t ch_id,
                            ICC_IN unsigned int fifo_id,
                            ICC_OUT ICC_Event_t    * event );

/*
 * Set channel specific (already configured) event.
 * Used to differentiate between different wake-up sources.
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Set_Event( ICC_IN ICC_Channel_t ch_id,
                            ICC_IN unsigned int fifo_id );


/*
 * Clear specified event.
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Clear_Event( ICC_IN ICC_Channel_t ch_id,
                              ICC_IN unsigned int  fifo_id,
                              ICC_IN ICC_Event_t   event );

/*
 * Set relative alarm for specific channel, at trigger do configured action
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Set_Rel_Alarm( ICC_IN ICC_Channel_t ch_id,
                                ICC_IN unsigned int fifo_id,
                                ICC_IN ICC_Timeout_t    timeout );



/*
 * Cancel relative alarm for specific channel
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_OS_Cancel_Alarm( ICC_IN ICC_Channel_t ch_id,
                               ICC_IN unsigned int fifo_id );

    #ifdef ICC_CFG_HEARTBEAT_ENABLED

        /*
         * Set Heartbeat channel specific (already configured) event.
         */
        ICC_ATTR_SEC_TEXT_CODE
        extern
        ICC_Err_t ICC_OS_Set_Rate_Event( ICC_IN ICC_Channel_t ch_id,
                                         ICC_IN unsigned int fifo_id );

        /*
         * Wait on the Heartbeat channel the rate event.
         */
        ICC_ATTR_SEC_TEXT_CODE
        extern
        ICC_Err_t
        ICC_OS_Wait_Rate_Event( ICC_IN ICC_Channel_t ch_id,
                          ICC_IN unsigned int fifo_id );


        /*
         * Clear rate event for Heartbeat.
         */
        ICC_ATTR_SEC_TEXT_CODE
        extern
        ICC_Err_t ICC_OS_Clear_Rate_Event( ICC_IN ICC_Channel_t ch_id,
                                           ICC_IN unsigned int  fifo_id );

        /*
         * Set recurrent relative alarm for ICC Heartbeat reserved channel, at trigger do configured action
         */
        ICC_ATTR_SEC_TEXT_CODE
        extern
        ICC_Err_t ICC_OS_Set_Recurrent_Rel_Alarm( ICC_IN ICC_Channel_t ch_id,
                                                  ICC_IN unsigned int fifo_id );

        /*
         * Cancel recurrent relative alarm for ICC Heartbeat reserved channel
         */
        ICC_ATTR_SEC_TEXT_CODE
        extern
        ICC_Err_t ICC_OS_Cancel_Recurrent_Alarm( ICC_IN ICC_Channel_t ch_id,
                                                 ICC_IN unsigned int fifo_id );

    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

#endif /* no ICC_CFG_NO_TIMEOUT */

#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif

#endif /* ICC_OS_H */
