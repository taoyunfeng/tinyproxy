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

#include "trace.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cross_platform.h"

#define BUF_SIZE 1024

static printf_t intl_printf = printf;
static vprintf_t intl_vprintf = vprintf;

#ifdef _DEBUG
static int intl_dbglvl = LOGLVL_DEBUG;
#else
static int intl_dbglvl = LOGLVL_ERROR;
#endif


int log_printf(int dbglvl, const char *fmt, ...)
{
	char buf[BUF_SIZE];
	int len;
	va_list ap;

	if (dbglvl > intl_dbglvl)
		return -1;

	va_start(ap, fmt);
	len = vsnprintf(buf, array_size(buf), fmt, ap);
	va_end(ap);
	assert(len < sizeof(buf));

	len = intl_printf(buf);

	return len;
}

int log_print_hex(int dbglvl, void *buf, int len)
{
	#define hex_part_len  ((2 + 1) * 16)
	#define sep_part_len  (1)
	#define str_part_len  (16 + 1)
	#define line_width    (hex_part_len + str_part_len)

	int i, j, k;
	int line = (len + line_width - 1) / line_width;
	int offs;
	int bytes;

	if (dbglvl > intl_dbglvl)
		return -1;

	for (i = 0, bytes = 16, offs = 0; i < line; i++, offs += bytes)
	{
		if (i == line - 1 && offs + bytes > len)
		{
			bytes = len - offs;
		}

		for (j = 0, k = 0; j < bytes; j++)
		{
			k += intl_printf("%02X ", ((unsigned char *)buf)[offs + j]);
		}

		while (k < hex_part_len)
		{
			k += intl_printf("   ");
		}

		k += intl_printf("\t");

		for (j = 0; j < bytes; j++)
		{
			unsigned char c = ((unsigned char *)buf)[offs + j];
			if (!isprint(c))
			{
				c = '.';
			}
			k += intl_printf("%c", (char)c);

		}

		k += intl_printf("\n");
	}

	return k;
}

int log_vprintf(int dbglvl, const char *fmt, va_list ap)
{
	int len;

	if (dbglvl > intl_dbglvl)
		return -1;

	len = intl_vprintf(fmt, ap);

	return len;
}

int init_log_level(int dbglvl)
{
	int old = intl_dbglvl;
	assert(dbglvl <= LOGLVL_HIGHEST && dbglvl >= LOGLVL_NEVER);

	intl_dbglvl = dbglvl;
	return old;
}

printf_t hook_printf(printf_t func)
{
	printf_t old = intl_printf;
	intl_printf = func ? func : printf;
	return old;
}

vprintf_t hook_vprintf(vprintf_t func)
{
	vprintf_t old = intl_vprintf;
	intl_vprintf = func ? func : vprintf;
	return old;

}
