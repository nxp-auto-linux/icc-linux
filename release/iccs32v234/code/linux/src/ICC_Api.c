/**
*   @file    ICC_Api.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication API implementation
*   @details Inter Core Communication API implementation
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


#ifdef __cplusplus
extern "C"
{
#endif


#include "ICC_Api.h"
#include "ICC_Private.h"

#include "ICC_Fifo.h"

#include "ICC_Sig_Fifo.h"

#include "ICC_Hw.h"
#include "ICC_Os.h"




/*==================================================================================================
                                       DEFINES AND MACROS
==================================================================================================*/

#define ICC_CHECK_ERR_CODE(icc_function_to_call)                 \
            return_code = (icc_function_to_call);                \
            if (ICC_SUCCESS != return_code) {return return_code;}

#define ICC_CHECK_ERR_CODE_NO_RETURN(icc_function_to_call)       \
            tmp_return_code = (icc_function_to_call);            \
            if (ICC_SUCCESS != tmp_return_code) {                \
                return_code = tmp_return_code;                   \
            }



#ifdef ICC_CFG_HEARTBEAT_ENABLED

#define ICC_CFG_HEARTBEAT_STARTING_MSG  (0x00001UL)  /**< the value of the initial msg the HB mechanism sends/expects */

#define ICC_HEARTBEAT_CHECK_ERR_CODE(icc_function_to_call)                                    \
             return_code = (icc_function_to_call);                                             \
             if (ICC_SUCCESS != return_code) {                                                 \
                 ICC_Heartbeat_State = ICC_HEARTBEAT_STATE_ERROR;                             \
                 if (  ((*ICC_Initialized)[ ICC_GET_CORE_ID ] != ICC_NODE_STATE_UNINIT) &&  \
                       (NULL_PTR != ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Node_Update_Cb)) ) {                       \
                     ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Node_Update_Cb)( ICC_NODE_REMOTE, ICC_NODE_STATE_DEAD); \
                 }                                                                            \
                 return return_code;                                                          \
             }

#endif /* ICC_CFG_HEARTBEAT_ENABLED */

/*===============================================================================================
*                                       GLOBAL VARIABLES
===============================================================================================*/

#define ICC_START_SEC_VAR_UNSPECIFIED
#include "ICC_MemMap.h"

        /*static*/
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA ICC_Config_t * ICC_Config_Ptr = NULL_PTR;  /**< pointer to current configuration structure */

#ifndef ICC_BUILD_FOR_M4
        extern
        ICC_Config_t * ICC_Config_Ptr_M4;          /**< pointer to M4 current configuration */
#endif

        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_PTR_VECTOR(u32)      ICC_Initialized  = NULL_PTR;                            /**< shows if ICC is initialized */
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_Channel_Ram_t      * ICC_Channels_Ram = NULL_PTR;                            /**< runtime structure for each channel */
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_PTR_MATRIX(ICC_Fifo_Ram_t)        ICC_Fifo_Ram = NULL_PTR;    /**< fifos_ram ordered priority wise for each node */
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_PTR_VECTOR(ICC_Signal_Fifo_Ram_t) ICC_Node_Sig_Fifo_Ram = NULL_PTR;        /**< signal fifo for each node */
        #ifdef ICC_CFG_HEARTBEAT_ENABLED
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_Heartbeat_State_t    ICC_Heartbeat_State  = ICC_HEARTBEAT_STATE_UNINIT;          /**< shows ICC_Heartbeat mechanism state */
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile u32                      ICC_Heartbeat_RunId  = 0;                                   /**< the current RunId for the HB mechanism */
        #endif /* ICC_CFG_HEARTBEAT_ENABLED */

        ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS ICC_Fifo_Os_Ram_t ICC_Fifo_Os_Ram_APP[ ICC_CFG_MAX_CHANNELS ][ 2 /* tx/rx */ ];   /**< Fifo OS specific Ram structure */
        #define ICC_Fifo_Os_Ram ICC_Fifo_Os_Ram_APP

    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_Signal_Fifo_Ram_t ICC_Signal_Fifo_Ch_Ram_Local [ ICC_CFG_MAX_CHANNELS ]; /**< local signal queue for each channel */
        ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA volatile ICC_Signal_Fifo_Ram_t ICC_Signal_Fifo_Node_Ram_Local; /**< node local signal queue */
    #endif


#define ICC_STOP_SEC_VAR_UNSPECIFIED
#include "ICC_MemMap.h"



/*===============================================================================================
*                                       LOCAL FUNCTIONS
===============================================================================================*/


#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"




ICC_ATTR_SEC_TEXT_CODE
static
ICC_Err_t
ICC_Compare_Fifo_Conf( const ICC_Fifo_Config_t * fifo_config_APP,
                       const ICC_Fifo_Config_t * fifo_config_M4 )
{

    if (fifo_config_M4->fifo_size != fifo_config_APP->fifo_size) {

        return ICC_ERR_CONFIG_MATCH_FAILED;
    }

    if (fifo_config_M4->fifo_prio != fifo_config_APP->fifo_prio) {

        return ICC_ERR_CONFIG_MATCH_FAILED;
    }

    if (fifo_config_M4->max_msg_size != fifo_config_APP->max_msg_size) {

        return ICC_ERR_CONFIG_MATCH_FAILED;
    }

    if (fifo_config_M4->alignment != fifo_config_APP->alignment) {

        return ICC_ERR_CONFIG_MATCH_FAILED;
    }

    return ICC_SUCCESS;

}



/*===============================================================================================*
 *                                       GLOBAL FUNCTIONS                                        *
 *===============================================================================================*/

#ifdef ICC_USE_POLLING

/* Declare notify functions here as extern.
 * Impementation must be provided elsewhere.
 */

ICC_ATTR_SEC_TEXT_CODE
extern
void ICC_Notify_Remote( void );

ICC_ATTR_SEC_TEXT_CODE
extern
void ICC_Clear_Notify_Remote( void );

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
ICC_ATTR_SEC_TEXT_CODE
extern
void ICC_Notify_Local( void );

ICC_ATTR_SEC_TEXT_CODE
extern
void ICC_Clear_Notify_Local( void );
#endif

