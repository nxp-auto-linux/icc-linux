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

#ifndef ICC_LOG_H
#define ICC_LOG_H

#include <linux/kern_levels.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/stddef.h>

#include "ICC_Args.h"

#define xstr(s) str(s)
#define str(s) #s

#ifdef pr_fmt
#undef pr_fmt
#endif

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#ifdef CONFIG_DYNAMIC_DEBUG
#define DYNAMIC_DEBUG_SUPPORT
#endif

/**
 * for DEBUG, use kernel log level INFO, since pr_debug is dependent on DEBUG compile flag
 *  (for dynamic debug support).
 * for INFO use log level NOTICE.
 * for the others (up to ERR) use the corresponding kernel levels.
 * no need for levels more severe than ERR in our log messages.
 */
#define ICC_LOGLEVEL_DEBUG LOGLEVEL_INFO
#define ICC_LOGLEVEL_INFO LOGLEVEL_NOTICE
#define ICC_LOGLEVEL_WARN LOGLEVEL_WARNING
#define ICC_LOGLEVEL_ERR LOGLEVEL_ERR

#define ICC_LOGLEVEL_DEFAULT ICC_LOGLEVEL_INFO
#define ICC_LOG_LEVEL_STR(level) KERN_SOH xstr(level)

#ifdef DYNAMIC_DEBUG_SUPPORT

#define PR_ICC_LOGLEVEL_DEBUG pr_info
#define PR_ICC_LOGLEVEL_INFO pr_notice
#define PR_ICC_LOGLEVEL_WARN pr_warning
#define PR_ICC_LOGLEVEL_ERR pr_err

#else

#define ICC_PRINTK(level, format, ...) printk(ICC_LOG_LEVEL_STR(level) pr_fmt("[%s]" format "\n"), __func__, ##__VA_ARGS__)
#define ICC_PRINTK_INFO(format, ...) printk(ICC_LOG_LEVEL_STR(ICC_LOGLEVEL_INFO) pr_fmt(format "\n"), ##__VA_ARGS__)

#define PR_ICC_LOGLEVEL_DEBUG(format, ...) ICC_PRINTK(ICC_LOGLEVEL_DEBUG, format, ##__VA_ARGS__)
#define PR_ICC_LOGLEVEL_INFO(format, ...) ICC_PRINTK_INFO(format, ##__VA_ARGS__)
#define PR_ICC_LOGLEVEL_WARN(format, ...) ICC_PRINTK(ICC_LOGLEVEL_WARN, format, ##__VA_ARGS__)
#define PR_ICC_LOGLEVEL_ERR(format, ...) ICC_PRINTK(ICC_LOGLEVEL_ERR, format, ##__VA_ARGS__)

#endif

bool icc_should_log(uint32_t log_level);

#define icc_log(level, format, ...) \
    if (icc_should_log(level)) { \
        PR_ ## level(format, ##__VA_ARGS__); \
    }

/**
 * Log macros, to be called from the ICC code.
 */
#define ICC_DEBUG(...) icc_log(ICC_LOGLEVEL_DEBUG, ##__VA_ARGS__)
#define ICC_INFO(...) icc_log(ICC_LOGLEVEL_INFO, ##__VA_ARGS__)
#define ICC_WARN(...) icc_log(ICC_LOGLEVEL_WARN, ##__VA_ARGS__)
#define ICC_ERR(...) icc_log(ICC_LOGLEVEL_ERR, ##__VA_ARGS__)

#ifdef ICC_LOG_SRC

/**
 * ICC log level (default is NOTICE).
 * Can be specified as argument when inserting the module.
 * The ICC code will not output any log message that has a higher value than this level.
 */
ICC_CMD_LINE_ARG(icc_log_level, int, ICC_LOGLEVEL_DEFAULT, \
        "ICC Log level (" xstr(ICC_LOGLEVEL_ERR) "-" xstr(ICC_LOGLEVEL_DEBUG) ")");

bool icc_should_log(uint32_t log_level)
{
    return log_level <= icc_log_level;
}

#endif

#endif /* ICC_LOG_H */
