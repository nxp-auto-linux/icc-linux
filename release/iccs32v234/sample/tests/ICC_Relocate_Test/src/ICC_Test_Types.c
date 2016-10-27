/**
 * ICC_Test_Types.c - main source file for the test application.
 *
 * This application performs a parity check between the old dirty implementation
 * (with zero paddings) and the new one based on 64 bit unions.
 *
 * The result will be a message ending in PASSED or FAILED.
 *
 *  SW Version           : 0.8.0
 *  Build Version        : S32V234_ICC_0.8.0
 *
 *  Copyright (c) 2016 NXP Semiconductor
 *  All Rights Reserved.
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "inttypes.h"

#define ICC_BUILD_FOR_M4
//#define ICC_FSL_AUTOSAR_OS
#define ICC_MIX_32_64_SUPPORT
#define ICC_CFG_HEARTBEAT_ENABLED
#define OSGCCARM
#define ICC_GCC_ARM

#define ICC_LINUX2LINUX

#define ICC_UTEST

#define ERR_NONE 0
#define ERR_FAILED 1

#ifndef __GNUC__
#define __attribute__(t)
#define aligned(t)
#endif

#include "ICC_Os_Types.h"
#include "ICC_Config.h"
#include "ICC_Types.h"
#include "ICC_Private.h"

#ifdef ICC_CFG_HEARTBEAT_ENABLED
ICC_Heartbeat_Os_Config_t
ICC_Heartbeat_Os_Config0 = {
	ICC_CFG0_HEARTBEAT_CHANNEL_ID,
	1000,
	8000,
#ifdef ICC_BUILD_FOR_M4
#ifdef ICC_FSL_AUTOSAR_OS
    OsEvent_HB_RATE,
    OsAlarm_HB_RATE,
    WORKER3,
#endif /* ICC_FSL_AUTOSAR_OS */
#endif /* ICC_BUILD_FOR_M4 */
};

#endif  /* ICC_CFG_HEARTBEAT_ENABLED */

#define ICC_CFG0_CH_0_FIFO_0_SIZE        (1024UL)
#define ICC_CFG0_CH_0_FIFO_1_SIZE        (1024UL)

#define ICC_CFG0_CH_1_FIFO_0_SIZE        (2048UL)
#define ICC_CFG0_CH_1_FIFO_1_SIZE        (2048UL)

unsigned char cfg_0_ch_0_fifo_0_buffer[ICC_CFG0_CH_0_FIFO_0_SIZE] = {1,2,3,4};
unsigned char cfg_0_ch_0_fifo_1_buffer[ICC_CFG0_CH_0_FIFO_1_SIZE] = {5,6,7,8};

unsigned char cfg_0_ch_1_fifo_0_buffer[ICC_CFG0_CH_1_FIFO_0_SIZE] = {9,10,11,12};
unsigned char cfg_0_ch_1_fifo_1_buffer[ICC_CFG0_CH_1_FIFO_1_SIZE] = {13,14,15,16};

ICC_Fifo_Config_t ICC_Fifo_Cfg0 = {
#ifdef ICC_BUILD_FOR_M4
    ICC_CROSS_INIT(cfg_0_ch_0_fifo_0_buffer), /**< the statically allocated buffer */
#else
    ICC_CROSS_INIT(NULL_PTR), /**< updated at runtime on APP side */
#endif

    2,    /**< prio */
    ICC_CFG0_CH_0_FIFO_0_SIZE, /**< size */
    64,   /**< max msg size */
    4,     /**< alignment */
	ICC_FIFO_FLAG_TIMEOUT_ENABLED /**< timeout enabled */
};