#else

ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Remote(void)
{
    ICC_HW_Trigger_Cpu2Cpu_Interrupt( ICC_CFG_HW_CPU2CPU_IRQ ); /**< trigger remote interrupt */
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_Remote(void)
{
    ICC_HW_Clear_Cpu2Cpu_Interrupt(ICC_CFG_HW_CPU2CPU_IRQ);
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
ICC_ATTR_SEC_TEXT_CODE
void ICC_Notify_Local(void)
{
    ICC_HW_Trigger_Local_Interrupt( ICC_CFG_HW_LOCAL_IRQ ); /**< trigger local interrupt */
}

ICC_ATTR_SEC_TEXT_CODE
void ICC_Clear_Notify_Local(void)
{
    ICC_HW_Clear_Local_Interrupt( ICC_CFG_HW_LOCAL_IRQ ); /**< trigger local interrupt */
}
#endif

#endif

/**
 *
 * Called by each core to initialize ICC for the current node.
 *
 * ICC_ERR_NODE_INITIALIZED - node already initialized
 * ICC_ERR_PARAM_NULL_CFG   - invalid configuration parameters passed: NULL_PTR
 * ICC_ERR_GENERAL          - TBD
 * ICC_SUCCESS              - OK
 *
 * For Linux2Linux over PCIE, EP must call this function with the 
 * relocated config object, and the RC with the static config object
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t 
ICC_Initialize( 
                ICC_IN ICC_Config_t * config
              )
{

    int i=0;
    ICC_Err_t tmp_return_code, return_code = ICC_SUCCESS;
    const ICC_Channel_Config_t * channel_conf;
    volatile ICC_Channel_Ram_t * channel_ram;

    /*
     * check if already initialized
     */
    if (NULL_PTR != ICC_Config_Ptr) {

        return ICC_ERR_NODE_INITIALIZED;

    } else {

        /*
         * check valid parameter
         */
        if (NULL_PTR == config) {

            return ICC_ERR_PARAM_NULL_CFG;

        } else {

            ICC_Config_Ptr = config;
        }
    }


#ifndef ICC_BUILD_FOR_M4
	
	/* Initialize local config with M4 config */
    {
        int j;
        ICC_Fifo_Config_t    * fifo_config_M4;
        const ICC_Channel_Config_t * channel_conf_M4;

        ICC_Fifo_Config_t          * fifo_config_APP;

        /*
         * Configuration validation VS other core
         */

        if (ICC_CROSS_VALUE_OF(ICC_Config_Ptr_M4->Channels_Count) != ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {

            return ICC_ERR_CONFIG_MATCH_FAILED;
        }


        /*
         * update configuration structure on APP side
         */

        ICC_CROSS_ASSIGN(ICC_Config_Ptr->ICC_Initialized_Shared, (ICC_PTR_VECTOR(u32)) ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(ICC_Config_Ptr_M4->ICC_Initialized_Shared)));
        ICC_CROSS_ASSIGN(ICC_Config_Ptr->ICC_Channels_Ram_Shared, (ICC_Channel_Ram_t *) ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(ICC_Config_Ptr_M4->ICC_Channels_Ram_Shared)));
        ICC_CROSS_ASSIGN(ICC_Config_Ptr->ICC_Fifo_Ram_Shared, (ICC_PTR_MATRIX(ICC_Fifo_Ram_t)) ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(ICC_Config_Ptr_M4->ICC_Fifo_Ram_Shared)));
        ICC_CROSS_ASSIGN(ICC_Config_Ptr->ICC_Node_Sig_Fifo_Ram_Shared, (ICC_PTR_VECTOR(ICC_Signal_Fifo_Ram_t)) ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(ICC_Config_Ptr_M4->ICC_Node_Sig_Fifo_Ram_Shared)));


        for (i = 0; i < ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count); i++) {

            channel_conf    = &ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Ptr)[i];
            channel_conf_M4 = (ICC_Channel_Config_t *) ICC_OS_Phys_To_Virt(&(ICC_CROSS_VALUE_OF(ICC_Config_Ptr_M4->Channels_Ptr)[i]));

            for (j=0; j<2; j++) {
                fifo_config_APP = &channel_conf->fifos_cfg[ j ];
                fifo_config_M4  = &channel_conf_M4->fifos_cfg[ j ];

                ICC_CHECK_ERR_CODE( ICC_Compare_Fifo_Conf(fifo_config_APP, fifo_config_M4) );

                /*
                 * update APP fifo buffer pointer from M4 side
                 */
                /*
                 * TBD: if address re-mapping is needed then it must be done here
                 */
                ICC_CROSS_ASSIGN(fifo_config_APP->fifo_buffer_ptr, (u8 *) ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(fifo_config_M4->fifo_buffer_ptr)));
            }
        }

    }

#endif



    ICC_Initialized          = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Initialized_Shared);
    ICC_Channels_Ram         = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Channels_Ram_Shared);
    ICC_Fifo_Ram             = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Fifo_Ram_Shared);
    ICC_Node_Sig_Fifo_Ram    = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Node_Sig_Fifo_Ram_Shared);

    for (i = 0; i < ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count); i++) {

        ICC_Channel_State_t    channel_new_state;

        channel_conf = &(ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Ptr)[i]);

        channel_ram = &ICC_Channels_Ram[i];

        /* close all end points on our side */
        channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] = ICC_ENDPOINT_STATE_CLOSED;

        ICC_DCACHE_FLUSH_MLINES((addr_t)&channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ], sizeof(ICC_Endpoint_State_t) );

        ICC_CHECK_ERR_CODE(ICC_Get_Channel_State( i, &channel_new_state ));

        /*
         * Init TX signal fifo from each side
         */
        ICC_CHECK_ERR_CODE( ICC_Sig_Fifo_Init( (ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ ICC_TX_FIFO ] ) );

        /* signal the channel state transition to REMOTE node */
        ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ ICC_TX_FIFO ], channel_new_state ) );

        #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
        /*
         * Init local signal fifo
         */
        ICC_CHECK_ERR_CODE( ICC_Sig_Fifo_Init( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Ch_Ram_Local[ i ] ) );

        /* signal the channel state transition to LOCAL node */
        ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Ch_Ram_Local[ i ], channel_new_state ) );
        #endif

        /*
         * link each Fifo priority wise
         */
        ICC_CROSS_ASSIGN(channel_ram->fifos_ram[ ICC_TX_FIFO ][ ICC_GET_CORE_ID ], (ICC_Fifo_Ram_t*) &((*ICC_Fifo_Ram)[ channel_conf->fifos_cfg[ ICC_TX_FIFO ].fifo_prio ][ ICC_TX_FIFO ]));
        ICC_CROSS_ASSIGN(channel_ram->fifos_ram[ ICC_RX_FIFO ][ ICC_GET_CORE_ID ], (ICC_Fifo_Ram_t*) &((*ICC_Fifo_Ram)[ channel_conf->fifos_cfg[ ICC_RX_FIFO ].fifo_prio ][ ICC_RX_FIFO ]));

        /* initialize TX fifo */
        ICC_FIFO_Init ( ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[  ICC_TX_FIFO ][ ICC_GET_CORE_ID ]),
                        &channel_conf->fifos_cfg[ ICC_TX_FIFO ],
                        &ICC_Fifo_Os_Ram    [ i ][ ICC_TX_FIFO ],
                         1 );

        /* initialize RX fifo */
        ICC_FIFO_Init ( ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_RX_FIFO ][ ICC_GET_CORE_ID ]),
                        &channel_conf->fifos_cfg[ ICC_RX_FIFO ],
                        &ICC_Fifo_Os_Ram    [ i ][ ICC_RX_FIFO ],
                         0 );


        ICC_DCACHE_FLUSH_MLINES((addr_t)channel_ram, sizeof(ICC_Channel_Ram_t) );

    }

    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
    ICC_CHECK_ERR_CODE( ICC_Sig_Fifo_Init( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Node_Ram_Local ) );
    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Node_Ram_Local , ICC_NODE_STATE_INIT ) );
    #endif

    ICC_CHECK_ERR_CODE( ICC_Sig_Fifo_Init( (ICC_Signal_Fifo_Ram_t *)&((*ICC_Node_Sig_Fifo_Ram)[ ICC_TX_FIFO ]) ) );
    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&((*ICC_Node_Sig_Fifo_Ram)[ ICC_TX_FIFO ]), ICC_NODE_STATE_INIT ) );

    ICC_CHECK_ERR_CODE( ICC_OS_Initialize( ICC_Config_Ptr ) );

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    ICC_Heartbeat_State  = ICC_HEARTBEAT_STATE_UNINIT;
    #endif

    (*ICC_Initialized)[ ICC_GET_CORE_ID ] = ICC_NODE_STATE_INIT;
    ICC_DCACHE_FLUSH_MLINES( (addr_t)&((*ICC_Initialized)[ ICC_GET_CORE_ID ]), sizeof(unsigned int) );

    ICC_CHECK_ERR_CODE( ICC_OS_Init_Interrupts() );

    /* when using interrupts, it's ok to rise an interrupt line and wait for the peer to see it.
     * this does not work on 'pings' (one shot signals) which may block or worse give bus errors
     * when the peer is not initialized.
     */
#ifndef ICC_USE_POLLING

    ICC_Notify_Remote();
#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
    ICC_Notify_Local();
#endif

#endif

    return return_code;
}

