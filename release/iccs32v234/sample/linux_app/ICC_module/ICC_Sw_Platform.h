/**
*   @file    ICC_Sw_Platform.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication generic platform definitions
*   @details       Inter Core Communication generic platform definitions
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : ARM
*   Peripheral           :
*   Dependencies         : none
*
*   Build Version        :
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

#ifndef ICC_PLATFORM_H
#define ICC_PLATFORM_H

#define MODULE_NAME     "ICC"
#define BASEMINOR       0
#define NUM_MINORS      1

#ifdef ICC_USE_POLLING
#include "ICC_Polling.h"
#endif

#define ICC_CONFIG_MAX_SIZE    1024 * 1024 * 1 /* 1 MB */

struct ICC_platform_data {
    struct platform_device *pdev;
#ifndef ICC_USE_POLLING
    int shared_irq;
    int local_irq;
#else
    struct ping_poll icc_polling;
#endif
};

#endif /* ICC_PLATFORM_H */
