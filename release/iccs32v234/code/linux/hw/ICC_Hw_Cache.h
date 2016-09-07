/**
*   @file    ICC_Hw_Cache.h
*   @version 0.8.0
*
*   @brief   ICC - Cache hw specific details
*   @details Cache hw specific details
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

#ifndef ICC_HW_CACHE_H_
#define ICC_HW_CACHE_H_







    #define ICC_DCACHE_ENABLE
    #define ICC_DCACHE_DISABLE

    #define ICC_DCACHE_INVALIDATE
    #define ICC_DCACHE_INVALIDATE_LINE(addr)
    #define ICC_DCACHE_INVALIDATE_MLINES(addr, length)

    #define ICC_DCACHE_FLUSH
    #define ICC_DCACHE_FLUSH_LINE(addr)
    #define ICC_DCACHE_FLUSH_MLINES(addr, length)


#endif /* ICC_HW_CACHE_H_ */
