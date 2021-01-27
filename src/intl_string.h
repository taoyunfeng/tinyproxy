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

#ifndef _HAVE_INTL_STRING_H_
#define _HAVE_INTL_STRING_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	static char *strtrim(char *__string) {

		size_t i;
		size_t len;

		len = strlen(__string);

		for (i = 0; i < len && __string[i] == ' '; i++) {}
		if (i != 0) {
			memmove(__string, __string + i, (len - i + 1) * sizeof(char));
		}

		len = strlen(__string);
		for (i = len - 1; i >= 0 && __string[i] == ' '; i--) {}
		if (i != len - 1) {
			__string[i + 1] = '\0';
		}

		return __string;
	}


#ifdef __cplusplus
}
#endif

#endif
