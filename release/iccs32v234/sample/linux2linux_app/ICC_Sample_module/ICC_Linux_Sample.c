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

/* Un-comment this if you want to debug the virtual memory */
//#define DUMP_SHARED_MEM

/* The following 2 macros define what is displayed by the applications:
 * - statistics - a statistic with ICC_Data_kthread execution count and average time - every n seconds
 * - verbose - a message for every data sent/received by ICC_Data_kthread
 */
#define ICC_STATISTICS
//#define VERBOSE_LOG

#define RCV_BUF_SIZE 128
#define SND_BUF_SIZE 128

#define ICC_TX_FIFO  ICC_GET_CORE_ID
#define ICC_RX_FIFO  ICC_GET_REMOTE_CORE_ID

#define ICC_CHECK_ERR_CODE(icc_function_to_call)                \
             return_code = icc_function_to_call;                \
             if (ICC_SUCCESS != return_code) return return_code

/* see all traffic in kernel log */
#define ICC_SAMPLE_LOG(...) printk(KERN_ALERT __VA_ARGS__)

#define ICC_NUM_CORES 2

volatile unsigned int watch_CB_Rx[ICC_NUM_CORES];
volatile unsigned int watch_CB_Tx[ICC_NUM_CORES];
volatile unsigned int watch_CB_Ch[ICC_NUM_CORES];

volatile unsigned int watch_Ch_Rx_ok[ICC_NUM_CORES];
volatile unsigned int watch_Ch_Tx_ok[ICC_NUM_CORES];

volatile unsigned int watch_Ch_Rx_ko[ICC_NUM_CORES];
volatile unsigned int watch_Ch_Tx_ko[ICC_NUM_CORES];

volatile unsigned int watch_CB_Node;

unsigned char snd_buffer[SND_BUF_SIZE];
unsigned char rcv_buffer[RCV_BUF_SIZE];

volatile atomic_t thread_on = { 1 };

#ifdef ICC_STATISTICS

#define ICC_PRINT_STATS_DELAY_SEC 2

#include <linux/time.h>

struct icc_func_stat_t {
    uint32_t func_calls;
    struct timespec func_time_ns;
};

static struct icc_func_stat_t icc_data_thread_stat = {0};
static uint64_t icc_last_func_sec;

#endif

static inline uint64_t get_ns(struct timespec *ts1, struct timespec *ts2)
{
    uint64_t val = 0;
    if (ts1) {
        val = ts1->tv_sec * NSEC_PER_SEC + ts1->tv_nsec;
        if (ts2) {
            val -= (ts2->tv_sec * NSEC_PER_SEC + ts2->tv_nsec);
        }
    }

    return val;
}

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
       ICC_SAMPLE_LOG("ICC_Heartbeat_Initialize return: %d\n", err);

       err = ICC_Heartbeat_Runnable();
       ICC_SAMPLE_LOG("ICC_Heartbeat_Runnable return  : %d\n", err);

       err = ICC_Heartbeat_Finalize();
       ICC_SAMPLE_LOG("ICC_Heartbeat_Finalize return  : %d\n", err);

       runID++;

    };

    ICC_SAMPLE_LOG("ICC_HeartBeat_kthread stopped\n");

    return 0;
}
#endif /* ICC_CFG_HEARTBEAT_ENABLED */

#ifdef ICC_BUILD_FOR_M4
#define SEND_TEXT "Hello_RootComplex"
#else
#define SEND_TEXT "Hello_EndPoint"
#endif
#define SEND_TEXT_SIZE sizeof(SEND_TEXT) + 1

const unsigned int channel_id = 1; /**< the data channel */

inline ICC_Err_t ICC_Data_Send(ICC_Timeout_t timeout)
{
    ICC_Err_t      icc_status = ICC_SUCCESS;

    /* TX */
    memcpy( snd_buffer, SEND_TEXT, SEND_TEXT_SIZE );

    icc_status = ICC_Msg_Send( channel_id, snd_buffer, SEND_TEXT_SIZE, timeout);
    if( icc_status != ICC_SUCCESS )
    {
        ICC_SAMPLE_LOG("SEND failed: err code:%d\n", icc_status);
        /* return; */
        watch_Ch_Tx_ko[channel_id]++;
    } else {
        watch_Ch_Tx_ok[channel_id]++;
#ifdef VERBOSE_LOG
        ICC_SAMPLE_LOG("Push: Tx message [%d] to [%u] via ch [%u]: %s \n",
                watch_Ch_Tx_ok[channel_id], ICC_GET_REMOTE_CORE_ID,
                channel_id, snd_buffer );
#endif
    }

    return icc_status;
}