ICC_Channel_Config_t
ICC_Cfg0_ChannelsConfig[ ICC_CFG_NO_CHANNELS_CONF0 ] = {

    /* channel 0 */
    {
        /* fifo 0 */
        { {
#ifdef ICC_BUILD_FOR_M4
                ICC_CROSS_INIT(cfg_0_ch_0_fifo_0_buffer), /**< the statically allocated buffer */
#else
                ICC_CROSS_INIT(NULL_PTR), /**< updated at runtime on APP side */
#endif

                0,    /**< prio */
                ICC_CFG0_CH_0_FIFO_0_SIZE, /**< size */
                64,   /**< max msg size */
                4,     /**< alignment */
                ICC_FIFO_FLAG_TIMEOUT_ENABLED /**< timeout enabled */
           },

         /* fifo 1 */
           {
#ifdef ICC_BUILD_FOR_M4
                ICC_CROSS_INIT(cfg_0_ch_0_fifo_1_buffer), /**< the statically allocated buffer */
#else
                ICC_CROSS_INIT(NULL_PTR), /**< updated at runtime on APP side */
#endif

                0,    /**< prio */
                ICC_CFG0_CH_0_FIFO_1_SIZE, /**< size */
                64,   /**< max msg size */
                8,     /**< alignment */
                ICC_FIFO_FLAG_TIMEOUT_ENABLED /**< timeout enabled */
        } },

    /* channel state update call back */
#ifdef ICC_BUILD_FOR_M4
        ICC_CROSS_INIT(USER_ICC_Callback_Channel_State_Update_CB_M4),
#else
        ICC_CROSS_INIT(USER_ICC_Callback_Channel_State_Update_CB_App),
#endif

    /* channel Rx call back */
#ifdef ICC_BUILD_FOR_M4
        ICC_CROSS_INIT(USER_ICC_Callback_Rx_CB_M4),
#else
        ICC_CROSS_INIT(USER_ICC_Callback_Rx_CB_App),
#endif

    /* channel Tx call back */
#ifdef ICC_BUILD_FOR_M4
        ICC_CROSS_INIT(USER_ICC_Callback_Tx_CB_M4),
#else
        ICC_CROSS_INIT(USER_ICC_Callback_Tx_CB_App)
#endif
    },
    /* channel 1 */
    {
        /* fifo 0 */
        { {
#ifdef ICC_BUILD_FOR_M4
                ICC_CROSS_INIT(cfg_0_ch_1_fifo_0_buffer), /**< the statically allocated buffer */
#else
                ICC_CROSS_INIT(NULL_PTR), /**< updated at runtime on APP side */
#endif

                1,    /**< prio */
                ICC_CFG0_CH_1_FIFO_0_SIZE, /**< size */
                64,   /**< max msg size */
                4,     /**< alignment */
                ICC_FIFO_FLAG_TIMEOUT_ENABLED /**< timeout enabled */
            },

            /* fifo 1 */
            {
#ifdef ICC_BUILD_FOR_M4
                ICC_CROSS_INIT(cfg_0_ch_1_fifo_1_buffer), /**< the statically allocated buffer */
#else
                ICC_CROSS_INIT(NULL_PTR), /**< updated at runtime on APP side */
#endif

                1,    /**< prio */
                ICC_CFG0_CH_1_FIFO_1_SIZE, /**< size */
                64,   /**< max msg size */
                8,     /**< alignment */
                ICC_FIFO_FLAG_TIMEOUT_ENABLED /**< timeout enabled */
            } },

            /* channel state update call back */
#ifdef ICC_BUILD_FOR_M4
        ICC_CROSS_INIT(USER_ICC_Callback_Channel_State_Update_CB_M4),
#else
        ICC_CROSS_INIT(USER_ICC_Callback_Channel_State_Update_CB_App),
#endif

        /* channel Rx call back */
#ifdef ICC_BUILD_FOR_M4
        ICC_CROSS_INIT(USER_ICC_Callback_Rx_CB_M4),
#else
        ICC_CROSS_INIT(USER_ICC_Callback_Rx_CB_App),
#endif

        /* channel Tx call back */
#ifdef ICC_BUILD_FOR_M4
        ICC_CROSS_INIT(USER_ICC_Callback_Tx_CB_M4),
#else
        ICC_CROSS_INIT(USER_ICC_Callback_Tx_CB_App)
#endif
    }
};


