/**
*   @file    ICC_Linux_Sample.c
*   @version 0.0.1
*
*   @brief   ICC - Inter Node Communication
*   @details       Inter Node Communication module
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

#include "ICC_Api.h"
#include <linux/kthread.h>

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    #include <asm/atomic.h>
#endif

#define RCV_BUF_SIZE 1024
#define SND_BUF_SIZE 100

#define ICC_TX_FIFO    ICC_GET_CORE_ID
#define ICC_RX_FIFO    ICC_GET_REMOTE_CORE_ID

#define ICC_CHECK_ERR_CODE(icc_function_to_call)                \
             return_code = icc_function_to_call;                \
             if (ICC_SUCCESS != return_code) return return_code


volatile unsigned int watch_CB_Rx[3];
volatile unsigned int watch_CB_Tx[3];
volatile unsigned int watch_CB_Ch[3];
volatile unsigned int watch_CB_Node;

volatile unsigned short wach_Ch_Rx[2];
volatile unsigned short wach_Ch_Tx[2];

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    /* heartbeat flag which determine if the node is prepare for
     * the heartbeat mechanism
     */
    volatile atomic_t heartbeat_on;
#endif

volatile unsigned int watch_total_Rx_msgs[2]={0};

#ifdef ICC_CFG_HEARTBEAT_ENABLED
int HeartBeat(void *data)
{
   int err =0;
   ICC_Heartbeat_Initialize();
   err = ICC_Heartbeat_Runnable();
   if( ICC_SUCCESS != err ){
       printk("Eroare ICC_Heartbeat %d \n", err);
   }

   return 0;
}
#endif


void USER_ICC_Callback_Rx_CB_App(
                               ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */
                               )
{
    ICC_Err_t icc_status;
    ICC_Timeout_t timeout;
    ICC_Msg_Size_t rx_msg_size;
    unsigned char msg[SND_BUF_SIZE];
    unsigned char * rcv_buffer;

#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( channel_id == 0 ){
            return;
        }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */
#endif /* not ICC_CFG_NO_TIMEOUT */
    rcv_buffer = kzalloc( RCV_BUF_SIZE, GFP_ATOMIC);
    watch_CB_Rx[channel_id]++;

    if( rcv_buffer == NULL )
        return;

    if( ICC_Config0.Channels_Ptr[channel_id].fifos_cfg[ICC_RX_FIFO].fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED ) {
        timeout = ICC_WAIT_FOREVER;
    }
    else{
        timeout = ICC_WAIT_ZERO;
    }



    icc_status = ICC_Msg_Recv( channel_id, rcv_buffer, RCV_BUF_SIZE, &rx_msg_size, timeout, ICC_RX_NORMAL );
    if( icc_status != ICC_SUCCESS )
    {
        printk("POP failed with err %d \n", icc_status );
        kfree(rcv_buffer);
        return;
    } else {

        watch_total_Rx_msgs[channel_id]++;
        printk("POP: Receive message [%d] from [%u] via ch [%u]: \n %s \n", watch_total_Rx_msgs[channel_id], ICC_GET_REMOTE_CORE_ID, channel_id, rcv_buffer );
    }

    wach_Ch_Rx[channel_id]++;






    memset(msg, 0, SND_BUF_SIZE);
    memcpy(msg,"Hello_AUTOSAR", 14 );

    icc_status = ICC_Msg_Send( channel_id, msg, 14, ICC_WAIT_ZERO);
    if( icc_status != ICC_SUCCESS )
    {
        printk("SEND failed: err code:%d\n", icc_status);
        kfree(rcv_buffer);
        return;
    }

    wach_Ch_Tx[channel_id]++;

    kfree(rcv_buffer);

}

void USER_ICC_Callback_Tx_CB_App(
                               ICC_IN const ICC_Channel_t   channel_id           /**< the id of the channel that just "received" free space */
                               )
{
    watch_CB_Tx[channel_id]++;
}


void USER_ICC_Callback_Channel_State_Update_CB_App(
                                                  ICC_IN const ICC_Channel_t          channel_id,   /**< the id of the channel that transitioned to a new state */
                                                  ICC_IN const ICC_Channel_State_t    channel_state /**< the new state that the channel transitioned to */
                                                 )
{
    watch_CB_Ch[channel_id]++;
}

void USER_ICC_Node_State_Update_CB_App(
                                       ICC_IN const ICC_Node_t        node_id,    /**< the id of the node that transitioned to a new state */
                                       ICC_IN const ICC_Node_State_t  node_state  /**< the new state the node transitioned to */
                                      )
{

    watch_CB_Node++;
#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    if( node_state == ICC_NODE_STATE_INIT )
{
        atomic_set( &heartbeat_on, 1);
   }
#endif /*ICC_CFG_HEARTBEAT_ENABLED*/

    }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */




/* Start_ICC_Sample - starts the sample code */
int Start_ICC_Sample(void)
{
    int i;
    ICC_Err_t return_code;

#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    atomic_set( &heartbeat_on, 0 );
    #endif
#endif /* not ICC_CFG_NO_TIMEOUT */

    for( i = 0; i < 3; i++ )
    {
            watch_CB_Rx[i]=0;
            watch_CB_Tx[i]=0;
            watch_CB_Ch[i]=0;
    }

    watch_CB_Node=0;
    /* initialize ICC */
    if ((return_code = ICC_Initialize( &ICC_Config0 )) != ICC_SUCCESS) {

        printk("Start_ICC_Sample: ICC_Initialize failed with error code: %d\n", return_code);

        return return_code;
    }
    printk("ICC_Initialize done...\n");
    /* open communication channels */
    for (i = 0; i < ICC_Config0.Channels_Count; i++)
    {

#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( i != ICC_Config0.ICC_Heartbeat_Os_Config->channel_id )
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */
#endif
       ICC_CHECK_ERR_CODE(ICC_Open_Channel(i));
    }
    printk("All channels open done...\n");
#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    while( atomic_read( &heartbeat_on) == 0 );

    kthread_run( HeartBeat, NULL, "%skthread%d", "HB", 0 );

    #endif
#endif /* not ICC_CFG_NO_TIMEOUT */


    return ICC_SUCCESS;
}

/* Stop_ICC_Sample - releases the ICC resources */
int Stop_ICC_Sample(void){
    ICC_Err_t return_code;

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    ICC_CHECK_ERR_CODE(ICC_Heartbeat_Finalize());
#endif /*ICC_CFG_HEARTBEAT_ENABLED*/


    /* ICC clean up */
    ICC_CHECK_ERR_CODE(ICC_Finalize());

    return ICC_SUCCESS;
}
