/**
*   @file    ICC_Sig_Fifo.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication SIGNALS FIFO API
*   @details Inter Core Communication SIGNALS FIFO API
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
 
#ifndef ICC_SIG_FIFO_H_
#define ICC_SIG_FIFO_H_


#ifdef __cplusplus
extern "C"
{
#endif



#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"

ICC_ATTR_SEC_TEXT_CODE
extern
unsigned int
ICC_Sig_Fifo_Full  (ICC_Signal_Fifo_Ram_t *queue);


ICC_ATTR_SEC_TEXT_CODE
extern
unsigned int
ICC_Sig_Fifo_Empty (ICC_Signal_Fifo_Ram_t *queue);


ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Sig_Fifo_Init  (ICC_Signal_Fifo_Ram_t * queue);

ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Sig_Fifo_Signal  (ICC_Signal_Fifo_Ram_t * queue, ICC_Signal_t   signal);

ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_Sig_Fifo_Dequeue_Signal(ICC_Signal_Fifo_Ram_t * queue, ICC_Signal_t * signal);


#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif

#endif /* ICC_SIG_FIFO_H_ */
