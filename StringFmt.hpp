#pragma once

#include "Arena.hpp"
#include "String.hpp"

String8 FormatString(Arena* arena, String8 format, ...);
String8 FormatInt(Arena* arena, int number);
s64 String8ToInt(String8 s);