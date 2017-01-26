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

#include "ICC_Types.h"
#include "ICC_Config.h"

/* dest is the destination address
 * obj is the source object (NOT pointer to an object!)
*/
#define COPY_OBJ(dest, obj) \
    { \
      memcpy(dest, (&obj), sizeof(obj)); \
      dest = (typeof(dest))((typeof(&obj))dest + 1); \
    }

/* dest is the destination address
 * src_ptr is the pointer to the address of the data to be relocated; the address
 *      will be updated with the destination address (which is the new address)
 * sz is the size of the data
 *
 * The data can be either an object (equivalent to **src_ptr), an array etc.
 */
#define RELOCATE_OBJ_PTR(dest, src_ptr, sz) \
    { \
      memcpy(dest, (*src_ptr), sz); \
      *src_ptr = (typeof(*src_ptr))dest; \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sz); \
    }

ICC_Fifo_Config_t * RELOCATE_ICC_Fifo_Config_t(char **dest_ptr, ICC_Fifo_Config_t *obj)
{
    ICC_Fifo_Config_t * relocated_obj = NULL;

    if (obj && dest_ptr && *dest_ptr) {
        char *dest = *dest_ptr;

        relocated_obj = (ICC_Fifo_Config_t *)dest;
        COPY_OBJ(dest, *obj);
        RELOCATE_OBJ_PTR(dest, &ICC_CROSS_VALUE_OF(relocated_obj->fifo_buffer_ptr), obj->fifo_size);

        *dest_ptr = dest;
    }

    return relocated_obj;
}

#ifdef ICC_FSL_AUTOSAR_OS

ICC_Fifo_Os_Config_Array_t * RELOCATE_ICC_Fifo_Os_Config_Array_t(char **dest_ptr, const ICC_Fifo_Os_Config_Array_t obj, int size)
{
    ICC_Fifo_Os_Config_Array_t *relocated_obj = NULL;

    if (obj && dest_ptr && *dest_ptr) {
        char *dest = *dest_ptr;

        relocated_obj = (ICC_Fifo_Os_Config_Array_t *)dest;
        memcpy(dest, obj, size);
        dest = (char *)((int)dest + size);

        *dest_ptr = dest;
    }

    return relocated_obj;
}
#endif

ICC_Channel_Config_t *RELOCATE_ICC_ChannelsConfig_Array_t(char **dest_ptr, const ICC_ChannelsConfig_Array_t obj, int channels_count)
{
    ICC_Channel_Config_t *relocated_obj = NULL;

     if (obj && dest_ptr && *dest_ptr) {
        char *dest = *dest_ptr;
        int i, size;
        ICC_Fifo_Config_t * relocated_cfg;

        relocated_obj = (ICC_Channel_Config_t*)dest;
        size = channels_count * sizeof(ICC_Channel_Config_t);
        memcpy(dest, obj, size);
        dest = dest + size;

        for (i = 0; i < channels_count; i++) {
            relocated_cfg = (ICC_Fifo_Config_t *)&(relocated_obj[i].fifos_cfg[0]);
            RELOCATE_OBJ_PTR(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size);
            relocated_cfg = (ICC_Fifo_Config_t *)&(relocated_obj[i].fifos_cfg[1]);
            RELOCATE_OBJ_PTR(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size);
        }

        *dest_ptr = dest;
    }

    return relocated_obj;
}

ICC_Config_t * RELOCATE_ICC_Config_t(char **dest_ptr, const ICC_Config_t *obj)
{
    ICC_Config_t *relocated_obj = NULL;

    if (obj && dest_ptr && *dest_ptr) {
        char *dest = *dest_ptr;
        ICC_Channel_Config_t *relocated_channels_config = NULL;
        ICC_Runtime_Shared_t * relocated_runtime = NULL;
#ifdef ICC_CFG_HEARTBEAT_ENABLED
        ICC_Heartbeat_Os_Config_t *heartbeat = NULL, *relocated_heartbeat = NULL;
#endif

        ICC_Channel_Config_t *channels_config = (ICC_Channel_Config_t *)ICC_CROSS_VALUE_OF(obj->Channels_Ptr);
        ICC_Runtime_Shared_t *runtime = (ICC_Runtime_Shared_t *)ICC_CROSS_VALUE_OF(obj->ICC_Initialized_Shared);

        relocated_obj = (ICC_Config_t *)dest;
        COPY_OBJ(dest, *obj);

        relocated_obj->This_Ptr = (uint64_t)relocated_obj;

        if (channels_config) {
            relocated_channels_config = RELOCATE_ICC_ChannelsConfig_Array_t(
                &dest, channels_config, ICC_CROSS_VALUE_OF(obj->Channels_Count));

            ICC_CROSS_ASSIGN(relocated_obj->Channels_Ptr, relocated_channels_config);
        }
        if (runtime) {
            relocated_runtime = (ICC_Runtime_Shared_t *)dest;
            COPY_OBJ(dest, *runtime);

            ICC_CROSS_ASSIGN(relocated_obj->ICC_Initialized_Shared, &(relocated_runtime->ICC_Initialized_Shared));
            ICC_CROSS_ASSIGN(relocated_obj->ICC_Channels_Ram_Shared, (relocated_runtime->ICC_Channels_Ram_Shared));
            ICC_CROSS_ASSIGN(relocated_obj->ICC_Fifo_Ram_Shared, &(relocated_runtime->ICC_Fifo_Ram_Shared));
            ICC_CROSS_ASSIGN(relocated_obj->ICC_Node_Sig_Fifo_Ram_Shared, &(relocated_runtime->ICC_Node_Sig_Fifo_Ram_Shared));
        }

#ifdef ICC_CFG_HEARTBEAT_ENABLED
        heartbeat = ICC_CROSS_VALUE_OF(obj->ICC_Heartbeat_Os_Config);
        if (heartbeat) {
            relocated_heartbeat = dest;
            COPY_OBJ(dest, *heartbeat);

            ICC_CROSS_ASSIGN(relocated_obj->ICC_Heartbeat_Os_Config, relocated_heartbeat);
        }
#endif

        *dest_ptr = dest;
    }

    return relocated_obj;
}

#endif /* ICC_RELOCATE_H */
