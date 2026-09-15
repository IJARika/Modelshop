#pragma once

#include <model/studio/studio.h>

//=============
// STUDIO MODEL
//=============


// Apex Legends, version '54'

// note: unlessed explictly stated the season represents both the season, and mid season update (ex: v12 on s6 means s6+s6.1)
// todo: finish changing these to build numbers, remove above note once done
/*
MDL VERSION CHANGES:
V8 (R5pc_r5launch_J1557_CL387233_2019_01_28_07_43_PM to s1): RELEASE VERSION
V9 (s2 to s3): vtx, vvd, vvc, vvw combined and shipped as hardware data
V10 (s3 to s4): edges change in physics file
V11 (s5): not known
V12 (s6): changes to the bvh and the addition of new data to it
V12.1 (s7 to R5pc_r5-80_J197_CL828437_2021_02_16_17_49): ... do later
V12.2 (R5pc_r5-81_J94_CL862269_2021_03_01_16_25 to R5pc_r5-81_J289_CL979447_2021_04_14_11_19): int added after 'surfacepropLookup' in studiohdr_t
v12.3 (R5pc_r5-90_J135_CL1006802_2021_04_23_20_19 to R5pc_r5-100_J267_CL1609239_2021_08_24_20_00): int added after 'type' in mstudioevent_t
V12.4 (R5pc_r5-101_J60_CL1667383_2021_09_03_19_24 to R5pc_r5-101_J217_CL1813043_2021_10_13_11_53): two ints added to the end of studiohdr_t
V12.5 (R5pc_r5-110_J146_CL1859587_2021_10_26_03_27 to s11): one int added to the end of studiohdr_t
V13 (s11.1 to s12): pak header added a min and max bounds vector, add support for (unknown 48 bit) packed positions
V13.1 (s12 some gaps): int added at the end of mstudiomodel_t (likely uv3)
V14 (s13.1 some gaps): blendshapes added
V14.1 (s14): ... do later... something with node offsets...
V15 (s14.1): bodyparts gains two extra vars
V16 (R5pc_r5-150_J26_CL3438947_2022_10_27_16_21 to s19): format completely redone to be more efficient and save space, most structs changed
V17 (s19.1 to s23): unknown changes, four bytes added to end of header
V18 (s23.1 to R5pc_r5-250_J41_CL9194250_EX9275033_2025_06_05_11_52): nointerpframes added, adjusts how the float value 's' is calculated
V19 (R5pc_r5-251_J15_CL9347583_2025_06_13_15_48 to R5pc_r5-260_J40_CL9779662_2025_08_19_16_52): added datapoint animations, model version changed but not sequence
V19.1 (R5pc_r5-261_J15_CL9878594_2025_09_05_15_37 to R5pc_r5-281_J49_CL10880617_2026_04_07_15_27): animindex removed from mstudioanimdesc_t, as a result any non external animation tracks are stored in a new asset, 'asqd' which stores shared anim data.
v19.2 (R5pc_r5-290_J29_CL10970277_2026_04_24_15_33 to R5pc_r5-291_J84_CL11262393_2026_07_02_15_15): adds support for 1024 bones, hardware data weight structs adjusted, fields added to studiohdr for per lod bonestates
v19.3 (R5pc_r5-300_J23_CL11375875_2026_07_29_15_44 to retail): see animseq v13
*/

/*
ANIMRIG VERSION CHANGES:
V4 (R5pc_r5launch_J1557_CL387233_2019_01_28_07_43_PM to s14): RELEASE VERSION
V5 (R5pc_r5-150_J26_CL3438947_2022_10_27_16_21 to s18): header updated + model overhaul
V6 (s19 to R5pc_r5-250_J41_CL9194250_EX9275033_2025_06_05_11_52): unknown
V7 (R5pc_r5-251_J15_CL9347583_2025_06_13_15_48 to retail): added datapoint animations, animrig version changed but not sequence
*/

/*
ANIMSEQ VERSION CHANGES:
V7 (R5pc_r5launch_J1557_CL387233_2019_01_28_07_43_PM to s6): RELEASE VERSION
V7.1 (s7 to R5pc_r5-80_J197_CL828437_2021_02_16_17_49): adds external data, perm and streamed
V8 (R5pc_r5-81_J94_CL862269_2021_03_01_16_25 to R5pc_r5-81_J289_CL979447_2021_04_14_11_19): adds effect guids for lookup
V10 (R5pc_r5-90_J135_CL1006802_2021_04_23_20_19 to s14): event struct has a new var
V11 (R5pc_r5-150_J26_CL3438947_2022_10_27_16_21 to s23): model overhaul
V12 (s23.1 to R5pc_r5-260_J40_CL9779662_2025_08_19_16_52): nointerpframes added, adjusts how the float value 's' is calculated
V12.1 (R5pc_r5-261_J15_CL9878594_2025_09_05_15_37 to R5pc_r5-291_J84_CL11262393_2026_07_02_15_15): animindex removed from mstudioanimdesc_t, as a result any non external animation tracks are stored in a new asset, 'asqd' which stores shared anim data.
v13 (R5pc_r5-300_J23_CL11375875_2026_07_29_15_44 to retail): bone flag array uses six bits instead of four, for additional (unknown) flags
*/

#pragma pack(push, 4) // required for quite a few structs
namespace r5
{
	// note: rrigs do not have a checksum
	// note: many things that are normally written on load are saved in file, for example 'surfacepropLookup'
	
	//
	// Model UI Panels
	//

	struct UIPanelBounds_s
	{
		// it is utilized like this in code:
		// first four floats is the x axis, second four is y axis
		
		float x[4];
		float y[4];
	};

	struct UIPanelIndices_s
	{
		short vertexIndex[3];
	};
	static_assert(sizeof(UIPanelIndices_s) == 0x6);

	struct UIPanelVertex_s
	{
		int parent; // index into parent array, in most cases 0.
		Vector pos; // simple xyz pos local to bone.
	};

	constexpr size_t maxUIPanelVerts = 2048;
	constexpr size_t maxUIPanelFaces = 512;

	struct UIPanelMesh_s
	{
		uint16_t parentCount;	// limit of two bones per mesh  
		uint16_t vertexCount;	// number of verts
		uint16_t faceCount;	// number of faces
		uint16_t windingCount; // number of faces with clockwise winding, or the index of faces with counterclockwise winding
		inline const bool IsCCW(const int i) const { return i >= windingCount; }

		int parentOffset;	// this gets padding out front of it to even off the struct
		int vertexOffset;	// offset into vertex data
		int bitfieldOffset;	// checked per face, but only if there is more than eight faces, affects how bounds(?) are laid out
		int indiceOffset;	// offsets into a vertex map for each face
		int boundsOffset;	// offset into bounds section for parsing the vertices

		inline const short* const pParent(const int i) const { return reinterpret_cast<short*>((char*)this + parentOffset) + i; }
		inline const short Parent(const int i) const { return *pParent(i); }
		inline const UIPanelVertex_s* const pVertex(const int i) const { return reinterpret_cast<UIPanelVertex_s*>((char*)this + vertexOffset) + i; }
		inline const UIPanelIndices_s* const pIndices(const int i) const { return reinterpret_cast<UIPanelIndices_s*>((char*)this + indiceOffset) + i; }
		inline const UIPanelBounds_s* const pBounds(const int i) const { return reinterpret_cast<UIPanelBounds_s*>((char*)this + boundsOffset) + i; }
		inline const uint16_t* const pBitfield(const int i) const { return reinterpret_cast<const uint16_t* const>((char*)this + bitfieldOffset) + i; }
		inline const uint16_t Bitfield(const int i) const { return *pBitfield(i); }

