

/**
*   @file    ICC_Config.h
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

#ifndef ICC_CONFIG_H
#define ICC_CONFIG_H





/*==================================================================================================
                                       DEFINES AND MACROS
==================================================================================================*/



/*
 * ***************************************************** ICC global parameters across multiple configurations
 */
#define ICC_CFG_MAX_CHANNELS          (2U)      /**< maximum # channels, across multiple configurations */
#define ICC_CFG_MAX_SIGNALS           (32U)      /**< maximum signals, across multiple configurations */
#define ICC_CFG_MAX_TIMEOUT           (500000U)  /**< maximum timeout value for API, 500ms, across multiple configurations */

/*
 * Heartbeat mechanism is enabled in at least one configuration
 * TODO: fix this - currently the heartbeat is not synchronized
 * between Autosar and Linux
 */
//#define ICC_CFG_HEARTBEAT_ENABLED

#include "ICC_Types.h" /**< Yes, this must be here */

#define ICC_RATE_US          (75)         /**<  sent messages rate */
#define ICC_TXRX_TIMEOUT     (ICC_RATE_US * 8)

/*
 * ICC configured channels ID
 */
#define IccChannel_0 (0)
#define IccChannel_1 (1)


/*
 * macros for current and remote core id : TRESOS GENERATED
 */
#ifdef ICC_BUILD_FOR_M4
    /* RT */
    #define ICC_GET_CORE_ID            (0)
    #define ICC_GET_REMOTE_CORE_ID     (1)
#else
    /* APP */
    #define ICC_GET_CORE_ID            (1)
    #define ICC_GET_REMOTE_CORE_ID     (0)
#endif

/*
 * ICC sema4 channel id used
 */
#define ICC_CFG_HW_SEM0 (15U)

/*
 * ICC CPU2CPU IRQ used
 */
#define ICC_CFG_HW_CPU2CPU_IRQ (0U)

/*
 * Name of the remote ISR object from Autosar OS
 */
#define ICC_CFG_ASR_REMOTE_ISR (ICC_AsrOS_Reserved_Remote_ISR)


/*
 * ***************************************************** ICC configuration specific parameters
 */
#define ICC_CFG_NO_CHANNELS_CONF0     (2)  /**< channels in this configuration */
#define ICC_CFG0_HEARTBEAT_CHANNEL_ID     (IccChannel_0)  /**< channel on which the heartbeat mechanism will function */

/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/





/*==================================================================================================
                                 GLOBAL VARIABLES DECLARATION
==================================================================================================*/

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

    /*
     * declaration of ICC top level configuration structure
     */

#ifndef ICC_LINUX2LINUX

    #ifdef ICC_BUILD_FOR_M4
    extern const ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_DATA
    #else
    extern ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA
    #endif
    ICC_Config_t ICC_Config0;

#else

    #ifdef ICC_BUILD_FOR_M4

        #include "ICC_Relocate.h"

        extern ICC_Config_t * RELOCATED_PTR(ICC_Config0);

        #define ICC_Default_Config_Ptr (RELOCATED_PTR(ICC_Config0))

    #else

        extern ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA
        ICC_Config_t ICC_Config0;

        #define ICC_Default_Config_Ptr (&ICC_Config0)

    #endif

#endif

#ifdef ICC_BUILD_FOR_M4
    #define ICC_STOP_SEC_SHARED_VAR_UNSPECIFIED
    #include "ICC_MemMap.h"
#else
    #define ICC_STOP_SEC_VAR_UNSPECIFIED
    #include "ICC_MemMap.h"
#endif


/*==================================================================================================
*                                      FUNCTION PROTOTYPES
==================================================================================================*/

/* TRESOS GENERATED */

#ifdef ICC_BUILD_FOR_M4


        extern
        void USER_ICC_Callback_Rx_CB_M4(
                                   ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */
                                );
        
        extern
        void USER_ICC_Callback_Tx_CB_M4(
                                   ICC_IN const ICC_Channel_t   channel_id           /**< the id of the channel that just "received" free space */
                                );
        
        extern
        void USER_ICC_Callback_Channel_State_Update_CB_M4(
                                    ICC_IN const ICC_Channel_t          channel_id,   /**< the id of the channel that transitioned to a new state */
                                    ICC_IN const ICC_Channel_State_t    channel_state /**< the new state that the channel transitioned to */
                                );
        
        extern
        void USER_ICC_Node_State_Update_CB_M4(
                                    ICC_IN const ICC_Node_t        node_id,    /**< the id of the node that transitioned to a new state */
                                    ICC_IN const ICC_Node_State_t  node_state  /**< the new state the node transitioned to */
                                );
        
#else

        extern
        void USER_ICC_Callback_Rx_CB_App(
                                   ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */
                                );
        
        extern
        void USER_ICC_Callback_Tx_CB_App(
                                   ICC_IN const ICC_Channel_t   channel_id           /**< the id of the channel that just "received" free space */
                                );
        
        extern
        void USER_ICC_Callback_Channel_State_Update_CB_App(
                                    ICC_IN const ICC_Channel_t          channel_id,   /**< the id of the channel that transitioned to a new state */
                                    ICC_IN const ICC_Channel_State_t    channel_state /**< the new state that the channel transitioned to */
                                );
        
        extern
        void USER_ICC_Node_State_Update_CB_App(
                                    ICC_IN const ICC_Node_t        node_id,    /**< the id of the node that transitioned to a new state */
                                    ICC_IN const ICC_Node_State_t  node_state  /**< the new state the node transitioned to */
                                );
        
#endif

#ifdef ICC_LINUX2LINUX

    struct ICC_Runtime_Shared_t {
        unsigned int          ICC_Initialized_Shared[ 2 ];                       /**< ICC state on each node */
        ICC_Channel_Ram_t     ICC_Channels_Ram_Shared[ ICC_CFG_MAX_CHANNELS ]; /**< runtime structure for each channel */
        ICC_Fifo_Ram_t        ICC_Fifo_Ram_Shared[ ICC_CFG_MAX_CHANNELS ][ 2 ];  /**< fifos ordered priority wise for each node */
        ICC_Signal_Fifo_Ram_t ICC_Node_Sig_Fifo_Ram_Shared[ 2 ];              /**< signal fifo for each node */
    };

#ifdef ICC_BUILD_FOR_M4

    /*
     * This function relocates the main ICC config object and its dependencies
     * to a destination buffer received as argument.
     * The function returns the pointer where data was relocated.
     */
    extern
    char * ICC_Relocate_Config(void);
#endif

extern int ICC_Dump_Shared_Config(void);

#endif

#endif /* ICC_CONFIG_H */

