/**
*   @file    ICC_Config_Test.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication Config Logging Support
*   @details       Support for Logging Inter Core Communication Config data in
*                  a human readable format.
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

#include "ICC_Config.h"

#ifndef ICC_CONFIG_TEST_H
#define ICC_CONFIG_TEST_H

#define PRINT(...) count += printk(KERN_ALERT __VA_ARGS__)

#define ROWSIZE 32
#define GROUPSIZE 4

#define STR(s) XSTR(s)
#define XSTR(s) #s
#define PR_FIELD(obj, field) PRINT(STR(field) "=%d\n", obj->field);
#define PR_FIELDX(obj, field) PRINT(STR(field) "=%x\n", obj->field);
#define PR_FIELDADDR(obj, field) PRINT(STR(field) "=%llx\n", (uint64_t)ICC_CROSS_VALUE_OF(obj->field));
#define PR_FIELDLLX(obj, field) PRINT(STR(field) "=%llx\n", obj->field);

int print_ICC_hex_dump(const void *buf, size_t len, bool ascii)
{
    int count = 0;
    const u8 *ptr = buf;
    int i, linelen, remaining = len;
    int rowsize = (ascii ? ROWSIZE / 2 : ROWSIZE);
    unsigned char linebuf[ROWSIZE * 3 + 2 + ROWSIZE + 1];

    for (i = 0; i < len; i += rowsize) {
        linelen = min(remaining, rowsize);
        remaining -= rowsize;

        hex_dump_to_buffer(ptr + i, linelen, rowsize, GROUPSIZE,
                           linebuf, sizeof(linebuf), ascii);

        PRINT("%04x: %s\n", i, linebuf);
    }

    return count;
}

#define PR_FIELDARRAYX(obj, field, ascii) \
    {   \
        PRINT(STR(field) "[%d] (%p):\n", sizeof(obj->field), obj->field); \
        count += print_ICC_hex_dump(obj->field, sizeof(obj->field), ascii); \
    }

#define PR_ARRAYX(name, array, num_elements) \
    {   \
        PRINT("%s[%d] (%p):\n", name, num_elements, array); \
        count += print_ICC_hex_dump(array, num_elements, false); \
    }

int print_ICC_Fifo_Config_t(ICC_Fifo_Config_t * obj)
{
    int count = 0;

    if (!obj) {
        return count;
    }

    PR_FIELDADDR(obj, fifo_buffer_ptr);
    {
        char * fifo_ptr = ICC_OS_Phys_To_Virt((uint8_t*)ICC_CROSS_VALUE_OF(obj->fifo_buffer_ptr));
#ifndef ICC_BUILD_FOR_M4
        PRINT ("%p relocated to %p\n", obj->fifo_buffer_ptr, fifo_ptr);
#endif
        PR_ARRAYX("fifo_buffer_ptr", fifo_ptr, obj->fifo_size);
    }

    PR_FIELD(obj, fifo_prio);
    PR_FIELD(obj, fifo_size);
    PR_FIELD(obj, max_msg_size);
    PR_FIELD(obj, alignment);
    PR_FIELDX(obj, fifo_flags);

    return count;
}

int print_ICC_Channel_Config_t_Array(char * obj, int size)
{
    int count = 0;
    int i, chan_count;
    if (!obj) {
        return count;
    }

    chan_count = size / sizeof(ICC_Channel_Config_t);
    for (i = 0; i < chan_count; i++) {
        ICC_Channel_Config_t * config = (ICC_Channel_Config_t *)obj + i;
        count += print_ICC_Fifo_Config_t(&(config->fifos_cfg[0]));
        count += print_ICC_Fifo_Config_t(&(config->fifos_cfg[1]));
        PR_FIELDLLX(config, Channel_Update_Cb);
        PR_FIELDLLX(config, Channel_Rx_Cb);
        PR_FIELDLLX(config, Channel_Tx_Cb);
    }

    return count;
}

#define member_size(type, member) sizeof(((type *)0)->member)

int print_ICC_Config_t(ICC_Config_t * obj)
{
    int count = 0;
    if (!obj) {
        return count;
    }

    PR_FIELDARRAYX(obj, Config_Magic, true);
    PR_FIELDADDR(obj, Channels_Count);
    PR_FIELDADDR(obj, Channels_Ptr);

    {
        char * chan_config = ICC_OS_Phys_To_Virt((char *)(ICC_CROSS_VALUE_OF(obj->Channels_Ptr)));
#ifndef ICC_BUILD_FOR_M4
        PRINT ("%p relocated to %p\n", obj->Channels_Ptr, chan_config);
#endif
        count += print_ICC_Channel_Config_t_Array(chan_config, sizeof(ICC_Cfg0_ChannelsConfig));
    }

#ifdef ICC_CFG_HEARTBEAT_ENABLED
    /* TODO: print function not implemented.
     * When implemented, the call should look something like:
     * count += print_ICC_Heartbeat_Os_Config((char *)*ICC_CROSS_VALUE_OF(obj->ICC_Heartbeat_Os_Config), sizeof(ICC_Heartbeat_Os_Config0), test_reloc);
     */
