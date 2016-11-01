
/**
*   @file    ICC_Config.c
*   @version 0.8.0
*
*   @brief   ICC - Inter Node Communication Configuration file
*   @details       Inter Node Communication Configuration file generated by Tresos plug-in
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
*   All Rights Reserved.
==================================================================================================*/
/*==================================================================================================
==================================================================================================*/



#include "ICC_Config.h"

#include "ICC_Types.h"


#if (defined(ICC_LINUX2LINUX) && defined(ICC_BUILD_FOR_M4))
#define STATIC_ALLOC static
#else
#define STATIC_ALLOC
#endif


#ifdef ICC_BUILD_FOR_M4

#define ICC_START_SEC_CONST_UNSPECIFIED
#include "ICC_MemMap.h"


#ifdef ICC_FSL_AUTOSAR_OS


    #define ICC_CFG0_CH_0_FIFO_0_TAE_SIZE       (1UL)
    #define ICC_CFG0_CH_0_FIFO_1_TAE_SIZE       (1UL)

    #define ICC_CFG0_CH_1_FIFO_0_TAE_SIZE       (3UL)
    #define ICC_CFG0_CH_1_FIFO_1_TAE_SIZE       (3UL)



    ICC_ATTR_SEC_CONST_UNSPECIFIED
    const
    ICC_Fifo_Os_TAE_Config_t cfg_0_ch_0_fifo_0_tae[ ICC_CFG0_CH_0_FIFO_0_TAE_SIZE ] = {

        {
            HB_WORKER,
            OsAlarm_HBW_ch0_f0,
            OsEvent_HBW_ch0_f0
        }
        
    };

    ICC_ATTR_SEC_CONST_UNSPECIFIED
    const
    ICC_Fifo_Os_TAE_Config_t cfg_0_ch_0_fifo_1_tae[ ICC_CFG0_CH_0_FIFO_1_TAE_SIZE ] = {

        {
            HB_WORKER,
            OsAlarm_HBW_ch0_f1,
            OsEvent_HBW_ch0_f1
        }
        
    };

    ICC_ATTR_SEC_CONST_UNSPECIFIED
    const
    ICC_Fifo_Os_TAE_Config_t cfg_0_ch_1_fifo_0_tae[ ICC_CFG0_CH_1_FIFO_0_TAE_SIZE ] = {

        {
            WORKER1,
            OsAlarm_W1_ch1_f0,
            OsEvent_W1_ch1_f0
        },
        
        {
            WORKER2,
            OsAlarm_W2_ch1_f0,
            OsEvent_W2_ch1_f0
        },
        
        {
            WORKER3,
            OsAlarm_W3_ch1_f0,
            OsEvent_W3_ch1_f0
        }
        
    };

    ICC_ATTR_SEC_CONST_UNSPECIFIED
    const
    ICC_Fifo_Os_TAE_Config_t cfg_0_ch_1_fifo_1_tae[ ICC_CFG0_CH_1_FIFO_1_TAE_SIZE ] = {

        {
            WORKER1,
            OsAlarm_W1_ch1_f1,
            OsEvent_W1_ch1_f1
        },
        
        {
            WORKER2,
            OsAlarm_W2_ch1_f1,
            OsEvent_W2_ch1_f1
        },
        
        {
            WORKER3,
            OsAlarm_W3_ch1_f1,
            OsEvent_W3_ch1_f1
        }
        
    };


#endif /* ICC_FSL_AUTOSAR_OS */

/*
 * Structure needed only for Autosar OS
 * in case it's needed for other OS delete this #ifdef
 */
#ifdef ICC_BUILD_FOR_M4

#ifdef ICC_FSL_AUTOSAR_OS

ICC_ATTR_SEC_CONST_UNSPECIFIED
const
ICC_Fifo_Os_Config_t
ICC_Fifo_Os_Config0[][ 2 ] = {

        /* channel 0 */
        {
                { /* fifo 0 */
                    OsResource_ch0_f0,
                    OsEvent_ch0_f0,
                    ICC_CFG0_CH_0_FIFO_0_TAE_SIZE,
                    cfg_0_ch_0_fifo_0_tae
                },
                { /* fifo 1 */
                    OsResource_ch0_f1,
                    OsEvent_ch0_f1,
                    ICC_CFG0_CH_0_FIFO_1_TAE_SIZE,
                    cfg_0_ch_0_fifo_1_tae
                }
        },
        /* channel 1 */
        {
                { /* fifo 0 */
                    OsResource_ch1_f0,
                    OsEvent_ch1_f0,
                    ICC_CFG0_CH_1_FIFO_0_TAE_SIZE,
                    cfg_0_ch_1_fifo_0_tae
                },
                { /* fifo 1 */
                    OsResource_ch1_f1,
                    OsEvent_ch1_f1,
                    ICC_CFG0_CH_1_FIFO_1_TAE_SIZE,
                    cfg_0_ch_1_fifo_1_tae
                }
        }

};

#else

ICC_ATTR_SEC_CONST_UNSPECIFIED
const
STATIC_ALLOC
ICC_Fifo_Os_Config_t
ICC_Fifo_Os_Config0[][ 2 ] = {{ 0 }};

