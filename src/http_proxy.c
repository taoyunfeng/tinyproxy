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

#include "http_proxy.h"
#include "trace.h"
#include "sock.h"
#include "intl_string.h"
#include "cross_platform.h"

static int find_remote_host(const char *response, char *host, int host_bufsz, unsigned short *port)
{
	char line[BUF_SIZE];
	char flag_string[256];
	char address_string[256];
	char port_string[256];
	int offs;
	int line_len;
	int found;
	int cnt;

	for (found = 0, offs = 0; sscanf(response + offs, "%[^\r\n]", line) > 0 && !found;)
	{
		line_len = strlen(line);

		while (response[line_len + offs] == '\r' ||
			response[line_len + offs] == '\n')
		{
			line_len++;
		}

		offs += line_len;

		cnt = sscanf(line, "%[^:]:%[^:]:%s", 
			flag_string, address_string, port_string);
		if (cnt < 2)
		{
			continue;
		}

		if (strcasecmp(flag_string, "Host") != 0)
		{
			continue;
		}

		switch (cnt)
		{
			case 3: strtrim(port_string);
			case 2: strtrim(address_string); break;
			default: continue;
		}

		snprintf(host, host_bufsz, "%s", address_string);

		if (cnt == 3 && port)
		{
			*port = (unsigned short)strtoul(port_string, NULL, 0);
		}

		found = 1;
	}

	return found;
}

static int find_http_version(const char *response, char *version, int version_bufsz)
{
	char line[BUF_SIZE];
	char sub_string[256];
	char sub_version[256];
	int offs;
	int line_len;
	int found;
	int cnt;

	for (found = 0, offs = 0; sscanf(response + offs, "%[^\r\n]", line) > 0 && !found;)
	{
		line_len = strlen(line);

		while (response[line_len + offs] == '\r' ||
			response[line_len + offs] == '\n')
		{
			line_len++;
		}

		offs += line_len;

		cnt = sscanf(line, "%*s %*s %s", sub_string);

		if (cnt != 1)
		{
			continue;
		}

		if (strncmp(sub_string, "HTTP/", 5) != 0)
		{
			continue;
		}

		if (1 != sscanf(sub_string, "%*5s%s", sub_version))
		{
			continue;
		}

		snprintf(version, version_bufsz, "%s", sub_version);
		found = 1;
	}

	return found;
}

static int http_proxy_handler(void *opaque, SOCKET proxy, SOCKET client)
{
	SOCKET server;
	struct sockaddr_in addr;
	char buf[BUF_SIZE];
	char host[256];
	char version[256];
	char resp_success[256];
	char resp_failed[256];
	unsigned short port;
	int len;

	server = INVALID_SOCKET;

	len = recv(client, buf, sizeof(buf) - 1, 0);
	if (len <= 0)
	{
		err_printf("failed to recv request from client, error: %d\n", sock_errno);
		goto exit;
	}

	buf[len] = '\0';
	port = 80;
	debug_printf("\n%s\n", buf);
	if (!find_remote_host(buf, host, array_size(host), &port))
	{
		err_printf("failed to find remote host from request of client\n");
		goto exit;
	}

	if (!find_http_version(buf, version, array_size(version)))
	{
		err_printf("failed to identify http version of proxy request\n");
		goto exit;
	}

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == server)
	{
		err_printf("failed to create socket, error: %d\n", sock_errno);
		goto exit;
	}

	memset(&addr, 0, sizeof(addr));
	if (ERROR_SUCCESS != safe_gethostbyname(host, &addr.sin_addr))
	{
		err_printf("failed to anaylize host address, error: %d\n", sock_errno);
		goto exit;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (SOCKET_ERROR == connect(server, (struct sockaddr *)&addr, sizeof(addr)))
	{
		len = snprintf(resp_failed, array_size(resp_failed), "HTTP/%s 500 Internal Server Error\r\n\r\n", version);
		send(client, resp_failed, len, 0);
		err_printf("failed to connect remote server, error: %d\n", sock_errno);
		goto exit;
	}

	len = snprintf(resp_success, array_size(resp_success), "HTTP/%s 200 Connection Established\r\n\r\n", version);
	len = send(client, resp_success, len, 0);
	if (len <= 0)
	{
		err_printf("failed to send response to client\n", sock_errno);
		goto exit;
	}
	
	info_printf("client %p http proxy established, remote host: %s:%hu\n", (void *)(ptrdiff_t)client, host, port);
	tcp_exchange_data(server, client);
exit:

	if (server != INVALID_SOCKET) closesocket(server);
	return 0;
}

int startup_http_proxy(unsigned short port)
{
	return start_tcp_server(port, http_proxy_handler, NULL);
}
