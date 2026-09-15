#include <pch.h>
#include <core/utils/strings.h>

namespace Strings
{
    char ToLower(const char c)
    {
        return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
    }

    wchar_t ToLowerW(const wchar_t c)
    {
        return (c >= L'A' && c <= L'Z') ? c + (L'a' - L'A') : c;
    }

    size_t GetLength(const char* str)
    {
        size_t len = 0;
        while (str[len] != '\0')
        {
            ++len;
        }
        return len;
    }

    size_t GetLength(const wchar_t* str)
    {
        size_t len = 0;
        while (str[len] != L'\0')
        {
            ++len;
        }
        return len;
    }

    int Equal(const char* str, const char* cmpStr)
    {
        while (true)
        {
            const char ch = *str++;
            const char cch = *cmpStr++;
            if (ch != cch)
                return ch < cch ? -1 : 1;

            if (ch == '\0')
                return 0;
        }
    }

    int EqualCaseSensitive(const char* s1, const char* s2, size_t n)
    {
        size_t i = 0;
        if (n == 0)
        {
            return 0;
        }

        while (i < n && s1[i] != '\0' && s2[i] != '\0')
        {
            if (s1[i] != s2[i])
            {
                return static_cast<unsigned char>(s1[i]) - static_cast<unsigned char>(s2[i]);
            }
            ++i;
        }

        if (i == n)
        {
            return 0;
        }

        return static_cast<unsigned char>(s1[i]) - static_cast<unsigned char>(s2[i]);
    }

    char* FindInString(const char* str, const char* find)
    {
        for (const char* s = str; *s; s++)
        {
            const char* s_it = s;
            const char* f_it = find;

            while (*s_it && *f_it && (*s_it == *f_it))
            {
                s_it++;
                f_it++;
            }

            if (*f_it == '\0')
            {
                return const_cast<char*>(s);
            }
        }

        return nullptr;
    }

    const char* FindCharInString(const char* str, char c)
    {
        while (*str)
        {
            if (*str == c)
            {
                return str;  // found it, return pointer to this char
            }
            str++;
        }

        // Special case: if c == '\0', return pointer to end of string
        if (c == '\0')
        {
            return str;
        }

        return nullptr; // not found
    }

    bool IsWhiteSpace(const unsigned char c)
    {
        switch (c)
        {
        case ' ':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
            return true;
        default:
            return false;
        }

        UNREACHABLE();
    }

    double strtod(const char* str, char** endptr)
    {
        const char* p = str;

        // Skip leading whitespace
        while (*p && IsWhiteSpace(static_cast<unsigned char>(*p)))
        {
            ++p;
        }

        bool neg = false;
        if (*p == '+' || *p == '-')
        {
            if (*p == '-') neg = true;
            ++p;
        }

        double value = 0.0;
        while (*p >= '0' && *p <= '9')
        {
            value = value * 10.0 + (*p - '0');
            ++p;
        }

        if (*p == '.')
        {
            ++p;
            double factor = 0.1;
            while (*p >= '0' && *p <= '9')
            {
                value += (*p - '0') * factor;
                factor *= 0.1;
                ++p;
            }
        }

        if (*p == 'e' || *p == 'E')
        {
            ++p;
            bool exp_neg = false;
            if (*p == '+' || *p == '-')
            {
                if (*p == '-') exp_neg = true;
                ++p;
            }

            int exp_val = 0;
            while (*p >= '0' && *p <= '9')
            {
                exp_val = exp_val * 10 + (*p - '0');
                ++p;
            }

            if (exp_neg)
            {
                exp_val = -exp_val;
            }

            value *= pow(10.0, exp_val);
        }

        if (neg)
        {
            value = -value;
        }

        if (endptr)
        {
            *endptr = const_cast<char*>(p);
        }

        return value;
    }

    int64_t atoi64(const char* str)
    {
        assert(str != nullptr);

        int64_t val;
        int64_t sign;
        int64_t c;

        if (*str == '-')
        {
            sign = -1;
            str++;
        }
        else if (*str == '+')
        {
            sign = 1;
            str++;
        }
        else
        {
            sign = 1;
        }

        val = 0;

        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        {
            str += 2;
            while (1)
            {
                c = *str++;
                if (c >= '0' && c <= '9')
                {
                    val = (val << 4) + c - '0';
                }
                else if (c >= 'a' && c <= 'f')
                {
                    val = (val << 4) + c - 'a' + 10;
                }
                else if (c >= 'A' && c <= 'F')
                {
                    val = (val << 4) + c - 'A' + 10;
                }
                else
                {
                    return val * sign;
                }
            }
        }

        if (str[0] == '\'')
        {
            return sign * str[1];
        }

        while (1)
        {
            c = *str++;
            if (c < '0' || c > '9')
            {
                return val * sign;
            }
            val = val * 10 + c - '0';
        }

        return 0;
    }
}