#endif /* ICC_FSL_AUTOSAR_OS */

#endif /* ICC_BUILD_FOR_M4 */


#define ICC_STOP_SEC_CONST_UNSPECIFIED
#include "ICC_MemMap.h"


#endif /* ICC_BUILD_FOR_M4 */

#ifdef ICC_CFG_HEARTBEAT_ENABLED

#define ICC_START_SEC_CONST_UNSPECIFIED
#include "ICC_MemMap.h"



ICC_ATTR_SEC_CONST_UNSPECIFIED
const
STATIC_ALLOC
ICC_Heartbeat_Os_Config_t
ICC_Heartbeat_Os_Config0 = {
	ICC_CFG0_HEARTBEAT_CHANNEL_ID,
	ICC_RATE_US,
	ICC_TXRX_TIMEOUT,
#ifdef ICC_BUILD_FOR_M4
	#ifdef ICC_FSL_AUTOSAR_OS
		OsEvent_HB_RATE,
		OsAlarm_HB_RATE,
		WORKER3,
	#endif /* ICC_FSL_AUTOSAR_OS */
#endif /* ICC_BUILD_FOR_M4 */
};



#define ICC_STOP_SEC_CONST_UNSPECIFIED
#include "ICC_MemMap.h"

#endif /* ICC_CFG_HEARTBEAT_ENABLED */



#define ICC_CFG0_CH_0_FIFO_0_SIZE        (1024UL)
#define ICC_CFG0_CH_0_FIFO_1_SIZE        (1024UL)

#define ICC_CFG0_CH_1_FIFO_0_SIZE        (2048UL)
#define ICC_CFG0_CH_1_FIFO_1_SIZE        (2048UL)




#ifdef ICC_BUILD_FOR_M4

#define ICC_START_SEC_SHARED_VAR_UNSPECIFIED
#include "ICC_MemMap.h"

    /* non cache-able statically allocated space */

    /* FIFO buffers allocated only on M4 side */




    ICC_ATTR_ALIGN_VAR ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS STATIC_ALLOC unsigned char cfg_0_ch_0_fifo_0_buffer[ ICC_CFG0_CH_0_FIFO_0_SIZE ];
    ICC_ATTR_ALIGN_VAR ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS STATIC_ALLOC unsigned char cfg_0_ch_0_fifo_1_buffer[ ICC_CFG0_CH_0_FIFO_1_SIZE ];

    ICC_ATTR_ALIGN_VAR ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS STATIC_ALLOC unsigned char cfg_0_ch_1_fifo_0_buffer[ ICC_CFG0_CH_1_FIFO_0_SIZE ];
    ICC_ATTR_ALIGN_VAR ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS STATIC_ALLOC unsigned char cfg_0_ch_1_fifo_1_buffer[ ICC_CFG0_CH_1_FIFO_1_SIZE ];



#define ICC_STOP_SEC_SHARED_VAR_UNSPECIFIED
#include "ICC_MemMap.h"

#endif




#ifdef ICC_BUILD_FOR_M4
    /*
     * configuration must be shared by the M4 side
     */
    #define ICC_START_SEC_SHARED_VAR_UNSPECIFIED
    #include "ICC_MemMap.h"
#else
    #define ICC_START_SEC_VAR_UNSPECIFIED
    #include "ICC_MemMap.h"
#endif







#ifdef ICC_BUILD_FOR_M4
    const ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_DATA
#else
    ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA
