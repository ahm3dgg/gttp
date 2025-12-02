#include <winsock2.h>
#include <ws2tcpip.h>

#include <string.h>
#include <cctype>

#include "Http.hpp"
#include "ThirdParty/xxHash/xxhash.h"

#include "ThreadPool.hpp"
#include "Network.hpp"
#include "Endpoint.hpp"
#include "ScratchArena.hpp"

#include "StringFmt.hpp"

HttpRequestParser HttpNewParser(const String8& httpRequest)
{
	HttpRequestParser requestParser{};

	requestParser.buffer = httpRequest;
	requestParser.pos = 0;
	requestParser.state = HttpParserState::ParseMethod;

	return requestParser;
}

HttpMethod HttpParseMethod(HttpRequestParser& httpRequestParser)
{
	HttpMethod httpMethod{};
	String8 httpParserBuffer = httpRequestParser.buffer;
	size_t &httpParserPos = httpRequestParser.pos;
	
	if (String8CompareSlice(httpParserBuffer, httpParserPos, 3, Str8("GET")))
	{
		httpMethod = HttpMethod::GET;
		httpParserPos += 3;
	}
	else if (String8CompareSlice(httpParserBuffer, httpParserPos, 4, Str8("POST")))
	{
		httpMethod = HttpMethod::POST;
		httpParserPos += 4;
	}
	else
	{
		// TODO: Setting as Invalid for Now, But Handle Rest of Request in the the not coming future.
		httpMethod = HttpMethod::INVALID;
	}

	return httpMethod;
}

String8 HttpParsePath(HttpRequestParser& httpRequestParser)
{
	String8 path{};
	String8 reqBuf = httpRequestParser.buffer;
	size_t& pos = httpRequestParser.pos;
	size_t path_length = 0;

	path = String8Slice(reqBuf, pos, reqBuf.length);

	while (pos < reqBuf.length)
	{
		if (reqBuf.data[pos] == ' ')
		{
			break;
		}

		pos++;
		path_length++;
	}

	path.length = path_length;
	return path;
}

HttpHeader HttpParseHeader(HttpRequestParser& httpRequestParser)
{
	HttpHeader httpHeader{.isValid = true};

	String8 reqBuf = httpRequestParser.buffer;
	size_t& pos = httpRequestParser.pos;

	String8 headerKey{};
	headerKey.data = reqBuf.data + pos;

	// Read Header Key
	while (pos < reqBuf.length)
	{
		if (reqBuf.data[pos] == ':')
		{
			pos++;
			break;
		}

		pos++;
		headerKey.length++;
	}

	if (pos >= reqBuf.length)
	{
		httpHeader.isValid = false;
		return httpHeader;
	}

	// Skip Spaces
	while (isspace(reqBuf.data[pos]))
	{
		pos++;
	}

	if (pos >= reqBuf.length)
	{
		httpHeader.isValid = false;
		return httpHeader;
	}

	// Read Header Value
	String8 headerValue{};
	headerValue.data = reqBuf.data + pos;
	while (pos < reqBuf.length)
	{
		if (String8CompareSlice(reqBuf, pos, pos + 2, Str8("\r\n")))
		{
			pos += 2;
			break;
		}

		pos++;
		headerValue.length++;
	}

	if (pos >= reqBuf.length)
	{
		httpHeader.isValid = false;
		return httpHeader;
	}

	httpHeader.key = headerKey;
	httpHeader.value = headerValue;

	return httpHeader;
}

String8 HttpGetHeaderValueByName(const HttpRequest& httpRequest, const String8& name)
{
	Temp scratch = ScratchBegin();

	String8 result{};

	String8 canocalizedKey = String8Lower(scratch.arena, name);
	u64 hash = XXH64(canocalizedKey.data, canocalizedKey.length, 0);
	
	HttpHeaderListEntry slot = httpRequest.httpHeadersSlots[hash % httpRequest.httpHeaderSlotsCount];

	SllIter(slot, next, header)
	{
		if (String8Equals(header->key, name, String8CompareFlags::CaseInSensitive))
		{
			result = header->value;
			break;
		}
	}

	ScratchEnd(scratch);
	return result;
}

