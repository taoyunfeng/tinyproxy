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

#include "sock5_proxy.h"
#include "trace.h"

#include "sock.h"
#include "cross_platform.h"

typedef struct sock5_proxy_auth_request {

	unsigned char Ver;
	unsigned char nMethods;
	unsigned char Methods[255];

} sock5_proxy_auth_request;

typedef struct sock5_proxy_auth_response {

	unsigned char Ver;
	unsigned char Method;

} sock5_proxy_auth_response;

typedef struct sock5_proxy_request {

	unsigned char Ver;
	unsigned char Cmd;
	unsigned char Rsv;
	unsigned char Atyp;
	union {
		struct {
			unsigned char Addr[4];
			unsigned short Port;
		} Ipv4;

		struct {
			unsigned char Len;
			char url[1024];
		} DomainName;

		struct {
			unsigned char Addr[16];
			unsigned short Port;
		} Ipv6;
	};

} sock5_proxy_request;

#define sock5_proxy_request_size(req) \
	offsetof(sock5_proxy_request, Ipv4) + \
	((req)->Atyp == IP_V4_ADDRESS ? 6 : \
	 (req)->Atyp == IP_V6_ADDRESS ? 18 : \
	 (req)->DomainName.Len + 2)


typedef struct sock5_proxy_reply {

	unsigned char Ver;
	unsigned char Rep;
	unsigned char Rsv;
	unsigned char Atyp;

	union {
		struct {
			unsigned char Addr[4];
			unsigned short Port;
		} Ipv4;

		struct {
			unsigned char Len;
			char url[1024];
		} DomainName;

		struct {
			unsigned char Addr[16];
			unsigned short Port;
		} Ipv6;
	};

} sock5_proxy_reply;

#define sock5_proxy_reply_size(rep) \
	offsetof(sock5_proxy_reply, Ipv4) + \
	((rep)->Atyp == IP_V4_ADDRESS ? 6 : \
	 (rep)->Atyp == IP_V6_ADDRESS ? 18 : \
	 (rep)->DomainName.Len + 2)

static int nt_sock5_get_request(SOCKET client, sock5_proxy_request *conn_req)
{
	char buf[BUF_SIZE] = { 0 };
	int len;
	int ret;

	ret = -1;
	len = recv(client, (char *)conn_req, offsetof(sock5_proxy_request, Ipv4), 0);
	if (len != offsetof(sock5_proxy_request, Ipv4))
	{
		err_printf("failed to recv connection request, error: %d\n", sock_errno);
		goto exit;
	}

	switch (conn_req->Atyp)
	{
		case IP_V4_ADDRESS:
		{
			len = recv(client, (char *)&conn_req->Ipv4, sizeof(struct in_addr) + sizeof(unsigned short), 0);
			if (len != sizeof(struct in_addr) + sizeof(unsigned short))
			{
				err_printf("failed to recv connection request, error: %d\n", sock_errno);
				goto exit;
			}

			break;
		}
		case DOMAINNAME:
		{
			len = recv(client, (char *)conn_req + len, 1, 0);
			if (len != 1)
			{
				err_printf("failed to recv connection request, error: %d\n", sock_errno);
				goto exit;
			}

			len = recv(client, (char *)conn_req +
				offsetof(sock5_proxy_request, DomainName) + 1,
				conn_req->DomainName.Len + sizeof(unsigned short), 0);
			if (len != conn_req->DomainName.Len + sizeof(unsigned short))
			{
				err_printf("failed to recv connection request, error: %d\n", sock_errno);
				goto exit;
			}

			break;

		}
		case IP_V6_ADDRESS:
		{
			err_printf("no implement for ipv6\n");
			goto exit;
		}
	}

	ret = 0;
exit:
	return ret;
}

