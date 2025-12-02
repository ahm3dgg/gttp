#pragma once

#include "String.hpp"
#include "Types.hpp"

enum class IPAddrKind
{
	IPv4 = 0,
	IPv6
};

struct IPv4Addr
{
	u32 addr;
};

struct IPv6Addr
{
	u64 addr;
};

struct IPAddr
{
	IPAddrKind kind;

	union
	{
		IPv4Addr addr4;
		IPv6Addr addr6;
	};
};

struct Endpoint
{
	IPAddr ip;
	u16 port;
};

IPv4Addr IPParse4(String8 ip);
IPv6Addr IPParse6(String8 ip);
IPAddr IPParse(String8 ip, IPAddrKind kind);
Endpoint EndpointNew(String8 ip, IPAddrKind ipKind, u16 port);
