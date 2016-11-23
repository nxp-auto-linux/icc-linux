/**
*   @file    ICC_Relocate.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication Config relocation
*   @details       Inter Core Communication Config relocation support
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

#ifndef ICC_RELOCATE_H
#define ICC_RELOCATE_H

#define RELOCATED_PTR(obj) obj##_Rel

/* dest is an integer representing the destination address
   obj is an object (NOT pointer to an object!)
*/
#define RELOCATE_OBJ(dest, obj) \
    { \
      memcpy(dest, (&obj), sizeof(obj)); \
      dest = (typeof(dest))((typeof(&obj))dest + 1); \
    }

#define RELOCATE_RAW(dest, src_ptr, sz) \
    { \
      memcpy(dest, (*src_ptr), sz); \
      *src_ptr = (typeof(*src_ptr))dest; \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sz); \
    }

#define RELOCATE_ICC_Fifo_Config_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      RELOCATE_OBJ(dest, obj); \
      RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->fifo_buffer_ptr), obj.fifo_size); \
    }

#define RELOCATE_ICC_Fifo_Os_Config_t_Array(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      *((uint64_t*)dest) = (uint64_t)((uint64_t*)dest + 1); \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      ((uint64_t*)dest) ++; \
      memcpy((char*)dest, (char*)obj, sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sizeof(obj)); \
    }

#define RELOCATE_ICC_Channel_Config_t_Array(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      int i, chan_count; \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      memcpy((char*)dest, (char*)obj, sizeof(obj)); \
      chan_count = (sizeof(obj)) / sizeof(ICC_Channel_Config_t); \
      dest = (typeof(dest))((uint64_t)RELOCATED_PTR(obj) + (uint64_t)sizeof(obj)); \
      for (i = 0; i < chan_count; i++) { \
          ICC_Fifo_Config_t * relocated_cfg = (ICC_Fifo_Config_t *)&((*RELOCATED_PTR(obj))[i].fifos_cfg[0]); \
          RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size); \
          relocated_cfg = (ICC_Fifo_Config_t *)&((*RELOCATED_PTR(obj))[i].fifos_cfg[1]); \
          RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size); \
      } \
    }

#ifdef ICC_CFG_HEARTBEAT_ENABLED
#define RELOCATE_ICC_Config_t_Heartbeat(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Heartbeat_Os_Config) = RELOCATED_PTR(ICC_Heartbeat_Os_Config0); \
    }
#else
#define RELOCATE_ICC_Config_t_Heartbeat(dest, obj)
#endif

#define RELOCATE_ICC_Runtime_Shared_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
        RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
        memcpy((char*)dest, (char*)&obj, sizeof(obj)); \
        dest = (typeof(dest))((uint64_t)dest + (uint64_t)sizeof(obj)); \
    }

/* TODO: relocate transparently the dependencies such as ICC_Cfg0_ChannelsConfig, ICC_Runtime_Shared etc., without
 *
 */
#define RELOCATE_ICC_Config_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
        RELOCATE_ICC_Channel_Config_t_Array(dest, ICC_Cfg0_ChannelsConfig); \
        RELOCATE_ICC_Runtime_Shared_t(dest, ICC_Runtime_Shared0); \
        \
        RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
        memcpy((char*)dest, (char*)&obj, sizeof(obj)); \
        dest = (typeof(dest))((uint64_t)RELOCATED_PTR(obj) + (uint64_t)sizeof(obj)); \
        RELOCATED_PTR(obj)->This_Ptr = (uint64_t)RELOCATED_PTR(obj); \
        ICC_CROSS_ASSIGN_CAST(RELOCATED_PTR(obj)->Channels_Ptr, RELOCATED_PTR(ICC_Cfg0_ChannelsConfig)); \
        ICC_CROSS_ASSIGN_CAST(RELOCATED_PTR(obj)->ICC_Initialized_Shared, &(RELOCATED_PTR(ICC_Runtime_Shared0)->ICC_Initialized_Shared)); \
        ICC_CROSS_ASSIGN_CAST(RELOCATED_PTR(obj)->ICC_Channels_Ram_Shared, (RELOCATED_PTR(ICC_Runtime_Shared0)->ICC_Channels_Ram_Shared)); \
        ICC_CROSS_ASSIGN_CAST(RELOCATED_PTR(obj)->ICC_Fifo_Ram_Shared, &(RELOCATED_PTR(ICC_Runtime_Shared0)->ICC_Fifo_Ram_Shared)); \
        ICC_CROSS_ASSIGN_CAST(RELOCATED_PTR(obj)->ICC_Node_Sig_Fifo_Ram_Shared, &(RELOCATED_PTR(ICC_Runtime_Shared0)->ICC_Node_Sig_Fifo_Ram_Shared)); \
        RELOCATE_ICC_Config_t_Heartbeat(dest, obj); \
    }

#endif /* ICC_RELOCATE_H */
