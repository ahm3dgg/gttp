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

enum class HttpStatusCode {
	Ok = 200,
	NotFound = 404,
	ServerError = 500
};

struct HttpStatus {
	String8 scode;
	HttpStatusCode code;
};

#define DHttpStatus(s, code) HttpStatus(Str8(s), HttpStatusCode::code)
constexpr HttpStatus HttpStatusOk = DHttpStatus("OK", Ok);
constexpr HttpStatus HttpStatusNotFound = DHttpStatus("Not Found", NotFound);
constexpr HttpStatus HttpStatusServerError = DHttpStatus("Server Error", ServerError);

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
	u64 sock;
	sockaddr addr;
	ThreadPool* threadPool;
	HttpRouteListEntry* routes;
};

struct HttpWorkerContext
{
	HttpServer server;
	u64 clientSock;
};

struct HttpResponseWriter
{
	Arena* arena;
	u64 sock;
	HttpStatus status;
	Sll(HttpHeader) headers;
};

internal(HttpRequestParser) HttpNewParser(const String8& httpRequest);
internal(HttpRequest) HttpParseRequest(Arena *arena, const String8& httpRequest);
internal(String8) HttpGetHeaderValueByName(const HttpRequest& httpRequest, const String8& name);
internal(HttpRouteHandler*) HttpGetHandler(const HttpServer& server, String8 path);
internal(proc) HttpWorker(ptr param, bool* persistant);
internal(HttpResponseWriter) HttpResponseWriterNew(Arena* arena, u64 socket);

HttpServer HttpServerNew(Arena* arena, Endpoint endpoint);
proc HttpSend(const HttpResponseWriter& rw, String8 data);
proc HttpSetStatus(HttpResponseWriter& rw, HttpStatus status = HttpStatusOk);
proc HttpAddHeader(HttpResponseWriter& rw, String8 key, String8 value);
proc HttpHandle(HttpServer& server, String8 path, HttpRouteHandler handler);
bool HttpListenAndServe(HttpServer& server);