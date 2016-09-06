/**
*   @file    ICC_Private.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication Private Types
*   @details Inter Core Communication Private Types
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


#ifndef ICC_PRIVATE_H
#define ICC_PRIVATE_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "ICC_Config.h"


/*
 * helper macros
 */
#define ICC_LOCAL_ENDPOINT    ICC_GET_CORE_ID
#define ICC_REMOTE_ENDPOINT   ICC_GET_REMOTE_CORE_ID

#define ICC_TX_FIFO    ICC_GET_CORE_ID
#define ICC_RX_FIFO    ICC_GET_REMOTE_CORE_ID

#ifdef __cplusplus
}
#endif

#endif /* ICC_PRIVATE_H */

