/**
*   @file    ICC_Fifo.c
*   @version 0.8.0
*
*   @brief   ICC - Inter Node Communication FIFO API implementation
*   @details Inter Node Communication FIFO API implementation
*
*   @addtogroup [ICC]
*   @{
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : Arm Architecture
*   Peripheral           : 
*   Dependencies         : none
*
*   SW Version           : 0.8.0
*   Build Version        : SAC58R_ICC_0.8.0
*
*   (c) Copyright 2014 Freescale Semiconductor Inc.
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
/*==================================================================================================
==================================================================================================*/


#ifdef __cplusplus
extern "C"
{
#endif


#include "ICC_Config.h"

#include "ICC_Fifo.h"
#include "ICC_Types.h"

#include "ICC_Hw.h"


        #include <linux/string.h>


#define ALIGN_WRITE(size)                                                                                       \
        queue_ICC->tail = queue_ICC->tail + (size);                                                             \
        if (excess_align)                                                                                       \
        {                                                                                                       \
            queue_ICC->tail += fifo_cfg->alignment - excess_align;                                              \
        }

#define ALIGN_READ(size)                                                                                        \
        queue_ICC->head = queue_ICC->head + (size);                                                             \
        if (excess_align)                                                                                       \
        {                                                                                                       \
            queue_ICC->head += fifo_cfg->alignment - excess_align;                                              \
        }


#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"


/*
* Use this to determine if the FIFO head is before/after its tail.
* Returns:
*   (*)  0, if tail address >= head address
    (**) 1, if tail address < head address 
*/
ICC_ATTR_SEC_TEXT_CODE
static inline unsigned int ICC_FIFO_HeadTail_Order( ICC_FIFO_IN ICC_Fifo_Ram_t * queue_ICC )
{
     /* In the caller function cache invalidation for queue_ICC->tail and queue_ICC->head has been already done
      * by a previous function (ICC_FIFO_Pending, ICC_FIFO_Free) or by itself */

    return ( queue_ICC->tail >= queue_ICC->head ) ? 0 : 1;
}

/*
 * initialize the ICC_Fifo_Ram_t structure
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Init(ICC_FIFO_IN       ICC_Fifo_Ram_t       * queue_ICC,
              ICC_FIFO_IN const ICC_Fifo_Config_t    * fifo_conf,
              ICC_FIFO_IN       ICC_Fifo_Os_Ram_t    * fifo_os_ram,
              ICC_FIFO_IN       unsigned int           init)
{
    queue_ICC->fifo_config[ ICC_GET_CORE_ID ] = fifo_conf; /**< set link to fifo configuration from channel structure */
    queue_ICC->fifo_os_ram[ ICC_GET_CORE_ID ] = fifo_os_ram;

    queue_ICC->rd[ ICC_GET_CORE_ID ]          = 0;
    queue_ICC->wr[ ICC_GET_CORE_ID ]          = 0;

    if (init)
    {
        queue_ICC->head                       = 0;
        queue_ICC->tail                       = 0;

        queue_ICC->pending_send_msg_size      = 0;
    }

    ICC_DCACHE_FLUSH_MLINES( (addr_t)queue_ICC, sizeof(ICC_Fifo_Ram_t) );

    return ICC_SUCCESS;
}



