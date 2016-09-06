/**
*   @file    ICC_Fifo.h
*   @version 0.8.0
*
*   @brief   ICC - Inter Core Communication FIFO API
*   @details Inter Core Communication FIFO API
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
*   Build Version        : S32V234_ICC_0.8.0
*
*   (c) Copyright 2014,2016 Freescale Semiconductor Inc.
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

#ifndef ICC_FIFO_H
#define ICC_FIFO_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "ICC_Err.h"   /**< for error codes */

#include "ICC_Types.h" /**< for the FIFO configuration type */


#define ICC_FIFO_IN
#define ICC_FIFO_OUT

#define ICC_START_SEC_TEXT_CODE
#include "ICC_MemMap.h"


void * memcpy (void *dst, const void *src, size_t n);

/*
 * initialize the ICC_Fifo_Ram_t structure
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Init(ICC_FIFO_IN       ICC_Fifo_Ram_t       * queue_ICC,
              ICC_FIFO_IN const ICC_Fifo_Config_t    * fifo_conf,
              ICC_FIFO_IN       ICC_Fifo_Os_Ram_t    * fifo_os_ram,
              ICC_FIFO_IN       unsigned int           init);


/*
 * simplified version of Push for the header only
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Push_Header(ICC_FIFO_IN ICC_Fifo_Ram_t   * queue_ICC,
                     ICC_FIFO_IN ICC_Header_t     * header);

/*
 * simplified version of Pop for the header only
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Pop_Header(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC,
                    ICC_FIFO_OUT ICC_Header_t     * header);

/*
 * simplified version of Peek for the header only
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Peek_Header(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC,
                     ICC_FIFO_OUT ICC_Header_t     * header);




/*
 * push all the data that is handled to it
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Push(ICC_FIFO_IN ICC_Fifo_Ram_t   * queue_ICC,
              ICC_FIFO_IN const void   * data,
              ICC_FIFO_IN unsigned int   size);

/*
 * pop specified amount of data to buffer
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Pop(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC,
             ICC_FIFO_OUT void         * data,
             ICC_FIFO_IN  unsigned int   size);


/* 
 * like pop but do not free the fifo buffer 
 */
ICC_ATTR_SEC_TEXT_CODE
extern
ICC_Err_t
ICC_FIFO_Peek(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC,
              ICC_FIFO_OUT void         * data,
              ICC_FIFO_IN  unsigned int   size);



/*
 * get fifo pending data in bytes
 */
ICC_ATTR_SEC_TEXT_CODE
extern
unsigned int 
ICC_FIFO_Pending(ICC_FIFO_IN  ICC_Fifo_Ram_t   * queue_ICC);

/*
 * get fifo free data in bytes
 */
ICC_ATTR_SEC_TEXT_CODE
extern
unsigned int 
ICC_FIFO_Free( ICC_FIFO_IN  ICC_Fifo_Ram_t    * queue_ICC );


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
extern
ICC_Err_t
ICC_Fifo_Msg_Fits( ICC_FIFO_IN  ICC_Fifo_Ram_t * queue_ICC,
                   ICC_FIFO_IN  unsigned int     msg_size  );

/*
 * signal message written
 */
#define ICC_FIFO_Msg_Wr_Sig(queue_ICC)                                                                        \
            ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&(queue_ICC->wr[ICC_GET_CORE_ID]), sizeof(unsigned int) );  \
            (queue_ICC)->wr[ICC_GET_CORE_ID]++;                                                               \
            ICC_DCACHE_FLUSH_MLINES( (addr_t)&(queue_ICC->wr[ICC_GET_CORE_ID]), sizeof(unsigned int) );
/*
 * ack message written
 */
#define ICC_FIFO_Msg_Wr_Ack(queue_ICC)                                                                        \
            ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&(queue_ICC->wr[ICC_GET_CORE_ID]), sizeof(unsigned int) );  \
            (queue_ICC)->wr[ICC_GET_CORE_ID]++;                                                               \
            ICC_DCACHE_FLUSH_MLINES( (addr_t)&(queue_ICC->wr[ICC_GET_CORE_ID]), sizeof(unsigned int) );

/*
 * wr signals pending ?
 */
ICC_ATTR_SEC_TEXT_CODE
extern
unsigned int
ICC_FIFO_Msg_Wr_Sig_Pending(ICC_FIFO_IN  ICC_Fifo_Ram_t * queue_ICC);


/*
 * signal message read
 */
#define ICC_FIFO_Msg_Rd_Sig(queue_ICC)                                                                        \
            ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&(queue_ICC->rd[ICC_GET_CORE_ID]), sizeof(unsigned int) );  \
            (queue_ICC)->rd[ICC_GET_CORE_ID]++;                                                               \
            ICC_DCACHE_FLUSH_MLINES( (addr_t)&(queue_ICC->rd[ICC_GET_CORE_ID]), sizeof(unsigned int) );
/*
 * ack message read
 */
#define ICC_FIFO_Msg_Rd_Ack(queue_ICC)                                                                        \
            ICC_DCACHE_INVALIDATE_MLINES( (addr_t)&(queue_ICC->rd[ICC_GET_CORE_ID]), sizeof(unsigned int) );  \
            (queue_ICC)->rd[ICC_GET_CORE_ID]++;                                                               \
            ICC_DCACHE_FLUSH_MLINES( (addr_t)&(queue_ICC->rd[ICC_GET_CORE_ID]), sizeof(unsigned int) );

/*
 * rd signals pending ?
 */
ICC_ATTR_SEC_TEXT_CODE
extern
unsigned int
ICC_FIFO_Msg_Rd_Sig_Pending(ICC_FIFO_IN  ICC_Fifo_Ram_t * queue_ICC);





#define ICC_STOP_SEC_TEXT_CODE
#include "ICC_MemMap.h"


#ifdef __cplusplus
}
#endif

#endif /* ICC_FIFO_H */
