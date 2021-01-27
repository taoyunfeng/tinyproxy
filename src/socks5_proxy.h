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

#ifndef _HAVE_socks5_H_
#define _HAVE_socks5_H_

#include "cross_platform.h"

#define NO_AUTHENTICATION_REQUIRED              0
#define GSSAPI                                  1
#define USERNAME_PASSWORD                       2

#define IP_V4_ADDRESS                           1
#define DOMAINNAME                              3
#define IP_V6_ADDRESS                           4

#define CMD_CONNECT                             1
#define CMD_BIND                                2
#define CMD_UDP_ASSOCIATE                       3

#define REPLY_SUCCEEDED                         0
#define REPLY_GENERAL_SOCKS_SERVER_FAILURE      1
#define REPLY_CONNECTION_NOT_ALLOWRD_BY_RULESET 2
#define REPLY_NETWORK_UNREACHABLE               3
#define REPLY_HOST_REFUSED                      4
#define REPLY_CONNECTION_REFUSED                5
#define REPLY_TTL_EXPIRED                       6
#define REPLY_COMMAND_NOT_SUPPORTED             7
#define REPLY_ADDRESS_TYPE_NOT_SUPPORTED        8


#ifdef __cplusplus
extern "C" {
#endif

	int startup_socks5_proxy(unsigned short port);

#ifdef __cplusplus
};
#endif

#endif