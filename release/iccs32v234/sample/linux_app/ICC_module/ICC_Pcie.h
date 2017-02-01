/**
*   @file    ICC_Pcie.h
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

#ifndef ICC_PCIE_H
#define ICC_PCIE_H

#ifdef ICC_LINUX2LINUX

struct s32v_bar {
    uint64_t bar_addr;
    uint32_t bar_size;
    uint32_t bar_nr;
};

struct handshake {
    struct s32v_bar rc_bar;
    uint64_t rc_ddr_addr;
};

struct ICC_platform_data;

/* first words in shared memory are reserved for the handshake */
#define ICC_CONFIG_OFFSET_FROM_BASE (sizeof(struct handshake))

void pcie_init_shmem(struct ICC_platform_data *icc_data);
void pcie_cleanup_shmem(struct ICC_platform_data *icc_data);

#ifdef ICC_BUILD_FOR_M4

struct s32v_inbound_region {
    uint32_t  bar_nr;
    uint32_t  target_addr;
    uint32_t  region;
};

struct s32v_outbound_region {
    uint64_t target_addr;
    uint64_t base_addr;
    uint32_t size;
    uint32_t region;
    uint32_t region_type;
};

int pcie_init_inbound(void);
int pcie_init_outbound(struct handshake *phshake);

#else

/* BAR attributes are initialized on the RC side only with EP defaults */
int pcie_init_bar(struct s32v_bar *bar);

#endif
#endif


#endif /* ICC_PCIE_H */