HttpRequest HttpParseRequest(Arena* arena, const String8& httpRequestBuffer)
{
	HttpRequest httpRequest{.isValid = true};
	HttpRequestParser httpRequestParser = HttpNewParser(httpRequestBuffer);
	size_t& pos = httpRequestParser.pos;

	httpRequest.httpHeaderSlotsCount = 1024;

	while (pos < httpRequestBuffer.length)
	{
		while (isspace(httpRequestBuffer.data[pos])) pos++;

		switch (httpRequestParser.state)
		{
			case HttpParserState::ParseMethod:
			{
				HttpMethod httpMethod = HttpParseMethod(httpRequestParser);
				
				if (httpMethod == HttpMethod::INVALID)
				{
					httpRequest.isValid = false;
					return httpRequest;
				}
				
				if (!String8CompareSlice(httpRequestBuffer, pos, pos + 1, Str8(" ")))
				{
					httpRequest.isValid = false;
					return httpRequest;
				}
			
				httpRequest.method = httpMethod;

				httpRequestParser.state = HttpParserState::ParsePath;
			} break;

			case HttpParserState::ParsePath:
			{
				String8 path = HttpParsePath(httpRequestParser);
				httpRequest.path = path;
				httpRequestParser.state = HttpParserState::ParseVersion;
			} break;

			case HttpParserState::ParseVersion:
			{
				if (!String8CompareSlice(httpRequestBuffer, pos, pos + HttpVersion.length, HttpVersion))
				{
					httpRequest.isValid = false;
					return httpRequest;
				}

				pos += HttpVersion.length;

				if (!String8CompareSlice(httpRequestBuffer, pos, pos + 2, Str8("\r\n")))
				{
					httpRequest.isValid = false;
					return httpRequest;
				}

				pos += 2;

				httpRequestParser.state = HttpParserState::ParseHeaders;
			} break;

			case HttpParserState::ParseHeaders:
			{
				Temp scratch = ScratchBegin();

				if (pos >= httpRequestBuffer.length)
				{
					httpRequest.isValid = false;
					return httpRequest;
				}

				HttpHeaderListEntry* httpHeadersSlots = PushArray(arena, HttpHeaderListEntry, httpRequest.httpHeaderSlotsCount);
				httpRequest.httpHeadersSlots = httpHeadersSlots;

				while (pos < httpRequestBuffer.length)
				{
					// Check End of Headers
					if (String8CompareSlice(httpRequestBuffer, pos, pos + 2, Str8("\r\n")))
					{
						break;
					}

					HttpHeader httpHeader = HttpParseHeader(httpRequestParser);
					String8 httpHeaderCanoc = String8Lower(scratch.arena, httpHeader.key);

					u64 hash = XXH64(httpHeaderCanoc.data, httpHeaderCanoc.length, 0);
					HttpHeaderListEntry& httpHeaderSlot = httpHeadersSlots[hash % httpRequest.httpHeaderSlotsCount];

					// Check if the Node is Duplicate.
					// We Update the Node but don't push it again in the list.
					bool duplicateNode = false;
					SllIter(httpHeaderSlot, next, entry) 
					{
						if (String8Equals(entry->key, httpHeader.key, String8CompareFlags::CaseInSensitive)) 
						{
							*entry = httpHeader;
							duplicateNode = true;
							break;
						}
					}
					
					if (!duplicateNode)
					{
						HttpHeader* httpHeaderEntry = PushArray(arena, HttpHeader, 1);
						httpHeaderEntry->key = httpHeader.key;
						httpHeaderEntry->value = httpHeader.value;

						SllPush(httpHeaderSlot, next, httpHeaderEntry);
					}
				}

				ScratchEnd(scratch);

			} break;

			default:
			{
				pos++;
			}
		}
	}

	return httpRequest;
}

