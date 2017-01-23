/**
*   @file    ICC_Base_Types.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication Basic Types and Definitions
*   @details Inter Core Communication Basic Types and Definitions
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
/*==================================================================================================
==================================================================================================*/


#ifndef ICC_BASE_TYPES_H
#define ICC_BASE_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/types.h>

#ifdef __GNUC__

/** Macro for unified stuct alignment, to be used for all supported compilers */
#define ICC_ALIGN(n) __attribute__ ((aligned (n)))

/** Macro for defining minimum sized enums, equivalent to compile option -fshort-enums
 * but without its side effects.
 * MUST be used for compatibility with M4 code. */
#define ICC_ENUM typedef enum __attribute__ ((__packed__))

#else
#warning "Struct alignment not supported - ICC may not work."
#define ICC_ALIGN(n)

#warning "Enum packing not supported - ICC may not work."
#define ICC_ENUM typedef enum
#endif

#define NULL_PTR ((void *)0)

#define ICC_IN  /**< input  parameter */
#define ICC_OUT /**< output parameter */

typedef uint32_t ICC_Prio_t;           /**< priority type       */
typedef uint32_t ICC_Msg_Size_t;       /**< message size        */
typedef uint32_t ICC_Channel_t;        /**< the channel type    */
typedef uint32_t ICC_Timeout_t;        /**< timeout in uS       */
typedef uint32_t ICC_Header_t;         /**< message header type */
typedef uint32_t ICC_Signal_t;         /**< signal type         */
typedef uint32_t ICC_Endpoint_State_t; /**< endpoint state type */

#ifdef __cplusplus
}
#endif

#endif /* ICC_TYPES_H */