inline ICC_Err_t ICC_Data_Receive(ICC_Timeout_t timeout)
{
    ICC_Err_t      icc_status = ICC_SUCCESS;
    ICC_Msg_Size_t rx_msg_size;

    /* RX */
    icc_status = ICC_Msg_Recv( channel_id, rcv_buffer, RCV_BUF_SIZE,
            &rx_msg_size, timeout, ICC_RX_NORMAL );
    if( icc_status != ICC_SUCCESS )
    {
        ICC_SAMPLE_LOG("POP failed with err %d \n", icc_status );
        /* return; */
        watch_Ch_Rx_ko[channel_id]++;

    } else {
        watch_Ch_Rx_ok[channel_id]++;
#ifdef VERBOSE_LOG
        ICC_SAMPLE_LOG("POP: Receive message [%d] from [%u] via ch [%u]: %s \n",
                watch_Ch_Rx_ok[channel_id], ICC_GET_REMOTE_CORE_ID,
                channel_id, rcv_buffer );
#endif
    }

    return icc_status;
}

extern void ICC_Timer_Update_ns(uint64_t new_time_ns);

int ICC_Data_kthread(void *data)
{
    ICC_Err_t      icc_status = ICC_SUCCESS;
    ICC_Timeout_t  timeout = ICC_WAIT_FOREVER;

    /* Ignore the timeout for now, we'll re-enable it in the future:
    if ( ICC_CROSS_VALUE_OF(ICC_Default_Config_Ptr->Channels_Ptr)[channel_id].fifos_cfg[ICC_RX_FIFO].fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED ) {
        timeout = ICC_WAIT_FOREVER;
    } else {
        timeout = ICC_WAIT_ZERO;
    }*/

    while ((icc_status == ICC_SUCCESS) && atomic_read(&thread_on)) {
        struct timespec ts1 = {0};
        getnstimeofday(&ts1);

#ifdef ICC_BUILD_FOR_M4

        icc_status = ICC_Data_Send(timeout);

        icc_status = ICC_Data_Receive(timeout);

#else

        icc_status = ICC_Data_Receive(timeout);

        icc_status = ICC_Data_Send(timeout);
#endif

        if (icc_status == ICC_SUCCESS) {
            struct timespec ts2 = {0};
            uint64_t exec_time_ns;
            getnstimeofday(&ts2);

#ifdef ICC_STATISTICS
            icc_data_thread_stat.func_calls++;
            /* in the following assignments we don't care for nanoseconds overflow */
            icc_data_thread_stat.func_time_ns.tv_sec += ts2.tv_sec;
            icc_data_thread_stat.func_time_ns.tv_sec -= ts1.tv_sec;
            icc_data_thread_stat.func_time_ns.tv_nsec += ts2.tv_nsec;
            icc_data_thread_stat.func_time_ns.tv_nsec -= ts1.tv_nsec;
            exec_time_ns = get_ns(&ts2, NULL);
            if ((exec_time_ns / NSEC_PER_SEC - icc_last_func_sec) > ICC_PRINT_STATS_DELAY_SEC) {
                ICC_SAMPLE_LOG("icc data kthread: exec %d times; %llu ns in average\n",
                        icc_data_thread_stat.func_calls,
                        get_ns(&icc_data_thread_stat.func_time_ns, NULL) / icc_data_thread_stat.func_calls);
                icc_last_func_sec = exec_time_ns / NSEC_PER_SEC;
            }
#endif
            exec_time_ns = get_ns(&ts2, &ts1);
            ICC_Timer_Update_ns(exec_time_ns);
        }
    }; /* end while(1) */

    ICC_SAMPLE_LOG("ICC_Data_kthread stopped\n");

    return icc_status;
}

/*-----------------------------------------------------------*/

#ifdef ICC_BUILD_FOR_M4

/*
 * ICC callback for RX free buffer.
 */
