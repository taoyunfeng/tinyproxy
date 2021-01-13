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

int safe_gethostbyname(const char *name, struct in_addr *addr)
{
#ifdef _WIN32
	int found;
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (0 != getaddrinfo(name, NULL, &hints, &result))
	{
		return sock_errno;
	}

	for (ptr = result, found = 0; ptr && !found; ptr = ptr->ai_next)
	{
		if (ptr->ai_family == AF_INET &&
			ptr->ai_socktype == SOCK_STREAM)
		{
			found = 1;
			memcpy(addr, &((struct sockaddr_in *)ptr->ai_addr)->sin_addr, sizeof(*addr));
			break;
		}
	}

	freeaddrinfo(result);
	return found ? ERROR_SUCCESS : ERROR_NOT_FOUND;
#elif defined(__linux__)
	struct hostent result, *presult;
	char buf[8192];
	int ret;
	int err;
	int i;
	int found;

	ret = gethostbyname_r(name, &result, buf, sizeof(buf), &presult, &err);
	if (ret != 0) 
	{
		return err;
	}

	found = 0;
	if (presult->h_addrtype == AF_INET)
	{
		memcpy(addr, presult->h_addr_list[i], sizeof(*addr));
		found = 1;
	}

	return found ? 0 : EAGAIN;
#endif
}

#ifdef _WIN32

struct win32_thread_parameter {
	void *(*thread_routine)(void *arg);
	void *arg;
};

static DWORD __stdcall __thread_start(struct win32_thread_parameter *arg)
{
	DWORD ret;
	ret = (DWORD)arg->thread_routine(arg->arg);
	free(arg);
	return ret;
}

#endif

int create_thread(void *(*thread_routine)(void *arg), void *arg)
{
#ifdef _WIN32
	HANDLE thread;
	struct win32_thread_parameter *param;

	param = (struct win32_thread_parameter *)malloc(sizeof(struct win32_thread_parameter));
	if (param == NULL)
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	param->thread_routine = thread_routine;
	param->arg = arg;

	thread = CreateThread(
		NULL, 
		0, 
		(LPTHREAD_START_ROUTINE)__thread_start,
		param, 
		0, 
		NULL);

	if (thread == NULL)
	{
		return GetLastError();
	}

	CloseHandle(thread);
	return ERROR_SUCCESS;
#elif defined(__linux__)
	pthread_t tid;
	int err;

	err = pthread_create(&tid, 0, thread_routine, arg);
	if (err != 0)
	{
		return err;
	}

	pthread_detach(tid);
	return 0;
#endif
}
