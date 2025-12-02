#include "Types.hpp"
#include "StringFmt.hpp"
#include <string.h>
#include <stdarg.h>

s64 String8ToInt(String8 s)
{
	s64 n = 0;

	for (size_t i = 0; i < s.length; i++)
	{
		n = (n * 10) + (s.data[i] - '0');
	}

	return s.data[0] == '-' ? -n : n;
}

String8 FormatInt(Arena* arena, int number)
{
	String8 Int{};
	Int.data = PushBytes(arena, 12);

	if (number < 0)
	{
		Int.data[0] = '-';
		Int.length++;
	}

	int n = number;
	int digitsCount = 0;

	while (n != 0)
	{
		digitsCount++;
		n /= 10;
	}

	n = number;
	while (n != 0)
	{
		Int.data[digitsCount - Int.length - 1] = '0' + (n % 10);
		Int.length++;
		n /= 10;
	}

	return Int;
}

String8 FormatString(Arena* arena, String8 format, ...)
{
	va_list args;
	va_start(args, format);

	String8 fmtStr{};
	fmtStr.data = PushArray(arena, char, format.length);
	fmtStr.length = format.length;	// Initial Length
	
	size_t p = 0;
	size_t i = 0;
	while (i < format.length)
	{
		if (String8CompareSlice(format, i, i + 2, Str8("%s")))
		{
			String8 Str = va_arg(args, String8);
			PushBytes(arena, Str.length);
			
			fmtStr.length -= 2;
			fmtStr.length += Str.length;

			String8CopyAt(fmtStr, Str, p);

			i += 2;
			p += Str.length;
		}

		else if (String8CompareSlice(format, i, i + 2, Str8("%d")))
		{
			int number = va_arg(args, int);
			String8 Int = FormatInt(arena, number);
			
			fmtStr.length -= 2;
			fmtStr.length += Int.length;

			String8CopyAt(fmtStr, Int, p);

			p += Int.length;
			i += 2;
		}
		
		else
		{
			fmtStr.data[p++] = format.data[i++];
		}
	}

	return fmtStr;
}