#endif
STATIC_ALLOC
ICC_Channel_Config_t
ICC_Cfg0_ChannelsConfig[ ICC_CFG_NO_CHANNELS_CONF0 ] = {

    /* channel 0 */
    {
            /* fifo 0 */
            {{
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
            }},

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
            {{
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
            }},

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




#ifdef ICC_BUILD_FOR_M4

    /* multicore shared variables allocated on M4 side only */

#ifdef ICC_LINUX2LINUX

    /* multicore shared variables allocated on M4 side only */

    ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_DATA
    volatile
    struct ICC_Runtime_Shared_t ICC_Runtime_Shared = {
        { ICC_NODE_STATE_UNINIT, ICC_NODE_STATE_UNINIT },
        {{ 0 }}
    };

    #define ICC_RUNTIME_SHARED(field) (ICC_Runtime_Shared.field)

#else

    extern
    ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS
    volatile
    unsigned int          ICC_Initialized_Shared[2]; /**< shows if ICC is initialized */

    extern
    ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS
    volatile
    ICC_Channel_Ram_t     ICC_Channels_Ram_Shared[ ICC_CFG_MAX_CHANNELS ]; /**< runtime structure for each channel */

    extern
    ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS
    volatile
    ICC_Fifo_Ram_t        ICC_Fifo_Ram_Shared[ ICC_CFG_MAX_CHANNELS ][2];  /**< fifos ordered priority wise for each node */

    extern ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS
    volatile
    ICC_Signal_Fifo_Ram_t  ICC_Node_Sig_Fifo_Ram_Shared[ 2 ];               /**< signal fifo for each node */

    #define ICC_RUNTIME_SHARED(field) (field)

#endif

#endif


#ifdef ICC_BUILD_FOR_M4
const ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_DATA
#else
ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA
#endif
STATIC_ALLOC
ICC_Config_t ICC_Config0 = {
        ICC_CONFIG_MAGIC,
        NULL,               /**< This_Ptr is NULL for static defined objects.
                                 Relocated objects must populate it with their own virtual address */

        ICC_CROSS_INIT(ICC_CFG_NO_CHANNELS_CONF0),           /**< number of configured ICC channels in this configuration */
		ICC_CROSS_INIT(ICC_Cfg0_ChannelsConfig),             /**< ICC channels */

        #ifdef ICC_FSL_AUTOSAR_OS
		    ICC_CROSS_INIT(&ICC_Fifo_Os_Config0),            /**< pointer to Fifo Os configuration */
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
			ICC_CROSS_INIT(&ICC_RUNTIME_SHARED(ICC_Initialized_Shared)),          /**<  pointer to shared variable */
        #else
			ICC_CROSS_INIT(NULL_PTR),
        #endif

        #ifdef ICC_BUILD_FOR_M4
			ICC_CROSS_INIT(ICC_RUNTIME_SHARED(ICC_Channels_Ram_Shared)),         /**<  pointer to shared variable */
        #else
			ICC_CROSS_INIT(NULL_PTR),
        #endif

        #ifdef ICC_BUILD_FOR_M4
			ICC_CROSS_INIT(&ICC_RUNTIME_SHARED(ICC_Fifo_Ram_Shared)),            /**<  pointer to shared variable */
        #else
			ICC_CROSS_INIT(NULL_PTR),
        #endif

        #ifdef ICC_BUILD_FOR_M4
			ICC_CROSS_INIT(&ICC_RUNTIME_SHARED(ICC_Node_Sig_Fifo_Ram_Shared)),       /**<  pointer to shared variable */
        #else
			ICC_CROSS_INIT(NULL_PTR),
        #endif
};



#ifdef ICC_BUILD_FOR_M4
    #define ICC_STOP_SEC_SHARED_VAR_UNSPECIFIED
    #include "ICC_MemMap.h"
#else
    #define ICC_STOP_SEC_VAR_UNSPECIFIED
    #include "ICC_MemMap.h"
#endif


#ifdef ICC_LINUX2LINUX

#ifdef ICC_BUILD_FOR_M4

RELOCATABLE(ICC_Fifo_Os_Config0);

RELOCATABLE(ICC_Cfg0_ChannelsConfig);

#ifdef ICC_CFG_HEARTBEAT_ENABLED
RELOCATABLE(ICC_Heartbeat_Os_Config0);
#endif

ICC_Config_t * RELOCATED_PTR(ICC_Config0);

RELOCATABLE(ICC_Runtime_Shared);

extern char * ICC_Shared_Virt_Base_Addr;

/*
 * This function relocates the main ICC config object and its dependencies
 * to a destination buffer received as argument.
 * The function returns the pointer where data was relocated.
 *
 * TODO: move this to ICC_lib.c or another location after the relocation of ICC_Config_t
 * is done transparently for the inner members.
*/
extern char * ICC_Relocate_Config(void)
{
    char * dest = ICC_Shared_Virt_Base_Addr;

    if (dest == NULL) {
        printk(KERN_ERR "Null destination: unable to relocate configuration\n");
        return NULL;
    }

    //RELOCATE_ICC_Fifo_Os_Config_t_Array(dest, ICC_Fifo_Os_Config0);

    RELOCATE_ICC_Channel_Config_t_Array(dest, ICC_Cfg0_ChannelsConfig);

    RELOCATE_ICC_Runtime_Shared_t(dest, ICC_Runtime_Shared);

    RELOCATE_ICC_Config_t(dest, ICC_Config0);

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    RELOCATE_ICC_Config_t_Heartbeat(dest, ICC_Config0);
#endif

    return ICC_Shared_Virt_Base_Addr;
}

EXPORT_SYMBOL(ICC_Relocate_Config);

#endif  /* ICC_BUILD_FOR_M4 */

#include "ICC_Config_Test.h"

#ifndef ICC_BUILD_FOR_M4
extern
ICC_Config_t * ICC_Config_Ptr_M4;          /**< pointer to M4 current configuration */
#endif

#define MB (1024 * 1024)

#define BUF_SIZE (10 * MB)
char out[BUF_SIZE];

void reset(char out[BUF_SIZE])
{
    memset(out, 0, BUF_SIZE);
}

extern int ICC_Dump_Shared_Config(void)
{
    int dump_count = 0;
#ifdef ICC_BUILD_FOR_M4
    ICC_DUMP_PTR(dump_count, ICC_Config_t, ICC_Default_Config_Ptr);
#else
    ICC_DUMP_PTR(dump_count, ICC_Config_t, ICC_Config_Ptr_M4);
#endif
    return dump_count;
}

EXPORT_SYMBOL(ICC_Dump_Shared_Config);

#endif

