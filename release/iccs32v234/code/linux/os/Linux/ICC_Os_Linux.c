/*
 *  ICC_Os_Linux.c
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication AUTOSAR API implementation
*   @details Inter Core Communication AUTOSAR API implementation
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

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <asm/atomic.h>

#include "ICC_Config.h"

#include "ICC_Os.h"
#include "ICC_Types.h"
#include "ICC_Fifo.h"
#include "ICC_Private.h"

/*
 * Implement Os API using Linux specifics
 */

#define ICC_START_SEC_VAR_UNSPECIFIED
#include "ICC_MemMap.h"

    ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS extern ICC_Fifo_Ram_t (* ICC_Fifo_Ram)[ ICC_CFG_MAX_CHANNELS ][ 2 ];
    ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA extern volatile unsigned int          (* ICC_Initialized)[2];

    ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS extern ICC_Config_t * ICC_Config_Ptr;

#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED

    ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS wait_queue_head_t * rate_wait_queue;

    /*
     * rate_cond - 1 when Heartbeat mechanism will not take into account the sending message rate
     * rate_cond - 0 when Heartbeat mechanism will take into account the sending message rate.
     *
     */
    ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS atomic_t rate_cond;

    /* kernel timer to determine the messages rate */
    ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS struct timer_list timer;
    ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA ICC_Timeout_t rate_timeout;

    #endif /* ICC_CFG_HEARTBEAT_ENABLED */
#endif /* no ICC_CFG_NO_TIMEOUT */




#define ICC_STOP_SEC_VAR_UNSPECIFIED
#include "ICC_MemMap.h"



#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    /* timer handler */
    static void ICC_HB_timer_handler(unsigned long var)
    {
        atomic_set(&rate_cond, 1);
        mod_timer(&timer, jiffies + usecs_to_jiffies(rate_timeout));
    }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */
#endif /* no ICC_CFG_NO_TIMEOUT */
/*
 * OS specific initialization
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Initialize(ICC_IN const ICC_Config_t * unused_config_ptr)
{

    int i, j;

    ICC_Fifo_Os_Ram_t       * fifo_os_ram;
    ICC_Fifo_Config_t       * fifo_config;
    ICC_Fifo_Ram_t          * fifo_ram;

#ifndef ICC_CFG_NO_TIMEOUT
    wait_queue_head_t* wait_queue = kmalloc(sizeof(wait_queue_head_t), GFP_KERNEL);

    if (!wait_queue)
        return ICC_ERR_OS_LINUX_CREATE_WAITQUEUE;

    init_waitqueue_head(wait_queue);

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        rate_wait_queue = kmalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
        if (!rate_wait_queue)
        {
            return ICC_ERR_OS_LINUX_CREATE_RATEWAITQUEUE;
        }

        init_waitqueue_head(rate_wait_queue);

        atomic_set(&rate_cond, 0);
        rate_timeout = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config)->rate_usec;

        setup_timer(&timer, ICC_HB_timer_handler, 0);
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

#endif /* no ICC_CFG_NO_TIMEOUT */

    /* assign waitqueue for each fifo and semaphore */
    for (i = 0; i < ICC_Config_Ptr->Channels_Count; i++)
    {
        for (j = 0; j < 2; j++)
        {
            fifo_ram    = &((*ICC_Fifo_Ram)[i][j]);
            fifo_os_ram = ICC_CROSS_VALUE_OF(fifo_ram -> fifo_os_ram[ ICC_GET_CORE_ID ]);
            fifo_config = ICC_CROSS_VALUE_OF(fifo_ram -> fifo_config[ ICC_GET_CORE_ID ]);
#ifndef ICC_CFG_NO_TIMEOUT
            if (fifo_config->fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED) fifo_os_ram->wait_queue = wait_queue;
                                                                        else fifo_os_ram->wait_queue = NULL;

            fifo_os_ram->timeout = 0;
#endif /* no ICC_CFG_NO_TIMEOUT */

            sema_init(&(fifo_os_ram->fifo_lock), 1);

        }
    }

    return ICC_SUCCESS;
}

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Finalize(void)
{

#ifndef ICC_CFG_NO_TIMEOUT
    ICC_Fifo_Ram_t           * fifo_ram = NULL;
    ICC_Fifo_Os_Ram_t        * fifo_os_ram = NULL;
#endif /* no ICC_CFG_NO_TIMEOUT */

#ifndef ICC_CFG_NO_TIMEOUT
    /* free the waitqueue */
    fifo_ram    = &((*ICC_Fifo_Ram)[0][0]);
   	fifo_os_ram = ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ]);

    if (fifo_os_ram && (fifo_os_ram->wait_queue != NULL))
    {
        kfree( fifo_os_ram->wait_queue );
    }


    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( rate_wait_queue != NULL )
        {
            kfree( rate_wait_queue );
        }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

