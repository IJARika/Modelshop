#pragma once
#include <core/utils/strings.h>

namespace FnvHash
{
    constexpr unsigned long long BASIS = 0x811c9dc5;
    constexpr unsigned long long PRIME = 0x1000193;

    inline constexpr unsigned int GetConst(const char* txt, const unsigned int value = BASIS) noexcept
    {
        if (txt[0] == '\0')
            return value;

        return GetConst(&txt[1], (value ^ (unsigned int)txt[0]) * PRIME);
    }

    inline constexpr unsigned int GetConstInsensitive(const char* txt, const unsigned int value = BASIS) noexcept
    {
        if (txt[0] == '\0')
            return value;

        return GetConstInsensitive(&txt[1], (value ^ (unsigned int)Strings::ToLowerCT(txt[0])) * PRIME);
    }

    inline unsigned int Get(const char* txt)
    {
        unsigned int ret = BASIS;
        if (!txt)
            return ret;

        const size_t length = Strings::GetLength(txt);
        for (size_t i = 0u; i < length; ++i)
        {
            ret ^= txt[i];
            ret *= PRIME;
        }

        return ret;
    }

    inline unsigned int GetInsensitive(const char* txt)
    {
        unsigned int ret = BASIS;
        if (!txt)
            return ret;

        size_t length = Strings::GetLength(txt);
        for (size_t i = 0u; i < length; ++i)
        {
            ret ^= Strings::ToLower(txt[i]);
            ret *= PRIME;
        }

        return ret;
    }

    inline unsigned int Get(const wchar_t* txt)
    {
        unsigned int ret = BASIS;
        if (!txt)
            return ret;

        size_t length = Strings::GetLength(txt);
        for (size_t i = 0u; i < length; ++i)
        {
            ret ^= txt[i];
            ret *= PRIME;
        }

        return ret;
    }

    inline unsigned int GetInsensitive(const wchar_t* txt)
    {
        unsigned int ret = BASIS;
        if (!txt)
            return ret;

        size_t length = Strings::GetLength(txt);
        for (size_t i = 0u; i < length; ++i)
        {
            ret ^= Strings::ToLowerW(txt[i]);
            ret *= PRIME;
        }

        return ret;
    }

    inline unsigned int Get(char txt)
    {
        unsigned int ret = BASIS;
        ret ^= txt;
        ret *= PRIME;
        return ret;
    }

    inline unsigned int GetInsensitive(char txt)
    {
        unsigned int ret = BASIS;
        ret ^= Strings::ToLower(txt);
        ret *= PRIME;
        return ret;
    }

    inline unsigned int Get(wchar_t txt)
    {
        unsigned int ret = BASIS;

        ret ^= txt;
        ret *= PRIME;
        return ret;
    }

    inline unsigned int GetInsensitive(wchar_t txt)
    {
        unsigned int ret = BASIS;
        ret ^= Strings::ToLowerW(txt);
        ret *= PRIME;
        return ret;
    }
}

#define CT_HASH( str ) \
[ ]( ) { \
    constexpr unsigned int ret = FnvHash::GetConst(str); \
    return ret; \
}( )

#define CT_HASH_INSENSITIVE(str) \
[ ]( ) { \
    constexpr unsigned int ret = FnvHash::GetConstInsensitive(str); \
    return ret; \
}( )

#define HASH(str) FnvHash::Get(str)
#define HASH_INSENSITIVE(str) FnvHash::GetInsensitive(str)