/*
 * simplified version of Push for the header only
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Push_Header(ICC_FIFO_IN ICC_Fifo_Ram_t   * queue_ICC,
                     ICC_FIFO_IN ICC_Header_t     * header)
{

    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ];  /**< shortcut variable */
    unsigned int excess_align, wrapped_around, gap_to_end;
    void * intermediate_data = NULL_PTR;
    void * fifo_ptr = NULL_PTR;

    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->tail, sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->head, sizeof(unsigned int) );

    gap_to_end = fifo_cfg->fifo_size - queue_ICC->tail;  /* distance from tail to the end of data buffer */

    excess_align = 0; /* header is written without alignment;  during push data, the message bytes ( header + payload ) will be aligned */

    wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

    if ( ((wrapped_around == 0U) && (ICC_HEADER_SIZE <= gap_to_end)) || (wrapped_around == 1U))
    {

        fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->tail;
        memcpy(fifo_ptr, header, ICC_HEADER_SIZE);
        ALIGN_WRITE(ICC_HEADER_SIZE);

        ICC_DCACHE_FLUSH_MLINES( (addr_t) fifo_ptr, ICC_HEADER_SIZE );
    }
    else
    {
        if ( (wrapped_around == 0U) && (gap_to_end < ICC_HEADER_SIZE) ) {
            fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->tail;
            memcpy(fifo_ptr, header, gap_to_end);

            ICC_DCACHE_FLUSH_MLINES( (addr_t) fifo_ptr, gap_to_end );

            /* write the remained data */
            queue_ICC->tail = 0;

            fifo_ptr = fifo_cfg->fifo_buffer_ptr;
            intermediate_data = (char *)header + gap_to_end;
            memcpy(fifo_ptr, intermediate_data, ICC_HEADER_SIZE - gap_to_end);

            ALIGN_WRITE(ICC_HEADER_SIZE - gap_to_end);

            ICC_DCACHE_FLUSH_MLINES( (addr_t) fifo_ptr, ICC_HEADER_SIZE - gap_to_end );
        }
    }

    ICC_DCACHE_FLUSH_MLINES( (addr_t)&(queue_ICC->tail), sizeof(unsigned int) );

    return ICC_SUCCESS;
}

/*
 * simplified version of Pop for the header only
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Pop_Header(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC,
                    ICC_FIFO_OUT ICC_Header_t     * header)
{
        const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ]; /**< shortcut variable */
        unsigned int excess_align, pending_bytes, wrapped_around, gap_to_end;
        void * fifo_ptr = NULL_PTR;

        /* ICC_FIFO_Pending will do cache invalidation for queue_ICC->tail and queue_ICC->head */

        pending_bytes = ICC_FIFO_Pending(queue_ICC);  /* find out the value of the pending byte to be processed */

        wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

        gap_to_end = fifo_cfg->fifo_size - queue_ICC->head;  /* distance from tail to the end of data buffer */

          excess_align = 0; /* header was written without alignment;  during push data, the message bytes ( header + payload ) was aligned */

        if (ICC_HEADER_SIZE > pending_bytes)
        {
            return ICC_ERR_FIFO_EMPTY;
        }
        else
        {
            if ( ((wrapped_around != 0U) && (ICC_HEADER_SIZE <= gap_to_end)) || (wrapped_around == 0U))
            {
                fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;

                ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, ICC_HEADER_SIZE );

                memcpy(header, fifo_ptr, ICC_HEADER_SIZE);
                ALIGN_READ(ICC_HEADER_SIZE);
            }
            else
            {
                if ( (wrapped_around != 0U) && (gap_to_end < ICC_HEADER_SIZE) ) {
                    fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;

                    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, gap_to_end );

                    memcpy(header, fifo_ptr, gap_to_end);  /* firstly, copy until the end of the buffer */

                    queue_ICC->head = 0;                   /* place head at the beginning of the buffer */
                    fifo_ptr = fifo_cfg->fifo_buffer_ptr;
                    header = (ICC_Header_t *)( (char*)header + gap_to_end );

                    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, ICC_HEADER_SIZE - gap_to_end );

                    memcpy(header, fifo_ptr, ICC_HEADER_SIZE - gap_to_end);  /* copy the rest of the data*/

                    ALIGN_READ(ICC_HEADER_SIZE - gap_to_end);
                }
            }
        }

        ICC_DCACHE_FLUSH_MLINES( (addr_t)&queue_ICC->head, sizeof(unsigned int) );

    return ICC_SUCCESS;

}


