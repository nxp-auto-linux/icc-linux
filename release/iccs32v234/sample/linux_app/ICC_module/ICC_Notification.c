/**
*   @file    ICC_Notification.c
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

#include "ICC_Notification.h"

const uint32_t get_shmem_size(void)
{
    return ICC_CONFIG_MAX_SIZE - ICC_CONFIG_OFFSET_FROM_BASE;
}

void init_shmem(struct ICC_platform_data *icc_data)
{
#ifdef ICC_LINUX2LINUX
    pcie_init_shmem(icc_data);
#else
    intr_init_shmem(icc_data);
#endif
}

void cleanup_shmem(struct ICC_platform_data *icc_data)
{
#ifdef ICC_LINUX2LINUX
    pcie_cleanup_shmem(icc_data);
#else
    intr_cleanup_shmem(icc_data);
#endif
}

void init_notifications(struct ICC_platform_data *icc_data)
{
#ifndef ICC_USE_POLLING
    init_interrupt_data(icc_data);
#endif
}

int notify_peer(struct ICC_platform_data *icc_data)
{
#ifdef ICC_USE_POLLING
    return poll_notify_peer(icc_data);
#else
    return intr_notify_peer();
#endif
}

void clear_notify_from_peer(struct ICC_platform_data *icc_data)
{
#ifdef ICC_USE_POLLING
    poll_clear_notify_from_peer(icc_data);
#else
    intr_clear_notify_from_peer();
#endif
}

#ifndef ICC_BUILD_FOR_M4

int notify_peer_alive(struct ICC_platform_data *icc_data)
{
#ifdef ICC_USE_POLLING
    return poll_notify_peer_alive(icc_data);
#else
    /* nothing required when using interrupts */
    return 0;
#endif
}

#else

int wait_for_peer(struct ICC_platform_data *icc_data)
{
#ifdef ICC_USE_POLLING
    return poll_wait_for_peer(icc_data);
#else
    /* nothing required when using interrupts */
    return 0;
#endif
}

#endif  /* ICC_BUILD_FOR_M4 */

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)

void notify_local(void)
{
    /* not supported for polling */

#ifndef ICC_USE_POLLING
    intr_notify_local();
#endif
}

void clear_notify_local(void)
{
    /* not supported for polling */

#ifndef ICC_USE_POLLING
    intr_clear_notify_local();
#endif
}

#endif