#endif /* no ICC_CFG_NO_TIMEOUT */

    return ICC_SUCCESS;
}



/*
 * Try and get OS semaphore, starts single core critical section
 *
 * Return messages:
 *         ICC_ERR_OS_SEM_BUSY : semaphore already taken
 *         ICC_SUCCESS : semaphore was taken successfully
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Get_Semaphore( ICC_IN ICC_Channel_t ch_id,
                      ICC_IN unsigned int fifo_id)
{

    ICC_Fifo_Ram_t       * fifo_ram     = &((*ICC_Fifo_Ram)[ch_id][fifo_id]);
    struct semaphore     * sem          = &ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->fifo_lock;

    /* try and get semaphore */
    int err_lock = down_trylock(sem);

    if (err_lock != 0)
        return ICC_ERR_OS_LINUX_SEM_BUSY;


    return ICC_SUCCESS;
}


/*
 * Release OS semaphore, starts single core critical section
 *
 * Return messages:
 *         ICC_SUCCESS : semaphore was releases successfully
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Release_Semaphore( ICC_IN ICC_Channel_t ch_id,
                          ICC_IN unsigned int fifo_id)
{

    ICC_Fifo_Ram_t       * fifo_ram  = &((*ICC_Fifo_Ram)[ch_id][fifo_id]);
    struct semaphore     * sem       = &ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->fifo_lock;

    /* release semaphore */
    up (sem);

    return ICC_SUCCESS;
}

