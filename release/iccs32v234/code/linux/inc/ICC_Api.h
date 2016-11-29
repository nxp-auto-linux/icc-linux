/**
*   @file    ICC_Api.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication API
*   @details Inter Core Communication API
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




#ifndef ICC_API_H
#define ICC_API_H


#ifdef __cplusplus
extern "C"
{
#endif



#include "ICC_Config.h"
#include "ICC_Types.h"

/*==================================================================================================
                                       DEFINES AND MACROS
==================================================================================================*/


#define ICC_WAIT_ZERO      (0x0U)         /**< ICC_Msg_Send/ICC_Msg_Recv immediate */
#define ICC_WAIT_FOREVER   (0xFFFFFFFFU)  /**< ICC_Msg_Send/ICC_Msg_Recv wait forever */



/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/




typedef enum {

    ICC_RX_NORMAL,                    /**< normal RX operation, message removed from queue        */
    ICC_RX_PEEK_MSG                   /**< only copy the message from the queue but do not remove */

} ICC_Rx_Op_t;

/*==================================================================================================
                                     FUNCTION PROTOTYPES
==================================================================================================*/

#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"

#if (defined(ICC_BUILD_FOR_M4) && defined(ICC_LINUX2LINUX))

/**
 *
 * Called by the master node (which initializes the shared memory) when required
 * to relocate a static config object and its dependencies to a shared memory
 * buffer received as argument.
 *
 * If base_addr is NULL, by default the start of the shared memory is used as
 * base address for the relocation.
 * In case of multiple configuration, it is recommended to use:
 * - for config 0: base_addr = NULL,
 * - for config 1: base_addr = address of relocated config 0 + sizeof(ICC_Config_t),
 * - etc.
 *
 * Returns the address of the relocated config object.
 */

ICC_ATTR_SEC_TEXT_CODE
extern
void *
ICC_Relocate_Config(
                ICC_IN ICC_Config_t * config,
                ICC_IN void         * base_addr
              );

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
 */

ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Initialize(
                ICC_IN ICC_Config_t * config
              );

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
extern
ICC_Err_t
ICC_Finalize( void );


/**
 *
 * Called by the core using the shared memory (slave) to notify the
 * core who initialized the shared memory (master) that it is connected
 * and ready to exchange messages.
 *
 */

#if (defined(ICC_USE_POLLING) && !defined(ICC_BUILD_FOR_M4))
ICC_ATTR_SEC_TEXT_CODE
extern
void ICC_Notify_Peer_Alive( void );
#else
#define ICC_Notify_Peer_Alive()
#endif

/**
 *
 * Called by the core who initialized the shared memory (master) after the
 * initialization is completed, in order to place itself in a wait mode until
 * a peer (slave) connected to its shared memory.
 *
 * After ICC_Wait_For_Peer() returns, messages can be safely exchanged between
 * the two cores.
 *
 */

#if (defined(ICC_USE_POLLING) && defined(ICC_BUILD_FOR_M4))
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t ICC_Wait_For_Peer( void );
#else
#define ICC_Wait_For_Peer() (ICC_SUCCESS)
#endif

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
extern
ICC_Err_t
ICC_Get_Node_State(
                     ICC_IN const ICC_Node_t          node,
                     ICC_OUT      ICC_Node_State_t *  node_state
                  );



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
extern
ICC_Err_t
ICC_Open_Channel(
                 ICC_IN const ICC_Channel_t   channel_id
                );



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
extern
ICC_Err_t
ICC_Close_Channel(
                  ICC_IN const ICC_Channel_t   channel_id
                 );




/**
 *
 * Interrogate state of caller side for specified channel
 *
 * ICC_ERR_NODE_NOT_INIT      - The node was not initialized
 * ICC_ERR_PARAM_CHAN_INVALID - The specified channel is not valid
 * ICC_ERR_GENERAL            - TBD
 * ICC_SUCCESS                - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Get_Channel_State(
                       ICC_IN const ICC_Channel_t         channel_id,
                       ICC_OUT      ICC_Channel_State_t * channel_state
                     );


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
extern
ICC_Err_t
ICC_Get_Channel_Free(
                     ICC_IN const ICC_Channel_t          channel_id,
                     ICC_OUT      unsigned int         * channel_free_bytes
                    );


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
extern
ICC_Err_t
ICC_Get_Channel_Pending(
                        ICC_IN const ICC_Channel_t          channel_id,
                        ICC_OUT      unsigned int         * channel_pending_bytes
                       );


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
extern
ICC_Err_t
ICC_Get_Next_Msg_Size(
                      ICC_IN const ICC_Channel_t          channel_id,
                      ICC_OUT      unsigned int         * next_msg_size
                     );


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
 *   ICC_ERR_PARAM_BUFF_SIZE - Invalid buffer size
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
extern
ICC_Err_t
ICC_Msg_Send(
             ICC_IN const ICC_Channel_t  channel_id,
             ICC_IN const void        *  tx_user_buffer,
             ICC_IN const ICC_Msg_Size_t tx_user_buffer_size,
             ICC_IN const ICC_Timeout_t  timeout_val
           );



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
extern
ICC_Err_t
ICC_Msg_Recv(
              ICC_IN  const ICC_Channel_t    channel_id,
              ICC_IN  void                 * rx_user_buffer,        /**< the RX buffer used to store the received message */
              ICC_IN  const ICC_Msg_Size_t   rx_user_buffer_size,   /**< the RX buffer size */
              ICC_OUT       ICC_Msg_Size_t * rx_msg_size,           /**< the actual size of the received message */
              ICC_IN  const ICC_Timeout_t    timeout_val,           /**< the time out value */
              ICC_IN  const ICC_Rx_Op_t      rx_operation           /**< the type of Rx operation performed */
            );


#ifdef ICC_CFG_HEARTBEAT_ENABLED

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
extern
ICC_Err_t
ICC_Heartbeat_Initialize(
                          ICC_IN unsigned int runId   /**< the current runId */
                        );

/**
 *
 * Called by each core to stop Heartbeat mechanism.
 *
 * ICC_ERR_NODE_NOT_INIT        - Local node is not initialized
 * ICC_ERR_HEARTBEAT_NOT_INIT   - ICC Heartbeat mechanism is not initialized.
 * ICC_ERR_GENERAL              - TBD
 * ICC_SUCCESS                  - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Heartbeat_Finalize( void );

/**
 *
 * Called by each core to run Heartbeat mechanism.
 *
 * ICC_ERR_NODE_NOT_INIT        - Local node is not initialized
 * ICC_ERR_HEARTBEAT_NOT_INIT   - ICC Heartbeat mechanism is not initialized.
 * ICC_ERR_HEARTBEAT_RUNNIG     - ICC Heartbeat mechanism is already running
 * ICC_ERR_GENERAL              - TBD
 * ICC_SUCCESS                  - OK
 *
 */

ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Heartbeat_Runnable( void );

#endif /* ICC_CFG_HEARTBEAT_ENABLED */

#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif


#endif /* ICC_API_H */
