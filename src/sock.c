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

#include "sock.h"
#include "trace.h"
#include "cross_platform.h"

struct client_handler_argument {
	SOCKET proxy;
	SOCKET client;
	client_handler_t client_handler;
	void *opaque;
};

static void *dummy_client_handler(struct client_handler_argument *arg)
{
	struct client_handler_argument copy = *arg;
	free(arg);
	(void)copy.client_handler(copy.opaque, copy.proxy, copy.client);
	closesocket(copy.client);
	return NULL;
}

int init_sock()
{
#ifdef _WIN32
	WSADATA wd = { 0 };
	int err;

	err = WSAStartup(MAKEWORD(2, 2), &wd);
	if (err == ERROR_SUCCESS && wd.wVersion != MAKEWORD(2, 2))
	{
		err = WSAEINVAL;
	}

	return err;
#else
	return 0;
#endif
}

int exit_sock()
{
#ifdef _WIN32
	return WSACleanup();
#else
	return 0;
#endif
}

int start_tcp_server(unsigned short port, client_handler_t client_handler, void *opaque)
{
	SOCKET proxy;
	SOCKET client;
	struct sockaddr_in addr;
	struct client_handler_argument *arg;
	int err;
	int reuse;

	proxy = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == proxy)
	{
		err_printf("failed to create socket, error: %d\n", sock_errno);
		goto exit;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	reuse = 1;
	setsockopt(proxy, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

	if (SOCKET_ERROR == bind(proxy, (struct sockaddr *)&addr, sizeof(addr)))
	{
		err_printf("failed to bind socket, error: %d\n", (err = sock_errno));
		goto exit;
	}

	if (SOCKET_ERROR == listen(proxy, 10))
	{
		err_printf("failed to listen socket, error: %d\n", (err = sock_errno));
		goto exit;
	}

	for (err = 0; 1; )
	{
		client = accept(proxy, NULL, NULL);
		if (INVALID_SOCKET == client)
		{
			err_printf("failed to accept client, error: %d\n", (err = sock_errno));
			break;
		}

		arg = (struct client_handler_argument *)malloc(sizeof(struct client_handler_argument));
		if (arg == NULL)
		{
			err_printf("failed to alloc memory, error: %d\n", (err = sock_errno));
			break;
		}

		arg->proxy          = proxy;
		arg->client         = client;
		arg->client_handler = client_handler;
		arg->opaque         = opaque;

		if (0 != create_thread((void *(*)(void *))dummy_client_handler, arg))
		{
			free(arg);
			err_printf("failed to create thread of client(%p) handler, error: %d\n", (void *)(ptrdiff_t)client, (err = sock_errno));
			break;
		}
	}

exit:
	if (proxy != INVALID_SOCKET) closesocket(proxy);
	return err;
}

int tcp_transport_data(SOCKET from, SOCKET to)
{
	int len;
	int tmp;
	int done_len;
	char buf[BUF_SIZE];

	len = recv(from, buf, sizeof(buf) - 1, 0);
	if (len <= 0)
	{
		return len;
	}

	buf[len] = '\0';
	for (done_len = 0; done_len < len; done_len += tmp)
	{
		tmp = send(to, buf + done_len, len - done_len, 0);
		if (tmp <= 0)
		{
			break;
		}
	}

	return done_len;
}

int tcp_exchange_data(SOCKET server, SOCKET client)
{
	SOCKET fds[2] = { server, client };
	fd_set rd_set = { 0 };
	int ret;
	int i;

	FD_ZERO(&rd_set);
	FD_SET(server, &rd_set);
	FD_SET(client, &rd_set);

	for (; (ret = select(max(fds[0], fds[1]) + 1, &rd_set, NULL, NULL, NULL)) >= 0; )
	{
		for (i = 0; i < array_size(fds); i++)
		{
			if (!FD_ISSET(fds[i], &rd_set))
			{
				continue;
			}

			if (tcp_transport_data(fds[i], fds[!i]) <= 0)
			{
				goto exit;
			}

		}

		FD_ZERO(&rd_set);
		FD_SET(server, &rd_set);
		FD_SET(client, &rd_set);
	}

exit:
	info_printf("client: %p out\n", (void *)(ptrdiff_t)client);
	return sock_errno;
}