#ifndef ICC_CFG_NO_TIMEOUT
/*
 * Wait on channel specific (already configured) event.
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Wait_Event( ICC_IN ICC_Channel_t ch_id,
                   ICC_IN unsigned int fifo_id )
{

    ICC_Fifo_Ram_t       * fifo_ram  = &((*ICC_Fifo_Ram)[ch_id][fifo_id]);
    unsigned int         current_status, ret;


    ICC_Timeout_t timeout = ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->timeout;

    if (timeout != 0)
    {
        switch (fifo_id)
        {
            case ICC_TX_FIFO:
                current_status = fifo_ram->rd[ICC_GET_CORE_ID];
                ret = wait_event_interruptible_timeout (*(ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->wait_queue),
                                                       ((*ICC_Initialized)[ ICC_GET_CORE_ID ] == ICC_NODE_STATE_UNINIT) || 
                                                        (current_status != fifo_ram->rd[ICC_GET_REMOTE_CORE_ID])  || 
                                                        (ICC_SUCCESS == ICC_Fifo_Msg_Fits(fifo_ram, fifo_ram->pending_send_msg_size)),
                                                       usecs_to_jiffies(timeout));
                break;

            case ICC_RX_FIFO:
                current_status = fifo_ram->wr[ICC_GET_CORE_ID];
                ret = wait_event_interruptible_timeout (*(ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->wait_queue),
                                                       ((*ICC_Initialized)[ ICC_GET_CORE_ID ] == ICC_NODE_STATE_UNINIT) || 
                                                       (current_status != fifo_ram->wr[ICC_GET_REMOTE_CORE_ID]) || 
                                                       ( ICC_FIFO_Pending(fifo_ram) > ICC_HEADER_SIZE ),
                                                       usecs_to_jiffies(timeout));
                break;

            default:
                ret = -1;/* error */
                break;

            }

        if (ret < 0)
            return ICC_ERR_OS_LINUX_ERESTARTSYS;
        else if (ret == 0)
            /* the timeout elapsed */
            ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->event_type = ICC_EVENT_TIMEOUT_ALM;
        else
            ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->event_type = ICC_EVENT_ACTIVITY_ISR;

        ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->timeout = 0;
    } else {

        switch (fifo_id)
        {
            case ICC_TX_FIFO:
                current_status = fifo_ram->rd[ICC_GET_CORE_ID];
                ret = wait_event_interruptible (*(ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->wait_queue),
                                               ((*ICC_Initialized)[ ICC_GET_CORE_ID ] == ICC_NODE_STATE_UNINIT) || 
                                               (current_status != fifo_ram->rd[ICC_GET_REMOTE_CORE_ID]) || 
                                               (ICC_SUCCESS == ICC_Fifo_Msg_Fits(fifo_ram, fifo_ram->pending_send_msg_size))  );
                break;

            case ICC_RX_FIFO:
                current_status = fifo_ram->wr[ICC_GET_CORE_ID];
                ret = wait_event_interruptible (*(ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->wait_queue),
                                               ((*ICC_Initialized)[ ICC_GET_CORE_ID ] == ICC_NODE_STATE_UNINIT) || 
                                               (current_status != fifo_ram->wr[ICC_GET_REMOTE_CORE_ID]) || 
                                               ( ICC_FIFO_Pending(fifo_ram) > ICC_HEADER_SIZE ) );
                break;

            default:
                ret = -1; /* error */
                break;
        }

        if (ret != 0)
            return ICC_ERR_OS_LINUX_ERESTARTSYS;

        ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->event_type = ICC_EVENT_ACTIVITY_ISR;

    }

    return ICC_SUCCESS;
}




/*
 * Get channel specific (already configured) event.
 * Used to differentiate between different wake-up sources.
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Get_Event( ICC_IN ICC_Channel_t ch_id,
                  ICC_IN unsigned int fifo_id,
                  ICC_OUT       ICC_Event_t    * event )
{
    ICC_Fifo_Ram_t       * fifo_ram  = &((*ICC_Fifo_Ram)[ch_id][fifo_id]);

    /* differentiate between events */
    *event = ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->event_type;

    return ICC_SUCCESS;
}


/*
 * Clear specified event.
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Clear_Event( ICC_IN ICC_Channel_t ch_id,
                    ICC_IN unsigned int  fifo_id,
                    ICC_IN ICC_Event_t   event)
{

    return ICC_SUCCESS;
}


/*
 * Set relative alarm for specific channel, at trigger do configured action
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Set_Rel_Alarm( ICC_IN ICC_Channel_t ch_id,
                      ICC_IN unsigned int fifo_id,
                      ICC_IN       ICC_Timeout_t    timeout )
{

    ICC_Fifo_Ram_t       * fifo_ram  = &((*ICC_Fifo_Ram)[ch_id][fifo_id]);
    ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->timeout = timeout;

    return ICC_SUCCESS;
}



ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Cancel_Alarm( ICC_IN ICC_Channel_t ch_id,
                               ICC_IN unsigned int fifo_id )
{
    return ICC_SUCCESS;
}


ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Set_Event( ICC_IN ICC_Channel_t ch_id,
                  ICC_IN unsigned int fifo_id )
{
    ICC_Fifo_Ram_t       * fifo_ram  = &((*ICC_Fifo_Ram)[ch_id][fifo_id]);

    wait_queue_head_t    * fifo_wq   = ICC_CROSS_VALUE_OF(fifo_ram->fifo_os_ram[ ICC_GET_CORE_ID ])->wait_queue;

    if (waitqueue_active( fifo_wq ))
    {
        wake_up_interruptible( fifo_wq );
    }

    return ICC_SUCCESS;
}

#ifdef ICC_CFG_HEARTBEAT_ENABLED
/*
 * Set Heartbeat channel specific (already configured) event.
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Set_Rate_Event( ICC_IN ICC_Channel_t ch_id,
                       ICC_IN unsigned int fifo_id )
{
    ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);

    if( ch_id != HB_OS_config->channel_id )
    {
        return ICC_ERR_OS_LINUX_WRONGCHNID;
    }

    if( waitqueue_active( rate_wait_queue ) )
    {
        wake_up_interruptible( rate_wait_queue );
    }

    /* disable sending messages rate */
    atomic_set(&rate_cond, 0);

    return ICC_SUCCESS;
}

