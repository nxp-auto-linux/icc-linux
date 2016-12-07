/**
*   @file    ICC_Pcie.c
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication device driver PCI Express support
*   @details       Inter Core Communication device driver PCI Express support
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

#ifdef ICC_LINUX2LINUX

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>

#include <linux/interrupt.h>

#include "ICC_Api.h"
#include "ICC_Sw_Platform.h"

#define LOG_LEVEL       KERN_ALERT

#ifndef ICC_USE_BAR
#define ICC_USE_BAR 	2
#endif

#define DO_TEST_EXPAND(VAL)  VAL ## 0
#define TEST_EXPAND(VAL)     DO_TEST_EXPAND(VAL)

#define DO_GET_BAR_ADDRESS(BAR, SUFFIX) EP_BAR ## BAR ## _ ## SUFFIX
#define GET_BAR_ADDRESS(BAR) 	DO_GET_BAR_ADDRESS(BAR, ADDR)
#define GET_BAR_END(BAR) 		DO_GET_BAR_ADDRESS(BAR, END)

/* FIXME remove hardcoding of addresses. */

#define EP_PCIE_BASE_ADDR 0x72000000
#define EP_DDR_ADDR 0x8FF00000

#ifndef ICC_BUILD_FOR_M4

    /* define BARs for communication from the RC side
     * TODO: take them from device tree */
    #ifndef ICC_PCIE_SHMEM_BLUEBOX
    #define EP_BAR0_ADDR	0x72100000
    #define EP_BAR0_END 	0x721fffff  /* 1MB */
    #define EP_BAR2_ADDR	0x72200000
    #define EP_BAR2_END		0x722fffff  /* 1MB */
    #define EP_BAR4_ADDR	0x72310000
    #define EP_BAR4_END		0x72310fff  /* 4KB */
    #define EP_BAR5_ADDR	0x72300000
    #define EP_BAR5_END		0x7230ffff  /* 64Kb */
    #else
    #define EP_BAR0_ADDR	0x1446000000
    #define EP_BAR0_END 	0x14460fffff  /* 1MB */
    #define EP_BAR2_ADDR	0x1446100000
    #define EP_BAR2_END		0x14461fffff  /* 1MB */
    #define EP_BAR4_ADDR	0x1446210000
    #define EP_BAR4_END		0x1446210fff  /* 4KB */
    #define EP_BAR5_ADDR	0x1446200000
    #define EP_BAR5_END		0x144620ffff  /* 64Kb */
    #endif

    #define EP_BAR_ADDR	GET_BAR_ADDRESS(ICC_USE_BAR)

    #if (TEST_EXPAND(EP_BAR_ADDR) == 0)
    #error Invalid BAR selected
    #else
    #define EP_BAR_SIZE	(GET_BAR_END(ICC_USE_BAR) - EP_BAR_ADDR + 1)
    #endif

    /* Physical memory mapped by the RC CPU.
     * The RC's shared DDR mapping is different in the Bluebox vs S32V EVB case.
     * For the moment, this setting is statically defined in the Makefile.
     */
    #ifdef ICC_PCIE_SHMEM_BLUEBOX  /* LS2-S32V */
    /* for LS2 kernel started with mem=13568M, unallocated RAM starts at 0x8350000000 */
    #define RC_DDR_ADDR     0x8350000000
    #else                           /* EVB-PCIE */
    #define RC_DDR_ADDR     0x8FF00000
    #endif

#endif

#ifdef ICC_BUILD_FOR_M4

#ifndef CONFIG_PCI_S32V234_EP
#error Kernel must be configured as PCIe End Point
#endif

    /* M4 stands for the module replacing the RTOS app, that is the one initializing the shared mem.
     * In our case that would be the PCIe EndPoint. */
    #define IRAM_BASE_ADDR EP_DDR_ADDR
    #define IRAM_PING_ADDR EP_PCIE_BASE_ADDR
    #define IRAM_POLL_ADDR EP_DDR_ADDR

#else

    #define IRAM_BASE_ADDR EP_BAR_ADDR
    #define IRAM_PING_ADDR EP_BAR_ADDR
    #define IRAM_POLL_ADDR RC_DDR_ADDR

#endif

#define MAP_DDR_SIZE    1024 * 1024 * 1 /* 1 MB */

const uint64_t get_shmem_base_address(void)
{
    return IRAM_BASE_ADDR;
}

const uint32_t get_shmem_size(void)
{
    return MAP_DDR_SIZE;
}

#ifdef ICC_USE_POLLING

const uint64_t get_shmem_poll_addr(void)
{
    return IRAM_POLL_ADDR;
}
const uint64_t get_shmem_ping_addr(void)
{
    return IRAM_PING_ADDR;
}

#endif


/* following code is calling an API custom-exported from the PCIe driver */

#ifdef ICC_BUILD_FOR_M4

extern int s32v_pcie_setup_outbound(void * data);
extern int s32v_pcie_setup_inbound(void * data);

int pcie_init_inbound(void)
{
    int err = 0;
    /* Inbound region structure */
    static struct s32v_inbound_region icc_inb = {
        ICC_USE_BAR,    /* BAR */
        EP_DDR_ADDR,    /* locally-mapped DDR on EP */
        0               /* region 0 */
    };

    /* Setup the inbound window for transactions from RC */
    err = s32v_pcie_setup_inbound(&icc_inb);

    if (err) {
        printk(KERN_ERR "[pcie_init_inbound] Error while setting inbound region\n");
    } else {
        printk("Inbound region setup successfully\n");
        printk("\tEP %#llx mapped to EP BAR %d\n", icc_inb.target_addr, icc_inb.bar_nr);
    }

    return err;
}

int pcie_init_outbound(struct handshake *phshake)
{
    int err = 0;
    if (phshake) {
        /* Outbound region structure */
        struct s32v_outbound_region icc_outb = {
            0,      /* target_addr */
            EP_PCIE_BASE_ADDR,    /* base_addr */
            MAP_DDR_SIZE,   /* size >= 64K(min for PCIE on S32V) */
            0,      /* region number */
            0       /* region type = mem */
        };
        struct s32v_bar icc_bar = phshake->rc_bar;

        icc_outb.target_addr = phshake->rc_ddr_addr;

        /* Setup outbound window for accessing RC mem */
        err = s32v_pcie_setup_outbound(&icc_outb);

        if (!err) {
            printk("Outbound region setup successfully\n");
            printk("\tRC %#llx mapped to EP BAR %d, size %d\n", icc_bar.bar_addr, icc_bar.bar_nr, icc_bar.bar_size);
            printk("\tEP %#llx mapped to RC %#llx, size %d\n", icc_outb.base_addr, icc_outb.target_addr, icc_outb.size);
        }
    } else
        err = -EINVAL;

    return err;
}

#else

int pcie_init_bar(struct s32v_bar *bar)
{
    if (bar) {
        bar->bar_addr = EP_BAR_ADDR;
        bar->bar_size = EP_BAR_SIZE;
        bar->bar_nr = ICC_USE_BAR;

        return 0;
    }

    return -EINVAL;
}

#endif  /* ICC_BUILD_FOR_M4 */

#endif  /* ICC_LINUX2LINUX */