		inline const char* const pszPanelName() const { return ((char*)this + sizeof(UIPanelMesh_s)); }
	};

	struct UIPanelHeader_s
	{
		int meshHash; // uses rui hash algo
		int meshOffset; // offset to the actual mesh, aligned to 16 bytes within the mdl
		inline UIPanelMesh_s* const pPanelMesh() const { return reinterpret_cast<UIPanelMesh_s*>((char*)this + meshOffset); }
	};


	//
	// Model BVH Collision
	// 

	// leaf and node data is located in bvh.h (not in this project currently)

	// headers
	struct dsurfaceproperty_t
	{
		short unk_0;
		uint8_t surfacePropId;
		uint8_t contentMaskOffset;
		int surfaceNameOffset;
	};

	struct mstudiocollmodel_t
	{
		int contentMasksIndex;
		int surfacePropsIndex;
		int surfaceNamesIndex;
		int headerCount;

		const dsurfaceproperty_t* const pSurfaceProp(const int idx) const { return reinterpret_cast<const dsurfaceproperty_t* const>((char*)this + surfacePropsIndex) + idx; }
	};

	struct mstudiocollheader_t
	{
		int unk;
		int bvhNodeIndex;
		int vertIndex;
		int bvhLeafIndex;

		float origin[3];
		float scale;
	};


	//
	// Model Bones
	//

	#define JIGGLE_IS_FLEXIBLE				0x01 // JIGGLE_IS_RIGID if not set
	#define JIGGLE_HAS_FLEX					0x02 // (JIGGLE_IS_FLEXIBLE | JIGGLE_IS_RIGID) in old code, see note below
	#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
	#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
	#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
	#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20 // having this flag should mean is_flexible or is_rigid is used, however it's used regardless of if 'JIGGLE_HAS_FLEX', which would skip any code for this, and rigid or flexible
	#define JIGGLE_HAS_BASE_SPRING			0x40
	// JIGGLE_HAS_FLEX: this implementation allows it to be independent of type, despite not being utilized in code (flex/rigid codepaths are skipped if this is not set).
	// JIGGLE_HAS_FLEX: some cases of this not being set while flex is, how does that work?

	// apex changed this a bit, 'is_rigid' cut
	struct mstudiojigglebone_t
	{
		uint8_t flags;
		uint8_t bone; // id of bone, might be single byte

		uint8_t pad[2];

		// general params
		float length; // how far from bone base, along bone, is tip
		float tipMass;
		float tipFriction; // friction applied to tip velocity, 0-1

		// flexible params
		float yawStiffness;
		float yawDamping;
		float pitchStiffness;
		float pitchDamping;
		float alongStiffness;
		float alongDamping;

		// angle constraint
		float angleLimit; // maximum deflection of tip in radians

		// yaw constraint
		float minYaw; // in radians
		float maxYaw; // in radians
		float yawFriction;
		float yawBounce;

		// pitch constraint
		float minPitch; // in radians
		float maxPitch; // in radians
		float pitchFriction;
		float pitchBounce;

		// base spring
		float baseMass;
		float baseStiffness;
		float baseDamping;
		float baseMinLeft;
		float baseMaxLeft;
		float baseLeftFriction;
		float baseMinUp;
		float baseMaxUp;
		float baseUpFriction;
		float baseMinForward;
		float baseMaxForward;
		float baseForwardFriction;
	};

	// some of these are likely different in apex
	#define BONE_CALCULATE_MASK			0x1F	// 0x1F -> 0x2F (cannot be confirmed, unused)
	#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
	#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
	#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
	#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
	#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.
	#define BONE_IKCHAIN_INFLUENCE		0x20	// bone is influenced by IK chains, added in V52 (Titanfall 1)

	#define BONE_USED_MASK				0x0007FF00
	#define BONE_USED_BY_ANYTHING		0x0007FF00
	#define BONE_USED_BY_HITBOX			0x00000100	// bone (or child) is used by a hit box
	#define BONE_USED_BY_ATTACHMENT		0x00000200	// bone (or child) is used by an attachment point
	#define BONE_USED_BY_VERTEX_MASK	0x0003FC00
	#define BONE_USED_BY_VERTEX_LOD0	0x00000400	// bone (or child) is used by the toplevel model via skinned vertex
	#define BONE_USED_BY_VERTEX_LOD1	0x00000800	
	#define BONE_USED_BY_VERTEX_LOD2	0x00001000  
	#define BONE_USED_BY_VERTEX_LOD3	0x00002000
	#define BONE_USED_BY_VERTEX_LOD4	0x00004000
	#define BONE_USED_BY_VERTEX_LOD5	0x00008000
	#define BONE_USED_BY_VERTEX_LOD6	0x00010000
	#define BONE_USED_BY_VERTEX_LOD7	0x00020000
	#define BONE_USED_BY_BONE_MERGE		0x00040000	// bone is available for bone merge to occur against it
	#define BONE_FLAG_UNK_80000			0x00080000	// where?

	#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
	#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

	#define BONE_TYPE_MASK				0x00F00000
	#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

	#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Vector48
	#define BONE_HAS_SAVEFRAME_ROT64	0x00400000	// Quaternion64
	#define BONE_HAS_SAVEFRAME_ROT32	0x00800000	// Quaternion32
	#define BONE_FLAG_UNK_1000000		0x01000000	// where?

	struct mstudiobone_t
	{
		int sznameindex;
		inline const char* const pszName() const { return ((char*)this + sznameindex); }

		int parent; // parent bone
		int bonecontroller[6]; // bone controller index, -1 == none

		// default values
		Vector pos; // base bone position
		Quaternion quat;
		RadianEuler rot; // base bone rotation
		Vector scale; // base bone scale

		matrix3x4_t poseToBone;
		Quaternion qAlignment;

		int flags;
		int proctype;
		int procindex; // procedural rule offset
		inline const void* const pProcedure() const { return procindex ? reinterpret_cast<const char* const>(this) + procindex : nullptr; }

		int physicsbone; // index into physically simulated bone
		// from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
		int surfacepropidx; // index into string tablefor property name
		inline const char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

		int contents; // See BSPFlags.h for the contents flags

		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		int unk_B0;

		int collisionIndex; // index into sections of collision, phy, bvh, etc. needs confirming
	};

	// this struct is the same in r1 and r2
	struct mstudiolinearbone_t
	{
		int numbones;

		int flagsindex;
		inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }
		inline int flags(int i) const { return *pFlags(i); }

		int	parentindex;
		inline int* pParent(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<int*>((char*)this + parentindex) + i;
		}

		int	posindex;
		inline const Vector* pPos(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Vector*>((char*)this + posindex) + i;
		}

