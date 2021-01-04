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

#ifndef _HAVE_CROSS_PLATFORM_H_
#define _HAVE_CROSS_PLATFORM_H_

#ifdef __cplusplus
template<class T, size_t N>
char(*dummy_number_of(T(&)[N]))[N];
#define array_size(v) (sizeof(*dummy_number_of(v)))
#else
#define array_size(v) (sizeof((v))/sizeof((v)[0]))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include "getopt.h"
#pragma comment(lib, "ws2_32.lib")

#define sock_errno WSAGetLastError()
#ifndef strdup
#define strdup(s) _strdup(s)
#endif
#define strcasecmp _stricmp

#undef snprintf
#define snprintf _snprintf

#elif defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#define sock_errno errno
typedef int SOCKET;
#define ERROR_SUCCESS (0)
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)
#define closesocket(fd) close(fd)

#ifdef __GNUC__
#define max(x, y) ({ typeof(x) _x = (x); typeof(y) _y = (y); _x > _y ? _x : _y; })
#define min(x, y) ({ typeof(x) _x = (x); typeof(y) _y = (y); _x < _y ? _x : _y; })
#else
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

	int safe_gethostbyname(const char *name, struct in_addr *addr);

	int create_thread(void *(* thread_routine)(void *arg), void *arg);

#ifdef __cplusplus
}
#endif

#endif // 
