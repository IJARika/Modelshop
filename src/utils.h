#include <iostream>
#include <filesystem>

#pragma once

#define MAX_PATH_SOURCE		256 // respawn may have changed this

#define GET_FILE_NAME(filePath) std::filesystem::path(filePath).filename().u8string()
#define GET_FILE_STEM(filePath) std::filesystem::path(filePath).stem().u8string()
#define GET_FILE_EXTN(filePath) std::filesystem::path(filePath).extension().u8string().substr(1, std::string::npos) // extension without '.'
#define GET_FILE_PATH(filePath) std::filesystem::path(filePath).parent_path().u8string()

static void Error(const char* errorMessage)
{
	printf("%s\n", errorMessage);
	exit(EXIT_FAILURE);
}