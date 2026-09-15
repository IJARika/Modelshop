#pragma once

class CmdLine
{
public:
	CmdLine(int inArgc, char** inArgv) : argc(inArgc), argv(inArgv) {};
	~CmdLine() {};

	int GetArgIndex(const char* pszArg) const
	{
		for (int argIdx = 0; argIdx < argc; argIdx++)
		{
			if (!strcmp(pszArg, argv[argIdx]))
				return argIdx;
		}

		return -1;
	}

	const char* GetArgValue(const char* pszArg) const
	{
		const int argIdx = GetArgIndex(pszArg);

		// bounds check arg index, add one because we're always expecting a value here
		if (argIdx < 0 || (argIdx + 1) >= argc)
			return nullptr;

		const char* const argValue = GetArg(argIdx + 1);

		// the next arg is not a value, check for '-', the verify it's not a valid number
		if (argValue[0] == '-' && atoi(argValue) == 0)
			return nullptr;

		return argValue;
	}

	inline const int GetArgInt(const char* const pszArg, const int iBase = 0) const
	{
		const char* const argValue = GetArgValue(pszArg);

		if (!argValue)
			return 0;

		return strtol(argValue, nullptr, iBase);
	}

	inline const unsigned int GetArgUInt(const char* const pszArg, const int iBase = 0) const
	{
		const char* const argValue = GetArgValue(pszArg);

		if (!argValue)
			return 0u;

		return strtoul(argValue, nullptr, iBase);
	}

	inline const char* GetArg(int argIdx) const { return argv[argIdx]; }
	inline bool HasArg(const char* pszArg) const { return GetArgIndex(pszArg) > -1 ? true : false; }
	inline const char* operator[](int argIdx) const { return GetArg(argIdx); }
	inline const int ArgCount() const { return argc; }

private:
	int argc;
	char** argv;
};