struct ICC_Runtime_Shared_t {
    unsigned int          ICC_Initialized_Shared[2]; /**< shows if ICC is initialized */
    ICC_Fifo_Os_Ram_t      ICC_Fifo_Os_Ram_M4[ICC_CFG_MAX_CHANNELS][2 /* tx/rx */];
    ICC_Channel_Ram_t     ICC_Channels_Ram_Shared[ ICC_CFG_MAX_CHANNELS ]; /**< runtime structure for each channel */
    ICC_Fifo_Ram_t        ICC_Fifo_Ram_Shared[ ICC_CFG_MAX_CHANNELS ][2];  /**< fifos ordered priority wise for each node */
    ICC_Signal_Fifo_Ram_t  ICC_Node_Sig_Fifo_Ram_Shared[ 2 ];               /**< signal fifo for each node */
};

struct ICC_Runtime_Shared_t ICC_Runtime_Shared = {
        { ICC_NODE_STATE_UNINIT, ICC_NODE_STATE_UNINIT },
        { 0 }, { 0 }, { 0 }, { 0 }
};

ICC_Config_t ICC_Config0 = {
    ICC_CONFIG_MAGIC,
    NULL,

    ICC_CROSS_INIT(ICC_CFG_NO_CHANNELS_CONF0),           /**< number of configured ICC channels in this configuration */
    ICC_CROSS_INIT(ICC_Cfg0_ChannelsConfig),                   /**< ICC channels */

#ifdef ICC_FSL_AUTOSAR_OS
    ICC_CROSS_INIT(&ICC_Fifo_Os_Config0),              /**< pointer to Fifo Os configuration */
#else
    ICC_CROSS_INIT(NULL_PTR),                        /**< currently not used for APP core */
#endif

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    ICC_CROSS_INIT(&ICC_Heartbeat_Os_Config0),  /** < pointer to Heartbeat OS configuration */
#endif

#ifdef ICC_BUILD_FOR_M4
    ICC_CROSS_INIT(USER_ICC_Node_State_Update_CB_M4), /**< Node state update call-back M4 */
#else
    ICC_CROSS_INIT(USER_ICC_Node_State_Update_CB_App), /**< Node state update call-back APP */
#endif

#ifdef ICC_BUILD_FOR_M4
    ICC_CROSS_INIT(&(ICC_Runtime_Shared.ICC_Initialized_Shared)),          /**<  pointer to shared variable */
#else
    ICC_CROSS_INIT(NULL_PTR),
#endif

#ifdef ICC_BUILD_FOR_M4
    ICC_CROSS_INIT(ICC_Runtime_Shared.ICC_Channels_Ram_Shared),         /**<  pointer to shared variable */
#else
    ICC_CROSS_INIT(NULL_PTR),
#endif

#ifdef ICC_BUILD_FOR_M4
    ICC_CROSS_INIT(&ICC_Runtime_Shared.ICC_Fifo_Ram_Shared),            /**<  pointer to shared variable */
#else
    ICC_CROSS_INIT(NULL_PTR),
#endif

#ifdef ICC_BUILD_FOR_M4
    ICC_CROSS_INIT(&ICC_Runtime_Shared.ICC_Node_Sig_Fifo_Ram_Shared),       /**<  pointer to shared variable */
#else
    ICC_CROSS_INIT(NULL_PTR),
#endif

};

void USER_ICC_Callback_Rx_CB_M4(
    ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */
    )
{
}

/*
* ICC callback for tx free buffer,
* users must not use blocking calls in this callback
* should use ICC_Msg_Send_i
*/
void USER_ICC_Callback_Tx_CB_M4(
    ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that just "received" free space */
    )
{
}

void USER_ICC_Callback_Channel_State_Update_CB_M4(
    ICC_IN const ICC_Channel_t          channel_id,   /**< the id of the channel that transitioned to a new state */
    ICC_IN const ICC_Channel_State_t    channel_state /**< the new state that the channel transitioned to */
    )
{
}

void USER_ICC_Node_State_Update_CB_M4(
    ICC_IN const ICC_Node_t        node_id,    /**< the id of the node that transitioned to a new state */
    ICC_IN const ICC_Node_State_t  node_state  /**< the new state the node transitioned to */
    )
{
}

