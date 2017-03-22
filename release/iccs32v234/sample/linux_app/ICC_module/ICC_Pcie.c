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

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/mm.h>

#include <linux/of_reserved_mem.h>
#include <linux/pci.h>
#include <uapi/linux/pci_regs.h>

#include "ICC_Api.h"
#include "ICC_Notification.h"
#include "ICC_Pcie.h"
#include "ICC_Log.h"
#include "ICC_Args.h"
#include "ICC_Of.h"

#ifndef ICC_USE_BAR
#define ICC_USE_BAR 	2
#endif

#ifdef ICC_BUILD_FOR_M4
static uint64_t ICC_EP_PCIE_Phys_Base_Addr;
#else
static uint64_t ICC_EP_BAR_Phys_Base_Addr;
static uint32_t ICC_EP_BAR_Size;

/* Set this arg to true if you want to check the shared memory */
ICC_CMD_LINE_ARG(icc_list_pci, bool, false, \
        "List all PCI devices");

#endif

static uint64_t ICC_Shm_Phys_Base_Addr;
static uint32_t ICC_Shm_Size;

#ifdef ICC_BUILD_FOR_M4

    /* M4 stands for the module replacing the RTOS app, that is the one initializing the shared mem.
     * In our case that would be the PCIe EndPoint. */
    #define SHM_BASE_ADDR ICC_Shm_Phys_Base_Addr
    #define SHM_PING_ADDR ICC_EP_PCIE_Phys_Base_Addr
    #define SHM_POLL_ADDR ICC_Shm_Phys_Base_Addr

#else

    #define SHM_BASE_ADDR ICC_EP_BAR_Phys_Base_Addr
    #define SHM_PING_ADDR ICC_EP_BAR_Phys_Base_Addr
    #define SHM_POLL_ADDR ICC_Shm_Phys_Base_Addr

#endif

extern char * ICC_Shared_Virt_Base_Addr;

static const uint64_t get_pcie_shmem_base_phys_address(void)
{
    return SHM_BASE_ADDR;
}

#ifdef ICC_USE_POLLING

const uint64_t get_shmem_poll_phys_addr(void)
{
    return SHM_POLL_ADDR;
}

const uint64_t get_shmem_ping_phys_addr(void)
{
    return SHM_PING_ADDR;
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
        0,              /* locally-mapped DDR on EP, to be updated */
        0               /* region 0 */
    };

    icc_inb.target_addr = ICC_Shm_Phys_Base_Addr;

    /* Setup the inbound window for transactions from RC */
    err = s32v_pcie_setup_inbound(&icc_inb);

    if (err) {
        ICC_ERR("Error while setting inbound region");
    } else {
        ICC_INFO("Inbound region setup successfully");
        ICC_INFO("\tEP %#llx mapped to EP BAR %d", icc_inb.target_addr, icc_inb.bar_nr);
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
            0,      /* base_addr, to be updated */
            ICC_CONFIG_MAX_SIZE,   /* size >= 64K(min for PCIE on S32V) */
            0,      /* region number */
            0       /* region type = mem */
        };
        struct s32v_bar icc_bar = phshake->rc_bar;

        icc_outb.base_addr = ICC_EP_PCIE_Phys_Base_Addr;
        icc_outb.target_addr = phshake->rc_ddr_addr;

        /* Setup outbound window for accessing RC mem */
        err = s32v_pcie_setup_outbound(&icc_outb);

        if (!err) {
            ICC_INFO("Outbound region setup successfully");
            ICC_INFO("\tRC %#llx mapped to EP BAR %d, size %d", icc_bar.bar_addr, icc_bar.bar_nr, icc_bar.bar_size);
            ICC_INFO("\tEP %#llx mapped to RC %#llx, size %d", icc_outb.base_addr, icc_outb.target_addr, icc_outb.size);
        }
    } else
        err = -EINVAL;

    return err;
}

#else

int pcie_init_bar(struct s32v_bar *bar)
{
    if (bar) {
        bar->bar_addr = ICC_EP_BAR_Phys_Base_Addr;
        bar->bar_size = ICC_EP_BAR_Size;
        bar->bar_nr = ICC_USE_BAR;

        return 0;
    }

    return -EINVAL;
}

