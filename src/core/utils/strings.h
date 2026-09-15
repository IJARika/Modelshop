#pragma once

namespace Strings
{
    constexpr char ToLowerCT(const char c) noexcept
    {
        return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
    }

    constexpr wchar_t ToLowerWCT(const wchar_t c) noexcept
    {
        return (c >= L'A' && c <= L'Z') ? c + (L'a' - L'A') : c;
    }

    constexpr size_t GetLengthCT(const char* str)
    {
        size_t len = 0;
        while (str[len] != '\0')
        {
            ++len;
        }
        return len;
    }

    char ToLower(const char c);
    wchar_t ToLowerW(const wchar_t c);
    size_t GetLength(const char* str);
    size_t GetLength(const wchar_t* str);
    int Equal(const char* str, const char* cmpStr);
    int EqualCaseSensitive(const char* s1, const char* s2, size_t n);
    char* FindInString(const char* str, const char* find);
    const char* FindCharInString(const char* str, char c);
    bool IsWhiteSpace(const unsigned char c);
    double strtod(const char* str, char** endptr);
    int64_t atoi64(const char* str);
}