HttpServer HttpServerNew(Arena* arena, Endpoint endpoint)
{
	HttpServer server = {};

	s32 result = {};
	s64 sock = {};
	WSADATA wsaData = {};
	sockaddr srvaddr = {};

	server.arena = arena;
	server.threadPool = ThreadPoolNew(arena, OsGetNumberOfProcessors());
	server.routes = PushArray(arena, HttpRouteListEntry, MaxRoutesInSlot);

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	server.sock = NetOpen(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (endpoint.ip.kind == IPAddrKind::IPv4)
	{
		sockaddr_in& srvaddr4 = *rcast<sockaddr_in*>(&srvaddr);
		srvaddr4.sin_family = AF_INET;
		srvaddr4.sin_port = endpoint.port;
		srvaddr4.sin_addr.s_addr = endpoint.ip.addr4.addr;
	}

	else if (endpoint.ip.kind == IPAddrKind::IPv6)
	{
		sockaddr_in6& srvaddr6 = *rcast<sockaddr_in6*>(&srvaddr);
		srvaddr6.sin6_family = AF_INET6;
		srvaddr6.sin6_port = endpoint.port;
		memcpy(&srvaddr6.sin6_addr, &endpoint.ip.addr6, 16);
	}

	server.addr = srvaddr;

	return server;
}

HttpRouteHandler* HttpGetHandler(const HttpServer& server, String8 path)
{
	HttpRouteHandler* handler = {};
	String8 basePath = {};

	Temp scratch = ScratchBegin();

	for (size_t i = 1; i < path.length; i++)
	{
		basePath = String8Slice(path, 0, i + 1);

		if (path.data[i] == '/')
		{
			break;
		}
	}

	char* pathNT = String8ToCString(scratch.arena, basePath);

	HttpRouteListEntry& routes = server.routes[XXH64(pathNT, basePath.length, 0) % MaxRoutesInSlot];
	SllIter(routes, next, route)
	{
		if (
			String8Equals(route->path, path) ||
			(String8EndsWith(route->path, Str8("/")) && String8StartsWith(path, route->path))
			)

		{
			handler = route->handler;
			break;
		}
	}

	ScratchEnd(scratch);
	return handler;
}

void HttpWorker(void* param, bool* persistant)
{
	Arena* arena = {};
	String8 requestBuffer = {};
	String8 connectionHeader = {};
	HttpRequest request = {};
	HttpRouteHandler* routeHandler = {};

	bool closeconn = {};
	s64 csock = {};

	HttpWorkerContext* ctx = rcast<HttpWorkerContext*>(param);
	csock = ctx->clientSock;

	// A problem occurs when I re-enqueue a connection back to the Queue, 
	// since I am assuming that connections are persistant by default, 
	// the problem happens is that if the client doesn't have a "Connection: close", 
	// it will get re-enqueued again, but when we try to process that client again we will fail, 
	// because the connection has been closed and we don't know !
	if (!NetIsSockAvaliable(csock))
	{
		closesocket(csock);
		return;
	}

	arena = ArenaAlloc();
	constexpr u32 RequestBufBaseSize = 512;
	char* httpRequestBuffer = PushBytes(arena, RequestBufBaseSize);
	int bytesread = 0;

	for (;;)
	{
		if (bytesread > RequestBufBaseSize)
		{
			PushBytes(arena, RequestBufBaseSize);
		}

		int read = NetRecv(csock, &httpRequestBuffer[bytesread], 1);
		if (read < 0)
		{
			goto Release;
		}

		if (read <= 0) break;

		if (bytesread > 4)
		{
			bool readHeader =
				httpRequestBuffer[bytesread - 3] == '\r' && httpRequestBuffer[bytesread - 2] == '\n' &&
				httpRequestBuffer[bytesread - 1] == '\r' && httpRequestBuffer[bytesread - 0] == '\n';

			if (readHeader)
			{
				break;
			}

			if (bytesread >= 8_KB)
			{
				NetSendAll(csock, Str8("HTTP/1.1 413 Content Too Large"));
				goto Release;
			}
		}

		bytesread++;
	}

	requestBuffer = String8View(httpRequestBuffer, bytesread);
	request = HttpParseRequest(arena, requestBuffer);

	routeHandler = HttpGetHandler(ctx->server, request.path);
	if (routeHandler)
	{
		HttpResponseWriter rw = HttpResponseWriterNew(arena, csock);
		routeHandler(request, rw);
	}
	else
	{
		NetSendAll(csock, Str8("HTTP/1.1 404 Not Found\r\n\r\n"));
		closesocket(csock);
	}

Release:
	ArenaRelease(arena);
}

bool HttpListenAndServe(HttpServer& server)
{
	s32 result = {};

	result = NetBind(server.sock, &server.addr, sizeof(server.addr));
	if (result < 0)
	{
		return false;
	}

	result = NetListen(server.sock, SOMAXCONN);
	if (result < 0)
	{
		return false;
	}

	for (;;)
	{
		sockaddr_in caddr{};
		SOCKET csock = NetAccept(server.sock, &server.addr, 0);

		if (csock == INVALID_SOCKET)
		{
			continue;
		}

		HttpWorkerContext workerCtx = { server, csock };
		ThreadPoolSubmit(server.threadPool, HttpWorker, &workerCtx, sizeof(workerCtx));
	}
}

HttpResponseWriter HttpResponseWriterNew(Arena* arena, s64 socket)
{
	return { arena, socket };
}

void HttpSend(const HttpResponseWriter& rw, String8 data)
{
	Temp scratch = ScratchBegin();

	String8Builder builder = String8BuilderNew(rw.arena);
	String8BuilderAppend(builder, Str8("HTTP/1.1 200 OK\r\n"));
	String8BuilderAppend(builder, FormatString(scratch.arena, Str8("Content-Length: %d\r\n"), data.length));
	String8BuilderAppend(builder, Str8("Content-Type: text/plain\r\n"));

	SllIter(rw.headers, next, header) {
		String8 headerFmt = FormatString(scratch.arena, Str8("%s: %s\r\n"), header->key, header->value);
		String8BuilderAppend(builder, headerFmt);
	}

	String8BuilderAppend(builder, Str8("\r\n"));
	String8BuilderAppend(builder, data);

	NetSendAll(rw.sock, builder.str);
}

void HttpAddHeader(HttpResponseWriter& rw, String8 key, String8 value)
{
	HttpHeader* header = PushArray(rw.arena, HttpHeader, 1);
	header->key = key;
	header->value = value;
	
	SllPush(rw.headers, next, header);
} 

void HttpHandle(HttpServer& server, String8 path, HttpRouteHandler handler)
{
	Temp scratch = ScratchBegin();

	char* pathNT = String8ToCString(scratch.arena, path);

	HttpRouteListEntry& routesSlot = server.routes[XXH64(pathNT, path.length, 0) % MaxRoutesInSlot];

	HttpRoute* route = PushArray(server.arena, HttpRoute, 1);
	route->path = path;
	route->handler = handler;

	SllPush(routesSlot, next, route);

	ScratchEnd(scratch);
}
