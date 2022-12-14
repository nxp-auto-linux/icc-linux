/**
*   @file    ICC_Types.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication Types
*   @details Inter Core Communication Types
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
*   Build Version        : SAC58R_ICC_0.8.0
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
/*==================================================================================================
==================================================================================================*/


#ifndef ICC_TYPES_H
#define ICC_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "ICC_Err.h"  /**< the Error codes */
#include "ICC_Os_Types.h"

/*==================================================================================================
                                       DEFINES AND MACROS
==================================================================================================*/

#define NULL_PTR ((void *)0)

#define ICC_IN  /**< input  parameter */
#define ICC_OUT /**< output parameter */


#define ICC_FIFO_FLAG_TIMEOUT_ENABLED  (0x00000001UL)
#define ICC_FIFO_FLAG_TIMEOUT_DISABLED (0x00000000UL)


/*==================================================================================================
                                 STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

typedef unsigned int ICC_Prio_t;           /**< priority type       */
typedef unsigned int ICC_Msg_Size_t;       /**< message size        */
typedef unsigned int ICC_Channel_t;        /**< the channel type    */
typedef unsigned int ICC_Timeout_t;        /**< timeout in uS       */
typedef unsigned int ICC_Header_t;         /**< message header type */
typedef unsigned int ICC_Signal_t;         /**< signal type         */
typedef unsigned int ICC_Endpoint_State_t; /**< endpoint state type */

/*
 * ICC endpoint state
 */

#define    ICC_ENDPOINT_STATE_CLOSED  (0U)      /**< end point closed */
#define    ICC_ENDPOINT_STATE_OPEN    (1U)      /**< end point open */

/*
 * Events
 */
typedef enum {

    ICC_EVENT_ACTIVITY_ISR,  /**< activity event on fifo */
    ICC_EVENT_TIMEOUT_ALM    /**< the timeout alarm has expired */

} ICC_Event_t;


#define ICC_HEADER_SIZE sizeof(ICC_Header_t)

/*
 * ICC Node identifiers
 */
typedef enum  {

    ICC_NODE_LOCAL = 0, /**< local  node */
    ICC_NODE_REMOTE,    /**< remote node */

    ICC_NODE_LAST       /**< used to validate node parameter */

} ICC_Node_t;


/*
 * ICC node state
 */
typedef enum  {

    ICC_NODE_STATE_UNINIT = 16,     /**< ICC node in uninitialized state  */
    ICC_NODE_STATE_INIT,            /**< ICC node in initialized state    */
    ICC_NODE_STATE_DEAD,            /**< ICC node crashed unexpectedly    */

} ICC_Node_State_t;



/*
 * ICC channel states
 */
typedef enum  {

    ICC_CHANNEL_STATE_CLOSED = 0,        /**< channel closed : 16 in order to avoid signal confusion between channel and node */
    ICC_CHANNEL_STATE_OPEN_LOCAL,        /**< channel open on local  side  */
    ICC_CHANNEL_STATE_OPEN_REMOTE,       /**< channel open on remote side  */
    ICC_CHANNEL_STATE_OPEN_ESTABLISHED,  /**< channel established : open local and remote */

} ICC_Channel_State_t;






#ifdef ICC_CFG_HEARTBEAT_ENABLED
/*
 * ICC Heartbeat mechanism state
 */
typedef enum {

    ICC_HEARTBEAT_STATE_UNINIT=0,           /**< ICC HEARTBEAT's structure in uninitialized state*/
    ICC_HEARTBEAT_STATE_INIT,               /**< ICC HEARTBEAT's structure in initialized state*/
    ICC_HEARTBEAT_STATE_RUNNING,            /**< ICC HEARTBEAT's structure in running state*/
    ICC_HEARTBEAT_STATE_ERROR,              /**< ICC HEARTBEAT's structure in error state*/

} ICC_Heartbeat_State_t;

#endif /* ICC_CFG_HEARTBEAT_ENABLED */

/*
 * ICC callback for received messages,
 * users must not use blocking calls in this callback
 * should use ICC_Msg_Recv_i
 */
typedef void (* ICC_Callback_Rx_t)(
                                   ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that received a message */
                                  );

/*
 * ICC callback for tx free buffer,
 * users must not use blocking calls in this callback
 * should use ICC_Msg_Send_i
 */
typedef void (* ICC_Callback_Tx_t)(
                                   ICC_IN const ICC_Channel_t   channel_id /**< the id of the channel that just "received" free space */
                                  );

/*
 * ICC callback for channel transitioning to a new state
 */
typedef void (* ICC_Callback_Channel_State_Update_t)(
                                                     ICC_IN const ICC_Channel_t          channel_id,   /**< the id of the channel that transitioned to a new state */
                                                     ICC_IN const ICC_Channel_State_t    channel_state /**< the new state that the channel transitioned to */
                                                    );


/*
 * ICC callback for Node transitioning to a new state
 */
typedef void (* ICC_Callback_Node_State_Update_t)(
                                                  ICC_IN const ICC_Node_t        node_id,    /**< the id of the node that transitioned to a new state */
                                                  ICC_IN const ICC_Node_State_t  node_state  /**< the new state the node transitioned to */
                                                 );


