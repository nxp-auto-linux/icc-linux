/**
*   @file    ICC_Polling.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication device driver polling support
*   @details       Inter Core Communication device driver polling support
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

#ifndef ICC_POLLING_H
#define ICC_POLLING_H

struct ping_poll {
    uint32_t *poll_addr;
    uint32_t *ping_addr;
    struct task_struct *poll_thread;
    bool terminate_communication;
};

struct ICC_platform_data;

const uint64_t get_shmem_poll_addr(void);
const uint64_t get_shmem_ping_addr(void);

int shmem_poll_init(struct ICC_platform_data *icc_data);
void shmem_poll_exit(struct ICC_platform_data *icc_data);

int shmem_ping_init(struct ICC_platform_data *icc_data);
void shmem_ping_exit(struct ICC_platform_data *icc_data);

int poll_notify_peer(struct ICC_platform_data *icc_data);

#ifndef ICC_BUILD_FOR_M4
int poll_notify_peer_alive(struct ICC_platform_data *icc_data);
#else
int poll_wait_for_peer(struct ICC_platform_data *icc_data);
#endif

void poll_clear_notify_from_peer(struct ICC_platform_data *icc_data);

#endif /* ICC_POLLING_H */