#define ICC_CHECK_ERR_CODE(icc_function_to_call) (icc_function_to_call)

#define ICC_CHECK_ERR_CODE_NO_RETURN(icc_function_to_call) (icc_function_to_call)

ICC_Err_t
ICC_Get_Channel_State(
                       ICC_IN const ICC_Channel_t         channel_id,
                       ICC_OUT      ICC_Channel_State_t * channel_state
                     )
{
    ICC_Endpoint_State_t   remote_endpoint_state = ICC_ENDPOINT_STATE_CLOSED;

    *channel_state = (ICC_Channel_State_t)(((unsigned int)remote_endpoint_state << 1U));

    return ICC_SUCCESS;
}

void ICC_Sig_Fifo_Init(ICC_Signal_Fifo_Ram_t * queue)
{

    queue->signal_queue_head = 0;
    queue->signal_queue_tail = 0;
}

unsigned int ICC_Sig_Fifo_Full(ICC_Signal_Fifo_Ram_t * queue)
{
    return ((((queue)->signal_queue_tail + 1) % ICC_CFG_MAX_SIGNALS) == (queue)->signal_queue_head);
}

void ICC_Sig_Fifo_Signal(
    ICC_Signal_Fifo_Ram_t * queue,
    ICC_Signal_t  signal)
{
    int tail;
    ICC_Err_t return_err;

    if (ICC_Sig_Fifo_Full(queue)) {
        tail = queue->signal_queue_head;
        return_err = ICC_ERR_SQ_FULL;
    }
    else{
        tail = queue->signal_queue_tail;
        return_err = ICC_SUCCESS;
    }

    queue->signals_received[tail] = signal;

    queue->signal_queue_tail = (tail == (ICC_CFG_MAX_SIGNALS - 1)) ? 0 : tail + 1;
}

/***** Relocation ********/

#define RELOCATABLE(obj) typeof(obj)* obj##_Rel

#define RELOCATED_PTR(obj) obj##_Rel

/* dest is an integer representing the destination address
   obj is an object (NOT pointer to an object!)
*/
#define RELOCATE_OBJ(dest, obj) \
    { \
      memcpy((char*)dest, (char*)(&obj), sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sizeof(obj)); \
    }

#define RELOCATE_RAW(dest, src_ptr, sz) \
    { \
      memcpy((char*)dest, (char*)(*src_ptr), sz); \
      *src_ptr = (typeof(*src_ptr))dest; \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sz); \
    }

#define RELOCATE_ICC_Fifo_Config_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      RELOCATE_OBJ(dest, obj); \
      RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->fifo_buffer_ptr), obj.fifo_size); \
    }

#define RELOCATE_ICC_Channel_Config_t_Array(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      int i, chan_count; \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      memcpy((char*)dest, (char*)obj, sizeof(obj)); \
      chan_count = (sizeof(obj)) / sizeof(ICC_Channel_Config_t); \
      assert(chan_count == ICC_CFG_NO_CHANNELS_CONF0); \
      dest = (typeof(dest))((uint64_t)RELOCATED_PTR(obj) + (uint64_t)sizeof(obj)); \
      for (i = 0; i < chan_count; i++) { \
          ICC_Fifo_Config_t * relocated_cfg = &((*RELOCATED_PTR(obj))[i].fifos_cfg[0]); \
          RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size); \
          relocated_cfg = &((*RELOCATED_PTR(obj))[i].fifos_cfg[1]); \
          RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size); \
      } \
    }

#define RELOCATE_ICC_Config_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      memcpy((char*)dest, (char*)&obj, sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)RELOCATED_PTR(obj) + (uint64_t)sizeof(obj)); \
      RELOCATED_PTR(obj)->This_Ptr = RELOCATED_PTR(obj); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->Channels_Ptr) = RELOCATED_PTR(ICC_Cfg0_ChannelsConfig); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Initialized_Shared) = &(RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Initialized_Shared); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Channels_Ram_Shared) = (RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Channels_Ram_Shared); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Fifo_Ram_Shared) = &(RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Fifo_Ram_Shared); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Node_Sig_Fifo_Ram_Shared) = &(RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Node_Sig_Fifo_Ram_Shared); \
    }


