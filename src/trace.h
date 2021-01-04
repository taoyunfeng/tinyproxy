/* ==================================================================
 *
 * Copyright 2020 TAO Yun-feng 
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * ===================================================================
 * Author: TAO Yun-feng
 * E-mail: tao.yunfeng@outlook.com
 */

#ifndef _HAVE_TRACE_H_
#define _HAVE_TRACE_H_

#define LOGLVL_NEVER    -1
#define LOGLVL_INFO      0
#define LOGLVL_WARN      1
#define LOGLVL_ERROR     2
#define LOGLVL_DEBUG     3
#define LOGLVL_HIGHEST   LOGLVL_DEBUG

#define info_printf(fmt, ...) \
	log_printf(LOGLVL_INFO, "[INFO] " fmt, ##__VA_ARGS__) 

#define warn_printf(fmt, ...) \
	log_printf(LOGLVL_WARN, "[WARN] " fmt, ##__VA_ARGS__) 

#define err_printf(fmt, ...) \
	log_printf(LOGLVL_ERROR, "[ERR] " fmt, ##__VA_ARGS__) 

#define debug_printf(fmt, ...) \
	log_printf(LOGLVL_DEBUG, "[DEBUG] " fmt, ##__VA_ARGS__) 

#define info_hex_print(buf, len) \
	log_print_hex(LOGLVL_INFO, buf, len) 

#define warn_hex_print(buf, len) \
	log_print_hex(LOGLVL_WARN, buf, len) 

#define err_hex_print(buf, len) \
	log_print_hex(LOGLVL_ERROR, buf, len) 

#define debug_hex_print(buf, len) \
	log_print_hex(LOGLVL_DEBUG, buf, len) 


#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef int(*printf_t) (const char *, ...);

	typedef int(*vprintf_t) (const char *, va_list ap);

	printf_t hook_printf(printf_t func);

	vprintf_t hook_vprintf(vprintf_t func);

	int init_log_level(int dbglvl);

	int log_vprintf(int dbglvl, const char *fmt, va_list ap);

	int log_printf(int dbglvl, const char *fmt, ...);

	int log_print_hex(int dbglvl, void *buf, int len);

#ifdef __cplusplus
};
#endif

#endif
