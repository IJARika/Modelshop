#pragma once

#define GET_FILE_NAME(filePath) std::filesystem::path(filePath).filename().string()
#define GET_FILE_STEM(filePath) std::filesystem::path(filePath).stem().string()
#define GET_FILE_EXTN(filePath) std::filesystem::path(filePath).extension().string().substr(1, std::string::npos) // extension without '.'
#define GET_FILE_EXTD(filePath) std::filesystem::path(filePath).extension().string() // extension with '.'
#define GET_FILE_PATH(filePath) std::filesystem::path(filePath).parent_path().string()
#define PATH_TO_CSTRING(filePath) std::filesystem::path(filePath).string().c_str()

#define IALIGN(a, b) (((a)+((b)-1)) & ~((b)-1))
#define ALIGN(a, b) a = (char *)((__int64)((char *)a + (b-1)) & ~(b-1))

#define IALIGN2(a) IALIGN(a, 2)
#define IALIGN4(a) IALIGN(a, 4)
#define IALIGN8(a) IALIGN(a, 8)
#define IALIGN16(a) IALIGN(a, 16)

#define ALIGN2(a) ALIGN(a, 2)
#define ALIGN4(a) ALIGN(a, 4)
#define ALIGN8(a) ALIGN(a, 8)
#define ALIGN16(a) ALIGN(a, 16)
#define ALIGN64(a) ALIGN(a, 64)
#define ALIGN512(a) ALIGN(a, 512)

#define UNUSED(x) (void)(x)
#define ARRSIZE(a) (sizeof(a) / sizeof(*(a)))

#define assertm(exp, msg) assert(((void)msg, exp))

#define MAKEFOURCC(a, b, c, d) ((d<<24)+(c<<16)+(b<<8)+a)

#define sizeof_u32(var) static_cast<uint32_t>(sizeof(var))

#define unreachable() (__assume(false))
#define UNREACHABLE() unreachable()

// one liners to make it Look Fancy
#define FreeAllocArray(var) if (nullptr != var) { delete[] var; }
#define FreeAllocVar(var) if (nullptr != var) { delete var; }
#define FreeAllocVector(var) for (auto& v : var) { if (nullptr != v) delete v; }
#define FreeAllocPtrArray(var, count)	\
{										\
	for (int64_t i = 0; i < count; i++)	\
	{									\
		FreeAllocArray(var[i]);			\
	}									\
										\
	FreeAllocArray(var);				\
}

void Error(const char* errorMessage, ...);

const char* keepAfterLastSlashOrBackslash(const char* src); // keep filename
char* const removeExtension(char* const src); // create stem from filename

inline bool CreateDirectories(const std::filesystem::path& exportPath)
{
	if (!std::filesystem::exists(exportPath))
	{
		if (!std::filesystem::create_directories(exportPath))
		{
			return false;
		}
	}
	return true;
}

// copy a string into new memory
char* AllocString(const char* const src, const size_t MAX_SIZE = MAX_PATH);

// copy a buffer into new memory
template<typename T>
T* AllocBuffer(const T* const src, const size_t count)
{
	assertm(src, "invalid ptr");

	const size_t bufferSize = sizeof(T) * count;
	T* out = new T[count]{};
	memcpy_s(out, bufferSize, src, bufferSize);

	return out;
}

inline constexpr size_t strlen_ct(const char* str)
{
	size_t length = 0ull;
	while (str[length] != '\0')
	{
		++length;
	}

	return length;
}

// use memcpy_s to copy strings (faster than normal strncpy_s)
// returns size of data wrote sans null terminator
FORCEINLINE const size_t strncpy_mem(char* dest, size_t destsz, const char* src, size_t count)
{
	const size_t length = strnlen_s(src, count);
	const errno_t result = memcpy_s(dest, destsz, src, length + 1ull);

	assertm(result == 0, "failed to copy");

	// ensure null terminator, if memcpy_s did not fail, this should be valid memory
	dest[length] = '\0';

	return length;
}

// todo: different levels of log? logger?
#if 1
inline void Log(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
#else
#define Log(...) ((void)nullptr)
#endif

// adapted from rmdlconv
class CScopeTimer
{
public:
	CScopeTimer(const char* const name) : m_name(name), m_startTime(std::chrono::high_resolution_clock::now())
	{

	}

	~CScopeTimer()
	{
		const std::chrono::steady_clock::time_point endTime = std::chrono::high_resolution_clock::now();
		const std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime);
		const float flDuration = static_cast<float>(duration.count()) / 1000.0f;

		Log("%s: completed after %.3f ms...\n", m_name, flDuration);
	}

private:
	const char* const m_name;
	const std::chrono::steady_clock::time_point m_startTime;
};

// todo
// "Returns time in seconds since the module was loaded."
inline const float Plat_FloatTime()
{
	return 0.0f;
}

void itoa64(int64_t v, char* out);