#ifdef ICC_CFG_HEARTBEAT_ENABLED
#define RELOCATE_ICC_Config_t_Heartbeat(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Heartbeat_Os_Config) = RELOCATED_PTR(ICC_Heartbeat_Os_Config0); \
    }
#endif

#define RELOCATE_ICC_Runtime_Shared_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      memcpy((char*)dest, (char*)&obj, sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sizeof(obj)); \
    }

RELOCATABLE(ICC_Fifo_Cfg0);

RELOCATABLE(ICC_Cfg0_ChannelsConfig);

#ifdef ICC_CFG_HEARTBEAT_ENABLED
RELOCATABLE(ICC_Heartbeat_Os_Config0);
#endif

RELOCATABLE(ICC_Config0);

RELOCATABLE(ICC_Runtime_Shared);

/* relocation test */

#define MB (1024 * 1024)
uint8_t reloc_buf[MB] = {0}; /* 1 MB */

int is_relocated(uint8_t * addr)
{
    return (((uint64_t)addr >= (uint64_t)reloc_buf) && ((uint64_t)addr < ((uint64_t)reloc_buf + sizeof(reloc_buf))));
}

#define STR(s) XSTR(s)
#define XSTR(s) #s
#define PR_FIELD(obj, field) out += sprintf(out, STR(field) "=%d\n", obj->field);
#define PR_FIELDX(obj, field) out += sprintf(out, STR(field) "=%x\n", obj->field);
#define PR_FIELDADDR(obj, field) out += sprintf(out, STR(field) "=%llx\n", (uint64_t)ICC_CROSS_VALUE_OF(obj->field));
#define PR_FIELDLLX(obj, field) out += sprintf(out, STR(field) "=%llx\n", obj->field);

#define PR_ARRAYX(array, count) \
    {   int i; \
        out += sprintf(out, STR(array) "[%d]: ", count); \
        for (i = 0; i < count; i++) { \
            out += sprintf(out, "%02x", ((char *)array)[i]); \
        } \
        out += sprintf(out, "\n"); \
    }

char * print_ICC_Fifo_Config_t(char * out, ICC_Fifo_Config_t * obj, uint8_t test_reloc)
{
    if (!out || !obj) {
        return out;
    }

    uint8_t* fifo_buffer_ptr = (uint8_t*)ICC_CROSS_VALUE_OF(obj->fifo_buffer_ptr);
    if (test_reloc && !is_relocated(fifo_buffer_ptr)) {
        out += sprintf(out, "not relocated\n");
        return out;
    } else {
        PR_ARRAYX(fifo_buffer_ptr, obj->fifo_size);
    }

    PR_FIELD(obj, fifo_prio);
    PR_FIELD(obj, fifo_size);
    PR_FIELD(obj, max_msg_size);
    PR_FIELD(obj, alignment);
    PR_FIELDX(obj, fifo_flags);

    return out;
}

char * print_ICC_Runtime_Shared_t(char * out, struct ICC_Runtime_Shared_t * obj, uint8_t test_reloc)
{
    if (!out || !obj) {
        return out;
    }

    if (test_reloc && !is_relocated(obj)) {
        out += sprintf(out, "not relocated\n");
        return out;
    } else {
        PR_ARRAYX(obj->ICC_Initialized_Shared, sizeof(obj->ICC_Initialized_Shared));
        PR_ARRAYX(obj->ICC_Fifo_Os_Ram_M4, sizeof(obj->ICC_Fifo_Os_Ram_M4));
        PR_ARRAYX(obj->ICC_Node_Sig_Fifo_Ram_Shared, sizeof(obj->ICC_Node_Sig_Fifo_Ram_Shared));
        PR_ARRAYX(obj->ICC_Channels_Ram_Shared, sizeof(obj->ICC_Channels_Ram_Shared));
        PR_ARRAYX(obj->ICC_Fifo_Ram_Shared, sizeof(obj->ICC_Fifo_Ram_Shared));
   }

    return out;
}

