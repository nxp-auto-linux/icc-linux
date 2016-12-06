/**
*   @file    ICC_Interrupts.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication device driver interrupt support
*   @details       Inter Core Communication device driver interrupt support
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

#ifndef ICC_INTERRUPTS_H
#define ICC_INTERRUPTS_H

#ifndef ICC_USE_POLLING

struct ICC_platform_data;

int intr_notify_peer(void);
void intr_clear_notify_from_peer(void);

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
void intr_notify_local(void);
void intr_clear_notify_local(void);
#endif

int init_interrupt_data(struct ICC_platform_data * icc_data);

#endif

#endif /* ICC_INTERRUPTS_H */