/*
 * simplified version of Peek for the header only
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Peek_Header(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC,
                     ICC_FIFO_OUT ICC_Header_t     * header)
{

    unsigned int pending_bytes, wrapped_around, gap_to_end;
    void * fifo_ptr = NULL_PTR;
    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ];  /**< shortcut variable */;

    /* ICC_FIFO_Pending will do cache invalidation for queue_ICC->tail and queue_ICC->head */

    pending_bytes = ICC_FIFO_Pending(queue_ICC);  /* find out the value of the pending byte to be processed */

    wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

    gap_to_end = fifo_cfg->fifo_size - queue_ICC->head;  /* distance from tail to the end of data buffer */

    if (ICC_HEADER_SIZE > pending_bytes)
    {
        return ICC_ERR_FIFO_EMPTY;
    }
    else
    {
        if ( ((wrapped_around != 0U) && (ICC_HEADER_SIZE <= gap_to_end)) || (wrapped_around == 0U))
        {
            fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;

            ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, ICC_HEADER_SIZE );

            memcpy(header, fifo_ptr, ICC_HEADER_SIZE);
        }
        else
        {
            if ( (wrapped_around != 0U) && (gap_to_end < ICC_HEADER_SIZE) ) {
                fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;

                ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, gap_to_end );

                memcpy(header, fifo_ptr, gap_to_end); /* firstly, copy until the end of the buffer */

                ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_cfg->fifo_buffer_ptr, ICC_HEADER_SIZE - gap_to_end );

                header = (ICC_Header_t *)( (char*)header + gap_to_end );
                memcpy(header, fifo_cfg->fifo_buffer_ptr, ICC_HEADER_SIZE - gap_to_end); /* copy the rest of the data*/
            }
        }
    }

    return ICC_SUCCESS;

}



/*
 * push all the data that is handled to it
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Push( ICC_FIFO_IN ICC_Fifo_Ram_t    * queue_ICC,
               ICC_FIFO_OUT const void   * data,
               ICC_FIFO_IN unsigned int   size )
{
    unsigned int excess_align, wrapped_around, gap_to_end;
    void * intermediate_data = NULL_PTR, * fifo_ptr = NULL_PTR;
    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ];  /**< shortcut variable */;


    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->head, sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->tail, sizeof(unsigned int) );

    gap_to_end = fifo_cfg->fifo_size - queue_ICC->tail;  /* distance from tail to the end of data buffer */

    excess_align = (ICC_HEADER_SIZE + size) & (fifo_cfg->alignment - 1);

    wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

    if ( ((wrapped_around == 0U) && (size <= gap_to_end)) || (wrapped_around != 0U))
    {
        fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->tail;
        memcpy(fifo_ptr, data, size);
        ALIGN_WRITE(size);

        ICC_DCACHE_FLUSH_MLINES( (addr_t) fifo_ptr, size );

    }
    else
    {
        if ( (wrapped_around == 0U) && (gap_to_end < size) ) {
            fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->tail;
            memcpy(fifo_ptr, data, gap_to_end);

            ICC_DCACHE_FLUSH_MLINES( (addr_t) fifo_ptr, gap_to_end );

            /* write the remained data */
            queue_ICC->tail = 0;
            fifo_ptr = fifo_cfg->fifo_buffer_ptr;
            intermediate_data = (char *)data + gap_to_end;
            memcpy(fifo_ptr, intermediate_data, size - gap_to_end);
            ALIGN_WRITE(size - gap_to_end);

            ICC_DCACHE_FLUSH_MLINES( (addr_t) fifo_ptr, size - gap_to_end );
        }
    }

    ICC_DCACHE_FLUSH_MLINES( (addr_t) &queue_ICC->tail, sizeof(unsigned int) );

    return ICC_SUCCESS;
}