static SOCKET nt_sock5_handle_connect(SOCKET client, sock5_proxy_request *conn_req)
{
	int len;
	int addr_len;
	char ipaddr[64];
	char buf[BUF_SIZE] = { 0 };
	char *domainname = NULL;
	sock5_proxy_reply conn_rep = { 0 };
	struct sockaddr_in addr = { 0 };
	SOCKET server = INVALID_SOCKET;

	switch (conn_req->Atyp)
	{
		case IP_V4_ADDRESS:
		{
			addr.sin_addr.s_addr = *(unsigned int *)conn_req->Ipv4.Addr;
			addr.sin_port        = conn_req->Ipv4.Port;
			addr.sin_family      = AF_INET;

			inet_ntop(AF_INET, &addr.sin_addr, ipaddr, sizeof(ipaddr));
			info_printf("client: %p, sock5 ipv4, address: %s:%hu\n", (void *)(ptrdiff_t)client, ipaddr, ntohs(addr.sin_port));

			break;
		}
		case DOMAINNAME:
		{
			domainname = (char *)malloc(conn_req->DomainName.Len + 1);
			domainname[conn_req->DomainName.Len] = '\0';
			memcpy(domainname, conn_req->DomainName.url, conn_req->DomainName.Len);

			if (ERROR_SUCCESS != safe_gethostbyname(domainname, &addr.sin_addr))
			{
				err_printf("failed to analyze domain name: %s, error: %d\n", domainname, sock_errno);
				goto exit;
			}

			addr.sin_port = *(unsigned char *)(conn_req->DomainName.url + conn_req->DomainName.Len);
			addr.sin_family = AF_INET;

			inet_ntop(AF_INET, &addr.sin_addr, ipaddr, sizeof(ipaddr));
			info_printf("client: %p, sock5 ipv4, address: %s(%s):%hu\n", (void *)(ptrdiff_t)client, domainname, ipaddr, ntohs(addr.sin_port));
			break;

		}
		case IP_V6_ADDRESS:
		{
			err_printf("no implement for ipv6\n");
			goto exit;
		}
	}

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == INVALID_SOCKET)
	{
		err_printf("failed to create socket, error: %d\n", sock_errno);
		goto exit;
	}

	conn_rep.Ver  = conn_req->Ver;
	conn_rep.Atyp = IP_V4_ADDRESS;
	conn_rep.Rsv  = 0;
	conn_rep.Rep  = REPLY_HOST_REFUSED;

	if (SOCKET_ERROR == connect(server, (struct sockaddr *)&addr, sizeof(addr)))
	{
		send(client, (char *)&conn_rep, sock5_proxy_reply_size(&conn_rep), 0);
		closesocket(server);
		err_printf("failed to connect server, error: %d\n", sock_errno);
		goto exit;
	}

	conn_rep.Atyp = IP_V4_ADDRESS;
	conn_rep.Rep = REPLY_SUCCEEDED;
	addr_len = sizeof(addr);
	getpeername(client, (struct sockaddr *)&addr, &addr_len);
	memcpy(conn_rep.Ipv4.Addr, &addr.sin_addr, sizeof(addr.sin_addr));
	conn_rep.Ipv4.Port = addr.sin_port;

	inet_ntop(AF_INET, &addr.sin_addr, ipaddr, sizeof(ipaddr));
	info_printf("client: %p, sock5 ipv4, reply address: %s:%hu\n", (void *)(ptrdiff_t)client, ipaddr, ntohs(addr.sin_port));

	len = send(client, (char *)&conn_rep, sock5_proxy_reply_size(&conn_rep), 0);
	if (len != sock5_proxy_reply_size(&conn_rep))
	{
		closesocket(server);
		err_printf("failed to reply client, error: %d\n", sock_errno);
		goto exit;
	}

exit:
	if (domainname) free(domainname);
	return server;
}

static int nt_sock5_handler_request(SOCKET client, sock5_proxy_request *conn_req)
{
	SOCKET server = INVALID_SOCKET;

	switch (conn_req->Cmd)
	{
		case CMD_CONNECT:
		{
			server = nt_sock5_handle_connect(client, conn_req);
			if (server == INVALID_SOCKET)
			{
				return -1;
			}

			tcp_exchange_data(server, client);
			break;
		}
		case CMD_BIND:
		{
			info_printf("CMD_BIND\n");
			break;
		}
		case CMD_UDP_ASSOCIATE:
		{
			info_printf("CMD_UDP_ASSOCIATE\n");
			break;
		}
	}

	if (server != INVALID_SOCKET) closesocket(server);
	return 0;
}

static int sock5_proxy_handler(void *opaque, SOCKET proxy, SOCKET client)
{
	char buf[BUF_SIZE] = { 0 };
	int len;
	sock5_proxy_auth_request auth_req = { 0 };
	sock5_proxy_auth_response auth_resp = { 0 };
	sock5_proxy_request conn_req = { 0 };

	len = recv(client, (char *)&auth_req, 1, 0);
	if (len <= 0) 
	{
		err_printf("failed to read sock5 identifier, error: %d\n", sock_errno);
		goto exit;
	}

	len = recv(client, ((char *)&auth_req) + 1, 1, 0);
	if (len <= 0)
	{
		err_printf("failed to read sock5 identifier, error: %d\n", sock_errno);
		goto exit;
	}

	len = recv(client, ((char *)&auth_req) + 2, auth_req.nMethods, 0);
	if (len <= 0)
	{
		err_printf("failed to read sock5 identifier, error: %d\n", sock_errno);
		goto exit;
	}

	info_printf("sock5 version: %x\n", auth_req.Ver);
	if (auth_req.nMethods == 0)
	{
		err_printf("no methods\n");
		goto exit;
	}
	else if (auth_req.Methods[0] != NO_AUTHENTICATION_REQUIRED)
	{
		err_printf("no implement of method: %d\n", auth_req.Methods[0]);
		goto exit;
	}

	auth_resp.Ver = auth_req.Ver;
	auth_resp.Method = auth_req.Methods[0];

	len = send(client, (char *)&auth_resp, sizeof(auth_resp), 0);
	if (len != sizeof(auth_resp)) 
	{
		err_printf("failed to send response, error: %d\n", sock_errno);
		goto exit;
	}

	if (0 != nt_sock5_get_request(client, &conn_req))
	{
		err_printf("failed to recv connection request, error: %d\n", sock_errno);
		goto exit;
	}

	if (0 != nt_sock5_handler_request(client, &conn_req))
	{
		err_printf("failed to recv connection request, error: %d\n", sock_errno);
		goto exit;
	}

exit:
	return 0;
}

int startup_sock5_proxy(unsigned short port)
{
	return start_tcp_server(port, sock5_proxy_handler, NULL);
}