/**
 *
 * Called by each core to un-initialize the ICC for the current node.
 *
 * ICC_ERR_NODE_NOT_INIT    - The node was not initialized
 * ICC_ERR_GENERAL          - TBD
 * ICC_SUCCESS              - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t 
ICC_Finalize( void )
{
    int i;
    ICC_Err_t tmp_return_code, return_code = ICC_SUCCESS;
    volatile ICC_Channel_Ram_t * channel_ram;
    ICC_Channel_State_t    channel_new_state;
    ICC_Fifo_Ram_t    * fifo_ram;
    const ICC_Fifo_Config_t * fifo_config;


    if (NULL_PTR == ICC_Config_Ptr) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    (*ICC_Initialized)[ ICC_GET_CORE_ID ] = ICC_NODE_STATE_UNINIT;
    ICC_DCACHE_FLUSH_MLINES( (addr_t)&((*ICC_Initialized)[ ICC_GET_CORE_ID ]), sizeof(unsigned int) );



    for (i = 0; i < ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count); i++) {

        channel_ram = &ICC_Channels_Ram[i];

        /* if channel local endpoint is open */
        if ( ICC_ENDPOINT_STATE_OPEN == channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] )
        {

            /*
             * Signal/wake-up blocked APIs on this channel
             */

#ifndef ICC_CFG_NO_TIMEOUT

            fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_RX_FIFO ][ ICC_GET_CORE_ID ]);
            fifo_config = ICC_CROSS_VALUE_OF(fifo_ram->fifo_config[ ICC_GET_CORE_ID ]);

            if (0 != (fifo_config->fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED)) {
                ICC_CHECK_ERR_CODE( ICC_OS_Set_Event(i, ICC_RX_FIFO) ); /**< wake-up RX blocked task if any */
            }

            fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_TX_FIFO ][ ICC_GET_CORE_ID ]);
            fifo_config = ICC_CROSS_VALUE_OF(fifo_ram->fifo_config[ ICC_GET_CORE_ID ]);

            if (0 != (fifo_config->fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED)) {
                ICC_CHECK_ERR_CODE( ICC_OS_Set_Event(i, ICC_TX_FIFO) ); /**< wake-up TX blocked task if any */
            }

#endif /* no ICC_CFG_NO_TIMEOUT */

            /* endpoint in closed state now */
            channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] = ICC_ENDPOINT_STATE_CLOSED;

            ICC_DCACHE_FLUSH_MLINES((addr_t)&channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ], sizeof(ICC_Endpoint_State_t) );

            ICC_CHECK_ERR_CODE(ICC_Get_Channel_State( i, &channel_new_state ));

            /* signal the channel state transition to REMOTE node */
            ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ ICC_TX_FIFO ], channel_new_state ) );

            #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
            /* signal the channel state transition to LOCAL node */
            ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Ch_Ram_Local[ i ], channel_new_state ) );
            #endif

        }

    }

    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&((*ICC_Node_Sig_Fifo_Ram)[ ICC_TX_FIFO ]), ICC_NODE_STATE_UNINIT ) );
    ICC_Notify_Remote();

    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Node_Ram_Local , ICC_NODE_STATE_UNINIT ) );
    ICC_Notify_Local();
    #endif

    ICC_OS_Finalize();

    ICC_Config_Ptr = NULL_PTR;

    return return_code;
}



/**
 *
 * Get the state of the specified node
 *
 * ICC_ERR_NODE_INVALID     - Invalid node parameter
 * ICC_ERR_GENERAL          - TBD
 * ICC_SUCCESS              - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Get_Node_State(
                     ICC_IN const ICC_Node_t          node,
                     ICC_OUT      ICC_Node_State_t *  node_state
                  )
{

    if ( node >= ICC_NODE_LAST ) {
        return ICC_ERR_NODE_INVALID;
    }

    if (ICC_NODE_LOCAL == node) {
        *node_state = (ICC_Node_State_t)((*ICC_Initialized)[ ICC_GET_CORE_ID ]);
    } else {
        ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&((*ICC_Initialized)[ ICC_GET_REMOTE_CORE_ID ]), sizeof(unsigned int) );
        *node_state = (ICC_Node_State_t)((*ICC_Initialized)[ ICC_GET_REMOTE_CORE_ID ]);
    }

    return ICC_SUCCESS;
}




/**
 *
 * Open caller side of the channel
 *
 * ICC_ERR_NODE_NOT_INIT      - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 * ICC_ERR_CHAN_ALREADY_OPEN  - The channel is already open
 * ICC_ERR_GENERAL            - TBD
 * ICC_SUCCESS                - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Open_Channel(
                 ICC_IN const ICC_Channel_t   channel_id
                )
{

    volatile ICC_Channel_Ram_t *    channel_ram;
    ICC_Channel_State_t    channel_new_state;
    ICC_Err_t tmp_return_code, return_code = ICC_SUCCESS;

    /* check node status */
    if (NULL_PTR == ICC_Config_Ptr) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check valid channel id parameter */
    if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
        return ICC_ERR_PARAM_CHAN_INVALID;
    }

    channel_ram = &ICC_Channels_Ram[ channel_id ];

    /* channel local endpoint is already open */
    if ( ICC_ENDPOINT_STATE_OPEN == channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] ) {
        return ICC_ERR_CHAN_ALREADY_OPEN;
    }

    /* endpoint in open state */
    channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] = ICC_ENDPOINT_STATE_OPEN;

    ICC_DCACHE_FLUSH_MLINES((addr_t)&channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ], sizeof(ICC_Endpoint_State_t) );

    ICC_CHECK_ERR_CODE(ICC_Get_Channel_State( channel_id, &channel_new_state ));

    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
    /* signal the channel state transition to LOCAL node */
    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Ch_Ram_Local[ channel_id ], channel_new_state ) );
    ICC_Notify_Local();
    #endif

    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ ICC_TX_FIFO ], channel_new_state ) );
    ICC_Notify_Remote();

    return return_code;
}




/**
 *
 * Close caller side of the channel
 *
 * ICC_ERR_NODE_NOT_INIT       - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID  - The specified channel is not valid
 * ICC_ERR_CHAN_ALREADY_CLOSED - The channel is already closed
 * ICC_ERR_GENERAL             - TBD
 * ICC_SUCCESS                 - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Close_Channel(
                  ICC_IN const ICC_Channel_t   channel_id
                 )
{
    volatile ICC_Channel_Ram_t *    channel_ram;
    ICC_Channel_State_t    channel_new_state;
    ICC_Err_t tmp_return_code, return_code = ICC_SUCCESS;

    /* check node status */
    if (NULL_PTR == ICC_Config_Ptr) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check valid channel id parameter */
    if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
        return ICC_ERR_PARAM_CHAN_INVALID;
    }

    channel_ram = &ICC_Channels_Ram[ channel_id ];

    /* channel local endpoint is already open */
    if ( ICC_ENDPOINT_STATE_CLOSED == channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] ) {
        return ICC_ERR_CHAN_ALREADY_CLOSED;
    }

    /* endpoint in closed state */
    channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] = ICC_ENDPOINT_STATE_CLOSED;

    ICC_DCACHE_FLUSH_MLINES((addr_t)&channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ], sizeof(ICC_Endpoint_State_t) );

    ICC_CHECK_ERR_CODE(ICC_Get_Channel_State( channel_id, &channel_new_state ));


    #if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
    /* signal the channel state transition to LOCAL node */
    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Ch_Ram_Local[ channel_id ], channel_new_state ) );
    ICC_Notify_Local();
    #endif

    ICC_CHECK_ERR_CODE_NO_RETURN( ICC_Sig_Fifo_Signal( (ICC_Signal_Fifo_Ram_t *)&channel_ram->sig_fifo_remote[ ICC_TX_FIFO ], channel_new_state ) );
    ICC_Notify_Remote();

    return return_code;
}


