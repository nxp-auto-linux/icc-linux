/**
 *   @file    ICC_Hw_platform.h
 *   @version 0.8.0
 *
 *   @brief   ICC - Inter Core Communication platform specific definitions
 *   @details Inter Core Communication platform specific definitions
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

#ifndef ICC_HW_PLATFORM_H_
#define ICC_HW_PLATFORM_H_

#define HWREG32( address ) /*lint -save -e923 -e960 */ ( *(volatile unsigned       int *)(address) ) /*lint -restore */ /* 32-bit register */
#define HWREG16( address ) /*lint -save -e923 -e960 */ ( *(volatile unsigned short int *)(address) ) /*lint -restore */ /* 16-bit register */
#define HWREG8( address )  /*lint -save -e923 -e960 */ ( *(volatile unsigned      char *)(address) ) /*lint -restore */ /*  8-bit register */




/* Treerunner */
#define ICC_HW_MSCM_BASE        (0x40081000UL)
#define ICC_HW_MSCM_SIZE        (1000U)
#define ICC_HW_LMEM_BASE        (0xE0082000UL)
#define ICC_HW_CACHE_LINE_SIZE  (32U)

#define ICC_SHARED_PHYS_BASE_ADDR  (0X3E900000UL)


#endif /* ICC_HW_PLATFORM_H_ */