void USER_ICC_Callback_Rx_CB_M4( ICC_IN const ICC_Channel_t channel_id ) /** The id of the channel that received a message. */
{

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( channel_id == 0 ){
            return;
        }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

    watch_CB_Rx[channel_id]++;
}

/*
 * ICC callback for TX free buffer; no blocking calls allowed.
 */
void USER_ICC_Callback_Tx_CB_M4( ICC_IN const ICC_Channel_t channel_id ) /**< The id of the channel that just "received" free space. */
{
    watch_CB_Tx[channel_id]++;
}

/*
 * ICC callback for channel transitioning to a new state.
 */
void USER_ICC_Callback_Channel_State_Update_CB_M4( ICC_IN const ICC_Channel_t       channel_id, /** The id of the channel that transitioned to a new state. */
                                                   ICC_IN const ICC_Channel_State_t channel_state ) /** The new state that the channel transitioned to. */
{
    watch_CB_Ch[channel_id]++;
}

/*
 * ICC callback for node transitioning to a new state.
 */
void USER_ICC_Node_State_Update_CB_M4( ICC_IN const ICC_Node_t       node_id, /** The id of the node that transitioned to a new state. */
                                       ICC_IN const ICC_Node_State_t node_state ) /** The new state the node transitioned to. */
{
    watch_CB_Node++;

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( node_state == ICC_NODE_STATE_INIT )
        {
            atomic_set( &heartbeat_on, 1);
        }
    #endif /*ICC_CFG_HEARTBEAT_ENABLED*/
}

#else

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

#endif  /* ICC_BUILD_FOR_M4 */

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

    for( i = 0; i < ICC_NUM_CORES; i++ )
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

#ifdef ICC_BUILD_FOR_M4
    /* Re-locate the objects in ICC_Config.c, at address IRAM_BASE_ADDR + 4 (first uint32 is used for polling/synchronization) */
    ICC_Relocate_Config();
    /* TODO: use incoming icc_bar.bar_size to validate that the RC can access all shared mem */
#endif

    /* initialize ICC */
    if ((return_code = ICC_Initialize( ICC_Default_Config_Ptr )) != ICC_SUCCESS) {

        ICC_SAMPLE_LOG("Start_ICC_Sample: ICC_Initialize failed with error code: %d\n", return_code);

        return return_code;
    }

    ICC_SAMPLE_LOG("ICC_Initialize ... done\n");

#ifdef DUMP_SHARED_MEM
    ICC_Dump_Shared_Config();
#endif

#ifdef ICC_USE_POLLING

#ifdef ICC_BUILD_FOR_M4
    ICC_SAMPLE_LOG("Waiting for peer ...\n");
    if ((return_code = ICC_Wait_For_Peer() != ICC_SUCCESS)) {
        ICC_SAMPLE_LOG("failed\n");
        return return_code;
    }
#else
    ICC_SAMPLE_LOG("Notifying peer ...\n");
    ICC_Notify_Remote_Alive();
#endif

    ICC_SAMPLE_LOG("Peers connected\n");

#endif

    /* open communication channels */
    for (i = 0; i < ICC_CROSS_VALUE_OF(ICC_Default_Config_Ptr->Channels_Count); i++)
    {

        #ifdef ICC_CFG_HEARTBEAT_ENABLED
            if( i != ICC_CROSS_VALUE_OF(ICC_Default_Config_Ptr->ICC_Heartbeat_Os_Config)->channel_id )
        #endif /* ICC_CFG_HEARTBEAT_ENABLED */

        ICC_CHECK_ERR_CODE(ICC_Open_Channel(i));

    }

    ICC_SAMPLE_LOG("Opening all channels ... done\n");

    atomic_set(&thread_on, 1);

    task_data = kthread_run( ICC_Data_kthread, NULL, "%s_kthread_%d", "ICC_Data", 0 );
    ICC_SAMPLE_LOG("ICC data kthread %s: started\n", task_data->comm );


    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        while( atomic_read( &heartbeat_on) == 0 );
        task_hb = kthread_run( ICC_HeartBeat_kthread, NULL, "%s_kthread_%d", "HB", 0 );
        ICC_SAMPLE_LOG("ICC HB kthread %s: started\n", task_hb->comm );
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
