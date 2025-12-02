#include "ScratchArena.hpp"
#include "String.hpp"
#include "Endpoint.hpp"
#include "BinUtils.hpp"

IPv4Addr IPParse4(String8 ip)
{
	Temp scratch = ScratchBegin();

	IPv4Addr addr = {};
	//inet_pton(AF_INET, String8ToCString(scratch.arena, ip), &addr.addr);

	ScratchEnd(scratch);

	return addr;
}

IPv6Addr IPParse6(String8 ip)
{
	Temp scratch = ScratchBegin();

	IPv6Addr addr = {};
	//inet_pton(AF_INET6, String8ToCString(scratch.arena, ip), &addr.addr);

	ScratchEnd(scratch);

	return addr;
}

IPAddr IPParse(String8 ip, IPAddrKind kind)
{
	IPAddr addr = { .kind = kind };

	if (kind == IPAddrKind::IPv4)
	{
		addr.addr4 = IPParse4(ip);
	}
	else if (kind == IPAddrKind::IPv6)
	{
		addr.addr6 = IPParse6(ip);
	}

	return addr;
}

Endpoint EndpointNew(String8 ip, IPAddrKind ipKind, u16 port)
{
	Endpoint endpoint = {};
	endpoint.ip = IPParse(ip, ipKind);
	endpoint.port = ToBigEndian16(port);
	return endpoint;
}