/**
 *
 * Interrogate state of caller side of specified channel
 *
 * ICC_ERR_NODE_NOT_INIT      - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 * ICC_ERR_GENERAL            - TBD
 * ICC_SUCCESS                - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Get_Channel_State(
                       ICC_IN const ICC_Channel_t         channel_id,
                       ICC_OUT      ICC_Channel_State_t * channel_state
                     )
{
    ICC_Endpoint_State_t   remote_endpoint_state;
    unsigned int           remote_node_state;

    volatile ICC_Channel_Ram_t *    channel_ram;

    /* check node status */
    if (NULL_PTR == ICC_Config_Ptr) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check valid channel id parameter */
    if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
        return ICC_ERR_PARAM_CHAN_INVALID;
    }

    channel_ram = &ICC_Channels_Ram[ channel_id ];

    ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&((*ICC_Initialized)[ ICC_GET_REMOTE_CORE_ID ]), sizeof(unsigned int) );
    remote_node_state = (ICC_Node_State_t)((*ICC_Initialized)[ ICC_GET_REMOTE_CORE_ID ]);

    if (ICC_NODE_STATE_UNINIT == remote_node_state) {

        /* this means all remote endpoints are closed */
        remote_endpoint_state = ICC_ENDPOINT_STATE_CLOSED;
    } else {

        ICC_DCACHE_INVALIDATE_MLINES((addr_t)&channel_ram->endpoint_state[ ICC_REMOTE_ENDPOINT ], sizeof(ICC_Endpoint_State_t) );
        remote_endpoint_state = channel_ram->endpoint_state[ ICC_REMOTE_ENDPOINT ];
    }

    * channel_state = (ICC_Channel_State_t)( ((unsigned int)remote_endpoint_state << 1U) | (unsigned int) channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ]);

    return ICC_SUCCESS;
}




/**
 *
 * Get the amount of free space available for the channel
 *
 * ICC_ERR_NODE_NOT_INIT      - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 * ICC_ERR_GENERAL            - TBD
 * ICC_SUCCESS                - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Get_Channel_Free(
                     ICC_IN const ICC_Channel_t          channel_id,
                     ICC_OUT      unsigned int         * channel_free_bytes
                    )
{

        volatile ICC_Channel_Ram_t * channel_ram;
        ICC_Fifo_Ram_t        * fifo_ram;


        /* check node status */
        if (NULL_PTR == ICC_Config_Ptr) {
            return ICC_ERR_NODE_NOT_INIT;
        }

        /* check valid channel id parameter */
        if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
            return ICC_ERR_PARAM_CHAN_INVALID;
        }

        channel_ram = &ICC_Channels_Ram[ channel_id ];

        fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_TX_FIFO ][ ICC_GET_CORE_ID ]);

        /* Subtract the size of the header */
        * channel_free_bytes = ICC_FIFO_Free(fifo_ram);
        if (* channel_free_bytes <= ICC_HEADER_SIZE)
            * channel_free_bytes = 0;
        else
            * channel_free_bytes -= ICC_HEADER_SIZE;

        return ICC_SUCCESS;

}

/**
 *
 * Get the amount of pending bytes for the channel
 *
 * ICC_ERR_NODE_NOT_INIT      - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 * ICC_ERR_GENERAL            - TBD
 * ICC_SUCCESS                - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Get_Channel_Pending(
                        ICC_IN const ICC_Channel_t          channel_id,
                        ICC_OUT      unsigned int         * channel_pending_bytes
                       )
{

        volatile ICC_Channel_Ram_t * channel_ram;
        ICC_Fifo_Ram_t        * fifo_ram;


        /* check node status */
        if (NULL_PTR == ICC_Config_Ptr) {
            return ICC_ERR_NODE_NOT_INIT;
        }

        /* check valid channel id parameter */
        if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)){
            return ICC_ERR_PARAM_CHAN_INVALID;
        }

        channel_ram = &ICC_Channels_Ram[ channel_id ];

        fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_TX_FIFO ][ ICC_GET_CORE_ID ]);

        * channel_pending_bytes = ICC_FIFO_Pending(fifo_ram);

        return ICC_SUCCESS;

}


/**
 *
 * Get the next pending message size
 *
 * ICC_ERR_NODE_NOT_INIT      - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 * ICC_ERR_GENERAL            - TBD
 * ICC_ERR_EMPTY              - no message pending
 * ICC_SUCCESS                - OK
 *
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Get_Next_Msg_Size(
                      ICC_IN const ICC_Channel_t          channel_id,
                      ICC_OUT      unsigned int         * next_msg_size
                     )
{
        ICC_Err_t return_code;
        volatile ICC_Channel_Ram_t * channel_ram;
        ICC_Fifo_Ram_t        * fifo_ram;

        /* check node status */
        if (NULL_PTR == ICC_Config_Ptr) {
            return ICC_ERR_NODE_NOT_INIT;
        }

        /* check valid channel id parameter */
        if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
            return ICC_ERR_PARAM_CHAN_INVALID;
        }

        channel_ram = &ICC_Channels_Ram[ channel_id ];

        fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_TX_FIFO ][ ICC_GET_CORE_ID ]);

        ICC_CHECK_ERR_CODE( ICC_FIFO_Peek_Header(fifo_ram, next_msg_size) );

        return ICC_SUCCESS;

}




/**
 *
 * Message Send: immediate / timeout / block forever
 *
 *   timeout = ICC_WAIT_ZERO                    - immediate, zero wait
 *   ICC_WAIT_ZERO < timeout < ICC_WAIT_FOREVER - in micro seconds = the timeout value
 *   timeout = ICC_WAIT_FOREVER                 - block/wait forever
 *
 * possible cases :
 *
 *   ICC_ERR_NODE_NOT_INIT  - The node is not initialized.
 *
 *   ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 *   ICC_ERR_PARAM_BUFF_NULL    - Invalid buffer ptr parameter
 *   ICC_ERR_PARAM_BUFF_SIZE    - Invalid buffer size
 *   ICC_ERR_PARAM_TIMEOUT      - Invalid timeout value
 *
 *   ICC_ERR_GENERAL       - TBD, other error
 *
 *   ICC_ERR_MSG_SIZE      - The message size exceeds the maximum size configured for this channel
 *   ICC_ERR_TIMEOUT       - timed out, after waiting for timeout_val (us) the message was not send as no free buffer was available
 *
 *   ICC_ERR_FULL          - message not transmitted, the TX fifo is full or not enough space is available
 *   ICC_ERR_CHAN_ENDPOINT_CLOSED - the channel is closed on the side the operation is performed
 *   ICC_ERR_NODE_DEINITIALIZED - The node was deinitialized while the blocking call was pending.
 *   ICC_SUCCESS           - OK, message send
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Msg_Send (
              ICC_IN const ICC_Channel_t  channel_id,
              ICC_IN const void        *  tx_user_buffer,
              ICC_IN const ICC_Msg_Size_t tx_user_buffer_size,
              ICC_IN const ICC_Timeout_t  timeout_val
             )
{
    ICC_Err_t return_code;
    volatile ICC_Channel_Ram_t * channel_ram;
    ICC_Fifo_Ram_t    * fifo_ram;

    const ICC_Fifo_Config_t * fifo_config;

    ICC_Header_t msg_header;

    /* check node status */
    if (NULL_PTR == ICC_Config_Ptr) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check valid channel id parameter */
    if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
        return ICC_ERR_PARAM_CHAN_INVALID;
    }

    if (tx_user_buffer == NULL_PTR){
        return ICC_ERR_PARAM_BUFF_NULL;
    }

    if (tx_user_buffer_size == 0) {
        return ICC_ERR_PARAM_BUFF_SIZE;
    }

    if ((timeout_val != ICC_WAIT_FOREVER) && (timeout_val > ICC_CFG_MAX_TIMEOUT)) {
        return ICC_ERR_PARAM_TIMEOUT;
    }

    channel_ram = &ICC_Channels_Ram[ channel_id ];

    /* check that the channel end point is in the open state */
    if (channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] != ICC_ENDPOINT_STATE_OPEN) {
        return ICC_ERR_CHAN_ENDPOINT_CLOSED;
    }

    fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_TX_FIFO ][ ICC_GET_CORE_ID ]);

    fifo_config = ICC_CROSS_VALUE_OF(fifo_ram->fifo_config[ ICC_GET_CORE_ID ]);

    /* check the message is not larger than maximum configured for this channel */
    if (tx_user_buffer_size > fifo_config->max_msg_size) {
        return ICC_ERR_MSG_SIZE;
    }

    if ( (0 == (fifo_config->fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED)) && (ICC_WAIT_ZERO != timeout_val)) {
        return ICC_ERR_TIMEOUT_NOT_CONF;
    }

    if (ICC_SUCCESS != ICC_Fifo_Msg_Fits(fifo_ram, tx_user_buffer_size)) {

        fifo_ram->pending_send_msg_size = tx_user_buffer_size; /**< set pending size */

        if (ICC_WAIT_ZERO == timeout_val) {

            return ICC_ERR_FULL;
        }

#ifndef ICC_CFG_NO_TIMEOUT

        if (ICC_WAIT_FOREVER == timeout_val) { /* wait forever */

            do {

                ICC_CHECK_ERR_CODE( ICC_OS_Wait_Event( channel_id, ICC_TX_FIFO ) );

                ICC_CHECK_ERR_CODE( ICC_OS_Clear_Event(channel_id, ICC_TX_FIFO, ICC_EVENT_ACTIVITY_ISR) );

                if ( ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_CORE_ID ] ) {
                    return ICC_ERR_NODE_DEINITIALIZED; /**< node deinitialized while waiting */
                }

            } while (ICC_SUCCESS != ICC_Fifo_Msg_Fits(fifo_ram, tx_user_buffer_size));

        }


        else { /* wait timeout */

            ICC_Event_t event;

            ICC_CHECK_ERR_CODE( ICC_OS_Set_Rel_Alarm( channel_id, ICC_TX_FIFO, timeout_val ) );

            do {

                ICC_CHECK_ERR_CODE( ICC_OS_Wait_Event( channel_id, ICC_TX_FIFO ) );

                if ( ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_CORE_ID ] ){

                    ICC_CHECK_ERR_CODE( ICC_OS_Cancel_Alarm( channel_id, ICC_TX_FIFO ) );

                    return ICC_ERR_NODE_DEINITIALIZED; /**< node deinitialized while waiting */

                }

                ICC_CHECK_ERR_CODE( ICC_OS_Get_Event( channel_id, ICC_TX_FIFO, &event ) );

                ICC_CHECK_ERR_CODE( ICC_OS_Clear_Event(channel_id, ICC_TX_FIFO, event) );

                if (event == ICC_EVENT_TIMEOUT_ALM) {
                    return ICC_ERR_TIMEOUT; /**< alarm event arrived first : TIMEOUT */
                }

            } while (ICC_SUCCESS != ICC_Fifo_Msg_Fits(fifo_ram, tx_user_buffer_size));

            ICC_CHECK_ERR_CODE( ICC_OS_Cancel_Alarm( channel_id, ICC_TX_FIFO ) );
        } /* end timeout */

