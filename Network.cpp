#pragma once

#include "Network.hpp"

#ifndef UNICODE
#define UNICODE 1
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

int NetSend(s64 sock, char* data, int len, int flags)
{
	return send(sock, data, len, flags);
}

int NetRecv(s64 sock, char* dst, int len, int flags)
{
	return recv(sock, dst, len, flags);
}

int NetBind(s64 sock, const struct sockaddr* name, int namelen)
{
	return bind(sock, name, namelen);
}

int NetListen(s64 sock, int backlog)
{
	return listen(sock, backlog);
}

s64 NetAccept(s64 sock, struct sockaddr* addr, int* addrlen)
{
	return accept(sock, addr, addrlen);
}

s64 NetOpen(int af, int type, int protocol)
{
	return socket(af, type, protocol);
}

int NetShutdown(s64 sock, int how)
{
	return shutdown(sock, how);
}

int NetClose(s64 sock)
{
	return closesocket(sock);
}

bool NetIsSockAvaliable(s64 sock)
{
	s32 result = {};
	char c = {};

	// Important to just Peek, so we don't consume the buffer in case the socket was available.
	result = recv(sock, &c, 1, MSG_PEEK);
	return result > 0;
}

void NetSendAll(s64 sock, String8 data)
{
	int sent = 0;
	
	while (sent < data.length)
	{
		sent += NetSend(sock, data.data, static_cast<int>(data.length - sent));
	}
}

void NetRecvAll(s64 sock, String8 dest)
{
	int recvd = 0;
	
	while (recvd < dest.length)
	{
		recvd += NetRecv(sock, &dest.data[recvd], 512);
	}
}
