/**
*   @file    ICC_Log.h
*   @version 0.0.1
*
*   @brief   ICC - Inter Core Communication logging support
*   @details       Inter Core Communication logging support
*/
/*==================================================================================================
*   Project              : ICC
*   Platform             : ARM
*   Peripheral           :
*   Dependencies         : none
*
*   Build Version        :
*
*   (c) Copyright 2017 NXP
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

#ifndef ICC_LOG_H
#define ICC_LOG_H

#include <linux/kern_levels.h>
#include <linux/stddef.h>

#define xstr(s) str(s)
#define str(s) #s

#ifdef pr_fmt
#undef pr_fmt
#endif

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt "\n"

#ifdef CONFIG_DYNAMIC_DEBUG

#define icc_pr_debug pr_debug
#define icc_pr_info pr_info
#define icc_pr_warning pr_warning
#define icc_pr_err pr_err

#else

#define ICC_LOG_LEVEL_STR(level) KERN_SOH xstr(level)

/* add function information to the log messages */
#define ICC_PRINTK(level, format, ...) printk(ICC_LOG_LEVEL_STR(level) pr_fmt("[%s] " format), __func__, ##__VA_ARGS__)
#define ICC_PRINTK_INFO(format, ...) printk(ICC_LOG_LEVEL_STR(LOGLEVEL_INFO) pr_fmt(format), ##__VA_ARGS__)

/* use these macros to unify the printk and the pr_* APIs */
#define icc_pr_debug(format, ...) ICC_PRINTK(LOGLEVEL_DEBUG, format, ##__VA_ARGS__)
#define icc_pr_info(format, ...) ICC_PRINTK_INFO(format, ##__VA_ARGS__)
#define icc_pr_warning(format, ...) ICC_PRINTK(LOGLEVEL_WARN, format, ##__VA_ARGS__)
#define icc_pr_err(format, ...) ICC_PRINTK(LOGLEVEL_ERR, format, ##__VA_ARGS__)

#endif

/**
 * Log macros, to be called from the ICC code.
 */
#define ICC_DEBUG icc_pr_debug
#define ICC_INFO icc_pr_info
#define ICC_WARN icc_pr_warning
#define ICC_ERR icc_pr_err

#endif /* ICC_LOG_H */