#endif /* no ICC_CFG_NO_TIMEOUT */

        fifo_ram->pending_send_msg_size = 0; /**< reset pending */

    }


    /* at this point there is enough space available */

    ICC_CHECK_ERR_CODE( ICC_OS_Get_Semaphore( channel_id, ICC_TX_FIFO ) );

       msg_header = tx_user_buffer_size; /* build header */

       return_code = ICC_FIFO_Push_Header( fifo_ram, &msg_header );
       if (ICC_SUCCESS != return_code) {

           /* release semaphore before we return with error code */
           ICC_OS_Release_Semaphore( channel_id, ICC_TX_FIFO );
           return return_code;
       }

       return_code = ICC_FIFO_Push( fifo_ram, tx_user_buffer, tx_user_buffer_size );
       if (ICC_SUCCESS != return_code) {
           /* release semaphore before we return with error code */
           ICC_OS_Release_Semaphore( channel_id, ICC_TX_FIFO );
           return return_code;
       }

       ICC_FIFO_Msg_Wr_Sig(fifo_ram); /**< signal new message */


    ICC_Notify_Remote();
    ICC_CHECK_ERR_CODE( ICC_OS_Release_Semaphore( channel_id, ICC_TX_FIFO  ) );


    return ICC_SUCCESS;
}


/**
 *
 * Message Receive: immediate / timeout / block forever
 *
 *   timeout = ICC_WAIT_ZERO                    - immediate, zero wait
 *   ICC_WAIT_ZERO < timeout < ICC_WAIT_FOREVER - in micro seconds = the timeout value
 *   timeout = ICC_WAIT_FOREVER                 - block/wait forever
 *
 * possible cases :
 *
 *   ICC_ERR_NODE_NOT_INIT        - The node is not initialized.
 *
 *   ICC_ERR_PARAM_CHAN_INVALID   - The specified channel is not valid
 *   ICC_ERR_PARAM_BUFF_NULL      - Invalid buffer ptr parameter
 *   ICC_ERR_PARAM_BUFF_SIZE      - Invalid buffer size
 *   ICC_ERR_PARAM_TIMEOUT        - Invalid timeout value
 *   ICC_ERR_PARAM_RX_OP          - Invalid RX operation parameter
 *
 *   ICC_ERR_GENERAL              - TBD, other error
 *
 *   ICC_ERR_EMPTY                - no message pending, rx fifo empty, rx_msg_size set to 0
 *   ICC_ERR_MSG_TRUNCATED        - message received but truncated as rx_user_buffer_size < actual message size, rx_msg_size = rx_user_buffer_size.
 *   ICC_ERR_TIMEOUT              - timed out, after waiting for timeout_val (us) no message was received
 *   ICC_ERR_CHAN_ENDPOINT_CLOSED - the channel is closed on the side the operation is performed
 *   ICC_ERR_NODE_DEINITIALIZED   - The node was deinitialized while the blocking call was pending.
 *   ICC_SUCCESS                  - OK, message received, rx_msg_size <= rx_user_buffer_size
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Msg_Recv(
              ICC_IN  const ICC_Channel_t    channel_id,
              ICC_IN  void                 * rx_user_buffer,        /**< the RX buffer used to store the received message */
              ICC_IN  const ICC_Msg_Size_t   rx_user_buffer_size,   /**< the RX buffer size */
              ICC_OUT       ICC_Msg_Size_t * rx_msg_size,           /**< the actual size of the received message */
              ICC_IN  const ICC_Timeout_t    timeout_val,           /**< the time out value */
              ICC_IN  const ICC_Rx_Op_t      rx_operation           /**< the type of Rx operation performed */
            )
{
    ICC_Err_t return_code;
    volatile ICC_Channel_Ram_t * channel_ram;
    ICC_Fifo_Ram_t * fifo_ram;

    const ICC_Fifo_Config_t * fifo_config;

    ICC_Header_t msg_header;

    /* check node status */
    if (NULL_PTR == ICC_Config_Ptr) {

        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check valid channel id parameter */
    if (channel_id >= ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count)) {
        return ICC_ERR_PARAM_CHAN_INVALID;
    }

    if (rx_user_buffer == NULL_PTR) {
        return ICC_ERR_PARAM_BUFF_NULL;
    }

    if (rx_user_buffer_size == 0) {
        return ICC_ERR_PARAM_BUFF_SIZE;
    }

    if ((timeout_val != ICC_WAIT_FOREVER) && (timeout_val > ICC_CFG_MAX_TIMEOUT)) {
        return ICC_ERR_PARAM_TIMEOUT;
    }

    if ((rx_operation != ICC_RX_NORMAL) && (rx_operation != ICC_RX_PEEK_MSG)) {
        return ICC_ERR_PARAM_RX_OP;
    }

    channel_ram = &ICC_Channels_Ram[ channel_id ];

    /* check that the channel end point is in the open state */
    if (channel_ram->endpoint_state[ ICC_LOCAL_ENDPOINT ] != ICC_ENDPOINT_STATE_OPEN) {
        return ICC_ERR_CHAN_ENDPOINT_CLOSED;
    }

    fifo_ram = ICC_CROSS_VALUE_OF(channel_ram->fifos_ram[ ICC_RX_FIFO ][ ICC_GET_CORE_ID ]);

    fifo_config = ICC_CROSS_VALUE_OF(fifo_ram->fifo_config[ ICC_GET_CORE_ID ]);

    if ( (0 == (fifo_config->fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED)) && (ICC_WAIT_ZERO != timeout_val)) {
        return ICC_ERR_TIMEOUT_NOT_CONF;
    }

    if ( ICC_FIFO_Pending(fifo_ram) <= ICC_HEADER_SIZE )
    {
        if (ICC_WAIT_ZERO == timeout_val) {

            return ICC_ERR_EMPTY; /**< no message pending */
        }

#ifndef ICC_CFG_NO_TIMEOUT

        if (ICC_WAIT_FOREVER == timeout_val) {   /* wait forever */

            do {
                ICC_CHECK_ERR_CODE( ICC_OS_Wait_Event( channel_id, ICC_RX_FIFO ) );

                ICC_CHECK_ERR_CODE( ICC_OS_Clear_Event(channel_id, ICC_RX_FIFO, ICC_EVENT_ACTIVITY_ISR) );

                if (ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_CORE_ID ]) {
                    return ICC_ERR_NODE_DEINITIALIZED; /**< node deinitialized while waiting */
                }

            } while ( ICC_FIFO_Pending(fifo_ram) <= ICC_HEADER_SIZE );

        }
        else { /* wait timeout */

            ICC_Event_t event;

            ICC_CHECK_ERR_CODE( ICC_OS_Set_Rel_Alarm( channel_id, ICC_RX_FIFO, timeout_val ) );

            do {
                ICC_CHECK_ERR_CODE( ICC_OS_Wait_Event( channel_id, ICC_RX_FIFO ) );

                if (ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_CORE_ID ]) {

                    ICC_CHECK_ERR_CODE( ICC_OS_Cancel_Alarm( channel_id, ICC_RX_FIFO ) );

                    return ICC_ERR_NODE_DEINITIALIZED; /**< node deinitialized while waiting */

                }

                ICC_CHECK_ERR_CODE( ICC_OS_Get_Event( channel_id, ICC_RX_FIFO, &event ) );

                ICC_CHECK_ERR_CODE( ICC_OS_Clear_Event(channel_id, ICC_RX_FIFO, event) );

                if (event == ICC_EVENT_TIMEOUT_ALM) {

                    return ICC_ERR_TIMEOUT; /**< alarm event arrived first : TIMEOUT */
                }

            } while ( ICC_FIFO_Pending(fifo_ram) <= ICC_HEADER_SIZE );

            ICC_CHECK_ERR_CODE( ICC_OS_Cancel_Alarm( channel_id, ICC_RX_FIFO ) );

        } /* end timeout */