#endif  /* ICC_BUILD_FOR_M4 */

/* API for accessing shared memory when the device is built in kernel */
static int init_shared_memory(struct reserved_mem *rmem)
{
    ICC_Shm_Phys_Base_Addr = rmem->base;
    ICC_Shm_Size = rmem->size;

    ICC_INFO("Reserved shared mem block: addr=%#llx size=%d", ICC_Shm_Phys_Base_Addr, ICC_Shm_Size);
    WARN_ON(!(ICC_Shm_Phys_Base_Addr && ICC_Shm_Size));

    return 0;
}
RESERVEDMEM_OF_DECLARE(shared_memory, "fsl,s32v234-shm", init_shared_memory);

static void setup_shm_from_device_tree(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct device *dev = &icc_data->pdev->dev;

        if (!ICC_Shm_Phys_Base_Addr) {
            struct device_node *shm = NULL;

            shm = of_parse_phandle(dev->of_node, "memory-region", 0);
            if (shm) {
                uint64_t reg_size = 0;
                const __be32 * shm_base = of_get_address(shm, 0, &reg_size, NULL);
                ICC_Shm_Phys_Base_Addr = of_translate_address(shm, shm_base);
                ICC_Shm_Size = reg_size;
                ICC_INFO("Reserved shared mem block: addr=%#llx size=%d", ICC_Shm_Phys_Base_Addr, ICC_Shm_Size);

                of_node_put(shm);
            } else {
                ICC_ERR("Reference to shared mem block not found");
            }
        }

        if (ICC_Shm_Size < get_shmem_size()) {
            ICC_WARN("Reserved shared mem block is too small (%d bytes)", ICC_Shm_Size);
        }
    }
}

#ifdef ICC_BUILD_FOR_M4
static void setup_pcie_from_device_tree(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        struct device_node *pcie = NULL;

        pcie = icc_of_find_pcie_node();
        if (pcie) {
            uint64_t reg_size = 0;
            uint32_t reg_flags = 0;
            const __be32 * pcie_base = of_get_address(pcie, 0, &reg_size, &reg_flags);
            uint64_t pcie_ctrl_base = of_translate_address(pcie, pcie_base);
            ICC_INFO("Found PCIe control block: addr=%#llx size=%d", pcie_ctrl_base, reg_size);

            of_node_put(pcie);

            if (pcie_ctrl_base) {
                struct device *dev = &icc_data->pdev->dev;
                char *pcie_ctrl_virt_base = devm_ioremap_nocache(dev, pcie_ctrl_base, reg_size);
                if (pcie_ctrl_virt_base) {
                    /* Both EP and RC must be running in order to have the BARs populated on EP side.
                     * Otherwise we read 0x0 from EP BAR0.
                     */
                    ICC_EP_PCIE_Phys_Base_Addr = readl(pcie_ctrl_virt_base + PCI_BASE_ADDRESS_0 + ICC_USE_BAR * 4);
                    ICC_INFO("Found PCIe BAR%d base addr=%#llx", ICC_USE_BAR, ICC_EP_PCIE_Phys_Base_Addr);
                }
            }
        }
    }
}
#else

#define VENDOR_FSL 0x1957
#define DEVICE_S32V 0x4001

static void pbus_browse_resources(const struct pci_bus *bus, int resno)
{
    struct pci_dev *dev;
    struct list_head *pos;

    if (icc_list_pci) {
        ICC_INFO("Browsing PCI bus %04x:%02x", pci_domain_nr(bus), bus->number);
    }

    list_for_each(pos, &bus->devices) {
        int i;
        dev = (struct pci_dev *)pos;

        if (icc_list_pci) {
            ICC_INFO("    Browsing device %s", dev->dev.kobj.name);
        }

        for (i = 0; i < PCI_NUM_RESOURCES; i++) {
            struct resource *r = &dev->resource[i];

            if (r->end == r->start)
                continue;

            if (icc_list_pci) {
                ICC_INFO("    BAR %d: %pR", i, r);
            }

            if (!ICC_EP_BAR_Phys_Base_Addr && (i == resno) && (dev->vendor == VENDOR_FSL) && (dev->device == DEVICE_S32V)) {
                ICC_EP_BAR_Phys_Base_Addr = r->start;
                ICC_EP_BAR_Size = resource_size(r);
                ICC_INFO("    Found EP on BAR %d base addr=%#llx device=%s", i, ICC_EP_BAR_Phys_Base_Addr, dev->dev.kobj.name);
                if (!icc_list_pci) {
                    return;
                }
            }
        }
    }
}

