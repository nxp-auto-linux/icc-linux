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

#include <linux/kthread.h>

#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/stddef.h>
#include <linux/time.h>

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    #include <asm/atomic.h>
#endif

#include "ICC_Api.h"
#include "ICC_Log.h"
#include "ICC_Args.h"

#ifdef ICC_LINUX2LINUX

#include "ICC_Config_Test.h"

/* Set this arg to true if you want to check the shared memory */
ICC_CMD_LINE_ARG(icc_mem_dump, bool, false, \
        "Print ICC shared data");

#endif

/* The following 2 args define what is displayed by the applications:
 * - statistics - a statistic with ICC_Data_kthread execution count and average time
 *                  every n seconds (enabled by default)
 * - verbose - a message for every data sent/received by ICC_Data_kthread
 *                  (disabled by default)
 */

/* Set this arg to true if you want to see data kthread statistics */
ICC_CMD_LINE_ARG(icc_exec_stats, bool, true, \
        "Print data thread statistics");

ICC_CMD_LINE_ARG(icc_verbose_comm_log, bool, false, \
        "Print verbose communication information");

/** statistics data and helpers */

#define ICC_PRINT_STATS_DELAY_SEC 2

struct icc_func_stat_t {
    uint32_t func_calls;
    struct timespec func_time_ns;
};

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

/** sample data */

#define RCV_BUF_SIZE 128
#define SND_BUF_SIZE 128

#define ICC_TX_FIFO  ICC_GET_CORE_ID
#define ICC_RX_FIFO  ICC_GET_REMOTE_CORE_ID

#define ICC_CHECK_ERR_CODE(icc_function_to_call)                \
             return_code = icc_function_to_call;                \
             if (ICC_SUCCESS != return_code) return return_code


#define ICC_NUM_CORES 2

struct icc_watch_cb_flags_t {
    uint32_t watch_CB_Rx[ICC_NUM_CORES];
    uint32_t watch_CB_Tx[ICC_NUM_CORES];
    uint32_t watch_CB_Ch[ICC_NUM_CORES];

    uint32_t watch_Ch_Rx_ok[ICC_NUM_CORES];
    uint32_t watch_Ch_Tx_ok[ICC_NUM_CORES];

    uint32_t watch_Ch_Rx_ko[ICC_NUM_CORES];
    uint32_t watch_Ch_Tx_ko[ICC_NUM_CORES];

    uint32_t watch_CB_Node;
};

/** we must define this global because it is used by the callbacks,
 *  which cannot receive it as additional argument.
 */
static struct icc_watch_cb_flags_t cbs = {{0},};

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
       ICC_DEBUG("ICC_Heartbeat_Initialize return: %d", err);

       err = ICC_Heartbeat_Runnable();
       ICC_DEBUG("ICC_Heartbeat_Runnable return  : %d\n", err);

       err = ICC_Heartbeat_Finalize();
       ICC_DEBUG("ICC_Heartbeat_Finalize return  : %d\n", err);

       runID++;

    };

    ICC_INFO("ICC_HeartBeat_kthread stopped");

    return 0;
}
#endif /* ICC_CFG_HEARTBEAT_ENABLED */

#ifdef ICC_BUILD_FOR_M4
#define SEND_TEXT "Hello_RootComplex"
#else
    #ifdef ICC_LINUX2LINUX
    #define SEND_TEXT "Hello_EndPoint"
    #else
    #define SEND_TEXT "Hello_RTOS"
    #endif
#endif
#define SEND_TEXT_SIZE sizeof(SEND_TEXT) + 1

inline ICC_Err_t ICC_Data_Send(struct icc_watch_cb_flags_t *cbs,
                               const ICC_Channel_t channel_id,
                               unsigned char *snd_buffer,
                               ICC_Timeout_t timeout)
{
    ICC_Err_t      icc_status = ICC_SUCCESS;

    if (!cbs || !snd_buffer) {
        ICC_ERR("Invalid arguments");
        icc_status = ICC_ERR_PARAM_INVAL;
        return icc_status;
    }

    /* TX */
    memcpy(snd_buffer, SEND_TEXT, SEND_TEXT_SIZE);

    icc_status = ICC_Msg_Send(channel_id, snd_buffer, SEND_TEXT_SIZE, timeout);
    if (icc_status != ICC_SUCCESS)
    {
        ICC_ERR("Send failed with err %d", icc_status);
        /* return; */
        cbs->watch_Ch_Tx_ko[channel_id]++;
    } else {
        cbs->watch_Ch_Tx_ok[channel_id]++;
        if (icc_verbose_comm_log) {
            ICC_INFO("Send: Tx message [%u] to [%u] via ch [%u]: %s",
                    cbs->watch_Ch_Tx_ok[channel_id], ICC_GET_REMOTE_CORE_ID,
                    channel_id, snd_buffer);
        }
    }

    return icc_status;
}

