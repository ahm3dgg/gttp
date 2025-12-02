# GTTP

A Simple HTTP Server & Library written entirly from Scratch (Except for xxHash and Gzip),
That I decided to share online.

```cpp
u32 HttpServerMain()
{
	Arena* arena = ArenaAlloc();
	HttpServer server = HttpServerNew(arena, EndpointNew(Str8("0.0.0.0"), IPAddrKind::IPv4, 1337));
	
	HttpHandle(server, Str8("/echo/"), [](const HttpRequest& request, HttpResponseWriter& writer) {
		HttpAddHeader(writer, Str8("WTF"), Str8("WITH YOU"));
		HttpSend(writer, Str8("Tsunamii !!!!"));
	});

	printf("Listening on :1337\n");
	if (!HttpListenAndServe(server))
	{
		fprintf(stderr, "[ERROR]: Failed to start server on port 1337\n");
		return 1;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	ThreadStartParams params = { ThreadRoutine(HttpServerMain), nullptr };
	OsHandle mainThread = OsCreateThread(ThreadMainEntry, &params);
	OsWaitForThread(mainThread, -1);
}
```