char * print_ICC_Channel_Config_t_Array(char * out, char * obj, int size, uint8_t test_reloc)
{
    int i, chan_count;
    if (!out || !obj) {
        return out;
    }

    if (test_reloc && !is_relocated(obj)) {
        out += sprintf(out, "not relocated\n");
        return out;
    }

    chan_count = size / sizeof(ICC_Channel_Config_t);
    assert(chan_count == ICC_CFG_NO_CHANNELS_CONF0);
    for (i = 0; i < chan_count; i++) {
        ICC_Channel_Config_t * config = (ICC_Channel_Config_t *)obj + i;
        out = print_ICC_Fifo_Config_t(out, &(config->fifos_cfg[0]), test_reloc);
        out = print_ICC_Fifo_Config_t(out, &(config->fifos_cfg[1]), test_reloc);
        PR_FIELDLLX(config, Channel_Update_Cb);
        PR_FIELDLLX(config, Channel_Rx_Cb);
        PR_FIELDLLX(config, Channel_Tx_Cb);
    }

    return out;
}

#define TEST_RELOC_FIELD(field) \
    if (test_reloc && !is_relocated(ICC_CROSS_VALUE_OF(obj->field))) { \
        out += sprintf(out, "not relocated\n"); \
        return out; \
    }

char * print_ICC_Config_t(char * out, ICC_Config_t * obj, uint8_t test_reloc)
{
    char * crt_field;
    if (!out || !obj) {
        return out;
    }

    if (test_reloc && !is_relocated(obj)) {
        out += sprintf(out, "not relocated\n");
        return out;
    }

    PR_ARRAYX(obj->Config_Magic, ICC_CONFIG_MAGIC_SIZE);
    PR_FIELDADDR(obj, Channels_Count);
    TEST_RELOC_FIELD(Channels_Ptr);
    out = print_ICC_Channel_Config_t_Array(out, (char *)(ICC_CROSS_VALUE_OF(obj->Channels_Ptr)), sizeof(ICC_Cfg0_ChannelsConfig), test_reloc);

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    // TODO: print function not implemented
    // out = print_ICC_Heartbeat_Os_Config(out, (char *)*ICC_CROSS_VALUE_OF(obj->ICC_Heartbeat_Os_Config), sizeof(ICC_Heartbeat_Os_Config0), test_reloc);
#endif

    PR_FIELDADDR(obj, Node_Update_Cb);

    TEST_RELOC_FIELD(ICC_Initialized_Shared);
    PR_ARRAYX(*ICC_CROSS_VALUE_OF(obj->ICC_Initialized_Shared), sizeof(obj->ICC_Initialized_Shared));
    TEST_RELOC_FIELD(ICC_Channels_Ram_Shared);
    PR_ARRAYX(ICC_CROSS_VALUE_OF(obj->ICC_Channels_Ram_Shared), sizeof(obj->ICC_Channels_Ram_Shared));
    TEST_RELOC_FIELD(ICC_Fifo_Ram_Shared);
    PR_ARRAYX(*ICC_CROSS_VALUE_OF(obj->ICC_Fifo_Ram_Shared), sizeof(obj->ICC_Fifo_Ram_Shared));
    TEST_RELOC_FIELD(ICC_Node_Sig_Fifo_Ram_Shared);
    PR_ARRAYX(*ICC_CROSS_VALUE_OF(obj->ICC_Node_Sig_Fifo_Ram_Shared), sizeof(obj->ICC_Node_Sig_Fifo_Ram_Shared));

    return out;
}

void reset(char out[MB])
{
    memset(out, 0, MB);
}

#define PRINT(...) printf(__VA_ARGS__)