#endif

    PR_FIELDADDR(obj, Node_Update_Cb);
    PR_FIELDADDR(obj, ICC_Initialized_Shared);
    PR_FIELDADDR(obj, ICC_Channels_Ram_Shared);
    PR_FIELDADDR(obj, ICC_Fifo_Ram_Shared);
    PR_FIELDADDR(obj, ICC_Node_Sig_Fifo_Ram_Shared);

    {
        char * initialized = ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(obj->ICC_Initialized_Shared));
        char * channels_ram = ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(obj->ICC_Channels_Ram_Shared));
        char * fifo_ram = ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(obj->ICC_Fifo_Ram_Shared));
        char * sig_fifo_ram = ICC_OS_Phys_To_Virt(ICC_CROSS_VALUE_OF(obj->ICC_Node_Sig_Fifo_Ram_Shared));

#ifndef ICC_BUILD_FOR_M4
        PRINT ("%p relocated to %p\n", obj->ICC_Initialized_Shared, initialized);
        PRINT ("%p relocated to %p\n", obj->ICC_Channels_Ram_Shared, channels_ram);
        PRINT ("%p relocated to %p\n", obj->ICC_Fifo_Ram_Shared, fifo_ram);
        PRINT ("%p relocated to %p\n", obj->ICC_Node_Sig_Fifo_Ram_Shared, sig_fifo_ram);
#endif


        PR_ARRAYX("ICC_Initialized_Shared", initialized, member_size(struct ICC_Runtime_Shared_t, ICC_Initialized_Shared));
        PR_ARRAYX("ICC_Channels_Ram_Shared", channels_ram, member_size(struct ICC_Runtime_Shared_t, ICC_Channels_Ram_Shared));
        PR_ARRAYX("ICC_Fifo_Ram_Shared", fifo_ram, member_size(struct ICC_Runtime_Shared_t, ICC_Fifo_Ram_Shared));
        PR_ARRAYX("ICC_Node_Sig_Fifo_Ram_Shared", sig_fifo_ram, member_size(struct ICC_Runtime_Shared_t, ICC_Node_Sig_Fifo_Ram_Shared));
    }

    return count;
}

#define ICC_DUMP_OBJ(dump_count, type, obj) \
        {   int count = 0; \
            PRINT("===========================\nTesting " STR(obj) "\n"); \
            PRINT(STR(obj) " (%08llx)\n", (uint64_t)&obj); \
            count += print_##type(&obj, 0); \
            if (__builtin_types_compatible_p(type, ICC_Config_t)) { \
                  PRINT("This_Ptr=%08llx\n", obj.This_Ptr); \
            } \
            dump_count += count; \
        }

#define ICC_DUMP_PTR(dump_count, type, obj) \
        {   int count = 0; \
            PRINT("===========================\nTesting " STR(obj) "\n"); \
            PRINT(STR(obj) " (%08llx)\n", (uint64_t)obj); \
            if (__builtin_types_compatible_p(type, ICC_Config_t)) { \
                  PRINT("This_Ptr=%08llx\n", (ICC_Config_t *)obj->This_Ptr); \
            } \
            count += print_##type(obj); \
            dump_count += count; \
        }

#define ICC_DUMP_ARRAY(dump_count, type, obj) \
        {   int count = 0; \
            PRINT("===========================\nTesting " STR(obj) "\n"); \
            PRINT(STR(obj) "(%08llx)\n", (uint64_t)obj); \
            count += print_##type##_Array(obj, sizeof(obj)); \
            dump_count += count; \
        }


#endif /* ICC_CONFIG_TEST_H */
