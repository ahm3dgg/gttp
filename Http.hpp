#pragma once

#include "String.hpp"
#include "Arena.hpp"
#include "DataStructures.hpp"
#include "Endpoint.hpp"
#include "ThreadPool.hpp"

constexpr String8 HttpVersion = Str8("HTTP/1.1");
constexpr u32 MaxRoutesInSlot = 1024;

enum class HttpMethod {
	GET = 0,
	POST,
	INVALID,
};

enum class HttpParserState {
	ParseMethod = 0,
	ParsePath,
	ParseVersion,
	ParseHeaders,
	ParseBody
};

struct HttpRequestParser {
	HttpParserState state;
	String8 buffer;
	size_t pos;
};

struct HttpHeader {
	HttpHeader* next;

	bool isValid;
	String8 key;
	String8 value;
};

Sll_N(HttpHeaderListEntry, HttpHeader);
struct HttpRequest {
	bool isValid;

	HttpMethod method;
	String8 path;
	
	u64 httpHeaderSlotsCount;
	HttpHeaderListEntry* httpHeadersSlots;
};

struct HttpRequest;
struct HttpResponseWriter;
using HttpRouteHandler = void(const HttpRequest&, HttpResponseWriter&);

struct HttpRoute
{
	HttpRoute* next;

	String8 path;
	HttpRouteHandler* handler;
};

Sll_N(HttpRouteListEntry, HttpRoute);
struct HttpServer
{
	Arena* arena;
	s64 sock;
	sockaddr addr;
	ThreadPool* threadPool;
	HttpRouteListEntry* routes;
};

struct HttpWorkerContext
{
	HttpServer server;
	s64 clientSock;
};

struct HttpResponseWriter
{
	Arena* arena;
	s64 sock;
	Sll(HttpHeader) headers;
};

static HttpRequestParser HttpNewParser(const String8& httpRequest);
HttpRequest HttpParseRequest(Arena *arena, const String8& httpRequest);
String8 HttpGetHeaderValueByName(const HttpRequest& httpRequest, const String8& name);

HttpServer HttpServerNew(Arena* arena, Endpoint endpoint);
HttpRouteHandler* HttpGetHandler(const HttpServer& server, String8 path);
void HttpWorker(void* param, bool* persistant);
bool HttpListenAndServe(HttpServer& server);
HttpResponseWriter HttpResponseWriterNew(Arena* arena, s64 socket);

void HttpSend(const HttpResponseWriter& rw, String8 data);
void HttpAddHeader(HttpResponseWriter& rw, String8 key, String8 value);

void HttpHandle(HttpServer& server, String8 path, HttpRouteHandler handler);