/*
 * pop specified amount of data to buffer
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Pop( ICC_FIFO_IN ICC_Fifo_Ram_t   * queue_ICC,
              ICC_FIFO_OUT void        * data,
              ICC_FIFO_IN unsigned int   size )
{
    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ]; /**< shortcut variable */
    unsigned int excess_align, pending_bytes, wrapped_around, gap_to_end;
    void * fifo_ptr = NULL_PTR;

    /* ICC_FIFO_Pending will do cache invalidation for queue_ICC->tail and queue_ICC->head */

    pending_bytes = ICC_FIFO_Pending(queue_ICC);  /* find out the value of the pending byte to be processed */

    wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

    excess_align = (ICC_HEADER_SIZE + size) & (fifo_cfg->alignment - 1);

    gap_to_end = fifo_cfg->fifo_size - queue_ICC->head;  /* distance from tail to the end of data buffer */

    if (size > pending_bytes)
    {
        return ICC_ERR_FIFO_EMPTY;
    }
    else
    {
        if ( ((wrapped_around != 0U) && (size <= gap_to_end)) || (wrapped_around == 0U)) 
        {
            fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;
            if (NULL_PTR != data){

                ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, size );

                memcpy(data, fifo_ptr, size);
            }
            ALIGN_READ(size);
        }
        else
        {
            if ( (wrapped_around != 0U) && (gap_to_end < size) ) {
                fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;
                if (NULL_PTR != data){

                    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, gap_to_end );

                    memcpy(data, fifo_ptr, gap_to_end);  /* firstly, copy until the end of the buffer */
                }

                queue_ICC->head = 0;    /* place head at the beginning of the buffer */
                fifo_ptr = fifo_cfg->fifo_buffer_ptr;

                if (NULL_PTR != data){
                    data = data + gap_to_end;

                    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, size - gap_to_end );

                    memcpy(data, fifo_ptr, size - gap_to_end);  /* copy the rest of the data*/
                }

                ALIGN_READ(size - gap_to_end);
            }
        }
    }

    ICC_DCACHE_FLUSH_MLINES( (addr_t) &queue_ICC->head, sizeof(unsigned int) );


    return ICC_SUCCESS;
}

/*
 * like pop but do not free the fifo buffer
 */
ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_FIFO_Peek( ICC_FIFO_IN ICC_Fifo_Ram_t   * queue_ICC,
               ICC_FIFO_OUT void         * data,
               ICC_FIFO_IN unsigned int   size )
{
    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ];  /**< shortcut variable */
    unsigned int pending_bytes, wrapped_around, gap_to_end;
    void * fifo_ptr = NULL_PTR;

    /* ICC_FIFO_Pending will do cache invalidation for queue_ICC->tail and queue_ICC->head */

    pending_bytes = ICC_FIFO_Pending(queue_ICC);  /* find out the value of the pending byte to be processed */

    wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

    gap_to_end = fifo_cfg->fifo_size - queue_ICC->head;  /* distance from tail to the end of data buffer */

    if (size > pending_bytes)
    {
        return ICC_ERR_FIFO_EMPTY;
    }
    else
    {
        if ( ((wrapped_around != 0U) && (size <= gap_to_end)) || (wrapped_around == 0U)) 
        {
            fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;

            ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, size );

            memcpy(data, fifo_ptr, size);
        }
        else
        {
            if ( (wrapped_around != 0U) && (gap_to_end < size) ) {
                fifo_ptr = fifo_cfg->fifo_buffer_ptr + queue_ICC->head;

                ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_ptr, gap_to_end );

                memcpy(data, fifo_ptr, gap_to_end); /* firstly, copy until the end of the buffer */

                data = data + gap_to_end;

                ICC_DCACHE_INVALIDATE_MLINES( (addr_t) fifo_cfg->fifo_buffer_ptr, size - gap_to_end );

                memcpy(data, fifo_cfg->fifo_buffer_ptr, size - gap_to_end); /* copy the rest of the data*/
            }
        }
    }

    return ICC_SUCCESS;
}

/*
 * get fifo pending data in bytes
 */
ICC_ATTR_SEC_TEXT_CODE
unsigned int 
ICC_FIFO_Pending( ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC )
{

    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ]; /**< shortcut variable */
    unsigned int wrapped_around;
    int gap_to_end, distance_to_tail;

    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->tail, sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->head, sizeof(unsigned int) );

    wrapped_around = ICC_FIFO_HeadTail_Order( queue_ICC );

    if (wrapped_around != 0U)  /* wrapped around, tail has started from beginning */
    {

        /* case tail <= head */

        gap_to_end = fifo_cfg->fifo_size - queue_ICC->head;  /* distance from head to the end of data buffer */

        return (queue_ICC->tail + gap_to_end);
    }
    else  /* initially tail is in front of head */
    {
        /* case head <= tail */
        distance_to_tail = queue_ICC->tail - queue_ICC->head;

        return distance_to_tail;
    }
}

/*
 * get fifo free data in bytes
 */
