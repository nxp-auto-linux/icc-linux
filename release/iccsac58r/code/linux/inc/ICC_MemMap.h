/**
*   @file    ICC_MemMap.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication Memory Sections
*   @details Inter Core Communication Memory Sections
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



#ifdef __cplusplus
extern "C"
{
#endif


#if !defined(ICC_GCC_ARM) && !defined(ICC_GHS_ARM) && !defined(ICC_DS5_ARM)
#error "Unknown or unsupported compiler"
#endif


#if defined(ICC_GCC_ARM) || defined(ICC_GHS_ARM) || defined(ICC_DS5_ARM)

    /*
     * variables alignment to 32bit
     */
    #define ICC_ATTR_ALIGN_VAR __attribute__((aligned(4)))

#else

    /*
     * other compilers
     */
    #define ICC_ATTR_ALIGN_VAR

#endif


#define ICC_ATTR_SEC_TEXT_CODE

#define ICC_ATTR_SEC_VAR_UNSPECIFIED_DATA
#define ICC_ATTR_SEC_VAR_UNSPECIFIED_BSS

#define ICC_ATTR_SEC_VAR_FAST_UNSPECIFIED_DATA
#define ICC_ATTR_SEC_VAR_FAST_UNSPECIFIED_BSS

#define ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_DATA
#define ICC_ATTR_SEC_SHARED_VAR_UNSPECIFIED_BSS

#define ICC_ATTR_SEC_CONST_UNSPECIFIED
#define ICC_ATTR_SEC_CONST_FAST_UNSPECIFIED


#define ICC_MEMMAP_ERROR


/*
 * ICC CODE start
 */

#if defined(ICC_START_SEC_TEXT_CODE)
#undef ICC_START_SEC_TEXT_CODE
#undef ICC_MEMMAP_ERROR



/*
 * ICC CODE stop
 */
#elif defined(ICC_STOP_SEC_TEXT_CODE)
#undef ICC_STOP_SEC_TEXT_CODE
#undef ICC_MEMMAP_ERROR



/*
 * ICC VAR sections start
 */
#elif defined(ICC_START_SEC_VAR_UNSPECIFIED)
#undef ICC_START_SEC_VAR_UNSPECIFIED
#undef ICC_MEMMAP_ERROR



/*
 * ICC VAR sections stop
 */
#elif defined(ICC_STOP_SEC_VAR_UNSPECIFIED)
#undef ICC_STOP_SEC_VAR_UNSPECIFIED
#undef ICC_MEMMAP_ERROR





/*
 * NON CACHEABLE SHARED VAR sections start
 */

#elif defined(ICC_START_SEC_SHARED_VAR_UNSPECIFIED)
#undef ICC_START_SEC_SHARED_VAR_UNSPECIFIED
#undef ICC_MEMMAP_ERROR




/*
 * NON CACHEABLE SHARED VAR sections stop
 */
#elif defined(ICC_STOP_SEC_SHARED_VAR_UNSPECIFIED)
#undef ICC_STOP_SEC_SHARED_VAR_UNSPECIFIED
#undef ICC_MEMMAP_ERROR






/*
 * CONST sections start
 */
#elif defined(ICC_START_SEC_CONST_UNSPECIFIED)
#undef ICC_START_SEC_CONST_UNSPECIFIED
#undef ICC_MEMMAP_ERROR





/*
 * CONST sections stop
 */
#elif defined(ICC_STOP_SEC_CONST_UNSPECIFIED)
#undef ICC_STOP_SEC_CONST_UNSPECIFIED
#undef ICC_MEMMAP_ERROR





#endif


#if defined(ICC_MEMMAP_ERROR)
#error "ICC_Sections.h: ICC_MEMMAP_ERROR is defined, this means: invalid section"
#endif


#ifdef __cplusplus
}
#endif



/** @} */
