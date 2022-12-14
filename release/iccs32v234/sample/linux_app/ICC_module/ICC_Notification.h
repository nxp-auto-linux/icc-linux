/**
*   @file    ICC_Notification.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication device driver notification support
*   @details       Inter Core Communication device driver notification support
*
*   The main purpose of the notification layer is to route functionality towards
*   polling or interrupt based implementations.
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

#ifndef ICC_NOTIFICATION_H
#define ICC_NOTIFICATION_H

#include <linux/types.h>

#include "ICC_Sw_Platform.h"

#ifdef ICC_LINUX2LINUX
    #ifdef ICC_USE_POLLING
    #include "ICC_Polling.h"
    #endif
#else
#include "ICC_Interrupts.h"
#endif

#include "ICC_Pcie.h"

const uint32_t get_shmem_size(void);

void init_shmem(struct ICC_platform_data *icc_data);
void cleanup_shmem(struct ICC_platform_data *icc_data);

void init_notifications(struct ICC_platform_data *icc_data);

int notify_peer(struct ICC_platform_data *icc_data);
void clear_notify_from_peer(struct ICC_platform_data *icc_data);

#ifndef ICC_BUILD_FOR_M4

    int notify_peer_alive(struct ICC_platform_data *icc_data);

#else

    int wait_for_peer(struct ICC_platform_data *icc_data);

#endif  /* ICC_BUILD_FOR_M4 */

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

    void notify_local(void);
    void clear_notify_local(void);

#endif

#endif /* ICC_NOTIFICATION_H */
