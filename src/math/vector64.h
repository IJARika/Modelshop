#include <cstdint>

#include "vector.h"

#pragma once

//struct Vector64
//{
//	uint64_t x : 21;
//	uint64_t y : 21;
//	uint64_t z : 22;
//};

//Vector64 PackPos_UINT64(Vector vec)
//{
//	Vector64 pos;
//
//	pos.x = ((vec.x + 1024.0) / 0.0009765625);
//	pos.y = ((vec.y + 1024.0) / 0.0009765625);
//	pos.z = ((vec.z + 2048.0) / 0.0009765625);
//
//	return pos;
//}

// finish this
class Vector64
{
public:

private:
	uint64_t x : 21;
	uint64_t y : 21;
	uint64_t z : 22;
};