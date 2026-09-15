#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _USE_MATH_DEFINES

#include "Windows.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <math.h>
#include <algorithm>
#include <combaseapi.h>
#include <chrono>

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <stack>

#include <mutex>
#include <thread>

// TEMP!!! used for DMX GUIDs, I should do this better
#include <guiddef.h>

// simd
#include <emmintrin.h>
#include <smmintrin.h>
#include <directxmath.h>


//
#include <core/cmdline.h>
#include <core/shareddefs.h>
#include <core/threader.h>
#include <core/utils/utils.h>
#include <core/utils/stringtable.h>
#include <core/utils/buffermanager.h>
#include <core/utils/textbuffer.h>
#include <core/utils/keyvalue_parser.h>
#include <core/utils/strings.h>
#include <core/utils/fnvhash.h>

//#define DirectXMath
#define MATH_ASSERTS 1
#define MATH_SIMD 1

#ifdef M_PI
constexpr float s_M_PI = static_cast<float>(M_PI);
#undef M_PI
#define M_PI s_M_PI
#endif // M_PI


#include <core/math/mathlib.h>
#include <core/math/float16.h>
#include <core/math/vector.h>
#include <core/math/vector2d.h>
#include <core/math/vector4d.h>
#include <core/math/color32.h>
#include <core/math/compressed_vector.h>
#include <core/math/matrix3x4.h>
#include <core/math/vmatrix.h>

#include <model/datamodel/timeutils.h>
#include <model/datamodel/datamodel.h>
#include <model/datamodel/datamodel_classes.h>

#include <model/physicsmodel/physicsmodel.h>
#include <model/rmax/rmax.h>
#include <model/smd/smd.h>
#include <model/qc/qc.h>

#include <core/settings.h>