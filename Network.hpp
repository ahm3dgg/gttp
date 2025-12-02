#pragma once

#include "String.hpp"
#include "Types.hpp"

struct sockaddr;

int NetSend(s64 sock, char* data, int len, int flags = 0);
int NetRecv(s64 sock, char* dst, int len, int flags = 0);
int NetBind(s64 sock, const struct sockaddr* name, int namelen);
int NetListen(s64 sock, int backlog);
s64 NetAccept(s64 sock, struct sockaddr* addr, int* addrlen);
s64 NetOpen(int af, int type, int protocol);
int NetShutdown(s64 sock, int how);
int NetClose(s64 sock);
bool NetIsSockAvaliable(s64 sock);
void NetSendAll(s64 sock, String8 data);
void NetRecvAll(s64 sock, String8 dest);