inline ICC_Err_t ICC_Data_Receive(struct icc_watch_cb_flags_t *cbs,
                                  const ICC_Channel_t channel_id,
                                  unsigned char *rcv_buffer,
                                  ICC_Timeout_t timeout)
{
    ICC_Err_t      icc_status = ICC_SUCCESS;
    ICC_Msg_Size_t rx_msg_size;

    if (!cbs || !rcv_buffer) {
        ICC_ERR("Invalid arguments");
        icc_status = ICC_ERR_PARAM_INVAL;
        return icc_status;
    }

    /* RX */
    icc_status = ICC_Msg_Recv(channel_id, rcv_buffer, RCV_BUF_SIZE,
            &rx_msg_size, timeout, ICC_RX_NORMAL);
    if (icc_status != ICC_SUCCESS)
    {
        ICC_ERR("Recv failed with err %d", icc_status);
        /* return; */
        cbs->watch_Ch_Rx_ko[channel_id]++;

    } else {
        cbs->watch_Ch_Rx_ok[channel_id]++;
        if (icc_verbose_comm_log) {
            ICC_INFO("Recv: Rx message [%u] from [%u] via ch [%u]: %s",
                    cbs->watch_Ch_Rx_ok[channel_id], ICC_GET_REMOTE_CORE_ID,
                    channel_id, rcv_buffer);
        }
    }

    return icc_status;
}

#ifdef ICC_USE_POLLING
extern void ICC_Timer_Update_ns(uint64_t new_time_ns);
#endif

int ICC_Data_kthread(void *data)
{
    ICC_Err_t      icc_status = ICC_SUCCESS;
    ICC_Timeout_t  timeout = ICC_WAIT_ZERO;

    struct icc_watch_cb_flags_t *cbs = (struct icc_watch_cb_flags_t *)data;
    unsigned char snd_buffer[SND_BUF_SIZE] = {0};
    unsigned char rcv_buffer[RCV_BUF_SIZE] = {0};
    const ICC_Channel_t channel_id = IccChannel_1; /**< the data channel */

    struct icc_func_stat_t icc_data_thread_stat = {0};
    uint64_t icc_last_func_sec;

    if (!data)
        icc_status = ICC_ERR_PARAM_INVAL;

    /* if we have timeout enabled, force communication to wait forever */
    if ( ICC_CROSS_VALUE_OF(ICC_Config0.Channels_Ptr)[channel_id].fifos_cfg[ICC_RX_FIFO].fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED ) {
        timeout = ICC_WAIT_FOREVER;
    }

    while ((icc_status == ICC_SUCCESS) && atomic_read(&thread_on)) {
        struct timespec ts1 = {0};
        getnstimeofday(&ts1);

#ifdef ICC_BUILD_FOR_M4

        icc_status = ICC_Data_Send(cbs, channel_id, snd_buffer, timeout);

        icc_status = ICC_Data_Receive(cbs, channel_id, rcv_buffer, timeout);

#else

        icc_status = ICC_Data_Receive(cbs, channel_id, rcv_buffer, timeout);

        icc_status = ICC_Data_Send(cbs, channel_id, snd_buffer, timeout);

#endif

        if (icc_status == ICC_SUCCESS) {
            struct timespec ts2 = {0};
            uint64_t exec_time_ns;
            getnstimeofday(&ts2);

            if (icc_exec_stats) {
                icc_data_thread_stat.func_calls++;
                /* in the following assignments we don't care for nanoseconds overflow */
                icc_data_thread_stat.func_time_ns.tv_sec += ts2.tv_sec;
                icc_data_thread_stat.func_time_ns.tv_sec -= ts1.tv_sec;
                icc_data_thread_stat.func_time_ns.tv_nsec += ts2.tv_nsec;
                icc_data_thread_stat.func_time_ns.tv_nsec -= ts1.tv_nsec;
                exec_time_ns = get_ns(&ts2, NULL);
                if ((exec_time_ns / NSEC_PER_SEC - icc_last_func_sec) > ICC_PRINT_STATS_DELAY_SEC) {
                    ICC_INFO("exec %d times; %llu ns in average",
                            icc_data_thread_stat.func_calls,
                            get_ns(&icc_data_thread_stat.func_time_ns, NULL) / icc_data_thread_stat.func_calls);
                    icc_last_func_sec = exec_time_ns / NSEC_PER_SEC;
                }
            }

#ifdef ICC_USE_POLLING
            exec_time_ns = get_ns(&ts2, &ts1);
            ICC_Timer_Update_ns(exec_time_ns);
#endif
        }
    }; /* end while(1) */

    ICC_INFO("ICC_Data_kthread stopped");

    return icc_status;
}

/*-----------------------------------------------------------*/

/**
 * Define callbacks names here and keep the bodies in one place, rather than
 *  duplicate the entire function definitions.
 */
#ifdef ICC_BUILD_FOR_M4
#define ICC_CB_RX USER_ICC_Callback_Rx_CB_M4
#define ICC_CB_TX USER_ICC_Callback_Tx_CB_M4
#define ICC_CB_CHAN_STATE USER_ICC_Callback_Channel_State_Update_CB_M4
#define ICC_CB_NODE_STATE USER_ICC_Node_State_Update_CB_M4
#else
#define ICC_CB_RX USER_ICC_Callback_Rx_CB_App
#define ICC_CB_TX USER_ICC_Callback_Tx_CB_App
#define ICC_CB_CHAN_STATE USER_ICC_Callback_Channel_State_Update_CB_App
#define ICC_CB_NODE_STATE USER_ICC_Node_State_Update_CB_App
#endif


