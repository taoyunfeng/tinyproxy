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

#ifndef _HAVE_INTL_SOCK_H_
#define _HAVE_INTL_SOCK_H_

#include "cross_platform.h"

#define BUF_SIZE 8192

#ifdef __cplusplus
extern "C" {
#endif

	int init_sock();

	int exit_sock();

	typedef int (* client_handler_t) (void *opaque, SOCKET proxy, SOCKET client);

	int start_tcp_server(unsigned short port, client_handler_t client_handler, void *opaque);

	int tcp_transport_data(SOCKET from, SOCKET to);

	int tcp_exchange_data(SOCKET server, SOCKET client);

#ifdef __cplusplus
}
#endif

#endif