typedef struct {

    ICC_Signal_t signals_received[ ICC_CFG_MAX_SIGNALS ]; /**< queue for received signals */

    unsigned int signal_queue_head;                        /**< queue head */
    unsigned int signal_queue_tail;                        /**< queue tail */

} ICC_Signal_Fifo_Ram_t;


/*
 * ICC basic FIFO configuration structure
 */
typedef struct {

          unsigned char * fifo_buffer_ptr;    /**< pointer to the Fifo buffer (statically allocated on M4 side) */
    const ICC_Prio_t      fifo_prio;          /**< FIFO priority relative to all FIFOs in the same direction */
    const unsigned int    fifo_size;          /**< FIFO buffer size  */

    const ICC_Msg_Size_t  max_msg_size;       /**< maximum message size to be transmitted using this fifo */

    const unsigned int    alignment;          /**< message alignment in FIFO buffer : [1,2,4,8, ...] in bytes */

    const unsigned int   fifo_flags;          /**< store fifo configuration flags */

} ICC_Fifo_Config_t;



/*
 * the generic fifo structure
 */
typedef struct {

    const ICC_Fifo_Config_t * fifo_config[ 2 /* coreId */ ];     /**< link to Fifo configuration: avoid duplication at runtime/use config instead, [ core_id ] */

    ICC_Fifo_Os_Ram_t       * fifo_os_ram[ 2 /* coreId */ ];     /**< pointer to Fifo OS ram structure, [ core_id ] */

    unsigned int                                       head;     /**< consumer/reader index position */
    unsigned int                                       tail;     /**< producer/writer index position */

    unsigned int                       wr[ 2 /* coreId */ ];     /**< WR Sig and Ack */
    unsigned int                       rd[ 2 /* coreId */ ];     /**< RD Sig and Ack */

    unsigned int                      pending_send_msg_size;     /**< the size of the pending message */

} ICC_Fifo_Ram_t;



/*
 * Channel runtime structure
 */
typedef struct {

        ICC_Endpoint_State_t     endpoint_state[ 2 /* coreId */ ]; /**< end point state for each side of communication, 0 for Core0 endpoint, 1 for Core1 endpoint */

        ICC_Fifo_Ram_t         * fifos_ram[ 2 /* tx/rx */ ][ 2 /* coreId */ ]; /**< pointer to the FIFO ram structure, one for each direction [ tx/rx fifo ][ core_id ] */

        ICC_Signal_Fifo_Ram_t    sig_fifo_remote[ 2 /* tx/rx */ ]; /**< the channel signal queue, one for each direction */

} ICC_Channel_Ram_t;


/*
 * ICC channel configuration structure
 *
 * each channel contains one FIFO for each direction
 *
 * CONVENTION:
 *
 *  Node 0 --- FIFO 0 --> Node 1
 *  Node 0 <-- FIFO 1 --- Node 1
 *
 *  Node 0 : TX on FIFO 0
 *           RX on FIFO 1
 *
 *  Node 1 : TX on FIFO 1
 *         : RX on FIFO 0
 *
 */
typedef struct {

    ICC_Fifo_Config_t                   fifos_cfg[ 2 /* tx/rx */ ];      /**< one FIFO for each direction */

    ICC_Callback_Channel_State_Update_t Channel_Update_Cb; /**< channel state update call back function, different symbol on each side */

    ICC_Callback_Rx_t                   Channel_Rx_Cb;     /**< channel Rx call back function, different symbol for each side */
    ICC_Callback_Tx_t                   Channel_Tx_Cb;     /**< channel Tx call back function, different symbol for each side */

} ICC_Channel_Config_t;


/*
 * ICC top level configuration structure to be passed to ICC_Initialize function
 */
typedef struct {

    const unsigned int                 Channels_Count;                    /**< number  of configured channels */
    const ICC_Channel_Config_t       * Channels_Ptr;                      /**< pointer to configured channels */

    void                             * not_used_on_app;                   /**< not used on application side */

#ifndef ICC_CFG_NO_TIMEOUT
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        const ICC_Heartbeat_Os_Config_t * ICC_Heartbeat_Os_Config;             /**< pointer to Heartbeat OS configuration */
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */
#endif /* ICC_CFG_NO_TIMEOUT */


    ICC_Callback_Node_State_Update_t   Node_Update_Cb;                    /**< node update call back function, different symbol for each side */

    volatile unsigned int                    (* ICC_Initialized_Shared)[2];        /**< pointer to shared variable */
    volatile ICC_Channel_Ram_t                * ICC_Channels_Ram_Shared;           /**< pointer to shared variable */
    volatile ICC_Fifo_Ram_t                  (* ICC_Fifo_Ram_Shared) [][2];        /**< pointer to shared variable */
    volatile ICC_Signal_Fifo_Ram_t           (* ICC_Node_Sig_Fifo_Ram_Shared) [2]; /**< pointer to shared variable */


} ICC_Config_t;


#ifdef __cplusplus
}
#endif

#endif /* ICC_TYPES_H */