/*
 * ICC callback for RX free buffer.
 */
void ICC_CB_RX( ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */ )
{

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( channel_id == 0 ){
            return;
        }
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */

    cbs.watch_CB_Rx[channel_id]++;
}


/*
 * ICC callback for TX free buffer; no blocking calls allowed.
 */
void ICC_CB_TX( ICC_IN const ICC_Channel_t channel_id /**< The id of the channel that just "received" free space. */ )
{
    cbs.watch_CB_Tx[channel_id]++;
}

/*
 * ICC callback for channel transitioning to a new state.
 */
void ICC_CB_CHAN_STATE( ICC_IN const ICC_Channel_t       channel_id, /** The id of the channel that transitioned to a new state. */
                        ICC_IN const ICC_Channel_State_t channel_state /** The new state that the channel transitioned to. */ )
{
    cbs.watch_CB_Ch[channel_id]++;
}

/*
 * ICC callback for node transitioning to a new state.
 */
void ICC_CB_NODE_STATE( ICC_IN const ICC_Node_t       node_id, /** The id of the node that transitioned to a new state. */
                        ICC_IN const ICC_Node_State_t node_state /** The new state the node transitioned to. */ )
{
    cbs.watch_CB_Node++;

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        if( node_state == ICC_NODE_STATE_INIT )
        {
            atomic_set( &heartbeat_on, 1);
        }
    #endif /*ICC_CFG_HEARTBEAT_ENABLED*/
}


#ifdef ICC_LINUX2LINUX

#ifndef ICC_BUILD_FOR_M4
extern
ICC_Config_t * ICC_Config_Ptr_M4;          /**< pointer to M4 current configuration */
#endif

int ICC_Dump_Config(ICC_Config_t *config)
{
    int dump_count = 0;

    if (config) {
        ICC_DUMP_PTR(dump_count, ICC_Config_t, config);
    }

    return dump_count;
}

#endif

/* Start_ICC_Sample - starts the sample code */
int Start_ICC_Sample(void)
{
    int i;
    ICC_Err_t return_code;
    ICC_Config_t *config = (ICC_Config_t *)&ICC_Config0;

    struct task_struct *task_data;

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    struct task_struct *task_hb;

    atomic_set( &heartbeat_on, 0 );
    #endif

#if (defined(ICC_BUILD_FOR_M4) && defined(ICC_LINUX2LINUX))
    /* Re-locate the objects in ICC_Config.c, at address IRAM_BASE_ADDR + 4 (first uint32 is used for polling/synchronization) */
    config = ICC_Relocate_Config(config, NULL);
    if (!config) {
        ICC_ERR("Failed to relocate config");
    }
    /* TODO: use incoming icc_bar.bar_size to validate that the RC can access all shared mem */
#endif

    /* initialize ICC */
    if ((return_code = ICC_Initialize( config )) != ICC_SUCCESS) {

        ICC_ERR("ICC_Initialize failed with error code: %d", return_code);

        return return_code;
    }

    ICC_INFO("ICC Initialize ... done");

#ifdef ICC_LINUX2LINUX
    if (icc_mem_dump) {
#ifdef ICC_BUILD_FOR_M4
        ICC_Dump_Config(config);
#else
        ICC_Dump_Config(ICC_Config_Ptr_M4);
#endif
    }
#endif

#ifdef ICC_LINUX2LINUX

#ifdef ICC_BUILD_FOR_M4
    ICC_INFO("Waiting for peer ...");
    if ((return_code = ICC_Wait_For_Peer() != ICC_SUCCESS)) {
        ICC_ERR("Wait for peer failed");
        return return_code;
    }
#else
    ICC_INFO("Notifying peer ...");
    ICC_Notify_Peer_Alive();
#endif

    ICC_INFO("Peers connected");

#endif

    /* open communication channels */
    for (i = 0; i < ICC_CROSS_VALUE_OF(config->Channels_Count); i++)
    {

        #ifdef ICC_CFG_HEARTBEAT_ENABLED
            if( i != ICC_CROSS_VALUE_OF(config->ICC_Heartbeat_Os_Config)->channel_id )
        #endif /* ICC_CFG_HEARTBEAT_ENABLED */

        ICC_CHECK_ERR_CODE(ICC_Open_Channel(i));

    }

    ICC_INFO("Opening all channels ... done");

    atomic_set(&thread_on, 1);

    task_data = kthread_run( ICC_Data_kthread, &cbs, "%s_kthread_%d", "ICC_Data", 0 );
    ICC_INFO("ICC data kthread %s: started", task_data->comm );


    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        while( atomic_read( &heartbeat_on) == 0 );
        task_hb = kthread_run( ICC_HeartBeat_kthread, NULL, "%s_kthread_%d", "HB", 0 );
        ICC_INFO("ICC HB kthread %s: started", task_hb->comm );
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

    ICC_INFO("ICC Finalize ... done");

    return ICC_SUCCESS;
}