#endif /* no ICC_CFG_NO_TIMEOUT */

    }

    /* at this point there is a message pending */

    ICC_CHECK_ERR_CODE( ICC_OS_Get_Semaphore( channel_id, ICC_RX_FIFO ) );

        if ( ICC_RX_PEEK_MSG == rx_operation ) {

            return_code = ICC_FIFO_Peek_Header(fifo_ram, &msg_header);

            if (ICC_SUCCESS != return_code)    {

                /* release semaphore before we return with error code */
                ICC_OS_Release_Semaphore( channel_id, ICC_RX_FIFO );
                return return_code;
            }

            * rx_msg_size = ( msg_header <= rx_user_buffer_size ) ? msg_header : rx_user_buffer_size;

            return_code = ICC_FIFO_Peek( fifo_ram, rx_user_buffer, (* rx_msg_size)+4 );

        } else {

            return_code = ICC_FIFO_Pop_Header(fifo_ram, &msg_header);

            if (ICC_SUCCESS != return_code) {

                /* release semaphore before we return with error code */
                ICC_OS_Release_Semaphore( channel_id, ICC_RX_FIFO );
                return return_code;
            }

            * rx_msg_size = ( msg_header <= rx_user_buffer_size ) ? msg_header : rx_user_buffer_size;

            return_code = ICC_FIFO_Pop( fifo_ram, rx_user_buffer, * rx_msg_size );

            if (msg_header > rx_user_buffer_size) {

                /*
                 * dump remaining message content,
                 * as the rest of the message sits in the buffer without a header
                 */
                return_code = ICC_FIFO_Pop( fifo_ram, NULL_PTR, msg_header - *rx_msg_size );
            }

        }


        if (ICC_SUCCESS != return_code) {

            /* release semaphore before we return with error code */
            ICC_OS_Release_Semaphore( channel_id, ICC_RX_FIFO );
            return return_code;
        }


        if ( ICC_RX_NORMAL == rx_operation ) {

            ICC_FIFO_Msg_Rd_Sig(fifo_ram); /**< signal only if message extracted from fifo */
            ICC_Notify_Remote();
        }


        ICC_CHECK_ERR_CODE( ICC_OS_Release_Semaphore( channel_id, ICC_RX_FIFO  ) );


        if (msg_header > rx_user_buffer_size) {
            return ICC_ERR_MSG_TRUNCATED;
        }

        return ICC_SUCCESS;

}


/*
 * Handle ICC events from local and remote core
 * Called from OS specific ICC interrupt handler
 */

ICC_ATTR_SEC_TEXT_CODE
void
ICC_Remote_Event_Handler(void)
{
    int i;
    volatile ICC_Signal_Fifo_Ram_t * sig_fifo_remote_ptr;

    #ifdef ICC_CFG_HEARTBEAT_ENABLED
    const ICC_Heartbeat_Os_Config_t * HB_OS_config;
    unsigned int hb_channel_id;
    #endif
    
    ICC_Clear_Notify_Remote();


    /*
     * if interrupts are received with node not initialized
     */
    if ((NULL_PTR==ICC_Config_Ptr) || (ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_CORE_ID ] )){
        return;
    }


    /* FIFOs events available ? */
    if ( 1 /* TBD */ )
    {

        #ifdef ICC_CFG_HEARTBEAT_ENABLED
        HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);
        hb_channel_id = HB_OS_config->channel_id;
        #endif


        /*
         * dequeue events in FIFO priority order
         */
        for (i = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count) - 1; i >= 0; i--)
        {
            const ICC_Channel_Config_t  * channel_cfg  = &ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Ptr)[i];
            ICC_Fifo_Ram_t        * fifo_ram;

            #ifdef ICC_CFG_HEARTBEAT_ENABLED
            if ((i == hb_channel_id) && ((ICC_Heartbeat_State == ICC_HEARTBEAT_STATE_ERROR) || (ICC_Heartbeat_State == ICC_HEARTBEAT_STATE_UNINIT))) continue; /**< skip HeartBeat channel if HB not initialized */
            #endif

            /* RX fifo */

            fifo_ram = (ICC_Fifo_Ram_t *)&((*ICC_Fifo_Ram)[i][ ICC_RX_FIFO ]);

            while ( ICC_FIFO_Msg_Wr_Sig_Pending( fifo_ram ) )
            {
                if (NULL_PTR != ICC_CROSS_VALUE_OF(channel_cfg->Channel_Rx_Cb)) {
                    ICC_CROSS_VALUE_OF(channel_cfg->Channel_Rx_Cb)(i);
                }

                #ifndef ICC_CFG_NO_TIMEOUT
                if ( channel_cfg->fifos_cfg[ ICC_RX_FIFO ].fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED ) ICC_OS_Set_Event(i, ICC_RX_FIFO); /**< wake-up RX blocked task */
                #endif   /* not ICC_CFG_NO_TIMEOUT */

                ICC_FIFO_Msg_Wr_Ack( fifo_ram ); /**< Ack WR notification */
            }

            /* TX fifo */

            fifo_ram = (ICC_Fifo_Ram_t *) &((*ICC_Fifo_Ram)[i][ ICC_TX_FIFO ]);

            while ( ICC_FIFO_Msg_Rd_Sig_Pending( fifo_ram ) )
            {
                if (( 0 != fifo_ram->pending_send_msg_size ) && ( ICC_SUCCESS == ICC_Fifo_Msg_Fits( (ICC_Fifo_Ram_t *) fifo_ram, fifo_ram->pending_send_msg_size ) ) ) {

                    if (NULL_PTR != ICC_CROSS_VALUE_OF(channel_cfg->Channel_Tx_Cb)) {
                        ICC_CROSS_VALUE_OF(channel_cfg->Channel_Tx_Cb)(i);
                    }

                    #ifndef ICC_CFG_NO_TIMEOUT
                    if ( channel_cfg->fifos_cfg[ ICC_TX_FIFO ].fifo_flags & ICC_FIFO_FLAG_TIMEOUT_ENABLED ) ICC_OS_Set_Event(i, ICC_TX_FIFO); /**< wake-up TX blocked task */
                    #endif /* not ICC_CFG_NO_TIMEOUT */

                }

                ICC_FIFO_Msg_Rd_Ack( fifo_ram ); /**< Ack RD notification */
            }

        } /* end for channels */

    } /* end fifo events */


    /* Channels event available ? */
    if ( 1 /* TBD */ )
    {
        for (i = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Count) - 1; i >= 0; i--)
        {
            const ICC_Channel_Config_t  * channel_cfg  = &ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Channels_Ptr)[i];
            volatile ICC_Channel_Ram_t     * channel_ram  = &ICC_Channels_Ram[i];
            ICC_Signal_t            signal;
            sig_fifo_remote_ptr = &channel_ram->sig_fifo_remote[ ICC_GET_REMOTE_CORE_ID ];

            while(!ICC_Sig_Fifo_Empty( (ICC_Signal_Fifo_Ram_t *) sig_fifo_remote_ptr)) {

                if ( ICC_SUCCESS == ICC_Sig_Fifo_Dequeue_Signal( (ICC_Signal_Fifo_Ram_t *) sig_fifo_remote_ptr, &signal )) {

                    if (NULL_PTR != ICC_CROSS_VALUE_OF(channel_cfg->Channel_Update_Cb)) {
                        ICC_CROSS_VALUE_OF(channel_cfg->Channel_Update_Cb)(i, (ICC_Channel_State_t) signal);
                    }
                }

            }

        }

    }

    sig_fifo_remote_ptr = &(*ICC_Node_Sig_Fifo_Ram)[ ICC_GET_REMOTE_CORE_ID ];


    /* Node event ? */
    while ( !ICC_Sig_Fifo_Empty( (ICC_Signal_Fifo_Ram_t *)  sig_fifo_remote_ptr ) )
    {

        ICC_Signal_t            signal;


        if ( ICC_SUCCESS == ICC_Sig_Fifo_Dequeue_Signal( (ICC_Signal_Fifo_Ram_t *) sig_fifo_remote_ptr, &signal )) {

            if (NULL_PTR != ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Node_Update_Cb)) {
                ICC_CROSS_VALUE_OF(ICC_Config_Ptr->Node_Update_Cb)( ICC_NODE_REMOTE, (ICC_Node_State_t) signal);
            }
        }

    }


}  /* end ICC_Remote_Event_Handler */


