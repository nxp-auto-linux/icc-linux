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

#include "ICC_Api.h"
#include <linux/kthread.h>

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    #include <asm/atomic.h>
#endif

#define RCV_BUF_SIZE 128
#define SND_BUF_SIZE 128

#define ICC_TX_FIFO  ICC_GET_CORE_ID
#define ICC_RX_FIFO  ICC_GET_REMOTE_CORE_ID

#define ICC_CHECK_ERR_CODE(icc_function_to_call)                \
             return_code = icc_function_to_call;                \
             if (ICC_SUCCESS != return_code) return return_code


volatile unsigned int watch_CB_Rx[2];
volatile unsigned int watch_CB_Tx[2];
volatile unsigned int watch_CB_Ch[2];

volatile unsigned int watch_Ch_Rx_ok[2];
volatile unsigned int watch_Ch_Tx_ok[2];

volatile unsigned int watch_Ch_Rx_ko[2];
volatile unsigned int watch_Ch_Tx_ko[2];

volatile unsigned int watch_CB_Node;

unsigned char snd_buffer[SND_BUF_SIZE];
unsigned char rcv_buffer[RCV_BUF_SIZE];

volatile atomic_t thread_on = { 1 };

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    /* heartbeat flag which determine if the node is prepared for the heartbeat mechanism */
    volatile atomic_t heartbeat_on;
#endif /* ICC_CFG_HEARTBEAT_ENABLED */


#ifdef ICC_CFG_HEARTBEAT_ENABLED
int ICC_HeartBeat_kthread(void *data)
{
    int err;
    int i;

    unsigned int runID = 1;

    while (atomic_read(&thread_on)) {

       err = ICC_Heartbeat_Initialize(runID);
       printk("ICC_Heartbeat_Initialize return: %d\n", err);

       err = ICC_Heartbeat_Runnable();
       printk("ICC_Heartbeat_Runnable return  : %d\n", err);

       err = ICC_Heartbeat_Finalize();
       printk("ICC_Heartbeat_Finalize return  : %d\n", err);

       runID++;

    };

    printk("ICC_HeartBeat_kthread stopped\n");

    return 0;
}
#endif /* ICC_CFG_HEARTBEAT_ENABLED */


int ICC_Data_kthread(void *data)
{
    ICC_Err_t      icc_status;
    ICC_Timeout_t  timeout;
    ICC_Msg_Size_t rx_msg_size;

    const unsigned int channel_id = 1; /**< the data channel */

    if ( ICC_CROSS_VALUE_OF(ICC_Config0.Channels_Ptr)[channel_id].fifos_cfg[ICC_RX_FIFO].fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED ) {
        timeout = ICC_WAIT_FOREVER;
    } else {
        timeout = ICC_WAIT_ZERO;
    }

    while (atomic_read(&thread_on)) {

        /* RX */
        icc_status = ICC_Msg_Recv( channel_id, rcv_buffer, RCV_BUF_SIZE, &rx_msg_size, ICC_WAIT_FOREVER, ICC_RX_NORMAL );
        if( icc_status != ICC_SUCCESS )
        {
            /* printk("POP failed with err %d \n", icc_status ); */
            /* return; */
            watch_Ch_Rx_ko[channel_id]++;

        } else {
            watch_Ch_Rx_ok[channel_id]++;
            /*printk("POP: Receive message [%d] from [%u] via ch [%u]: %s \n", watch_Ch_Rx_ok[channel_id], ICC_GET_REMOTE_CORE_ID, channel_id, rcv_buffer );*/
        }

        /* TX */
        memcpy(snd_buffer,"Hello_AUTOSAR", 14 );

        icc_status = ICC_Msg_Send( channel_id, snd_buffer, 14, ICC_WAIT_FOREVER);
        if( icc_status != ICC_SUCCESS )
        {
            /* printk("SEND failed: err code:%d\n", icc_status); */
            /* return; */
            watch_Ch_Tx_ko[channel_id]++;
        } else {
            watch_Ch_Tx_ok[channel_id]++;
            /* printk("Push: Tx message [%d] to [%u] via ch [%u]: %s \n", watch_Ch_Tx_ok[channel_id], ICC_GET_REMOTE_CORE_ID, channel_id, snd_buffer ); */
        }

    }; /* end while(1) */

    printk("ICC_Data_kthread stopped\n");

    return 0;
}

void USER_ICC_Callback_Rx_CB_App( ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */ )
{

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( channel_id == 0 ){
            return;
        }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

    watch_CB_Rx[channel_id]++;

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

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( node_state == ICC_NODE_STATE_INIT )
        {
            atomic_set( &heartbeat_on, 1);
        }
    #endif /*ICC_CFG_HEARTBEAT_ENABLED*/
}


/* Start_ICC_Sample - starts the sample code */
int Start_ICC_Sample(void)
{
    int i;
    ICC_Err_t return_code;

    struct task_struct *task_data;

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    struct task_struct *task_hb;

    atomic_set( &heartbeat_on, 0 );
    #endif

    for( i = 0; i < 2; i++ )
    {
        watch_CB_Rx[i]=0;
        watch_CB_Tx[i]=0;
        watch_CB_Ch[i]=0;

        watch_Ch_Rx_ok[i]=0;
        watch_Ch_Tx_ok[i]=0;

        watch_Ch_Rx_ko[i]=0;
        watch_Ch_Tx_ko[i]=0;
    }

    watch_CB_Node=0;

    /* initialize ICC */
    if ((return_code = ICC_Initialize( &ICC_Config0 )) != ICC_SUCCESS) {

        printk("Start_ICC_Sample: ICC_Initialize failed with error code: %d\n", return_code);

        return return_code;
    }

    printk("ICC_Initialize ... done\n");

    /* open communication channels */
    for (i = 0; i < ICC_CROSS_VALUE_OF(ICC_Config0.Channels_Count); i++)
    {

        #ifdef ICC_CFG_HEARTBEAT_ENABLED
            if( i != ICC_CROSS_VALUE_OF(ICC_Config0.ICC_Heartbeat_Os_Config)->channel_id )
        #endif /* ICC_CFG_HEARTBEAT_ENABLED */

        ICC_CHECK_ERR_CODE(ICC_Open_Channel(i));

    }

    printk("Opening all channels ... done\n");

    atomic_set(&thread_on, 1);

    task_data = kthread_run( ICC_Data_kthread, NULL, "%s_kthread_%d", "ICC_Data", 0 );
    printk("ICC data kthread %s: started\n", task_data->comm );


    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        while( atomic_read( &heartbeat_on) == 0 );
        task_hb = kthread_run( ICC_HeartBeat_kthread, NULL, "%s_kthread_%d", "HB", 0 );
        printk("ICC HB kthread %s: started\n", task_hb->comm );
    #endif

    return ICC_SUCCESS;
}

/* Stop_ICC_Sample - releases the ICC resources */
int Stop_ICC_Sample(void)
{
    ICC_Err_t return_code;

    atomic_set(&thread_on, 0);

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        ICC_CHECK_ERR_CODE(ICC_Heartbeat_Finalize());
    #endif /*ICC_CFG_HEARTBEAT_ENABLED*/

    /* ICC clean up */
    ICC_CHECK_ERR_CODE(ICC_Finalize());

    return ICC_SUCCESS;
}