char out1[MB], out2[MB]; /* 1 MB */
#define ICC_TEST_OBJ(type, obj, result) \
        {   reset(out1); reset(out2); \
            PRINT("===========================\nTesting " STR(obj) "\n"); \
            print_##type(out1, &obj, 0); \
            print_##type(out2, RELOCATED_PTR(obj), 1); \
            PRINT(STR(obj) "(%08llx)\n" "%s", (uint64_t)&obj, out1); \
            PRINT("---------------------------\n" \
                  STR(RELOCATED_PTR(obj)) "(%08llx)\n" "%s", (uint64_t)RELOCATED_PTR(obj), out2); \
            if (__builtin_types_compatible_p(typeof(obj), ICC_Config_t)) { \
                  PRINT("This_Ptr=%08llx\n", ((ICC_Config_t *)RELOCATED_PTR(obj))->This_Ptr); \
              } \
            result = result || (strcmp(out1, out2) ? ERR_FAILED : ERR_NONE); \
        }

#define ICC_TEST_ARRAY(type, obj, result) \
        {   reset(out1); reset(out2); \
            PRINT("===========================\nTesting " STR(obj) "\n"); \
            print_##type##_Array(out1, obj, sizeof(obj), 0); \
            print_##type##_Array(out2, *RELOCATED_PTR(obj), sizeof(obj), 1); \
            PRINT(STR(obj) "(%08llx)\n" "%s", (uint64_t)obj, out1); \
            PRINT("---------------------------\n" \
                  STR(RELOCATED_PTR(obj)) "(%08llx)\n" "%s", (uint64_t)*RELOCATED_PTR(obj), out2); \
            result = result || (strcmp(out1, out2) ? ERR_FAILED : ERR_NONE); \
        }

/**** Initializations ****/

void ICC_FIFO_Init(
    ICC_IN       ICC_Fifo_Ram_t       * queue_ICC,
    ICC_IN const ICC_Fifo_Config_t    * fifo_conf,
    ICC_IN       ICC_Fifo_Os_Ram_t    * fifo_os_ram,
    ICC_IN       unsigned int             init)
{
    ICC_CROSS_ASSIGN(queue_ICC->fifo_config[ICC_GET_CORE_ID], fifo_conf); /**< set link to fifo configuration from channel structure */
    ICC_CROSS_ASSIGN(queue_ICC->fifo_os_ram[ICC_GET_CORE_ID], fifo_os_ram);

    queue_ICC->rd[ICC_GET_CORE_ID] = 0;
    queue_ICC->wr[ICC_GET_CORE_ID] = 0;

    if (init)
    {
        queue_ICC->head = 0;
        queue_ICC->tail = 0;

        queue_ICC->pending_send_msg_size = 0;
    }
}