ICC_ATTR_SEC_TEXT_CODE
unsigned int 
ICC_FIFO_Free( ICC_FIFO_IN ICC_Fifo_Ram_t    * queue_ICC )
{
    unsigned int wrapped_around, gap_to_end, distance_to_head;
    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ]; /**< shortcut variable */;


    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->tail, sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &queue_ICC->head, sizeof(unsigned int) );

    wrapped_around = ICC_FIFO_HeadTail_Order( queue_ICC );

    if (wrapped_around != 0U)  /* wrapped around, tail has started from beginning */
    {
        /* case tail <= head */
        distance_to_head = queue_ICC->head - queue_ICC->tail;

        return distance_to_head;
    }
    else  /* initially tail is in front of head */
    {
        /* case head <= tail */

        gap_to_end = fifo_cfg->fifo_size - queue_ICC->tail;  /* distance from tail to the end of data buffer */

       return (queue_ICC->head + gap_to_end);
    }
}


/*
 * check if the specified message fits into the fifo
 *
 * ICC_SUCCESS - is returned if there is enough space available in the fifo to fit the
 *               specified message size
 *
 * ICC_ERR_FIFO_FULL - otherwise
 *
 */

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t
ICC_Fifo_Msg_Fits( ICC_FIFO_IN  ICC_Fifo_Ram_t * queue_ICC,
                   ICC_FIFO_IN  unsigned int     msg_size  )
{
    const ICC_Fifo_Config_t * fifo_cfg = queue_ICC->fifo_config[ ICC_GET_CORE_ID ]; /**< shortcut variable */
    unsigned int excess_align, free_bytes, wrapped_around, real_size, gap_to_end;
    unsigned int size = ICC_HEADER_SIZE + msg_size;                                    /* calculate HEADER_SIZE + msg_size */


    excess_align = size & (fifo_cfg->alignment - 1);

    /* ICC_FIFO_Free will do cache invalidation for queue_ICC->tail and queue_ICC->head */

    free_bytes     = ICC_FIFO_Free(queue_ICC);            /* find out the value of free bytes of FIFO */

    wrapped_around = ICC_FIFO_HeadTail_Order(queue_ICC);  /* find out the value of wrapped_around */

    gap_to_end = fifo_cfg->fifo_size - queue_ICC->tail;   /* distance from tail to the end of data buffer */

    real_size = ((excess_align == 0) ? size : size + fifo_cfg->alignment - excess_align);

    if (real_size > free_bytes)
    {
        return ICC_ERR_FIFO_FULL;
    }
    else
    {
        if ( ((wrapped_around == 0U) && (size <= gap_to_end)) || (wrapped_around != 0U))
        {
            if( wrapped_around && (real_size == free_bytes) )
            {
                return ICC_ERR_FIFO_FULL;
            }
        }
        else
        {
            if ( (wrapped_around == 0U) && (gap_to_end < size) ){
                if ( real_size == free_bytes )
                {
                    return ICC_ERR_FIFO_FULL;
                }
            }
        }
    }

    return ICC_SUCCESS;

}

/*
 * wr signals pending ?
 */
ICC_ATTR_SEC_TEXT_CODE
unsigned int
ICC_FIFO_Msg_Wr_Sig_Pending( ICC_FIFO_IN  ICC_Fifo_Ram_t * queue_ICC ){
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &(queue_ICC->wr[ICC_GET_CORE_ID]), sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &(queue_ICC->wr[ICC_GET_REMOTE_CORE_ID]), sizeof(unsigned int) );
    return ((queue_ICC)->wr[ICC_GET_CORE_ID] != (queue_ICC)->wr[ICC_GET_REMOTE_CORE_ID]);
}

/*
 * rd signals pending ?
 */
ICC_ATTR_SEC_TEXT_CODE
unsigned int
ICC_FIFO_Msg_Rd_Sig_Pending( ICC_FIFO_IN  ICC_Fifo_Ram_t *queue_ICC ){

    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &(queue_ICC->rd[ICC_GET_CORE_ID]), sizeof(unsigned int) );
    ICC_DCACHE_INVALIDATE_MLINES( (addr_t) &(queue_ICC->rd[ICC_GET_REMOTE_CORE_ID]), sizeof(unsigned int) );
    return ((queue_ICC)->rd[ICC_GET_CORE_ID] != (queue_ICC)->rd[ICC_GET_REMOTE_CORE_ID]);
}



#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"

#ifdef __cplusplus
}
#endif
