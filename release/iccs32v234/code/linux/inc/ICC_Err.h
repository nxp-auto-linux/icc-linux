/**
*   @file    ICC_Err.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication Error codes
*   @details Inter Core Communication Error codes 
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

#ifndef ICC_ERR_H
#define ICC_ERR_H


#ifdef __cplusplus
extern "C"
{
#endif


/*
 * ICC error codes
 */
typedef enum  {

    ICC_SUCCESS = 0,                /**< Indicates a successful operation */

    ICC_ERR_TIMEOUT,                /**< The operation timed out */
    ICC_ERR_EMPTY,                  /**< no message received, the RX fifo is empty */
    ICC_ERR_FULL,                   /**< message not transmitted, the TX fifo is full or not enough space is available */

    ICC_ERR_PARAM_CHAN_INVALID,     /**< The specified channel is not valid */
    ICC_ERR_PARAM_NULL_CFG,         /**< invalid configuration pointer */
    ICC_ERR_PARAM_BUFF_NULL,        /**< Invalid buffer pointer parameter */
    ICC_ERR_PARAM_BUFF_SIZE,        /**< Invalid buffer size */
    ICC_ERR_PARAM_TIMEOUT,          /**< Invalid timeout value */
    ICC_ERR_PARAM_RX_OP,            /**< Invalid RX operation parameter */

    ICC_ERR_CONFIG_MATCH_FAILED,    /**< The ICC configuration does not match the config on the other core  */

    ICC_ERR_NODE_INITIALIZED,       /**< The ICC node is already initialized */
    ICC_ERR_NODE_NOT_INIT,          /**< The ICC node was not initialized  */
    ICC_ERR_NODE_DEINITIALIZED,     /**< The node was deinitialized while the blocking call was pending */
    ICC_ERR_NODE_INVALID,           /**< Invalid node parameter */

    ICC_ERR_CHAN_ENDPOINT_CLOSED,   /**< The channel is closed on the side the operation is performed */
    ICC_ERR_CHAN_ALREADY_OPEN,      /**< The channel is already open */
    ICC_ERR_CHAN_ALREADY_CLOSED,    /**< The channel is already closed */

    ICC_ERR_TIMEOUT_NOT_CONF,       /**< No timeout configuration for this fifo */

    ICC_ERR_MSG_SIZE,               /**< The message size exceeds the maximum size configured for this channel */
    ICC_ERR_MSG_TRUNCATED,          /**< The message size exceeds the buffer size and it was truncated */

    ICC_ERR_FIFO_EMPTY,             /**< Fifo is empty */
    ICC_ERR_FIFO_FULL,              /**< Fifo is full or not enough space is available */

    ICC_ERR_SQ_FULL,                /**< Signal queue is full */
    ICC_ERR_SQ_EMPTY,               /**< Signal queue is empty, nothing to dequeue */

    ICC_ERR_OS_SEM_BUSY,            /**< semaphore is busy, try again later */
    ICC_ERR_OS_SEM_ERR,             /**< semaphore handling error */

    ICC_ERR_OS_LINUX_SEM_BUSY,          /**< Linux semaphore is busy, try again later */
    ICC_ERR_OS_LINUX_CREATE_WAITQUEUE,  /**< waitqueue could not be allocated successfully on Linux side*/
    ICC_ERR_OS_LINUX_REGISTER_IRQ,      /**< error requesting interrupt line */
    ICC_ERR_OS_LINUX_ERESTARTSYS,       /**< waiting was interrupted by a signal */
    #ifdef ICC_CFG_HEARTBEAT_ENABLED
        ICC_ERR_OS_LINUX_CREATE_RATEWAITQUEUE, /**< rate_waitqueue could not be allocated successfully on Linux side*/
        ICC_ERR_OS_LINUX_WRONGCHNID,           /**< it is used a Heartbeat function on other channel than the reserved one*/
        ICC_ERR_OS_LINUX_BUSYTIMER,            /**< timer can not be removed */
        ICC_ERR_OS_LINUX_SETTIMER,             /**< timer can not be set */
    #endif /* ICC_CFG_HEARTBEAT_ENABLED */


#ifdef ICC_CFG_HEARTBEAT_ENABLED
    ICC_ERR_HEARTBEAT_NOT_INIT,     /**< The ICC Heartbeat mechanism was not initialized */
    ICC_ERR_HEARTBEAT_INITIALIZED,  /**< The ICC Heartbeat mechanism is already initialized */
    ICC_ERR_HEARTBEAT_RUNNING,      /**< The ICC Heartbeat mechanism is already running */
    ICC_ERR_HEARTBEAT_ERRSTATE,     /**< The ICC Heartbeat mechanism is in error state */
    ICC_ERR_HEARTBEAT_WRONGSEQ,     /**< The ICC Heartbeat mechanism - received wrong reply sequence */
#endif /* ICC_CFG_HEARTBEAT_ENABLED */

    ICC_ERR_PARAM_INVAL,				/**< To be used by implementations for invalid parameter error conditions not covered by the other status codes */
    ICC_ERR_GENERAL,                /**< To be used by implementations for error conditions not covered by the other status codes */

    ICC_ERR_LAST                    /**< This should always be last */

} ICC_Err_t;


#ifdef __cplusplus
}
#endif

#endif /* ICC_ERR_H */