void ICC_Init_Ram_Shared(
    const int Channels_Count,
    const ICC_IN ICC_Channel_Config_t* ICC_Channels_Config,
    ICC_OUT struct ICC_Runtime_Shared_t * ICC_Runtime_Shared
    )
{
    int i = 0;
    ICC_Channel_Ram_t* ICC_Channels_Ram = ICC_Runtime_Shared->ICC_Channels_Ram_Shared;
    ICC_Fifo_Ram_t(*ICC_Fifo_Ram)[][2] = &(ICC_Runtime_Shared->ICC_Fifo_Ram_Shared);
    ICC_Signal_Fifo_Ram_t(*ICC_Node_Sig_Fifo_Ram)[2] = &(ICC_Runtime_Shared->ICC_Node_Sig_Fifo_Ram_Shared);

    const ICC_Channel_Config_t * channel_conf;
    volatile ICC_Channel_Ram_t * channel_ram;

    for (i = 0; i < Channels_Count; i++) {

        ICC_Channel_State_t    channel_new_state;

        channel_conf = &ICC_Channels_Config[i];
        channel_ram = &ICC_Channels_Ram[i];

        /* close all end points on our side */
        channel_ram->endpoint_state[ICC_LOCAL_ENDPOINT] = ICC_ENDPOINT_STATE_CLOSED;

        ICC_CHECK_ERR_CODE(ICC_Get_Channel_State(i, &channel_new_state));

        /*
        * Init TX signal fifo from each side
        */
        ICC_CHECK_ERR_CODE(ICC_Sig_Fifo_Init((ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ICC_TX_FIFO]));

        /* signal the channel state transition to REMOTE node */
        ICC_CHECK_ERR_CODE_NO_RETURN(ICC_Sig_Fifo_Signal((ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ICC_TX_FIFO], channel_new_state));


        /*
        * link each Fifo priority wise
        */
        ICC_CROSS_ASSIGN(channel_ram->fifos_ram[ICC_TX_FIFO][ICC_GET_CORE_ID], (ICC_Fifo_Ram_t*)&((*ICC_Fifo_Ram)[channel_conf->fifos_cfg[ICC_TX_FIFO].fifo_prio][ICC_TX_FIFO]));
        ICC_CROSS_ASSIGN(channel_ram->fifos_ram[ICC_RX_FIFO][ICC_GET_CORE_ID], (ICC_Fifo_Ram_t*)&((*ICC_Fifo_Ram)[channel_conf->fifos_cfg[ICC_RX_FIFO].fifo_prio][ICC_RX_FIFO]));

        /* initialize TX fifo */
        ICC_FIFO_Init(
            ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ICC_TX_FIFO][ICC_GET_CORE_ID]),
            &channel_conf->fifos_cfg[ICC_TX_FIFO],
            &(ICC_Runtime_Shared->ICC_Fifo_Os_Ram_M4[i][ICC_TX_FIFO]),
            1);

        /* initialize RX fifo */
        ICC_FIFO_Init(
            ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ICC_RX_FIFO][ICC_GET_CORE_ID]),
            &channel_conf->fifos_cfg[ICC_RX_FIFO],
            &(ICC_Runtime_Shared->ICC_Fifo_Os_Ram_M4[i][ICC_RX_FIFO]),
            0);
    }

    ICC_CHECK_ERR_CODE(ICC_Sig_Fifo_Init((ICC_Signal_Fifo_Ram_t *)&((*ICC_Node_Sig_Fifo_Ram)[ICC_TX_FIFO])));
    ICC_CHECK_ERR_CODE_NO_RETURN(ICC_Sig_Fifo_Signal((ICC_Signal_Fifo_Ram_t *)&((*ICC_Node_Sig_Fifo_Ram)[ICC_TX_FIFO]), ICC_NODE_STATE_INIT));
}

int main(int argc, char* argv[])
{
	int result = ERR_NONE;
	unsigned * dest = (unsigned *)reloc_buf;

    RELOCATE_ICC_Fifo_Config_t(dest, ICC_Fifo_Cfg0);
    ICC_TEST_OBJ(ICC_Fifo_Config_t, ICC_Fifo_Cfg0, result);

    RELOCATE_ICC_Channel_Config_t_Array(dest, ICC_Cfg0_ChannelsConfig);
    ICC_TEST_ARRAY(ICC_Channel_Config_t, ICC_Cfg0_ChannelsConfig, result);

	RELOCATE_ICC_Runtime_Shared_t(dest, ICC_Runtime_Shared);
	ICC_TEST_OBJ(ICC_Runtime_Shared_t, ICC_Runtime_Shared, result);

	RELOCATE_ICC_Config_t(dest, ICC_Config0);
	ICC_TEST_OBJ(ICC_Config_t, ICC_Config0, result);

    ICC_Init_Ram_Shared(ICC_CFG_NO_CHANNELS_CONF0, ICC_Cfg0_ChannelsConfig, &ICC_Runtime_Shared);
    ICC_Init_Ram_Shared(ICC_CFG_NO_CHANNELS_CONF0, RELOCATED_PTR(ICC_Cfg0_ChannelsConfig), RELOCATED_PTR(ICC_Runtime_Shared));

    //ICC_TEST_OBJ(ICC_Runtime_Shared_t, ICC_Runtime_Shared, result);

    goto done;

done:
    
    if (result == ERR_NONE) {
    	printf ("Relocation test PASSED\n");
    } else {
    	printf ("Relocation test FAILED\n");
    }
	return result;
}

