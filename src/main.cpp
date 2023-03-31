#include <iostream>
#include <filesystem>
#include <fstream>

#include "utils.h"
#include "studio/studio_extract.h"

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

	// set this up here so it's not done per loop, will have to be moved wherever this ends up
	std::string dmxOutPath = std::filesystem::path(filePath).parent_path().append("dmx").u8string();
	std::filesystem::create_directories(dmxOutPath);

	DMXFromMDL(mdlBuf, dmxOutPath);

	delete[] mdlBuf;
	mdlIn.close(); // shouldn't need this
}