#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

ICC_ATTR_SEC_TEXT_CODE
void
ICC_Local_Event_Handler(void)
{
    int i;
    ICC_Signal_t  signal;
    volatile ICC_Signal_Fifo_Ram_t * sig_fifo_local_ptr;
    const    ICC_Channel_Config_t  * channel_cfg;

    ICC_Clear_Notify_Local();

    if ((NULL_PTR==ICC_Config_Ptr) || (ICC_NODE_STATE_UNINIT == (*ICC_Initialized)[ ICC_GET_CORE_ID ] )) {
        return;
    }

    /* handle channel local events */
    for (i = ICC_Config_Ptr->Channels_Count-1; i >= 0; i--)
    {
        sig_fifo_local_ptr = &ICC_Signal_Fifo_Ch_Ram_Local[ i ];
        channel_cfg        = &ICC_Config_Ptr->Channels_Ptr[i];

        while(!ICC_Sig_Fifo_Empty((ICC_Signal_Fifo_Ram_t *)sig_fifo_local_ptr)) {
            if ( ICC_SUCCESS == ICC_Sig_Fifo_Dequeue_Signal( (ICC_Signal_Fifo_Ram_t *) sig_fifo_local_ptr, &signal )) {

                if (NULL_PTR != channel_cfg->Channel_Update_Cb) {
                    channel_cfg->Channel_Update_Cb(i, (ICC_Channel_State_t) signal);
                }
            }

        }

    }


    /* handle node local events */
    while ( !ICC_Sig_Fifo_Empty( (ICC_Signal_Fifo_Ram_t *)&ICC_Signal_Fifo_Node_Ram_Local ) )
    {
        if ( ICC_SUCCESS == ICC_Sig_Fifo_Dequeue_Signal( (ICC_Signal_Fifo_Ram_t *) &ICC_Signal_Fifo_Node_Ram_Local, &signal )) {

            if (NULL_PTR != ICC_Config_Ptr->Node_Update_Cb) {
                ICC_Config_Ptr->Node_Update_Cb( ICC_NODE_LOCAL, (ICC_Node_State_t)signal);
            }
        }
    }

}

#endif



#ifdef ICC_CFG_HEARTBEAT_ENABLED

/**
 *
 * Called by each core to obtain the Heartbeat mechanism state.
 *
 * ICC_ERR_GENERAL          - TBD
 * ICC_SUCCESS              - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Heartbeat_Get_State( ICC_Heartbeat_State_t * state )
{
    * state = (ICC_Heartbeat_State_t)ICC_Heartbeat_State;

    return ICC_SUCCESS;
}



/**
 *
 * Called by each core to initialize the Heartbeat mechanism.
 *
 * ICC_ERR_NODE_NOT_INIT           - Local node is not initialized
 * ICC_ERR_HEARTBEAT_INITIALIZED   - ICC Heartbeat mechanism was already initialized
 * ICC_ERR_HEARTBEAT_RUNNING       - ICC Heartbeat mechanism was already running
 * ICC_ERR_GENERAL                 - TBD
 * ICC_SUCCESS                     - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Heartbeat_Initialize(
                          ICC_IN unsigned int runId   /**< the current runId */
                        )
{
    ICC_Err_t             return_code;
    ICC_Err_t             err_code;

    ICC_Node_State_t      node_state;
    ICC_Heartbeat_State_t crt_state;

    typedef struct {
        ICC_Header_t        header;
        ICC_Heartbeat_Msg_t actual_rcv_msg;
    } peek_msg_t;

    peek_msg_t          rcv_msg_p_header;

    ICC_Heartbeat_Msg_t rcv_msg;

    ICC_Msg_Size_t      actual_size;

    const unsigned int hb_msg_size = sizeof(ICC_Heartbeat_Msg_t);

    const ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);

    ICC_Heartbeat_RunId = runId; /**< set the current runId */

    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Clear_Rate_Event( HB_OS_config->channel_id, ICC_TX_FIFO ) );          /**< clear Rate event */

    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Clear_Event( HB_OS_config->channel_id, ICC_TX_FIFO, ICC_EVENT_ACTIVITY_ISR | ICC_EVENT_TIMEOUT_ALM )); /**< clear TX event */
    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Clear_Event( HB_OS_config->channel_id, ICC_RX_FIFO, ICC_EVENT_ACTIVITY_ISR | ICC_EVENT_TIMEOUT_ALM )); /**< clear RX event */


    /* check if the local endpoint was initialized */
    ICC_CHECK_ERR_CODE( ICC_Get_Node_State( ICC_NODE_LOCAL, &node_state ) );
    if( ICC_NODE_STATE_INIT != node_state ) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check if the Heartbeat mechanism is in other state than UNINIT */
    ICC_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );
    if( crt_state == ICC_HEARTBEAT_STATE_INIT ) {
        return ICC_ERR_HEARTBEAT_INITIALIZED;
    }

    if( crt_state == ICC_HEARTBEAT_STATE_RUNNING ) {
            return ICC_ERR_HEARTBEAT_RUNNING;
    }

    /* Open the assigned Heartbeat channel */
    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Open_Channel( HB_OS_config->channel_id ) );


    /* in order to re-synchronize we need to remove messages send by the other node in a previous run, if any */
    do {

        err_code = ICC_Msg_Recv( HB_OS_config->channel_id, &rcv_msg_p_header, hb_msg_size,  &actual_size, ICC_WAIT_ZERO, ICC_RX_PEEK_MSG );

        if ((err_code != ICC_SUCCESS) && (err_code != ICC_ERR_EMPTY))
        {
            return err_code;
        } else {

            if (err_code == ICC_ERR_EMPTY) {

                break; /**< no message available */
            } else {

                if (rcv_msg_p_header.actual_rcv_msg.run != ICC_Heartbeat_RunId)
                {
                    /* a message from the previous run must be removed */
                    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Msg_Recv( HB_OS_config->channel_id, &rcv_msg, hb_msg_size,  &actual_size, ICC_WAIT_ZERO, ICC_RX_NORMAL ) );
                } else {
                    /* it's the starting message - nothing to do */
                    break;
                }
            }
        }

    } while(1);


    ICC_Heartbeat_State = ICC_HEARTBEAT_STATE_INIT;

    return ICC_SUCCESS;
}


