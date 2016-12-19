/**
*   @file    ICC_Polling.c
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

#ifdef ICC_USE_POLLING

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include <linux/kthread.h>
#include <linux/delay.h>

#include "ICC_Sw_Platform.h"
#include "ICC_time.h"
#include "ICC_Err.h"
#include "ICC_Pcie.h"
#include "ICC_Log.h"

/* Value for 'incoming data from peer' */
#define WAKE_UP_PATTERN     42  /* answer to life the universe and everything */
/* Waiting value */
#define WAIT_PATTERN        0

void ICC_Remote_Event_Handler(void);

static int poll_thread_fn(void *arg)
{
    struct ping_poll *icc_polling = (struct ping_poll*)arg;
    uint32_t *addr = NULL;

    if (!icc_polling)
        return -EINVAL;

    addr = icc_polling->poll_addr;
    while (!kthread_should_stop()) {
        ICC_Sleep();
        if (*addr == WAKE_UP_PATTERN) {
            /* event detected */
            ICC_Remote_Event_Handler();
        }
    }

    icc_polling->poll_thread = NULL; /* notify completion */

    return 0;
}

int shmem_poll_init(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        int err = 0;
        struct ping_poll *icc_polling = NULL;
        struct device *dev;

        icc_polling = &icc_data->icc_polling;
        if (!icc_polling)
            return -EINVAL;

        dev = &icc_data->pdev->dev;

        icc_polling->poll_addr = devm_ioremap_nocache(dev, get_shmem_poll_phys_addr(), SZ_4K);
        if (!icc_polling->poll_addr) {
            ICC_ERR("Could not ioremap %#llx", get_shmem_poll_phys_addr());
            return -EIO;
        }
        /* Initialize it before we poll for changes */
        *(icc_polling->poll_addr) = WAIT_PATTERN;

        /* Start the polling thread */
        icc_polling->poll_thread = kthread_run(poll_thread_fn, icc_polling, "shmem_poll_thread");
        if (IS_ERR(icc_polling->poll_thread)) {
            ICC_ERR("Error starting poll thread");
            err = -EFAULT;
            goto poll_thread_error;
        }

        return 0;

poll_thread_error:
        icc_polling->poll_addr = NULL;
        icc_polling->poll_thread = NULL;
        return err;
    }

    return -EINVAL;
}

void shmem_poll_exit(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = &(icc_data->icc_polling);

        /* terminate functions waiting in module context */
        icc_polling->terminate_communication = true;

        /* terminate the spawned kthreads */
        if (icc_polling->poll_thread) {
            kthread_stop(icc_polling->poll_thread);
        }

        do {
            /* wait for the kthreads to actually stop, before cutting off the addresses */
            msleep_interruptible(DEFAULT_TIMEOUT_MS * 10);
        } while (icc_polling->poll_thread);

        iounmap(icc_polling->poll_addr);
        icc_polling->poll_addr = NULL;
    }
}

int shmem_ping_init(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = NULL;
        struct device *dev;

        icc_polling = &icc_data->icc_polling;
        if (!icc_polling)
            return -EINVAL;

        dev = &icc_data->pdev->dev;

        icc_polling->ping_addr = devm_ioremap_nocache(dev, get_shmem_ping_phys_addr(), SZ_4K);
        if (!icc_polling->ping_addr) {
            ICC_ERR("Could not ioremap %#x", get_shmem_ping_phys_addr());
            return -EIO;
        }

        return 0;
    }

    return -EINVAL;
}

void shmem_ping_exit(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = &(icc_data->icc_polling);
        icc_polling->ping_addr = NULL;
    }
}

int poll_notify_peer(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = &(icc_data->icc_polling);

        if (!icc_polling->ping_addr)
            return -EINVAL;

        /* wait for the peer to become available */
        while (!icc_polling->terminate_communication && (*(icc_polling->ping_addr) != WAIT_PATTERN))
            ICC_Sleep();

        if (icc_polling->terminate_communication)
            return -ETIMEDOUT;

        *(icc_polling->ping_addr) = WAKE_UP_PATTERN;

        return 0;
    }

    return -EINVAL;
}

#ifndef ICC_BUILD_FOR_M4

int poll_notify_peer_alive(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = &(icc_data->icc_polling);

        if (icc_polling->ping_addr) {

            struct handshake *phshake = (struct handshake *)(icc_polling->ping_addr);
            /* BAR attributes are initialized on the RC side only */
            struct s32v_bar icc_bar;
            pcie_init_bar(&icc_bar);

            phshake->rc_bar = icc_bar;
            phshake->rc_ddr_addr = get_shmem_poll_phys_addr();

            return 0;
        }
    }

    return -EINVAL;
}

#else

int poll_wait_for_peer(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = &(icc_data->icc_polling);

        if (icc_polling->poll_addr) {
            struct handshake * phshake;

            while (!icc_polling->terminate_communication &&
                   (*(icc_polling->poll_addr) == WAIT_PATTERN))
                ICC_Sleep();

            if (icc_polling->terminate_communication)
                return -ETIMEDOUT;

            phshake = (struct handshake *)(icc_polling->poll_addr);
            pcie_init_outbound(phshake);

            *(icc_polling->poll_addr) = WAIT_PATTERN;

            return 0;
        }
    }

    return -EINVAL;
}

#endif  /* ICC_BUILD_FOR_M4 */

void poll_clear_notify_from_peer(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct ping_poll *icc_polling = &(icc_data->icc_polling);

        if (icc_polling->poll_addr) {
            *(icc_polling->poll_addr) = WAIT_PATTERN;
        }
    }
}

/*
 * OS specific initialization of interrupts.
 * This function has restricted functionality, since interrupt
 * related code is device specific and should be handled by the
 * device initialization code.
 *
 * No special initialization when using polling.
 */
ICC_Err_t ICC_OS_Init_Interrupts( void )
{
    return ICC_SUCCESS;
}

#endif  /* ICC_USE_POLLING */
