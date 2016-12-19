/**
*   @file    ICC_Of.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication extension for the kernel of_* API
*   @details       Inter Core Communication extension for the kernel of_* API
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

#ifndef ICC_OF_H
#define ICC_OF_H

#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include "ICC_Log.h"

#define DT_SRAM_TYPE_STR "sram"


/**
 * Find the MSCM node in the device tree and @return a pointer to it.
 * User must call 'of_node_put()' on the returned pointer when done with it.
 */
struct device_node * icc_of_find_mscm_node(void)
{
    static struct of_device_id mscm_dt_ids[] = {
        {
            .compatible = "fsl,s32v234-mscm",
        },
        { /* sentinel */ }
    };

    struct device_node *mscm = of_find_matching_node_and_match(NULL, mscm_dt_ids, NULL);

    return mscm;
}

/**
 * Find the SRAM node in the device tree and @return a pointer to it.
 * User must call 'of_node_put()' on the returned pointer when done with it.
 */
struct device_node * icc_of_find_sram_node(void)
{
    static struct of_device_id sram_dt_ids[] = {
        {
            .compatible = "fsl,s32v234-sram",
        },
        { /* sentinel */ }
    };

    struct device_node *sram = of_find_node_by_type(NULL, DT_SRAM_TYPE_STR);
    if (!sram) {
        sram = of_find_matching_node_and_match(NULL, sram_dt_ids, NULL);
    }

    return sram;
}

#endif /* ICC_ARGS_H */