/**
 *
 * Called by each core to stop Heartbeat mechanism. This function has to be called only from
 * the reserved task for stopping mechanism.
 *
 * ICC_ERR_NODE_NOT_INIT        - Local node is not initialized
 * ICC_ERR_HEARTBEAT_NOT_INIT   - ICC Heartbeat mechanism is not initialized.
 * ICC_ERR_GENERAL              - TBD
 * ICC_SUCCESS                  - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Heartbeat_Finalize( void )
{
    ICC_Err_t return_code;
    ICC_Node_State_t node_state;
    ICC_Heartbeat_State_t crt_state;
    const ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);

    /* check if the local endpoint was initialized */
    ICC_CHECK_ERR_CODE( ICC_Get_Node_State( ICC_NODE_LOCAL, &node_state ) );
    if( ICC_NODE_STATE_INIT != node_state ) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check if the Heartbeat mechanism is initialized */
    ICC_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );
    if( crt_state == ICC_HEARTBEAT_STATE_UNINIT ) {
        return ICC_ERR_HEARTBEAT_NOT_INIT;
    }

    ICC_Heartbeat_State = ICC_HEARTBEAT_STATE_UNINIT;

    if (( crt_state == ICC_HEARTBEAT_STATE_RUNNING ) || ( crt_state == ICC_HEARTBEAT_STATE_ERROR)) {

        ICC_OS_Cancel_Recurrent_Alarm(  HB_OS_config->channel_id, ICC_TX_FIFO );  /**<  Cancel the alarm for HEARTBEAT send task*/

        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Set_Rate_Event(  HB_OS_config->channel_id, ICC_TX_FIFO ) );          /**< wake-up Rate send blocked task if any */

        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Set_Event( HB_OS_config->channel_id, ICC_TX_FIFO ) );                /**< wake-up TX blocked task if any */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Set_Event( HB_OS_config->channel_id, ICC_RX_FIFO ) );                /**< wake-up RX blocked task if any */

    }
    /* Close the assigned Heartbeat channel  */
    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Close_Channel(  HB_OS_config->channel_id ) );

    return ICC_SUCCESS;
}



/**
 *
 * Called by each core to run Heartbeat mechanism.
 *
 * ICC_ERR_NODE_NOT_INIT        - Local node is not initialized
 * ICC_ERR_HEARTBEAT_NOT_INIT    - ICC Heartbeat mechanism is not initialized.
 * ICC_ERR_HEARTBEAT_RUNNIG     - ICC Heartbeat mechanism is already running
 * ICC_ERR_GENERAL              - TBD
 * ICC_SUCCESS                  - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Heartbeat_Runnable( void )
{
    ICC_Err_t return_code, err_code;
    ICC_Node_State_t node_state;
    ICC_Heartbeat_State_t crt_state;
    const ICC_Heartbeat_Os_Config_t * HB_OS_config = ICC_CROSS_VALUE_OF(ICC_Config_Ptr->ICC_Heartbeat_Os_Config);

    const unsigned int hb_msg_size = sizeof(ICC_Heartbeat_Msg_t);
    ICC_Msg_Size_t  actual_size;

    ICC_Heartbeat_Msg_t rcv_msg, crt_msg;

    crt_msg.run = ICC_Heartbeat_RunId;
    crt_msg.val = ICC_CFG_HEARTBEAT_STARTING_MSG;

    /* check if the local endpoint was initialized */
    ICC_CHECK_ERR_CODE( ICC_Get_Node_State( ICC_NODE_LOCAL, &node_state ) );
    if( ICC_NODE_STATE_INIT != node_state ) {
        return ICC_ERR_NODE_NOT_INIT;
    }

    /* check if the Heartbeat mechanism is initialized */
    ICC_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );
    if( crt_state == ICC_HEARTBEAT_STATE_UNINIT )
    {
        return ICC_ERR_HEARTBEAT_NOT_INIT;
    }

    /* check if the Heartbeat mechanism is already running */
    if( crt_state == ICC_HEARTBEAT_STATE_RUNNING ) {
           return ICC_ERR_HEARTBEAT_RUNNING;
    }

    /* check if the Heartbeat mechanism is already running */
    if( crt_state == ICC_HEARTBEAT_STATE_ERROR ) {
           return ICC_ERR_HEARTBEAT_ERRSTATE;
    }

    /* the Heartbeat mechanism is running now */
    ICC_Heartbeat_State = ICC_HEARTBEAT_STATE_RUNNING;

    /* set rate alarm in order to determine the task reserved for HEARTBEAT to send messages at the specified rate */
    ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Set_Recurrent_Rel_Alarm( HB_OS_config->channel_id, ICC_TX_FIFO ) );

    do
    {

        /* check the Heartbeat mechanism state */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );

        if( ICC_HEARTBEAT_STATE_RUNNING != crt_state ) {
            break;
        }


        /* wait until the rate timeout is reached to send new message */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Wait_Rate_Event( HB_OS_config->channel_id, ICC_TX_FIFO ) );

        /* check the Heartbeat mechanism state */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );

        if( ICC_HEARTBEAT_STATE_RUNNING != crt_state ) {
            break;
        }


        /**< Clear rate event if any */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_OS_Clear_Rate_Event( HB_OS_config->channel_id, ICC_TX_FIFO ) );
        
        /* check the Heartbeat mechanism state */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );

        if( ICC_HEARTBEAT_STATE_RUNNING != crt_state ) {
            break;
        }


        err_code = ICC_Msg_Send( HB_OS_config->channel_id, &crt_msg, hb_msg_size, HB_OS_config->txrx_timeout );

        if( err_code == ICC_ERR_NODE_DEINITIALIZED ) {
            break;
        }
        else {
            ICC_HEARTBEAT_CHECK_ERR_CODE( err_code );
        }


        /* check the Heartbeat mechanism state */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );

        if( ICC_HEARTBEAT_STATE_RUNNING != crt_state ) {
            break;
        }


        err_code = ICC_Msg_Recv( HB_OS_config->channel_id, &rcv_msg, hb_msg_size,  &actual_size, HB_OS_config->txrx_timeout, ICC_RX_NORMAL );

        if( err_code == ICC_ERR_NODE_DEINITIALIZED ) {
            break;
        }
        else {
            ICC_HEARTBEAT_CHECK_ERR_CODE( err_code );
        }

        /* check the Heartbeat mechanism state */
        ICC_HEARTBEAT_CHECK_ERR_CODE( ICC_Heartbeat_Get_State( &crt_state ) );

        if( ICC_HEARTBEAT_STATE_RUNNING != crt_state ) {
            break;
        }

        if (( rcv_msg.val  != crt_msg.val) || ( rcv_msg.run  != crt_msg.run)) {
            ICC_Heartbeat_State = ICC_HEARTBEAT_STATE_ERROR;
            return ICC_ERR_HEARTBEAT_WRONGSEQ;
        }

        crt_msg.val++;


    }
    while( 1 );

    return ICC_SUCCESS;
}

#endif /* ICC_CFG_HEARTBEAT_ENABLED */



#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif

