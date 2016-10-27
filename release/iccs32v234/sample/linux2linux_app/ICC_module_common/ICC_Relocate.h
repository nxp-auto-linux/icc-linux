/*
 * ICC_Relocate.h
 *
 *  Created on: Oct 27, 2016
 *      Author: vcvv001
 */

#ifndef ICC_RELOCATE_H
#define ICC_RELOCATE_H

#define RELOCATABLE(obj) typeof(obj)* obj##_Rel

#define RELOCATED_PTR(obj) obj##_Rel

/* dest is an integer representing the destination address
   obj is an object (NOT pointer to an object!)
*/
#define RELOCATE_OBJ(dest, obj) \
    { \
      memcpy((char*)dest, (char*)(&obj), sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sizeof(obj)); \
    }

#define RELOCATE_RAW(dest, src_ptr, sz) \
    { \
      memcpy((char*)dest, (char*)(*src_ptr), sz); \
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
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
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
          ICC_Fifo_Config_t * relocated_cfg = &((*RELOCATED_PTR(obj))[i].fifos_cfg[0]); \
          RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size); \
          relocated_cfg = &((*RELOCATED_PTR(obj))[i].fifos_cfg[1]); \
          RELOCATE_RAW(dest, &ICC_CROSS_VALUE_OF(relocated_cfg->fifo_buffer_ptr), relocated_cfg->fifo_size); \
      } \
    }

/* TODO: relocate transparently the dependencies such as ICC_Cfg0_ChannelsConfig, ICC_Runtime_Shared etc., without
 *
 */
#define RELOCATE_ICC_Config_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      memcpy((char*)dest, (char*)&obj, sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)RELOCATED_PTR(obj) + (uint64_t)sizeof(obj)); \
      RELOCATED_PTR(obj)->This_Ptr = RELOCATED_PTR(obj); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->Channels_Ptr) = RELOCATED_PTR(ICC_Cfg0_ChannelsConfig); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Initialized_Shared) = &(RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Initialized_Shared); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Channels_Ram_Shared) = (RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Channels_Ram_Shared); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Fifo_Ram_Shared) = &(RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Fifo_Ram_Shared); \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Node_Sig_Fifo_Ram_Shared) = &(RELOCATED_PTR(ICC_Runtime_Shared)->ICC_Node_Sig_Fifo_Ram_Shared); \
    }


#ifdef ICC_CFG_HEARTBEAT_ENABLED
#define RELOCATE_ICC_Config_t_Heartbeat(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      ICC_CROSS_VALUE_OF(RELOCATED_PTR(obj)->ICC_Heartbeat_Os_Config) = RELOCATED_PTR(ICC_Heartbeat_Os_Config0); \
    }
#endif

#define RELOCATE_ICC_Runtime_Shared_t(dest, obj) \
    if (!RELOCATED_PTR(obj)) { \
      RELOCATED_PTR(obj) = (typeof(obj)*)dest; \
      memcpy((char*)dest, (char*)&obj, sizeof(obj)); \
      dest = (typeof(dest))((uint64_t)dest + (uint64_t)sizeof(obj)); \
    }



#endif /* ICC_RELOCATE_H */
