#include "String.hpp"
#include "Defines.hpp"

#include <ctype.h>
#include <string.h>

String8 String8View(const char* s, size_t slen)
{
	String8 s8;

	s8.data = const_cast<char*>(s);
	s8.length = slen;

	return s8;
}

String8 String8Slice(String8 s, size_t start, size_t end)
{
	String8 slice{};

	slice.data = s.data + start;
	slice.length = end - start;

	return slice;
}

void String8Copy(String8 s1, String8 s2)
{
	if (s1.length < s2.length) return;
	strncpy_s(s1.data, s1.length, s2.data, s2.length);
}

void String8CopyAt(String8 s1, String8 s2, size_t at)
{
	if (s1.length < s2.length) return;
	CopyMemory(s1.data + at, s2.data, s2.length);
}

void String8ReplaceInplace(String8 s, char replace, char with)
{
	String8 result = s;

	for (int i = 0; i < result.length; i++)
	{
		if (result.data[i] == replace) result.data[i] = with;
	}
}

bool String8IsEmpty(String8 s)
{
	return s.length == 0;
}

String8 String8Cat(String8 s1, String8 s2)
{
	String8 dst = String8Eat(s1, s1.length - s2.length - 1);
	
	for (int i = 0; i < s2.length; i++)
	{
		dst.data[i] = s2.data[i];
	}
	
	return s1;
}

String8 String8Eat(String8 s, size_t length)
{
	String8 result{};
	result.data = s.data + length;
	result.length = s.length - length;
	return result;
}

String8 String8New(Arena* arena, size_t length)
{
	String8 s{};
	s.data = PushArray(arena, char, length);
	s.length = length;
	return s;
}

String8 String8LowerInplace(String8 s)
{
	String8 result = s;

	for (size_t i = 0; i < s.length; i++)
	{
		result.data[i] = Lower(result.data[i]);
	}

	return result;
}

String8 String8Lower(Arena* arena, String8 src)
{
	String8 result{};

	result.data = PushArray(arena, char, src.length);
	result.length = src.length;

	for (size_t i = 0; i < result.length; i++)
	{
		result.data[i] = Lower(src.data[i]);
	}

	return result;
}

bool String8Contains(String8 s1, String8 s2, String8CompareFlags flags)
{
	bool result = false;

	for (size_t i = 0; (s1.length - i >= s2.length) && (i < s1.length); i++)
	{
		if (String8Equals(String8Slice(s1, i, i + s2.length), s2, flags))
		{
			result = true;
			break;
		}
	}

	return result;
}

String8List String8Fields(Arena* arena, String8 s, FieldsFunc f)
{
	size_t i = 0;
	String8 curstr{};
	String8List fields{};

	while (i < s.length)
	{
		if (f(s.data[i]))
		{
			if (!String8IsEmpty(curstr))
			{
				if (fields.length == 0)
				{
					fields.entries = PushArrayAligned(arena, String8, 1, 1);
				}
				else
				{
					PushArrayAligned(arena, String8, 1, 1);
				}

				ArrayAppend(fields, curstr);
				curstr = {};
			}
		}

		else
		{
			if (String8IsEmpty(curstr) && isascii(s.data[i]))
			{
				curstr.data = &s.data[i];
			}

			curstr.length++;
		}

		i++;
	}

	// Push Last String
	if (!String8IsEmpty(curstr))
	{
		if (ArrayLength(fields) == 0)
		{
			fields.entries = PushArrayAligned(arena, String8, 1, 1);
		}

		ArrayAppend(fields, curstr);
	}

	return fields;
}

String8 PushStr(Arena* arena, const char* s)
{
	String8 s8{};
	size_t slen = strlen(s);
	
	s8.data = PushArray(arena, char, slen);
	s8.length = slen;

	memcpy(s8.data, s, slen);
	return s8;
}

char* String8ToCString(Arena* arena, String8 s)
{
	char* snt = PushArray(arena, char, s.length + 1);
	memcpy(snt, s.data, s.length);
	return snt;
}

bool String8EndsWith(String8 s1, String8 s2)
{
	if (s1.length < s2.length)
	{
		return false;
	}

	return String8CompareSlice(s1, s1.length - s2.length, s1.length, s2);
}

bool String8StartsWith(String8 s1, String8 s2)
{
	if (s1.length < s2.length)
	{
		return false;
	}
	
	return String8CompareSlice(s1, 0, s2.length, s2);
}

String8Builder String8BuilderNew(Arena* arena)
{
	String8Builder builder = { arena, {} };
	return builder;
}

void String8BuilderAppend(String8Builder& builder, String8 s)
{
	char* str = PushBytes(builder.arena, s.length);

	if (builder.str.length == 0)
	{
		builder.str.data = str;
	}
	
	memcpy(builder.str.data + builder.str.length, s.data, s.length);
	builder.str.length += s.length;
}

String8 String8BuilderString(String8Builder& builder)
{
	return builder.str;
}

char StringIndex(String8 s, size_t index)
{
	return s.data[index];
}

bool String8Equals(String8 s1, String8 s2, String8CompareFlags flags)
{
	if (s1.length != s2.length)
	{
		return false;
	}

	for (size_t i = 0; i < s1.length; i++)
	{
		switch (flags)
		{
		case String8CompareFlags::CaseInSensitive:
		{
			if (Lower(s1.data[i]) != Lower(s2.data[i]))
			{
				return false;
			}
		} break;

		case String8CompareFlags::CaseSensitive:
		{
			if (s1.data[i] != s2.data[i])
			{
				return false;
			}
		} break;
		}
	}

	return true;
}

bool String8CompareSlice(String8 s1, size_t start, size_t end, String8 s2)
{
	return String8Equals(String8Slice(s1, start, end), s2);
}

size_t String8Length(String8 s)
{
	return s.length;
}

char* String8Data(String8 s)
{
	return s.data;
}
