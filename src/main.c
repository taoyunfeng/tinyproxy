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
#include "trace.h"
#include "sock.h"
#include "sock5_proxy.h"
#include "http_proxy.h"
#include "port_mapping.h"

#define type_unknown -1

static void sock5_proxy_proc(unsigned short port)
{
	info_printf("sock5 proxy startup, listen port: %hu\n", port);
	info_printf("running code: %d\n", startup_sock5_proxy(port));
	info_printf("sock5 proxy end\n");
}

static void http_proxy_proc(unsigned short port)
{
	info_printf("http proxy startup, listen port: %hu\n", port);
	info_printf("running code: %d\n", startup_http_proxy(port));
	info_printf("http proxy end\n");
}

static void port_mapping_proc(unsigned short from, const char *address, unsigned short to)
{
	info_printf("port mapping startup, from %hu to %s:%hu\n", from, address, to);
	info_printf("running code: %d\n", start_port_mapping(from, address, to));
	info_printf("port mapping end\n");
}

int main(int argc, char *const *argv)
{
	static const struct option options[] = {
		{ "listen" , required_argument, NULL, 'l' },
		{ "address", required_argument, NULL, 'a' },
		{ "port"   , required_argument, NULL, 'p' },
		{ "sock5"  , no_argument      , NULL, 's' },
		{ "http"   , no_argument      , NULL, 'h' },
		{ "mapping", no_argument      , NULL, 'm' },
		{ NULL, 0, NULL, 0 }
	};

	int opt;
	int err;
	int type;
	const char *address = NULL;
	const char *port = NULL;
	const char *des_port = NULL;

	type = type_unknown;

	while (-1 != (opt = getopt_long(argc, argv, "l:a:p:shm", options, NULL)))
	{
		switch (opt)
		{
			case 'l': port = optarg; break;
			case 'a': address = optarg; break;
			case 'p': des_port = optarg; break;
			case 'h':
			case 'm':
			case 's': type = opt; break;
		}
	}

	if (port == NULL || 
		type == type_unknown) 
	{
		err_printf("invalid argument.\n");
		goto exit;
	}

	if (ERROR_SUCCESS != (err = init_sock())) 
	{
		err_printf("failed to init sock, error: %d\n", err);
		goto exit;
	}

	info_printf("init sock success\n");

	switch (type)
	{
		case 's': sock5_proxy_proc((unsigned short)strtoul(port, NULL, 0)); break;
		case 'm': port_mapping_proc((unsigned short)strtoul(port, NULL, 0), address, (unsigned short)strtoul(des_port, NULL, 0)); break;
		case 'h': http_proxy_proc((unsigned short)strtoul(port, NULL, 0)); break;
	}

exit:
	exit_sock();
	return 0;
}