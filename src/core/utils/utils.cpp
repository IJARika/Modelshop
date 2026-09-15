#include <pch.h>

#include <core/utils/utils.h>

void Error(const char* errorMessage, ...)
{
    va_list inputArgs;
    va_start(inputArgs, errorMessage);

    vprintf(errorMessage, inputArgs);

    exit(EXIT_FAILURE);
}

const char* keepAfterLastSlashOrBackslash(const char* src)
{
    const char* lastSlash = strrchr(src, '/');
    const char* lastBackslash = strrchr(src, '\\');
    const char* lastSeparator = lastSlash > lastBackslash ? lastSlash : lastBackslash;

    if (!lastSeparator)
        return src;

    return lastSeparator + 1;
}

char* const removeExtension(char* const src)
{
    char* const extension = strrchr(src, '.');
    if (extension)
    {
        extension[0] = '\0';
    }

    return src;
}

// copy a string into new memory
char* AllocString(const char* const src, const size_t MAX_SIZE)
{
    assertm(src, "invalid ptr");

    const size_t length = strnlen_s(src, MAX_SIZE);
    const size_t lengthNT = length + 1; // null terminator

    char* out = new char[lengthNT] {};
    strncpy_s(out, lengthNT, src, length);

    return out;
}

// custom itoa
static char luts[40000];
static bool initLuts = false;

void itoa64(int64_t v, char* out)
{
	constexpr uint32_t base = 10;
	constexpr uint32_t lutNumCnt = (sizeof(luts) >> 2);

	// Building 4-digit look-up table on first usage to speed up conversion for future uses, could technically statically define but eh.
	if (!initLuts)
	{
		initLuts = true;
		for (uint32_t i = 0; i < lutNumCnt; ++i) // shifting faster than dividing or multiplying
		{
			uint32_t n = i;
			char* const p = &luts[i << 2];

			// Un-rolling instead of for loop for cache friendlyness, no jumps used.
			p[3] = static_cast<char>(('0' + (n % base)));
			n /= base;

			p[2] = static_cast<char>(('0' + (n % base)));
			n /= base;

			p[1] = static_cast<char>(('0' + (n % base)));
			n /= base;

			p[0] = static_cast<char>(('0' + (n % base)));
		}
	}

	// No need to null terminate, we offset later on to figure out the length
	char buf[32];
	char* p = (buf + sizeof(buf));

	// Converting to absolute value safely using two complements
	// If v is negative, we should wrap around with (~v + 1)
	bool negative = (v < 0);
	uint64_t x = negative ? static_cast<uint64_t>(~v) + 1 : static_cast<uint64_t>(v);

	// This loop extracts the least significant 4 digits of x each iteration
	// chunk = x % 10000` decouples the last 4 decimal digits
	// x /= 10000 removes those digits from x
	while (x >= lutNumCnt)
	{
		uint32_t chunk = x % lutNumCnt;
		x /= lutNumCnt;

		const char* const s = &luts[chunk << 2];
		*--p = s[3];
		*--p = s[2];
		*--p = s[1];
		*--p = s[0];
	}


	// Figure out how many digits to write from the lut
	// x >= 1000 -> 4 digits, x >= 100 -> 3 digits, x >= 10 -> 2 digits, otherwise 1 digit
	const char* const s = &luts[x << 2];
	if (x >= 1000)
	{
		*--p = s[3];
		*--p = s[2];
		*--p = s[1];
		*--p = s[0];
	}
	else if (x >= 100)
	{
		*--p = s[3];
		*--p = s[2];
		*--p = s[1];
	}
	else if (x >= 10)
	{
		*--p = s[3];
		*--p = s[2];
	}
	else
	{
		*--p = s[3];
	}

	if (negative)
	{
		*--p = '-';
	}


	const size_t len = (buf + sizeof(buf) - p);
	memcpy(out, p, len);
	out[len] = '\0';
}