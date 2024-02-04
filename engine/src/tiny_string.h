#ifndef TINY_STRING_H
#define TINY_STRING_H

#include "tiny_defines.h"

struct TString
{
    s8* data;
    size_t size;
};


TString TStringLit(const char str[]);
TString TStringFromView(s8* str, size_t size);
TString TStringSubstring(TString str, u32 start, u32 end);
bool TStringEqual(TString str1, TString str2);
// returns index of start of substring, or str.size if not found
u32 TStringFind(TString str, TString toFind, u32 startPos);


#endif