		int quatindex;
		inline const Quaternion* pQuat(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + quatindex) + i;
		}

		int rotindex;
		inline const RadianEuler* pRot(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<RadianEuler*>((char*)this + rotindex) + i;
		}

		int posetoboneindex;
		inline const matrix3x4_t* pPoseToBone(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<matrix3x4_t*>((char*)this + posetoboneindex) + i;
		}
	};

	// this struct is unchanged from p2
	struct mstudiosrcbonetransform_t
	{
		int sznameindex; // no name in versions 16 and up
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		matrix3x4_t	pretransform;
		matrix3x4_t	posttransform;
	};

	#define	ATTACHMENT_FLAG_WORLD_ALIGN 0x10000

	struct mstudioattachment_t
	{
		int sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }
		int flags;

		int localbone; // parent bone

		matrix3x4_t localmatrix; // attachment point, why did I rename this ?
	};

	struct mstudiobbox_t
	{
		int bone;
		int group; // intersection group
		Vector bbmin; // bounding box
		Vector bbmax;
		int szhitboxnameindex; // offset to the name of the hitbox.
		inline const char* const pszHitboxName() const
		{
			if (szhitboxnameindex == 0)
				return "";

			return reinterpret_cast<const char* const>(this) + szhitboxnameindex;
		}

		int forceCritPoint;	// value of one causes this to act as if it was group of 'head', may either be crit override or group override. mayhaps aligned boolean, look into this
		int hitdataGroupOffset;	// hit_data group in keyvalues this hitbox uses.
		inline const char* const pszHitDataGroup() const { return reinterpret_cast<const char* const>(this) + hitdataGroupOffset; }
	};

	// this struct is the same in r1, r2, and early r5, and unchanged from p2
	struct mstudiohitboxset_t
	{
		int	sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		int	numhitboxes;
		int	hitboxindex;
		inline const mstudiobbox_t* const pHitbox(const int i) const { return reinterpret_cast<const mstudiobbox_t* const>((char*)this + hitboxindex) + i; }
	};


	//
	// Model IK Info
	//

	struct mstudioiklock_t
	{
		int chain;
		float flPosWeight;
		float flLocalQWeight;
		int flags;
	};

	struct mstudioiklink_t
	{
		int bone;
		Vector kneeDir;	// ideal bending direction (per link, if applicable)
	};

	struct mstudioikchain_t
	{
		int sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		int	linktype;
		int	numlinks;
		int	linkindex;
		const mstudioiklink_t* const pLink(int i) const { return reinterpret_cast<const mstudioiklink_t* const>((char*)this + linkindex) + i; }

		float unk_10;	// tried, and tried to find what this does, but it doesn't seem to get hit in any normal IK code paths
						// however, in Apex Legends this value is 0.866 which happens to be cos(30) (https://github.com/NicolasDe/AlienSwarm/blob/master/src/public/studio.h#L2173)
						// and in Titanfall 2 it's set to 0.707 or alternatively cos(45).
						// TLDR: likely set in QC $ikchain command from a degrees entry, or set with a default value in studiomdl when not defined.
	};

	struct mstudiocompressedikerror_t
	{
		float scale[6]; // these values are the same as what posscale (if it was used) and rotscale are.
		int sectionframes; // frames per section, may not match animdesc
	};

	#define IK_SELF 1
	#define IK_WORLD 2
	#define IK_GROUND 3
	#define IK_RELEASE 4
	#define IK_ATTACHMENT 5
	#define IK_UNLATCH 6

	struct mstudioikrule_t
	{
		int index;
		int type;
		int chain;
		int bone; // gets it from ikchain now pretty sure

		int slot; // iktarget slot. Usually same as chain.
		float height;
		float radius;
		float floor;
		Vector pos;
		Quaternion q;

		// apex does this oddly
		mstudiocompressedikerror_t compressedikerror;
		int compressedikerrorindex;

		int iStart;
		int ikerrorindex;

		float start; // beginning of influence
		float peak; // start of full influence
		float tail; // end of full influence
		float end; // end of all influence

		float contact; // frame footstep makes ground concact
		float drop; // how far down the foot should drop when reaching for IK
		float top; // top of the foot box

		int szattachmentindex; // name of world attachment
		inline const char* const pszAttachment() const { return szattachmentindex ? reinterpret_cast<const char* const>(this) + szattachmentindex : nullptr; }

		float endHeight; // new in v52   
	};


	//
	// Model Animation
	// 

	union mstudioanimvalue_t
	{
		struct
		{
			uint8_t	valid;
			uint8_t	total; // total frames for this silly guy?
		} num;
		short value;
	};

	enum ValuePtrFlags_t : uint8_t
	{
		STUDIO_ANIMPTR_Z = 0x01,
		STUDIO_ANIMPTR_Y = 0x02,
		STUDIO_ANIMPTR_X = 0x04,
	};

	// to get mstudioanim_value_t for each axis:
	// if 1 flag, only read from offset
	// if 2 flags (e.g. y,z):
	// y @ offset;
	// z @ offset + (idx1 * sizeof(mstudioanimvalue_t));

	// if 3 flags:
	// x @ offset;
	// y @ offset + (idx1 * sizeof(mstudioanimvalue_t));
	// z @ offset + (idx2 * sizeof(mstudioanimvalue_t));
	struct mstudioanim_valueptr_t
	{
		uint16_t offset : 13;
		uint16_t flags : 3;
		uint8_t axisIdx1; // these two are definitely unsigned
		uint8_t axisIdx2;

		inline mstudioanimvalue_t* pAnimvalue() const { return reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset); }
	};
	static_assert(sizeof(mstudioanim_valueptr_t) == 0x4);

	// flags for the per bone animation headers
	// "mstudioanim_valueptr_t" indicates it has a set of offsets into anim tracks
	enum RleFlags_t : uint8_t
	{
		STUDIO_ANIM_ANIMSCALE = 0x01, // mstudioanim_valueptr_t
		STUDIO_ANIM_ANIMROT = 0x02, // mstudioanim_valueptr_t
		STUDIO_ANIM_ANIMPOS = 0x04, // mstudioanim_valueptr_t
	};

	// flags for the per bone array, in 4 bit sections (two sets of flags per char), aligned to two chars
	enum RleBoneFlags_t : uint8_t
	{
		STUDIO_ANIM_POS = 0x1,	// bone has pos values
		STUDIO_ANIM_ROT = 0x2,	// bone has rot values
		STUDIO_ANIM_SCALE = 0x4,	// bone has scale values
		STUDIO_ANIM_UNK8 = 0x8,	// used in virtual models for new anim type
		STUDIO_ANIM_UNK10 = 0x10,	// TBD
		STUDIO_ANIM_UNK20 = 0x20,	// TBD

		STUDIO_ANIM_DATA = (STUDIO_ANIM_POS | STUDIO_ANIM_ROT | STUDIO_ANIM_SCALE), // bone has animation data
		STUDIO_ANIM_MASK_RELEASE = (STUDIO_ANIM_POS | STUDIO_ANIM_ROT | STUDIO_ANIM_SCALE | STUDIO_ANIM_UNK8),
		STUDIO_ANIM_MASK_RETAIL = (STUDIO_ANIM_POS | STUDIO_ANIM_ROT | STUDIO_ANIM_SCALE | STUDIO_ANIM_UNK8 | STUDIO_ANIM_UNK10 | STUDIO_ANIM_UNK20),
	};

	// from release to season 29
	#define ANIM_BONEFLAG_BITS_4			4 // a nibble even
	#define ANIM_BONEFLAG_SIZE_4(count)		(IALIGN2((ANIM_BONEFLAG_BITS_4 * count + 7) / 8)) // size in bytes of the bone flag array. pads 7 bits for truncation, then align to 2 bytes
	#define ANIM_BONEFLAG_SHIFT_4(idx)		(ANIM_BONEFLAG_BITS_4 * (idx % 2)) // return four (bits) if this index is odd, as there are four bits per bone
	#define ANIM_BONEFLAGS_FLAG_4(ptr, idx)	(static_cast<uint8_t>(ptr[idx / 2] >> ANIM_BONEFLAG_SHIFT_4(idx)) & r5::RleBoneFlags_t::STUDIO_ANIM_MASK_RELEASE) // get byte offset, then shift if needed, mask to four bits

	// starting in season 30 we have six bits for bone flags
	#define ANIM_BONEFLAG_BITS_6			6 // a morsel ?
	#define ANIM_BONEFLAG_SIZE_6(count)		(IALIGN2((ANIM_BONEFLAG_BITS_6 * count + 7) / 8))	// size in bytes of the bone flag array. pads 7 bits for truncation, then align to 2 bytes !!! TBD if this works the same !!!
	#define ANIM_BONEFLAG_SHIFT_6(idx)		(ANIM_BONEFLAG_BITS_6 * (idx % 4))					// return four (bits) if this index is odd, as there are four bits per bone
		//#define ANIM_BONEFLAGS_FLAG_6(ptr, idx)	(static_cast<uint8_t>(*reinterpret_cast<const uint32_t* const>(ptr + ((idx / 4) * 3)) >> ANIM_BONEFLAG_SHIFT_6(idx)) & r5::RleBoneFlags_t::STUDIO_ANIM_MASK_RETAIL) // get byte offset, then shift if needed, mask to four bits
	#define ANIM_BONEFLAGS_FLAG_6(ptr, idx, bits, mask)	(((static_cast<uint8_t>(ptr[bits >> 3]) | (static_cast<uint8_t>(ptr[(bits >> 3) + 1]) << 8)) >> (bits & 7)) & mask) // get byte offset, then shift if needed, mask to four bits

	#define ANIM_BONEFLAG_SIZE(count, width) (width == ANIM_BONEFLAG_BITS_6 ? ANIM_BONEFLAG_SIZE_6(count) : ANIM_BONEFLAG_SIZE_4(count)) 
	#define ANIM_BONEFLAGS_FLAG(ptr, idx, width, bits) (width == ANIM_BONEFLAG_BITS_6 ? ANIM_BONEFLAGS_FLAG_6(ptr, idx, bits, r5::RleBoneFlags_t::STUDIO_ANIM_MASK_RETAIL) : ANIM_BONEFLAGS_FLAG_4(ptr, idx)) 

	struct mstudio_rle_anim_t
	{
	public:
		uint16_t size : 13; // total size of all animation data, not next offset because even the last one has it
		uint16_t flags : 3;

		inline char* pData() const { return ((char*)this + sizeof(mstudio_rle_anim_t)); } // gets start of animation data

		// we have to input the bone flags so we can get the proper offset of each data, pos is always first so it's fine
		// valid for animating data only
		inline mstudioanim_valueptr_t* pRotV(const char boneFlags) const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + PosSize(boneFlags)); } // returns rot as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pPosV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 4); } // returns pos as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pScaleV(const char boneFlags) const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + PosSize(boneFlags) + RotSize(boneFlags)); } // returns scale as mstudioanim_valueptr_t

		// valid if animation unvaring over timeline
		inline Quaternion64* pQuat64(const char boneFlags) const { return reinterpret_cast<Quaternion64*>(pData() + PosSize(boneFlags)); } // returns rot as static Quaternion64
		inline Vector48* pPos() const { return reinterpret_cast<Vector48*>(pData()); } // returns pos as static Vector48
		inline Vector48* pScale(const char boneFlags) const { return reinterpret_cast<Vector48*>(pData() + PosSize(boneFlags) + RotSize(boneFlags)); } // returns scale as static Vector48

		inline const float PosScale() const { return *reinterpret_cast<float*>(pData()); } // pos scale

		inline int Size() const { return static_cast<int>(size); } // size of all data including RLE header

		// points to next bone in the list
		inline mstudio_rle_anim_t* pNext() const { return reinterpret_cast<mstudio_rle_anim_t*>((char*)this + Size()); } // can't return nullptr

	private:
		inline const int PosSize(const char boneFlags) const { return (flags & RleFlags_t::STUDIO_ANIM_ANIMPOS ? 8 : 6) * (boneFlags & RleBoneFlags_t::STUDIO_ANIM_POS); }
		inline const int RotSize(const char boneFlags) const { return (flags & RleFlags_t::STUDIO_ANIM_ANIMROT ? 4 : 8) * (boneFlags & RleBoneFlags_t::STUDIO_ANIM_ROT) >> 1; } // shift one bit for half size

	};
	static_assert(sizeof(mstudio_rle_anim_t) == 0x2);

	// supported up until at least season 17.1 (latest I have looked at), struct is identical to normal source.
	struct mstudioikerror_t
	{
		Vector		pos;
		Quaternion	q;
	};

	#define STUDIO_X		0x00000001
	#define STUDIO_Y		0x00000002	
	#define STUDIO_Z		0x00000004
	#define STUDIO_XR		0x00000008
	#define STUDIO_YR		0x00000010
	#define STUDIO_ZR		0x00000020

	#define STUDIO_LX		0x00000040
	#define STUDIO_LY		0x00000080
	#define STUDIO_LZ		0x00000100
	#define STUDIO_LXR		0x00000200
	#define STUDIO_LYR		0x00000400
	#define STUDIO_LZR		0x00000800

	#define STUDIO_LINEAR			0x00001000
	#define STUDIO_QUADRATIC_MOTION 0x00002000 // tucked away in studiomdl.h

	#define STUDIO_TYPES	0x0003FFFF
	#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance

	struct mstudiomovement_t
	{
		int		endframe;
		int		motionflags;
		float	v0;			// velocity at start of block
		float	v1;			// velocity at end of block
		float	angle;		// YAW rotation at end of this blocks movement
		Vector	vector;		// movement vector relative to this blocks initial angle
		Vector	position;	// relative to start of animation???
	};

	// new in Titanfall 1
	// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
	// pos_x, pos_y, pos_z, yaw
	struct mstudioframemovement_t
	{
		float scale[4];
		int sectionframes;

		inline const int SectionCount(const int numframes) const { return (numframes - 1) / sectionframes + 1; } // this might be wrong :(
		inline int SectionIndex(const int frame) const { return frame / sectionframes; } // get the section index for this frame
		inline int SectionOffset(const int frame) const { return reinterpret_cast<int*>((char*)this + sizeof(mstudioframemovement_t))[SectionIndex(frame)]; } // offset is int
		inline int SectionOffsetRetail(const int frame) const { return FIX_OFFSET(reinterpret_cast<uint16_t*>((char*)this + sizeof(mstudioframemovement_t))[SectionIndex(frame)]); } // offset is uint16_t
		inline uint16_t* pSection(const int frame) const { return reinterpret_cast<uint16_t*>((char*)this + SectionOffset(frame)); }
		inline uint16_t* pSectionRetail(const int frame) const { return reinterpret_cast<uint16_t*>((char*)this + SectionOffsetRetail(frame)); }
		//inline uint16_t* pSection(const int frame, const bool retail) const { return reinterpret_cast<uint16_t*>((char*)this + (retail ? SectionOffset_V16(frame) : SectionOffset(frame))); }

		inline mstudioanimvalue_t* pAnimvalue(const int i, const uint16_t* section) const { return (section[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)section + section[i]) : nullptr; }

		inline const uint8_t* const pDatapointTrack() const
		{
			constexpr int baseOffset = sizeof(mstudioframemovement_t) + sizeof(uint16_t);
			return reinterpret_cast<const uint8_t* const>(this) + baseOffset;
		}
	};

	struct mstudioanimsections_t
	{
		int animindex;
	};

	// sequence and autolayer flags
	#define STUDIO_LOOPING		0x0001		// ending frame should be the same as the starting frame
	#define STUDIO_SNAP			0x0002		// do not interpolate between previous animation and this one
	#define STUDIO_DELTA		0x0004		// this sequence "adds" to the base sequences, not slerp blends
	#define STUDIO_AUTOPLAY		0x0008		// temporary flag that forces the sequence to always play
	#define STUDIO_POST			0x0010		// 
	#define STUDIO_ALLZEROS		0x0020		// this animation/sequence has no real animation data
	#define STUDIO_ANIM_UNK40	0x0040		// suppgest ?
	#define STUDIO_CYCLEPOSE	0x0080		// cycle index is taken from a pose parameter index
	#define STUDIO_REALTIME		0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
	#define STUDIO_LOCAL		0x0200		// sequence has a local context sequence
	#define STUDIO_HIDDEN		0x0400		// don't show in default selection views
	#define STUDIO_OVERRIDE		0x0800		// a forward declared sequence (empty)
	#define STUDIO_ACTIVITY		0x1000		// Has been updated at runtime to activity index
	#define STUDIO_EVENT		0x2000		// Has been updated at runtime to event index on server
	#define STUDIO_WORLD		0x4000		// sequence blends in worldspace
	#define STUDIO_NOFORCELOOP		0x8000	// do not force the animation loop
	#define STUDIO_EVENT_CLIENT		0x10000	// Has been updated at runtime to event index on client
	#define STUDIO_HAS_SCALE		0x20000 // only appears on anims with scale, used for quick lookup in ScaleBones, only reason this should be check is if there has been scale data parsed in, otherwise it is pointless (see ScaleBones checking if scale is 1.0f). note: in r5 this is only set on sequence
	#define STUDIO_HAS_ANIM			0x20000 // if not set there is no useable anim data
	#define STUDIO_FRAMEMOVEMENT    0x40000 // framemovements are only read if this flag is present
	#define STUDIO_SINGLE_FRAME		0x80000 // this animation/sequence only has one frame of animation data
	#define STUDIO_ANIM_UNK100000	0x100000	// reactive animations? seemingly only used on sequences for reactive skins, speicifcally the idle sequence
	#define STUDIO_DATAPOINTANIM	0x200000	// animation uses a packing method which adjusts off of framepoints

	struct mstudioanimdesc_t
	{
		int baseptr;

		// in apex the name seems to be generated when baking, it follows a format: 'name__cmd_hash'
		// where:
		// name is the name of the dmx file,
		// cmd is the qc commands on the $animation qc line
		// hash is an unknown number that trails it all
		int sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		float fps; // frames per second	
		int flags; // looping/non-looping flags

		int numframes;

		// piecewise movement
		int nummovements;
		int movementindex;
		inline const mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<const mstudiomovement_t* const>((char*)this + movementindex) + i; };

		int framemovementindex; // new in v52
		inline const mstudioframemovement_t* const pFrameMovement() const { return reinterpret_cast<const mstudioframemovement_t* const>((char*)this + framemovementindex); }

		int animindex; // non-zero when anim data isn't in sections
		char* pAnimdata(int* piFrame) const; // data array, starting with per bone flags
		mstudio_rle_anim_t* pAnim(int* piFrame, const int boneCount) const; // returns pointer to data and new frame index

		int numikrules;
		int ikruleindex; // non-zero when IK data is stored in the mdl
		inline const mstudioikrule_t* const pIKRule(const int i) const { return reinterpret_cast<const mstudioikrule_t* const>((char*)this + ikruleindex) + i; }

		int sectionindex;
		int sectionframes; // number of frames used in each fast lookup section, zero if not used
		inline const mstudioanimsections_t* pSection(const int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }
		// numsections = ((numframes - 1) / sectionframes) + 2
	};
	static_assert(sizeof(mstudioanimdesc_t) == 0x34);

	#define NEW_EVENT_STYLE ( 1 << 10 )

	struct mstudioevent_t
	{
		float	cycle;
		int		event;
		int		type;
		char	options[256];

		int		szeventindex;
		inline const char* const pszEvent() const { return reinterpret_cast<const char* const>(this) + szeventindex; }
	};

	// autolayer flags
	//							0x0001
	//							0x0002
	//							0x0004
	//							0x0008
	#define STUDIO_AL_POST		0x0010		// 
	//							0x0020
	#define STUDIO_AL_SPLINE	0x0040		// convert layer ramp in/out curve is a spline instead of linear
	#define STUDIO_AL_XFADE		0x0080		// pre-bias the ramp curve to compense for a non-1 weight, assuming a second layer is also going to accumulate
	//							0x0100
	#define STUDIO_AL_NOBLEND	0x0200		// animation always blends at 1.0 (ignores weight)
	//							0x0400
	//							0x0800
	#define STUDIO_AL_LOCAL		0x1000		// layer is a local context sequence
	#define STUDIO_AL_2000		0x2000		// skips parsing in AddSequenceLayer and AddLocalLayer if set
	#define STUDIO_AL_POSE		0x4000		// layer blends using a pose parameter instead of parent cycle
	#define STUDIO_AL_REALTIME	0x8000		// treats the layer sequence as if it uses STUDIO_REALTIME (sub_1401D9AD0 in R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM)

	struct mstudioautolayer_t
	{
		AssetGuid_t sequence; // hashed aseq guid asset, this needs to have a guid descriptor in rpak, converted to pointer on load

		int iPose;

		int flags;
		float start;	// beginning of influence
		float peak;	// start of full influence
		float tail;	// end of full influence
		float end;	// end of all influence
	};

	// todo: check for iklock struct even though it's never used

	struct mstudioactivitymodifier_t
	{
		int sznameindex;
		bool negate; // negate all other activity modifiers when this one is active?

		inline const char* const pszName() const { return (reinterpret_cast<const char* const>(this) + sznameindex); }
	};
	static_assert(sizeof(mstudioactivitymodifier_t) == 0x8);

	// weight fixups, mostly for procbones, uses linearprocbones
	// might not be correct, does some weird stuff
	// 01401D9F80
	struct mstudioseqweightfixup_t
	{
		// generally 0-1
		float unk_0; // used to scale the result

		int bone; // bone idx

		// used to adjust cycle
		float unk_8;
		float unk_C;
		float unk_10;
		float unk_14;
	};

	struct mstudioseqdesc_t
	{
		int baseptr;

		int	szlabelindex;
		inline const char* const pszLabel() const { return reinterpret_cast<const char* const>(this) + szlabelindex; }

		int szactivitynameindex;
		inline const char* const pszActivityName() const { return reinterpret_cast<const char* const>(this) + szactivitynameindex; }

		int flags; // looping/non-looping flags

		int activity; // initialized at loadtime to game DLL values
		int actweight;

		int numevents;
		int eventindex;
		template<typename T> inline const T* const pEvent(const int i) const { return reinterpret_cast<T*>((char*)this + eventindex) + i; } // two versions can be returned from here!

		Vector bbmin; // per sequence bounding box
		Vector bbmax;

		int numblends;

		// Index into array of ints which is groupsize[0] x groupsize[1] in length
		int animindexindex;
		const int AnimIndex(const int i) const { return reinterpret_cast<int*>((char*)this + animindexindex)[i]; }
		const int AnimCount() const { return  groupsize[0] * groupsize[1]; }
		//inline const mstudioanimdesc_t* const pAnimDesc(const int i) const { return reinterpret_cast<const mstudioanimdesc_t* const>((char*)this + AnimIndex(i)); }
		template<typename T> inline const T* const pAnimDesc(const int i) const { return reinterpret_cast<const T* const>((char*)this + AnimIndex(i)); }

		int movementindex;	// unused as of v49
		int groupsize[2];	// width x height of blends
		int paramindex[2];	// X, Y, Z, XR, YR, ZR
		float paramstart[2];// local (0..1) starting value
		float paramend[2];	// local (0..1) ending value
		int paramparent;	// unused as of v49

		float fadeintime; // ideal cross fate in time (0.2 default)
		float fadeouttime; // ideal cross fade out time (0.2 default)

		int localentrynode; // transition node at entry
		int localexitnode; // transition node at exit
		int nodeflags; // transition rules

		float entryphase; // used to match entry gait
		float exitphase; // used to match exit gait

		float lastframe; // frame that should generation EndOfSequence

		int nextseq; // auto advancing sequences
		int pose; // index of delta animation between end and nextseq

		int numikrules;

		int numautolayers;
		int autolayerindex;
		inline const mstudioautolayer_t* const pAutoLayer(const int i) const { return reinterpret_cast<const mstudioautolayer_t* const>((char*)this + autolayerindex) + i; }

		int weightlistindex;
		inline const float* const pBoneweight(const int i) const { return reinterpret_cast<float*>((char*)this + weightlistindex) + i; }
		inline const float weight(const int i) const { return *pBoneweight(i); }

		int posekeyindex;
		inline const float* const pPoseKey(int iParam, int iAnim) const { return reinterpret_cast<float*>((char*)this + posekeyindex) + (iParam * groupsize[0]) + iAnim; }
		inline float poseKey(int iParam, int iAnim) const { return *(pPoseKey(iParam, iAnim)); }

		int numiklocks;
		int iklockindex;
		inline const mstudioiklock_t* const pIKLock(const int i) const { return reinterpret_cast<const mstudioiklock_t* const>((char*)this + iklockindex) + i; }

		// Key values
		int keyvalueindex;
		int keyvaluesize;
		inline const char* const pKeyValues() const { return  keyvaluesize ? (reinterpret_cast<const char* const>(this) + keyvalueindex) : nullptr; }

		int cycleposeindex; // index of pose parameter to use as cycle index

		int activitymodifierindex;
		int numactivitymodifiers;
		inline const mstudioactivitymodifier_t* const pActivityModifier(const int i) const { return reinterpret_cast<mstudioactivitymodifier_t*>((char*)this + activitymodifierindex) + i; }

		int ikResetMask; // mask this ik rule type for reset, can't find the code for this, but it would either prevent reset of this type, or only allow reset of this time. only ever observed as IK_GROUND
		int unk_C4;

		// for adjusting bone weights, mostly used for procbones (jiggle)
		int weightFixupOffset;
		int weightFixupCount;
	};

	struct mstudioposeparamdesc_t
	{
		int		sznameindex;
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }

		int		flags;	// ???? (may contain 'STUDIO_LOOPING' flag if looping is used)
		float	start;	// starting value
		float	end;	// ending value
		float	loop;	// looping range, 0 for no looping, 360 for rotations, etc.
	};

	struct mstudiomodelgroup_t
	{
		int szlabelindex;	// textual name (NOTE: this is never actually set or used anywhere in reSource, or valve source)
		inline const char* const pszLabel() const { return reinterpret_cast<const char* const>(this) + szlabelindex; }

		int sznameindex;	// file name
		inline const char* const pszName() const { return reinterpret_cast<const char* const>(this) + sznameindex; }
	};


	//
	// Model Bodyparts
	// 

	struct mstudiomodel_t;

	// pack here for our silly little unknown pointer
	struct mstudiomesh_t
	{
		int material;

		int modelindex;
		mstudiomodel_t* const pModel() const { return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex); }

		int numvertices; // number of unique vertices/normals/texcoords
		int vertexoffset; // vertex mstudiovertex_t
		// offset by vertexoffset number of verts into vvd vertexes, relative to the models offset

		// Access thin/fat mesh vertex data (only one will return a non-NULL result)

		int deprecated_numflexes; // vertex animation
		int deprecated_flexindex;

		// special codes for material operations
		int deprecated_materialtype;
		int deprecated_materialparam;

		// a unique ordinal for this mesh
		int meshid;

		Vector center;

		mstudio_meshvertexloddata_t vertexloddata;

		void* unk_54; // unknown memory pointer, probably one of the older vertex pointers but moved
	};
	static_assert(sizeof(mstudiomesh_t) == 0x5C);

	struct mstudiomodel_t
	{
		char name[64];

		int unkStringOffset; // goes to bones sometimes
		inline const char* const pszUnknown() const { return reinterpret_cast<const char* const>(this) + unkStringOffset; }

		int type;

		float boundingradius;

		int nummeshes;
		int meshindex;

		inline const mstudiomesh_t* const pMesh(int i) const { return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i; }

		// cache purposes
		int numvertices;	// number of unique vertices/normals/texcoords
		int vertexindex;	// vertex Vector
		// offset by vertexindex number of chars into vvd verts
		int tangentsindex;	// tangents Vector
		// offset by tangentsindex number of chars into vvd tangents

		int numattachments;
		int attachmentindex;

		int deprecated_numeyeballs;
		int deprecated_eyeballindex;

		int pad[4];

		int colorindex; // vertex color
		// offset by colorindex number of chars into vvc vertex colors
		int uv2index;	// vertex second uv map
		// offset by uv2index number of chars into vvc secondary uv map
	};

	struct mstudiobodyparts_t
	{
		int	sznameindex;
		inline const char* const pszName() const { return ((char*)this + sznameindex); }

		int	nummodels;
		int	base;
		int	modelindex; // index into models array

		inline const mstudiomodel_t* const pModel(int i) const { return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i; };
	};

	struct mstudiotexture_t
	{
		int sznameindex;
		inline const char* const pszName() const { return ((char*)this + sznameindex); }

		AssetGuid_t material; // guid/hash of this material
	};
	static_assert(sizeof(mstudiotexture_t) == 0xC);


	//
	// Studio Header
	//

	// This flag is set if no hitbox information was specified
	#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX	0x1

	// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
	// models when we change materials.
	#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP		0x2

	// Use this when there are translucent parts to the model but we're not going to sort it 
	#define STUDIOHDR_FLAGS_FORCE_OPAQUE			0x4

	// previously 'STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS', now handled by materials
	// used on 'loading_mdl' a lot, however this flag existed before ODL so it's probably not directly related
	#define STUDIOHDR_FLAGS_UNK_8					0x8

	// This is set any time the .qc files has $staticprop in it
	// Means there's no bones and no transforms
	#define STUDIOHDR_FLAGS_STATIC_PROP				0x10

	// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
	// models when we change materials.
	#define STUDIOHDR_FLAGS_USES_FB_TEXTURE		    0x20

	// This flag is set by studiomdl.exe if a separate "$shadowlod" entry was present
	//  for the .mdl (the shadow lod is the last entry in the lod list if present)
	#define STUDIOHDR_FLAGS_HASSHADOWLOD			0x40

	// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
	// models when we change materials.
	#define STUDIOHDR_FLAGS_USES_BUMPMAPPING		0x80

	// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
	// instead of overriding them with the default one (necessary for translucent shadows)
	#define STUDIOHDR_FLAGS_USE_SHADOWLOD_MATERIALS	0x100

	// previously 'STUDIOHDR_FLAGS_OBSOLETE', no longer used/exists in titanfall 2
	#define STUDIOHDR_FLAGS_UNUSED_200				0x200

	#define STUDIOHDR_FLAGS_UNK_400					0x400

	// NOTE:  This flag is set at mdl build time
	#define STUDIOHDR_FLAGS_NO_FORCED_FADE			0x800

	// NOTE:  The npc will lengthen the viseme check to always include two phonemes
	#define STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE	0x1000

	// previously 'STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT'
	// models using this in apex don't have it set in titanfall 2, and don't have the header field set either.
	// probably cut in r1/r2 and replaced in r5
	#define STUDIOHDR_FLAGS_UNK_2000				0x2000

	// This flag indicates that the model has extra weights, which allows it to have 3< weights per bone.
	#define STUDIOHDR_FLAGS_USES_EXTRA_BONE_WEIGHTS	0x4000

	// Indicates the studiomdl was built in preview mode
	// replaced in apex?
	#define STUDIOHDR_FLAGS_BUILT_IN_PREVIEW_MODE	0x8000

	// Ambient boost (runtime flag)
	#define STUDIOHDR_FLAGS_AMBIENT_BOOST			0x10000

	// Don't cast shadows from this model (useful on first-person models)
	#define STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS		0x20000

	// alpha textures should cast shadows in vrad on this model (ONLY prop_static!)
	#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS	0x40000

	// if set we do not check local sequences (only hit if no virtual model). flag check replaces checking include models, arigs?
	// Model uses external sequences (studiohdr_t::pSeqDesc r5_250)
	#define STUDIOHDR_FLAGS_USES_EXTERNAL_SEQUENCES	0x80000

	// flagged on load to indicate no animation events on this model
	// might be a different thing on v54
	#define STUDIOHDR_FLAGS_NO_ANIM_EVENTS			0x100000

	// If flag is set then studiohdr_t.flVertAnimFixedPointScale contains the
	// scale value for fixed point vert anim data, if not set then the
	// scale value is the default of 1.0 / 4096.0.  Regardless use
	// studiohdr_t::VertAnimFixedPointScale() to always retrieve the scale value
	#define STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE	0x200000

	#define STUDIOHDR_FLAGS_UNK_800000					0x800000

	// If this flag is present the model has vertex color, and by extension a VVC (IDVC) file.
	#define STUDIOHDR_FLAGS_USES_VERTEX_COLOR	        0x1000000

	// If this flag is present the model has a secondary UV layer.
	#define STUDIOHDR_FLAGS_USES_UV2			        0x2000000

	// mdl/dev/material_preview.rmdl
	#define STUDIOHDR_FLAGS_UNK_4000000					0x4000000

	// 'wind' models, texcoord3, quirky data at end of struct
	// probably that extra var added in mstudiomodel_t / the 'extra' vvc data
	#define STUDIOHDR_FLAGS_USES_UV3			        0x8000000

	#define STUDIOHDR_FLAGS_UNK_10000000				0x10000000

	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 54 (0x36,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
		inline char* const pszName() const { return ((char*)this + sznameindex); }
		char name[64]; // The internal name of the model, padding with null chars.
		int length; // Data size of MDL file in chars.

		Vector eyeposition;	// ideal eye position

		Vector illumposition;	// illumination center

		Vector hull_min;		// ideal movement hull size
		Vector hull_max;

		Vector view_bbmin;		// clipping bounding box
		Vector view_bbmax;

		int flags;

		int numbones; // bones
		int boneindex;
		inline const mstudiobone_t* const pBone(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<mstudiobone_t*>((char*)this + boneindex) + i; }

		int numbonecontrollers; // bone controllers
		int bonecontrollerindex;

		int numhitboxsets;
		int hitboxsetindex;
		inline const mstudiohitboxset_t* const pHitboxSet(const int i) const
		{
			assert(i >= 0 && i < numhitboxsets);
			return reinterpret_cast<const mstudiohitboxset_t* const>((char*)this + hitboxsetindex) + i;
		};

		// seemingly unused now, as animations are per sequence
		int numlocalanim; // animations/poses
		int localanimindex; // animation descriptions
		inline const mstudioanimdesc_t* const pAnimdesc(const int i) const { assert(i >= 0 && i < numlocalanim); return reinterpret_cast<mstudioanimdesc_t*>((char*)this + localanimindex) + i; }

		int numlocalseq; // sequences
		int	localseqindex;
		inline const mstudioseqdesc_t* const pSeqdesc(const int i) const { assert(i >= 0 && i < numlocalseq); return reinterpret_cast<mstudioseqdesc_t*>((char*)this + localseqindex) + i; }

		int activitylistversion; // initialization flag - have the sequences been indexed?

		// mstudiotexture_t
		// short rpak path
		// raw textures
		// todo: stops getting written at somepoint before being outright removed, when?
		int materialtypesindex; // index into an array of MaterialShaderType_t type enums for each material used by the model
		int numtextures; // the material limit exceeds 128, probably 256.
		int textureindex;
		inline const MaterialShaderType_t pMatShaderType(int i) const { assert(i >= 0 && i < numtextures); return reinterpret_cast<MaterialShaderType_t*>((char*)this + materialtypesindex)[i]; }
		inline const mstudiotexture_t* const pTexture(int i) const { assert(i >= 0 && i < numtextures); return reinterpret_cast<mstudiotexture_t*>((char*)this + textureindex) + i; }

		// this should always only be one, unless using vmts.
		// raw textures search paths
		int numcdtextures;
		int cdtextureindex;
		inline const char* const pCdtexture(const int i) const { return reinterpret_cast<const char* const>(this) + reinterpret_cast<const int* const>((char*)this + cdtextureindex)[i]; }

		// replaceable textures tables
		int numskinref;
		int numskinfamilies;
		int skinindex;
		inline const int16_t* const pSkinref(const int i) const { return reinterpret_cast<const int16_t* const>((char*)this + skinindex) + i; }
		inline const int16_t* const pSkinFamily(const int i) const { return pSkinref(numskinref * i); };

		int numbodyparts;
		int bodypartindex;
		inline const mstudiobodyparts_t* const pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

		int numlocalattachments;
		int localattachmentindex;
		inline const mstudioattachment_t* const pLocalAttachment(const int i) const { assert(i >= 0 && i < numlocalattachments); return reinterpret_cast<const mstudioattachment_t* const>((char*)this + localattachmentindex) + i; }

		int numlocalnodes;
		int localnodeindex;			// legacy node data
		int localnodenameindex;
		int localNodeUnk;			// used sparsely in r2, unused in apex, removed in v16 rmdl
		int localNodeDataOffset;	// offset into an array of int sized offsets that read into the data for each node (note: bad name, this should be named something different to reflect it being a new node storage type)

		int meshOffset; // hard offset to the start of this models meshes

		// all flex related model vars and structs are stripped in respawn source
		int deprecated_numflexcontrollers;
		int deprecated_flexcontrollerindex;

		int deprecated_numflexrules;
		int deprecated_flexruleindex;

		int numikchains;
		int ikchainindex;
		inline const mstudioikchain_t* const pIKChain(const int i) const { assert(i >= 0 && i < numikchains); return reinterpret_cast<const mstudioikchain_t* const>((char*)this + ikchainindex) + i; }

		// mesh panels for using rui on models, primarily for weapons
		int uiPanelCount;
		int uiPanelOffset;
		inline const UIPanelHeader_s* const pUiPanel(int i) const { assert(i >= 0 && i < uiPanelCount); return reinterpret_cast<UIPanelHeader_s*>((char*)this + uiPanelOffset) + i; }

		int numlocalposeparameters;
		int localposeparamindex;
		inline const mstudioposeparamdesc_t* const pLocalPoseParameter(const int i) const { assert(i >= 0 && i < numlocalposeparameters); return reinterpret_cast<const mstudioposeparamdesc_t* const>((char*)this + localposeparamindex) + i; }

		int surfacepropindex;
		inline const char* const pszSurfaceProp() const { return reinterpret_cast<const char* const>(this) + surfacepropindex; }

		int keyvalueindex;
		int keyvaluesize;
		inline const char* const KeyValueText() const { return reinterpret_cast<const char* const>(this) + keyvalueindex; }

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;
		inline const mstudioiklock_t* const pLocalIKAutoplayLock(const int i) const { assert(i >= 0 && i < numlocalikautoplaylocks); return reinterpret_cast<const mstudioiklock_t* const>((char*)this + localikautoplaylockindex) + i; }

		float mass;
		int contents;

		// unused for packed models
		// technically still functional though I am unsure why you'd want to use it
		int numincludemodels;
		int includemodelindex;
		inline const mstudiomodelgroup_t* const pModelGroup(const int i) const { assert(i >= 0 && i < numincludemodels); return reinterpret_cast<const mstudiomodelgroup_t* const>((char*)this + includemodelindex) + i; }

		int /* mutable void* */ virtualModel;

		int bonetablebynameindex;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		uint8_t constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		uint8_t rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		uint8_t numAllowedRootLODs;

		uint8_t unused;

		float fadeDistance;	// set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
		// player/titan models seem to inherit this value from the first model loaded in menus.
		// works oddly on entities, probably only meant for static props

		float gatherSize;	// what. from r5r struct. no clue what this does, seemingly for early versions of apex bsp but stripped in release apex (season 0)
		// bad name, frustum culling
		// never written on disk, memory tbd

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale; // to be verified
		int surfacepropLookup; // saved in the file

		// this is in most shipped models, probably part of their asset bakery.
		// doesn't actually need to be written pretty sure, only four chars when not present.
		// this is not completely true as some models simply have nothing, such as animation models.
		int sourceFilenameOffset;
		inline const char* const pszSourceFiles() const { return ((char*)this + sourceFilenameOffset); }

		int numsrcbonetransform;
		int srcbonetransformindex;
		inline const mstudiosrcbonetransform_t* SrcBoneTransform(int i) const { return reinterpret_cast<const mstudiosrcbonetransform_t* const>((char*)this + srcbonetransformindex) + i; }

		int	illumpositionattachmentindex;

		int linearboneindex;
		inline mstudiolinearbone_t* const pLinearBones() const { return linearboneindex ? reinterpret_cast<mstudiolinearbone_t*>((char*)this + linearboneindex) : nullptr; }

		// used for adjusting weights in sequences, quick lookup into bones that have procbones, unsure what else uses this.
		int procBoneCount;
		int procBoneOffset; // in order array of procbones and their parent bone indice
		int linearProcBoneOffset; // byte per bone with indices into each bones procbone, 0xff if no procbone is present

		// depreciated as they are removed in 12.1
		int deprecated_m_nBoneFlexDriverCount;
		int deprecated_m_nBoneFlexDriverIndex;

		int deprecated_m_nPerTriAABBIndex;
		int deprecated_m_nPerTriAABBNodeCount;
		int deprecated_m_nPerTriAABBLeafCount;
		int deprecated_m_nPerTriAABBVertCount;

		// always "" or "Titan"
		int unkStringOffset;
		inline const char* const pszUnkString() const { return ((char*)this + unkStringOffset); }

		// offsets into vertex buffer for component files, suzes are per file if course
		// unused besides phy starting in rmdl v9
		int vtxOffset; // VTX
		int vvdOffset; // VVD / IDSV
		int vvcOffset; // VVC / IDCV 
		int phyOffset; // VPHY / IVPS

		int vtxSize;
		int vvdSize;
		int vvcSize;
		int phySize; // still used in models using vg

		// // removed from header in version 12.1
		int deprecated_collisionOffset;			// deprecated_imposterIndex
		int deprecated_staticCollisionCount;	// deprecated_numImposters

		// mostly seen on '_animated' suffixed models
		// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
		int boneFollowerCount;
		int boneFollowerOffset; // index only written when numbones > 1, means whatever func writes this likely checks this (would make sense because bonefollowers need more than one bone to even be useful). maybe only written if phy exists
		inline const int* const pBoneFollowers(int i) const { assert(i >= 0 && i < boneFollowerCount); return reinterpret_cast<int*>((char*)this + boneFollowerOffset) + i; }
		inline const int BoneFollower(int i) const { return *pBoneFollowers(i); }

		// BVH4 size (?)
		Vector bvhMin;
		Vector bvhMax; // seem to be the same as hull size

		int unk_208[3]; // never written on disk, memory tbd

		int bvhOffset; // bvh4 tree

		short bvhUnk[2]; // collision detail for bvh (?)

		// new in apex vertex weight file for verts that have more than three weights
		// vvw is a 'fake' extension name, we do not know the proper name.
		int vvwOffset; // index will come last after other vertex files
		int vvwSize;
	};
}

namespace r5_120
{
	//
	// Model BVH Collision
	// 

	// headers
	struct dsurfacepropertydata_t
	{
		uint8_t surfacePropId1;
		uint8_t surfacePropId2;
		uint16_t flags;
	};

	struct mstudiocollheader_t
	{
		int unk_0;
		int bvhNodeIndex;
		int vertIndex;
		int bvhLeafIndex;

		// [amos]
		// A new index that indexes into a buffer that contains surfacePropId's
		// The number of surfacePropId's is determined by surfacePropCount.
		// surfacePropArrayCount determines how many of these buffers we have,
		// they are the same size but sometimes with different surfacePropId's.
		int surfacePropDataIndex;

		uint8_t surfacePropArrayCount;
		uint8_t surfacePropCount;
		const dsurfacepropertydata_t* const pSurfaceProp(const int idx) const { return reinterpret_cast<const dsurfacepropertydata_t* const>((char*)this + surfacePropDataIndex) + idx; }

		short padding_maybe;

		float origin[3];
		float scale;
	};
}
#pragma pack(pop)
