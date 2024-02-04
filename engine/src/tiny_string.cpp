#include "tiny_string.h"
#include "math/tiny_math.h"
#include "tiny_log.h"


TString TStringLit(const char str[])
{
    TString res = {};
    res.size = ARRAY_SIZE(str);
    res.data = (s8*)&str[0];
    return res;
}

TString TStringFromView(s8* str, size_t size)
{
    TString res = {};
    res.size = size;
    res.data = str;
    return res;
}

bool TStringEqual(TString str1, TString str2)
{
    bool result = false;
    if (str1.size == str2.size)
    {
        result = true;
        for (u32 i = 0; i < str1.size; i++)
        {
            if (str1.data[i] != str2.data[i])
            {
                result = false;
                break;
            }
        }
    }
    return result;
}

TString TStringSubstring(TString str, u32 start, u32 end)
{
    start = Math::Clamp(start, start, end);
    end = Math::Clamp(end, start, end);
    TINY_ASSERT(start <= end);
    str.data += start;
    str.size = end - start;
    return str;
}

u32 TStringFind(TString str, TString toFind, u32 startPos)
{
    u32 found_idx = str.size;
    for (u32 i = startPos; i < str.size; i++)
    {
        if(i + toFind.size <= str.size)
        {
            TString substr_from_str = TStringSubstring(str, i, i+toFind.size);
            if(TStringEqual(substr_from_str, toFind))
            {
                found_idx = i;
                break;
            }
        }
    }
    return found_idx;
}