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

#include "cross_platform.h"

#include "port_mapping.h"
#include "trace.h"
#include "sock.h"

#define BUF_SIZE 8192

struct port_mapping_arg {
	char *address;
	unsigned short port;
};

static void mapping_handler(SOCKET client, const char *address, unsigned short port)
{
	SOCKET server = INVALID_SOCKET;
	struct sockaddr_in addr = { 0 };

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == INVALID_SOCKET)
	{
		err_printf("failed to create socket, error: %d\n", sock_errno);
		return;
	}

	if (ERROR_SUCCESS != safe_gethostbyname(address, &addr.sin_addr))
	{
		err_printf("failed to anaylize host address, error: %d\n", sock_errno);
		return;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (connect(server, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		err_printf("failed to connect server, error: %d\n", sock_errno);
		return;
	}

	info_printf("successed to establish port mapping from client %p to server %p\n", 
		(void *)(ptrdiff_t)client,
		(void *)(ptrdiff_t)server);
	tcp_exchange_data(server, client);
}

static int port_mapping_handler(void *opaque, SOCKET proxy, SOCKET client)
{
	struct port_mapping_arg *arg = (struct port_mapping_arg *)opaque;
	mapping_handler(client, arg->address, arg->port);
	return 0;
}

int start_port_mapping(unsigned short from, const char *to_address, unsigned short to_port)
{
	struct port_mapping_arg arg;
	arg.address = (char *)to_address;
	arg.port    = to_port;
	return start_tcp_server(from, port_mapping_handler, &arg);
}