void pci_bus_browse_resources(struct pci_bus *bus, int resno)
{
    struct pci_bus *b;
    struct pci_dev *dev;
    struct list_head *pos;

    pbus_browse_resources(bus, resno);

    list_for_each(pos, &bus->devices) {

        dev = (struct pci_dev *)pos;

        b = dev->subordinate;
        if (!b)
            continue;

        pci_bus_browse_resources(b, resno);
    }
}

static void setup_bar_address(struct ICC_platform_data *icc_data)
{
    struct pci_bus *bus = pci_find_next_bus(NULL);

    while (bus) {

        pci_bus_browse_resources(bus, ICC_USE_BAR);
        bus = pci_find_next_bus(bus);
    }
}
#endif

void pcie_init_shmem(struct ICC_platform_data *icc_data)
{
    if (icc_data) {
        setup_shm_from_device_tree(icc_data);

#ifdef ICC_BUILD_FOR_M4
        setup_pcie_from_device_tree(icc_data);
#else
        if (!icc_list_pci) {
            ICC_INFO("Scanning PCI buses for the EP");
        }
        setup_bar_address(icc_data);
#endif

        if (get_pcie_shmem_base_phys_address()) {
            struct device *dev = &icc_data->pdev->dev;

            /* Reserve shared memory */
            if (!devm_request_mem_region(dev, get_pcie_shmem_base_phys_address(), get_shmem_size(), "ICC_shmem")) {
                ICC_ERR("Failed to request mem region!");
                return;
            }

            /* ICC Shared mem is mapped differently on RC and EP, but in both cases it physically
             * resides on EP side.
             */
            ICC_Shared_Virt_Base_Addr = devm_ioremap_nocache(dev, get_pcie_shmem_base_phys_address(), get_shmem_size()) + ICC_CONFIG_OFFSET_FROM_BASE;
            ICC_INFO("reserved ICC_Shared_Virt_Base_Addr=%#llx size is %d", ICC_Shared_Virt_Base_Addr, get_shmem_size() - ICC_CONFIG_OFFSET_FROM_BASE);
            if( !ICC_Shared_Virt_Base_Addr ){
                ICC_ERR("ICC_Shared_Virt_Base_Addr virtual mapping has failed for %#x", get_pcie_shmem_base_phys_address());
                return;
            }

#ifdef ICC_BUILD_FOR_M4
            /* setup PCIE */
            pcie_init_inbound();
#endif

#ifdef ICC_USE_POLLING
            shmem_poll_init(icc_data);
            shmem_ping_init(icc_data);
#endif
        }
        else {
            ICC_ERR("Failed to get EP BAR address");
        }
    }
}

void pcie_cleanup_shmem(struct ICC_platform_data *icc_data)
{
#ifdef ICC_USE_POLLING
    shmem_poll_exit(icc_data);
    shmem_ping_exit(icc_data);
#endif
}

#ifndef ICC_BUILD_FOR_M4

extern ICC_Config_t * ICC_Config_Ptr_M4;
extern ICC_Config_t * ICC_Config_Ptr_M4_Remote;

#endif

char *ICC_Phys_To_Virt(char *phys_addr)
{
#ifdef ICC_BUILD_FOR_M4
    /* no translation, same virtual address space */
    return phys_addr;
#else
    /* Synchronize virtual addresses between the two sides of the PCIe
       We're not working with physical addresses, but the name of the macro
         is kept for backwards compatibility */
    return ((char*)((uint64_t)(phys_addr) - (uint64_t)ICC_Config_Ptr_M4_Remote + (uint64_t)ICC_Config_Ptr_M4));
#endif
}
EXPORT_SYMBOL(ICC_Phys_To_Virt);

