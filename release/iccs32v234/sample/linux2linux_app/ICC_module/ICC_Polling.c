/**
*   @file    ICC_lib.c
*   @version 0.0.2
*
*   @brief   ICC - Inter Core Communication polling support
*   @details       Inter Core Communication polling support
*
*   @addtogroup [ICC]
*   @{
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : ARM
*   Peripheral           :
*   Dependencies         : none
*
*   Build Version        :
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>

#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>

#define LOG_LEVEL       KERN_ALERT

#ifdef ICC_USE_POLLING

#include <linux/kthread.h>
#include <linux/delay.h>

#include "ICC_Api.h"
#include "ICC_Platform.h"
#include "ICC_time.h"

/* Value for 'incoming data from peer' */
#define WAKE_UP_PATTERN     0x42  /* answer to life the universe and everything */
/* Waiting value */
#define WAIT_PATTERN        0x0

struct ping_poll {
	uint32_t *poll_addr;
	uint32_t *ping_addr;
	struct task_struct *poll_thread;
	bool terminate_communication;
};

static struct ping_poll icc_polling = {0, 0, 0, false};

ICC_ATTR_SEC_TEXT_CODE
extern void
ICC_Remote_Event_Handler(void);

static int poll_thread_fn(void *arg)
{
    uint32_t *addr = (uint32_t *)arg;

    while (!kthread_should_stop()) {
        ICC_Sleep();
        if (*addr == WAKE_UP_PATTERN) {
            /* event detected */
            ICC_Remote_Event_Handler();
        }
    }

    icc_polling.poll_thread = NULL; /* notify completion */

    return 0;
}

int shmem_poll_init(void)
{
    int err = 0;

    icc_polling.poll_addr = ioremap_nocache(get_shmem_poll_addr(), SZ_4K);
    if (!icc_polling.poll_addr) {
        pr_warn("Could not ioremap %#llx\n", get_shmem_poll_addr());
        err = -EIO;
        goto ioremap_error;
    }
    /* Initialize it before we poll for changes */
    *icc_polling.poll_addr = WAIT_PATTERN;

    /* Start the polling thread */
    icc_polling.poll_thread = kthread_run(poll_thread_fn, icc_polling.poll_addr, "shmem_poll_thread");
    if (IS_ERR(icc_polling.poll_thread)) {
        pr_warn("Error starting poll thread");
        err = -EFAULT;
        goto poll_thread_error;
    }

    return 0;

poll_thread_error:
    iounmap(icc_polling.poll_addr);
    icc_polling.poll_addr = NULL;
    icc_polling.poll_thread = NULL;
ioremap_error:
    return err;
}

void shmem_poll_exit(void)
{
    /* terminate functions waiting in module context */
    icc_polling.terminate_communication = true;

    /* terminate the spawned kthreads */
    if (icc_polling.poll_thread) {
        kthread_stop(icc_polling.poll_thread);
    }

    do {
        /* wait for the kthreads to actually stop, before cutting off the addresses */
        msleep_interruptible(DEFAULT_TIMEOUT_MS * 10);
    } while (icc_polling.poll_thread);

    iounmap(icc_polling.poll_addr);
    icc_polling.poll_addr = NULL;
}

int shmem_ping_init(void)
{
    icc_polling.ping_addr = ioremap_nocache(get_shmem_ping_addr(), SZ_4K);
    if (!icc_polling.ping_addr) {
        pr_warn("Could not ioremap %#x\n", get_shmem_ping_addr());
        return -EIO;
    }

    return 0;
}

void shmem_ping_exit(void)
{
    iounmap(icc_polling.ping_addr);
    icc_polling.ping_addr = NULL;
}

ICC_ATTR_SEC_TEXT_CODE
ICC_Err_t ICC_Notify_Peer( void )
{
    if (!icc_polling.ping_addr) {
        return ICC_ERR_PARAM_INVAL;
    }

    /* wait for the peer to become available */
    while (!icc_polling.terminate_communication && (*icc_polling.ping_addr != WAIT_PATTERN)) {
        ICC_Sleep();
    }

    if (icc_polling.terminate_communication) {
        return ICC_ERR_TIMEOUT;
    }

    *icc_polling.ping_addr = WAKE_UP_PATTERN;

    return ICC_SUCCESS;
}

#ifndef ICC_BUILD_FOR_M4

void ICC_Notify_Peer_Alive( void )
{
    if (icc_polling.ping_addr) {

        struct handshake * phshake = (struct handshake *)icc_polling.ping_addr;
        /* BAR attributes are initialized on the RC side only */
        struct s32v_bar icc_bar;
        pcie_init_bar(&icc_bar);

        phshake->rc_bar = icc_bar;
        phshake->rc_ddr_addr = get_shmem_poll_addr();
    }
}

#else

ICC_Err_t ICC_Wait_For_Peer( void )
{
    if (icc_polling.poll_addr) {
        struct handshake * phshake;

        while (!icc_polling.terminate_communication && (*icc_polling.poll_addr == WAIT_PATTERN)) {
            ICC_Sleep();
        }

        if (icc_polling.terminate_communication) {
            return ICC_ERR_TIMEOUT;
        }

        phshake = (struct handshake *)icc_polling.poll_addr;
        pcie_init_outbound(phshake);

        *icc_polling.poll_addr = WAIT_PATTERN;

        return ICC_SUCCESS;
    }

    return ICC_ERR_PARAM_INVAL;
}

#endif  /* ICC_BUILD_FOR_M4 */

void ICC_Clear_Notify_From_Peer( void )
{
    if (icc_polling.poll_addr) {
        *icc_polling.poll_addr = WAIT_PATTERN;
    }
}

#if defined(ICC_CFG_LOCAL_NOTIFICATIONS)
void ICC_Notify_Local( void )
{
    // not supported
}

void ICC_Clear_Notify_Local( void )
{
    // not supported
}
#endif

#endif  /* ICC_USE_POLLING */
