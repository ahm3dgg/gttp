#pragma once

#include "Defines.hpp"
#include "DataStructures.hpp"
#include "Arena.hpp"

#define Str8(s) { (char*)s, sizeof(s) - 1 }

struct String8;

Array(String8Array, String8);
struct String8
{
	char* data;
	size_t length;
};

struct String8Builder
{
	Arena* arena;
	String8 str;
};

enum class String8CompareFlags
{
	CaseInSensitive = 0,
	CaseSensitive,
};

struct String8Node
{
	String8Node* next;
	String8 str;
};

struct String8List
{
	size_t count;
	Sll(String8Node) list;
};

using FieldsFunc = bool(*)(char);

String8 String8View(const char* s, size_t slen);
String8 String8Slice(String8 s, size_t start, size_t end);
String8 String8Cat(String8 s1, String8 s2);
String8 String8Eat(String8 s, size_t length);
String8 String8New(Arena* arena, size_t length);
String8 String8LowerInplace(String8 s);
String8 String8Lower(Arena* arena, String8 src);

proc String8Copy(String8 s1, String8 s2);
proc String8CopyAt(String8 s1, String8 s2, size_t at);
proc String8ReplaceInplace(String8 s, char replace, char with);
bool String8IsEmpty(String8 s);

bool String8Contains(String8 s1, String8 s2, String8CompareFlags flags = String8CompareFlags::CaseSensitive);
bool String8Equals(String8 s1, String8 s2, String8CompareFlags flags = String8CompareFlags::CaseSensitive);
bool String8CompareSlice(String8 s1, size_t start, size_t end, String8 s2);

char String8Index(String8 s, size_t index);
size_t String8Length(String8 s);
char* String8Data(String8 s);

String8List* String8FieldsList(Arena* arena, String8 s, FieldsFunc f);
String8Array String8Fields(Arena* arena, String8 s, FieldsFunc f);

String8 PushStr(Arena* arena, const char* s);

char* String8ToCString(Arena* arena, String8 s);
String8 CStringToString8(char* s);

bool String8EndsWith(String8 s1, String8 s2);
bool String8StartsWith(String8 s1, String8 s2);

String8Builder String8BuilderNew(Arena* arena);
proc String8BuilderAppend(String8Builder& builder, String8 s);
String8 String8BuilderString(String8Builder& builder);
