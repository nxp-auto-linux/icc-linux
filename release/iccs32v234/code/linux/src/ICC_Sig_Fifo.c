/**
*   @file    ICC_Sig_Fifo.c
*   @version 0.8.0
*
*   @brief   ICC - Inter Node Communication Signal FIFO API implementation
*   @details Inter Node Communication  Signal FIFO API implementation
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


#include "ICC_Config.h"

#include "ICC_Sig_Fifo.h"

#include "ICC_Hw.h"

#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"


ICC_ATTR_SEC_TEXT_CODE
unsigned int
ICC_Sig_Fifo_Full  (ICC_Signal_Fifo_Ram_t * queue)
{

    ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&queue->signal_queue_head, sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&queue->signal_queue_tail, sizeof(unsigned int) );

    return ((((queue)->signal_queue_tail + 1) % ICC_CFG_MAX_SIGNALS) == (queue)->signal_queue_head);
}


ICC_ATTR_SEC_TEXT_CODE
unsigned int
ICC_Sig_Fifo_Empty (ICC_Signal_Fifo_Ram_t * queue)
{
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&queue->signal_queue_head, sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&queue->signal_queue_tail, sizeof(unsigned int) );

    return ((queue)->signal_queue_head == (queue)->signal_queue_tail);
}



ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Sig_Fifo_Init  (ICC_Signal_Fifo_Ram_t * queue)
{

    queue->signal_queue_head = 0;
    queue->signal_queue_tail = 0;

    ICC_DCACHE_FLUSH_MLINES( (addr_t)&queue->signal_queue_head, sizeof(unsigned int) );
    ICC_DCACHE_FLUSH_MLINES( (addr_t)&queue->signal_queue_tail, sizeof(unsigned int) );

    return ICC_SUCCESS;
}


/*
 * Queue a signal
 *
 * Signal circular queue rules:
 *  - tail points to next free slot
 *  - head points to first occupied slot
 *  - head == tail indicates empty
 *  - (tail + 1) % len = full
 *
 * This method costs 1 slot since you need to differentiate
 * between full and empty (if you fill the last slot it looks
 * like empty since h == t)
 *
 * ICC_SUCCESS     - OK, signal queued
 * ICC_ERR_SQ_FULL - signal queue is full
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Sig_Fifo_Signal(ICC_Signal_Fifo_Ram_t * queue,
                    ICC_Signal_t         signal)
{
    int tail;
    ICC_Err_t return_err;

    ICC_DCACHE_INVALIDATE_MLINES((addr_t)queue, sizeof(ICC_Signal_Fifo_Ram_t));

    if (ICC_Sig_Fifo_Full(queue)) {
        tail = queue->signal_queue_head;
        return_err = ICC_ERR_SQ_FULL;
    }
    else{
        tail = queue->signal_queue_tail;
        return_err = ICC_SUCCESS;
    }

    queue->signals_received[tail] = signal;

    queue->signal_queue_tail = (tail == (ICC_CFG_MAX_SIGNALS-1)) ? 0 : tail+1;

    ICC_DCACHE_FLUSH_MLINES((addr_t)queue, sizeof(ICC_Signal_Fifo_Ram_t));

    return return_err;
}



/*
 * Dequeues a signal
 *
 * It dequeues a signal from the signal queue for the particular core.
 *
 *
 * ICC_SUCCESS      - OK, signal queued
 * ICC_ERR_SQ_EMPTY - signal queue is empty, nothing to dequeue
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Sig_Fifo_Dequeue_Signal(ICC_Signal_Fifo_Ram_t * queue,
                            ICC_Signal_t          * signal)
{
    int head;

    ICC_DCACHE_INVALIDATE_MLINES((addr_t)queue, sizeof(ICC_Signal_Fifo_Ram_t));

    if(ICC_Sig_Fifo_Empty(queue)) {return ICC_ERR_SQ_EMPTY;}

    head = queue->signal_queue_head;

    *signal = queue->signals_received[ head ];

    queue->signal_queue_head = (head == (ICC_CFG_MAX_SIGNALS-1)) ? 0 : head+1;

    ICC_DCACHE_FLUSH_MLINES((addr_t)queue, sizeof(ICC_Signal_Fifo_Ram_t));

    return ICC_SUCCESS;
}

#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif
