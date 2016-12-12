/**
*   @file    ICC_Args.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication command line arguments helper
*   @details       Inter Core Communication command line arguments helper
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

#ifndef ICC_ARGS_H
#define ICC_ARGS_H

#include <linux/moduleparam.h>
#include <linux/stat.h>

/** Define a command line argument.
 * Use for types: short, int, long, bool (other types require workarounds).
 */
#define ICC_CMD_LINE_ARG(name, type, val, desc) \
static type name = val; \
module_param(name, type, S_IRUSR | S_IWUSR); \
MODULE_PARM_DESC(name, desc " (default:" xstr(val) ")");

#endif /* ICC_ARGS_H */