/*
 * Wait on the Heartbeat channel the rate event.
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Wait_Rate_Event( ICC_IN ICC_Channel_t ch_id,
                        ICC_IN unsigned int fifo_id )
{

    unsigned int ret;
    ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);

    if( ch_id != HB_OS_config->channel_id )
    {
        return ICC_ERR_OS_LINUX_WRONGCHNID;
    }

    if ( HB_OS_config->rate_usec != 0 )
    {
        atomic_set(&rate_cond, 0);
        ret = wait_event_interruptible_timeout (
                                *(rate_wait_queue),
                                atomic_read(&rate_cond),
                                usecs_to_jiffies( HB_OS_config->rate_usec )
                                );

        if (ret < 0)
            return ICC_ERR_OS_LINUX_ERESTARTSYS;

    }

    return ICC_SUCCESS;
}

/*
 * Clear rate event for Heartbeat.
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Clear_Rate_Event( ICC_IN ICC_Channel_t ch_id,
                         ICC_IN unsigned int  fifo_id )
{
    ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);
    if( ch_id != HB_OS_config->channel_id )
    {
        return ICC_ERR_OS_LINUX_WRONGCHNID;
    }

    atomic_set( &rate_cond, 0);

    return ICC_SUCCESS;

}

/*
 * Set recurrent relative alarm for ICC Heartbeat reserved channel, at trigger do configured action
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Set_Recurrent_Rel_Alarm( ICC_IN ICC_Channel_t ch_id,
                                ICC_IN unsigned int fifo_id )
{
    int ret;

    ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);
    if( ch_id != HB_OS_config->channel_id )
    {
        return ICC_ERR_OS_LINUX_WRONGCHNID;
    }
    ret = mod_timer(&timer, jiffies + usecs_to_jiffies(rate_timeout));
    /* check whether mod_timer has modified the timer successfully or not
     * (i.e. mod_timer of an inactive timer returns 0, mod_timer of an active timer returns 1)
     * see mod_timer documentation
     */
    if( (ret != 0) && (ret != 1) )
        return ICC_ERR_OS_LINUX_SETTIMER;


    return ICC_SUCCESS;
}

/*
 * Cancel recurrent relative alarm for ICC Heartbeat reserved channel
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_OS_Cancel_Recurrent_Alarm( ICC_IN ICC_Channel_t ch_id,
                               ICC_IN unsigned int fifo_id )
{
    ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);
    int ret;
    if( ch_id != HB_OS_config->channel_id )
    {
        return ICC_ERR_OS_LINUX_WRONGCHNID;
    }

    ret = del_timer_sync(&timer);
    if( ret )
        return ICC_ERR_OS_LINUX_BUSYTIMER;
    return ICC_SUCCESS;
}

#endif /* ICC_CFG_HEARTBEAT_ENABLED */

#endif /* no  ICC_CFG_NO_TIMEOUT */

#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif

