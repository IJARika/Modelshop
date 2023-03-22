#include <iostream>
#include <filesystem>
#include <fstream>

#include "utils.h"
#include "dmx/dmx.h"

int main(int argc, char** argv)
{
	// error because we need a file
	if (argc < 2)
		Error("that's not what it's meant for, goodbye!");

	std::string filePath = argv[argc - 1];

	if (!std::filesystem::exists(filePath))
		Error("file not found");

	std::ifstream mdlIn(filePath, std::ios::in | std::ios::binary);

	char* mdlBuf = new char[std::filesystem::file_size(filePath)];

	mdlIn.read(mdlBuf, std::filesystem::file_size(filePath));

	DMXFromMDL(mdlBuf, GET_FILE_PATH(filePath));

	delete[] mdlBuf;

	mdlIn.close();
}
