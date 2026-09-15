#pragma once

// Modelshop version
// todo: resource file?
#define VERSION_MAJOR		0
#define VERSION_MINOR		0
#define VERSION_REVISION	1


#define MAX_PATH_SOURCE		512 // respawn may have changed this
								// adjust as needed


// ---------------------------
//  Hit Group standards
// ---------------------------
#define HITGROUP_INVALID	-1
#define	HITGROUP_GENERIC	0
#define	HITGROUP_HEAD		1
#define	HITGROUP_CHEST		2
#define	HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7
#define HITGROUP_GEAR		8			// alerts NPC, but doesn't do damage or bleed (1/100th damage)
#define HITGROUP_COUNT		9

// these are different in respawn games, but it's good to know them for future reference.
#define HITGROUP_NECK_LEGACY	8			// unsure where this even comes from
#define HITGROUP_GEAR_LEGACY	10			// alerts NPC, but doesn't do damage or bleed (1/100th damage)


// these should be moved to bspflags.h in the future

// ---------------------------
//  BSPFLAGS
// ---------------------------

// no ladder
namespace r2
{
	enum ContentFlags_t : int
	{
		CONTENTS_EMPTY =				0x0,
		CONTENTS_SOLID =				0x1,
		CONTENTS_WINDOW =				0x2,
		CONTENTS_AUX =					0x4,
		CONTENTS_GRATE =				0x8,
		CONTENTS_SLIME =				0x10,
		CONTENTS_WATER =				0x20,
		CONTENTS_WINDOW_NOCOLLIDE =		0x40,
		CONTENTS_OPAQUE =				0x80,
		CONTENTS_TESTFOGVOLUME =		0x100,
		CONTENTS_PHYSICSCLIP =			0x200,
		CONTENTS_BLOCKLIGHT =			0x400,
		CONTENTS_NOGRAPPLE =			0x800,
		CONTENTS_UNUSED_03 =			0x1000,
		CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
		CONTENTS_MOVEABLE =				0x4000,
		CONTENTS_TEST_SOLID_BODY_SHOT = 0x8000,
		CONTENTS_PLAYERCLIP =			0x10000,
		CONTENTS_MONSTERCLIP =			0x20000,
		CONTENTS_OPERATOR_FLOOR =		0x40000,
		CONTENTS_BLOCKLOS =				0x80000,
		CONTENTS_NOCLIMB =				0x100000,
		CONTENTS_TITANCLIP =			0x200000,
		CONTENTS_BULLETCLIP =			0x400000,
		CONTENTS_OPERATORCLIP =			0x800000,
		CONTENTS_MONSTER =				0x2000000,
		CONTENTS_DEBRIS =				0x4000000,
		CONTENTS_DETAIL =				0x8000000,
		CONTENTS_TRANSLUCENT =			0x10000000,
		CONTENTS_HITBOX =				0x40000000
	};
}

namespace r5
{
	enum ContentFlags_t : int
	{
		CONTENTS_EMPTY =				0x0,
		CONTENTS_SOLID =				0x1,
		CONTENTS_WINDOW =				0x2,
		CONTENTS_AUX =					0x4,
		CONTENTS_GRATE =				0x8,
		CONTENTS_SLIME =				0x10,
		CONTENTS_WATER =				0x20,
		CONTENTS_WINDOW_NOCOLLIDE =		0x40,
		CONTENTS_OPAQUE =				0x80,
		CONTENTS_TESTFOGVOLUME =		0x100,
		CONTENTS_PHYSICSCLIP =			0x200,
		CONTENTS_NOGRAPPLE =			0x400,
		CONTENTS_IGNORE_NODRAW_OPAQUE = 0x2000,
		CONTENTS_MOVEABLE =				0x4000,
		CONTENTS_TEST_SOLID_BODY_SHOT = 0x8000,
		CONTENTS_PLAYERCLIP =			0x10000,
		CONTENTS_MONSTERCLIP =			0x20000,
		CONTENTS_OPERATOR_FLOOR =		0x40000,
		CONTENTS_BLOCKLOS =				0x80000,
		CONTENTS_NOCLIMB =				0x100000,
		CONTENTS_TITANCLIP =			0x200000,
		CONTENTS_BULLETCLIP =			0x400000,
		CONTENTS_OPERATORCLIP =			0x800000,
		CONTENTS_NOAIRDROP =			0x1000000,
		CONTENTS_MONSTER =				0x2000000,
		CONTENTS_DEBRIS =				0x4000000,
		CONTENTS_DETAIL =				0x8000000,
		CONTENTS_TRANSLUCENT =			0x10000000,
		CONTENTS_BLOCK_PING =			0x20000000,
		CONTENTS_HITBOX =				0x40000000
	};
}

// TEMP!
#define	CONTENTS_EMPTY			0		// No contents
#define	CONTENTS_SOLID			0x1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x2		// translucent, but not watery (glass)
#define	CONTENTS_GRATE			0x8		// alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
#define	CONTENTS_WATER			0x20
#define CONTENTS_PLAYERCLIP		0x10000		// models/levels_terrain\mp_relic\mp_relic_ship_engine_collision.mdl
#define CONTENTS_TITANCLIP		0x200000	// models/levels_terrain\mp_relic\mp_relic_ship_engine_collision.mdl
#define CONTENTS_BULLETCLIP		0x400000	// models/imc_base\chain_link_imc_01.mdl (r1)
#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEBRIS			0x4000000
#define CONTENTS_LADDER			0x20000000 // removed in r1 and r2, replaced in r5

enum class CompressionType_t : unsigned char
{
	DEFAULT		= 0, // no compression
	PAKFILE		= 1, // bad name
	SNOWFLAKE	= 2,
	OODLE		= 3, // (n)oodle
	_COUNT
};

enum MaterialShaderType_t : uint8_t
{
	RGDU = 0x0,
	RGDP = 0x1,
	RGDC = 0x2,
	SKNU = 0x3,
	SKNP = 0x4,
	SKNC = 0x5,
	WLDU = 0x6,
	WLDC = 0x7,
	PTCU = 0x8,
	PTCS = 0x9,
	RGBS = 0xA,

	_MAT_SHADER_TYPE_COUNT,
	_MAT_SHADER_TYPE_INVALID = _MAT_SHADER_TYPE_COUNT,
};

union AssetGuid_t
{
	AssetGuid_t() = default;
	AssetGuid_t(const size_t guid_in) : guid(guid_in) {}
	AssetGuid_t(void* ptr_in) : ptr(ptr_in) {}

	size_t guid;
	void* ptr;
};