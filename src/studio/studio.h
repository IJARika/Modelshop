#include <cassert>
#include <cstddef>

#include "../math/vector.h"
#include "../math/compressed_vector.h"
#include "../math/vector4d.h"
#include "../math/vertexcolor.h"

#pragma once


//===========
// Data Types
//===========


struct matrix3x4_t
{	//    c0         c1         c2         c3
	float m00; float m01; float m02; float m03; // r0
	float m10; float m11; float m12; float m13; // r1
	float m20; float m21; float m22; float m23; // r2
};


#define STUDIO_VERSION_TITANFALL	52
#define STUDIO_VERSION_TITANFALL2	53
#define STUDIO_VERSION_APEX_LEGENDS	54

#define MAXSTUDIOSKINS		256		// total textures, actual limit may be higher than 256
#define MAXSTUDIOBONES		256		// total bones actually used

#define MAX_NUM_LODS 8 // consistent across games

#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_EXTRA_BONE_WEIGHTS	16 // for apex legends

#define FILEBUFSIZE (32 * 1024 * 1024)

#define FIX_OFFSET(offset) ((offset & 0xFFFE) << (4 * (offset & 1))) // for rmdl v16


//===================
// STUDIO VERTEX DATA
//===================

#pragma pack(push, 1)
//namespace OptimizedModel
namespace vtx
{
	struct BoneStateChangeHeader_t
	{
		int hardwareID;
		int newBoneID;
	};

	struct Vertex_t
	{
		// these index into the mesh's vert[origMeshVertID]'s bones
		unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
		unsigned char numBones;

		unsigned short origMeshVertID;

		// for sw skinned verts, these are indices into the global list of bones
		// for hw skinned verts, these are hardware bone indices
		char boneID[MAX_NUM_BONES_PER_VERT];
	};

	enum StripHeaderFlags_t
	{
		STRIP_IS_TRILIST = 0x01,
		STRIP_IS_QUADLIST_REG = 0x02,		// Regular sub-d quads
		STRIP_IS_QUADLIST_EXTRA = 0x04		// Extraordinary sub-d quads
	};


	// A strip is a piece of a stripgroup which is divided by bones 
	struct StripHeader_t
	{
		int numIndices;
		int indexOffset;

		int numVerts;
		int vertOffset;

		short numBones;

		unsigned char flags;

		int numBoneStateChanges;
		int boneStateChangeOffset;

		BoneStateChangeHeader_t* pBoneStateChange(int i)
		{
			return reinterpret_cast<BoneStateChangeHeader_t*>((char*)this + boneStateChangeOffset) + i;
		};

		// MDL Version 49 and up only
		int numTopologyIndices;
		int topologyOffset;
	};

	enum StripGroupFlags_t
	{
		STRIPGROUP_IS_HWSKINNED = 0x02,
		STRIPGROUP_IS_DELTA_FLEXED = 0x04,
		STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
	};

	struct StripGroupHeader_t
	{
		// These are the arrays of all verts and indices for this mesh.  strips index into this.
		int numVerts;
		int vertOffset;

		Vertex_t* pVertex(int i)
		{
			return reinterpret_cast<Vertex_t*>((char*)this + vertOffset) + i;
		}

		int numIndices;
		int indexOffset;

		unsigned short* pIndex(int i)
		{
			return reinterpret_cast<unsigned short*>((char*)this + indexOffset) + i;
		}

		int numStrips;
		int stripOffset;

		StripHeader_t* pStrip(int i)
		{
			return reinterpret_cast<StripHeader_t*>((char*)this + stripOffset) + i;
		}

		unsigned char flags;

		// The following fields are only present if MDL version is >=49
		// Points to an array of unsigned shorts (16 bits each)
		int numTopologyIndices;
		int topologyOffset;

		unsigned short* pTopologyIndex(int i)
		{
			return reinterpret_cast<unsigned short*>((char*)this + topologyOffset) + i;
		}
	};

	// likely unused in Respawn games as mouths and eyes are cut.
	enum MeshFlags_t {
		// these are both material properties, and a mesh has a single material.
		MESH_IS_TEETH = 0x01,
		MESH_IS_EYES = 0x02
	};


	struct MeshHeader_t
	{
		int numStripGroups;
		int stripGroupHeaderOffset;

		unsigned char flags; // never read these as eyeballs and mouths are depreciated

		StripGroupHeader_t* pStripGroup(int i)
		{
			return reinterpret_cast<StripGroupHeader_t*>((char*)this + stripGroupHeaderOffset) + i;
		}
	};

	struct ModelLODHeader_t
	{
		//Mesh array
		int numMeshes;
		int meshOffset;

		float switchPoint;

		MeshHeader_t* pMesh(int i)
		{
			return reinterpret_cast<MeshHeader_t*>((char*)this + meshOffset) + i;
		}
	};

	struct ModelHeader_t
	{
		//LOD mesh array
		int numLODs;   //This is also specified in FileHeader_t
		int lodOffset;

		ModelLODHeader_t* pLOD(int i)
		{
			return reinterpret_cast<ModelLODHeader_t*>((char*)this + lodOffset) + i;
		}
	};

	struct BodyPartHeader_t
	{
		// Model array
		int numModels;
		int modelOffset;

		ModelHeader_t* pModel(int i)
		{
			return reinterpret_cast<ModelHeader_t*>((char*)this + modelOffset) + i;
		}
	};

	struct MaterialReplacementHeader_t
	{
		short materialID;
		int replacementMaterialNameOffset;
		char* pMaterialReplacementName()
		{
			return ((char*)this + replacementMaterialNameOffset);
		}
	};

	struct MaterialReplacementListHeader_t
	{
		int numReplacements;
		int replacementOffset;

		MaterialReplacementHeader_t* pMaterialReplacement(int i)
		{
			return reinterpret_cast<MaterialReplacementHeader_t*>((char*)this + replacementOffset) + i;
		}
	};

	struct FileHeader_t
	{
		// file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
		int version;

		// hardware params that affect how the model is to be optimized.
		int vertCacheSize;
		unsigned short maxBonesPerStrip;
		unsigned short maxBonesPerFace;
		int maxBonesPerVert;

		// must match checkSum in the .mdl
		int checkSum;

		int numLODs; // Also specified in ModelHeader_t's and should match

		// Offset to materialReplacementList Array. one of these for each LOD, 8 in total
		int materialReplacementListOffset;

		MaterialReplacementListHeader_t* pMaterialReplacementList(int lodID)
		{
			return reinterpret_cast<MaterialReplacementListHeader_t*>((char*)this + materialReplacementListOffset) + lodID;
		}

		// Defines the size and location of the body part array
		int numBodyParts;
		int bodyPartOffset;

		BodyPartHeader_t* pBodyPart(int i)
		{
			return reinterpret_cast<BodyPartHeader_t*>((char*)this + bodyPartOffset) + i;
		}
	};
}
#pragma pack(pop)

namespace vvd
{
	struct mstudioweightextra_t
	{
		short weight[3]; // value divided by 32767.0

		short pad; // likely alignment

		int extraweightindex; // base index for vvw, add (weightIdx - 3)
	};

	struct mstudioboneweight_t
	{
		union
		{
			float	weight[MAX_NUM_BONES_PER_VERT];
			mstudioweightextra_t weightextra; // only in apex (v54)
		};

		unsigned char bone[MAX_NUM_BONES_PER_VERT]; // set to unsigned so we can read it
		char	numbones;
	};

	struct mstudiovertex_t
	{
		mstudioboneweight_t	m_BoneWeights;
		Vector m_vecPosition;
		Vector m_vecNormal;
		Vector2D m_vecTexCoord;
	};

	struct vertexFileFixup_t
	{
		int		lod;				// used to skip culled root lod
		int		sourceVertexID;		// absolute index from start of vertex/tangent blocks
		int		numVertexes;
	};

	struct vertexFileHeader_t
	{
		int id; // MODEL_VERTEX_FILE_ID
		int version; // MODEL_VERTEX_FILE_VERSION
		int checksum; // same as studiohdr_t, ensures sync

		int numLODs; // num of valid lods
		int numLODVertexes[MAX_NUM_LODS]; // num verts for desired root lod

		int numFixups; // num of vertexFileFixup_t

		int fixupTableStart; // offset from base to fixup table
		int vertexDataStart; // offset from base to vertex block
		int tangentDataStart; // offset from base to tangent block

		const vertexFileFixup_t* GetFixupData(int i) const
		{
			return reinterpret_cast<vertexFileFixup_t*>((char*)this + fixupTableStart) + i;
		}

		const mstudiovertex_t* GetVertexData(int i) const
		{
			return reinterpret_cast<mstudiovertex_t*>((char*)this + vertexDataStart) + i;
		}

		const Vector4D* GetTangentData(int i) const
		{
			return reinterpret_cast<Vector4D*>((char*)this + tangentDataStart) + i;
		}
	};
}

namespace vvc
{
	struct vertexColorFileHeader_t
	{
		int id; // IDCV
		int version; // 1
		int checksum; // same as studiohdr_t, ensures sync

		int numLODs; // num of valid lods
		int numLODVertexes[MAX_NUM_LODS]; // num verts for desired root lod

		int colorDataStart;
		int uv2DataStart;

		const VertexColor_t* GetColorData(int i) const
		{
			return reinterpret_cast<VertexColor_t*>((char*)this + colorDataStart) + i;
		}

		const Vector2D* GetUVData(int i) const
		{
			return reinterpret_cast<Vector2D*>((char*)this + uv2DataStart) + i;
		}
	};
}

namespace vvw
{
	struct mstudioboneweightextra_t
	{
		short	weight; // weight = this / 32767.0
		short   bone;
	};

	struct vertexBoneWeightsExtraFileHeader_t
	{
		int checksum; // same as studiohdr_t, ensures sync
		int version; // should be 1

		int numLODVertexes[MAX_NUM_LODS]; // maybe this but the others don't get filled?

		int weightDataStart; // index into mstudioboneweightextra_t array

		const mstudioboneweightextra_t* GetWeightData(int i) const
		{
			return reinterpret_cast<mstudioboneweightextra_t*>((char*)this + weightDataStart) + i;
		}
	};
}

namespace vg
{
	// rmdl versions 9-12
	namespace rev1
	{
		struct VertexGroupHeader_t
		{
			int id;		// 0x47567430	'0tVG'
			int version;	    // 1
			int unk;	        // Usually 0
			int dataSize;	// Total size of data + header in starpak

			// unsigned char
			__int64 boneStateChangeOffset; // offset to bone remap buffer
			__int64 numBoneStateChanges;  // number of "bone remaps" (size: 1)

			// MeshHeader_t
			__int64 meshOffset;   // offset to mesh buffer
			__int64 numMeshes;    // number of meshes (size: 0x48)

			// unsigned short
			__int64 indexOffset;     // offset to index buffer
			__int64 numIndices;      // number of indices (size: 2 (uint16_t))

			// uses dynamic sized struct, similar to RLE animations
			__int64 vertOffset;    // offset to vertex buffer
			__int64 numVerts;     // number of chars in vertex buffer

			// vvw::mstudioboneweightextra_t
			__int64 extraBoneWeightOffset;   // offset to extended weights buffer
			__int64 extraBoneWeightSize;    // number of chars in extended weights buffer

			// there is one for every LOD mesh
			// i.e, unknownCount == lod.meshCount for all LODs
			__int64 unknownOffset;   // offset to buffer
			__int64 numUnknown;    // count (size: 0x30)

			// vtx::ModelLODHeader_t
			__int64 lodOffset;       // offset to LOD buffer
			__int64 numLODs;        // number of LODs (size: 0x8)

			// vvd::mstudioboneweight_t
			__int64 legacyWeightOffset;	// seems to be an offset into the "external weights" buffer for this mesh
			__int64 numLegacyWeights;   // seems to be the number of "external weights" that this mesh uses

			// vtx::StripHeader_t
			__int64 stripOffset;    // offset to strips buffer
			__int64 numStrips;     // number of strips (size: 0x23)

			__int64 unused[8];
		};

		#define VERTEX_POSITION         0x1
		#define VERTEX_POSITION_PACKED  0x2
		#define VERTEX_COLOR			0x10
		#define VERTEX_WEIGHTS_PACKED   0x5000 // this is definitely two flags
		#define VERTEX_UV2				0x200000000

		struct MeshHeader_t
		{
			__int64 flags;	// mesh flags

			// uses dynamic sized struct, similar to RLE animations
			int vertOffset;			    // start offset for this mesh's vertices
			int vertCacheSize;		    // size of the vertex structure
			int numVerts;			    // number of vertices

			int unk1;

			// vvw::mstudioboneweightextra_t
			int extraBoneWeightOffset;	// start offset for this mesh's "extended weights"
			int extraBoneWeightSize;    // size or count of extended weights

			// unsigned short
			int indexOffset;			// start offset for this mesh's "indices"
			int numIndices;				// number of indices

			// vvd::mstudioboneweight_t
			int legacyWeightOffset;	// seems to be an offset into the "external weights" buffer for this mesh
			int numLegacyWeights;   // seems to be the number of "external weights" that this mesh uses

			// vtx::StripHeader_t
			int stripOffset;        // Index into the strips structs
			int numStrips;

			// might be stuff like topologies and or bonestates
			int unk[4];

			/*int numBoneStateChanges;
			int boneStateChangeOffset;
			// MDL Version 49 and up only
			int numTopologyIndices;
			int topologyOffset;*/
		};

		struct Vertex_t
		{
			// do functions here
		};

		struct unkdata
		{
			__int64 unk;
			float unk1;

			char data[0x24];
		};
	}
}


//=============
// STUDIO MODEL
//=============

// Titanfall 1, version '52'
namespace r1
{
	// This flag is set if no hitbox information was specified
	#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX	0x1

	// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
	// models when we change materials.
	#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP		0x2

	// Use this when there are translucent parts to the model but we're not going to sort it 
	#define STUDIOHDR_FLAGS_FORCE_OPAQUE			0x4

	// Use this when we want to render the opaque parts during the opaque pass
	// and the translucent parts during the translucent pass
	#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS		0x8

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

	// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
	// instead of overriding them with the default one (necessary for translucent shadows)
	#define STUDIOHDR_FLAGS_OBSOLETE				0x200

	#define STUDIOHDR_FLAGS_UNUSED					0x400

	// NOTE:  This flag is set at mdl build time
	#define STUDIOHDR_FLAGS_NO_FORCED_FADE			0x800

	// NOTE:  The npc will lengthen the viseme check to always include two phonemes
	#define STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE	0x1000

	// This flag is set when the .qc has $constantdirectionallight in it
	// If set, we use constantdirectionallightdot to calculate light intensity
	// rather than the normal directional dot product
	// only valid if STUDIOHDR_FLAGS_STATIC_PROP is also set
	#define STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT 0x2000

	// Flag to mark delta flexes as already converted from disk format to memory format
	#define STUDIOHDR_FLAGS_FLEXES_CONVERTED		0x4000

	// Indicates the studiomdl was built in preview mode
	#define STUDIOHDR_FLAGS_BUILT_IN_PREVIEW_MODE	0x8000

	// Ambient boost (runtime flag)
	#define STUDIOHDR_FLAGS_AMBIENT_BOOST			0x10000

	// Don't cast shadows from this model (useful on first-person models)
	#define STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS		0x20000

	// alpha textures should cast shadows in vrad on this model (ONLY prop_static!)
	#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS	0x40000

	// Model has a quad-only Catmull-Clark SubD cage
	#define STUDIOHDR_FLAGS_SUBDIVISION_SURFACE		0x80000

	// flagged on load to indicate no animation events on this model
	// might be a different thing on v54
	#define STUDIOHDR_FLAGS_NO_ANIM_EVENTS			0x100000

	// If flag is set then studiohdr_t.flVertAnimFixedPointScale contains the
	// scale value for fixed point vert anim data, if not set then the
	// scale value is the default of 1.0 / 4096.0.  Regardless use
	// studiohdr_t::VertAnimFixedPointScale() to always retrieve the scale value
	#define STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE	0x200000

	#define STUDIOHDR_FLAGS_RESPAWN_UNK                 0x800000

	// "colorindex is only shifted if 0x1000000 flag is set on the studiohdr" talking about colorindex in mstudiomodel struct
	// If this flag is present the model has vertex color, and by extension a VVC (IDVC) file.
	#define STUDIOHDR_FLAGS_USES_VERTEX_COLOR	        0x1000000

	// If this flag is present the model has a secondary UV layer, and by extension a VVC (IDVC) file.
	#define STUDIOHDR_FLAGS_USES_UV2			        0x2000000

	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 48 (0x30,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		char name[64]; // The internal name of the model, padding with null chars.
						// Typically "my_model.mdl" will have an internal name of "my_model"
		int length; // Data size of MDL file in chars.

		Vector eyeposition;	// ideal eye position

		Vector illumposition;	// illumination center

		Vector hull_min;		// ideal movement hull size
		Vector hull_max;

		Vector view_bbmin;		// clipping bounding box
		Vector view_bbmax;

		int flags;

		// highest observed: 250
		// max is definitely 256 because 8bit uint limit
		int numbones; // bones
		int boneindex;

		int numbonecontrollers; // bone controllers
		int bonecontrollerindex;

		int numhitboxsets;
		int hitboxsetindex;

		int numlocalanim; // animations/poses
		int localanimindex; // animation descriptions

		int numlocalseq; // sequences
		int	localseqindex;

		int activitylistversion; // initialization flag - have the sequences been indexed?
		int eventsindexed;

		// raw textures
		int numtextures;
		int textureindex;

		/// raw textures search paths
		int numcdtextures;
		int cdtextureindex;

		// replaceable textures tables
		int numskinref;
		int numskinfamilies;
		int skinindex;

		int numbodyparts;
		int bodypartindex;

		int numlocalattachments;
		int localattachmentindex;

		int numlocalnodes;
		int localnodeindex;
		int localnodenameindex;

		int deprecated_numflexdesc;
		int deprecated_flexdescindex;

		int deprecated_numflexcontrollers;
		int deprecated_flexcontrollerindex;

		int deprecated_numflexrules;
		int deprecated_flexruleindex;

		int numikchains;
		int ikchainindex;

		int deprecated_nummouths;
		int deprecated_mouthindex;

		int numlocalposeparameters;
		int localposeparamindex;

		int surfacepropindex;

		int keyvalueindex;
		int keyvaluesize;

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;


		float mass;
		int contents;

		// external animations, models, etc.
		int numincludemodels;
		int includemodelindex;

		// implementation specific back pointer to virtual data
		int /* mutable void* */ virtualModel;

		// for demand loaded animation blocks
		int szanimblocknameindex;

		int numanimblocks;
		int animblockindex;

		int /* mutable void* */ animblockModel;

		int bonetablebynameindex;

		// used by tools only that don't cache, but persist mdl's peer data
		// engine uses virtualModel to back link to cache pointers
		int /* void* */ pVertexBase;
		int /* void* */ pIndexBase;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		char constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		char rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		char numAllowedRootLODs;

		char unused;

		float fadedistance;	// set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
							// player/titan models seem to inherit this value from the first model loaded in menus.
							// works oddly on entities, probably only meant for static props

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale;
		int surfacepropLookup;	// this index must be cached by the loader, not saved in the file

		// NOTE: No room to add stuff? Up the .mdl file format version 
		// [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
		// or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
		int studiohdr2index;

		// this is in most shipped models, probably part of their asset bakery.
		// doesn't actually need to be written pretty sure, only four chars when not present.
		// this is not completely true as some models simply have nothing, such as animation models.
		int sourceFilenameOffset;
	};

	struct studiohdr2_t
	{
		int numsrcbonetransform;
		int srcbonetransformindex;

		int	illumpositionattachmentindex;

		float flMaxEyeDeflection; // default to cos(30) if not set

		int linearboneindex;

		int sznameindex;

		int m_nBoneFlexDriverCount;
		int m_nBoneFlexDriverIndex;

		// for static props (and maybe others)
		// Precomputed Per-Triangle AABB data
		int m_nPerTriAABBIndex;
		int m_nPerTriAABBNodeCount;
		int m_nPerTriAABBLeafCount;
		int m_nPerTriAABBVertCount;

		// always "" or "Titan"
		int unkstringindex;

		int reserved[39];
	};

	#define BONE_CALCULATE_MASK			0x1F
	#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
	#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
	#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
	#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
	#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.

	#define BONE_USED_BY_IKCHAIN		0x20 // bone is influenced by IK chains, added in V52 (Titanfall 1)

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

	#define BONE_FLAG_UNK				0x00080000 // where?

	#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
	#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

	#define BONE_TYPE_MASK				0x00F00000
	#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

	#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Vector48
	#define BONE_HAS_SAVEFRAME_ROT64	0x00400000	// Quaternion64
	#define BONE_HAS_SAVEFRAME_ROT32	0x00800000	// Quaternion32

	#define BONE_FLAG_UNK1				0x01000000 // where?

	struct mstudiobone_t
	{
		int sznameindex;

		int parent; // parent bone
		int bonecontroller[6]; // bone controller index, -1 == none

		// default values
		Vector pos; // base bone position
		Quaternion quat;
		RadianEuler rot; // base bone rotation

		// compression scale
		Vector posscale; // scale muliplier for bone position in animations
		Vector rotscale; // scale muliplier for bone rotation in animations

		matrix3x4_t poseToBone;
		Quaternion qAlignment;

		int flags;
		int proctype;
		int procindex; // procedural rule offset
		int physicsbone; // index into physically simulated bone

		int surfacepropidx; // index into string tablefor property name

		int contents; // See BSPFlags.h for the contents flags

		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// bone scale(?)
		Vector scale; // base bone scale
		Vector scalescale; // scale muliplier for bone scale in animations

		int unused; // remove as appropriate
	};

	#define JIGGLE_IS_FLEXIBLE				0x01
	#define JIGGLE_IS_RIGID					0x02
	#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
	#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
	#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
	#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20
	#define JIGGLE_HAS_BASE_SPRING			0x40

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiojigglebone_t
	{
		int flags;

		// general params
		float length; // how far from bone base, along bone, is tip
		float tipMass;

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

	// this struct is the same in r1 and r2
	struct mstudiolinearbone_t
	{
		int numbones;

		int flagsindex;

		int	parentindex;

		int	posindex;

		int quatindex;

		int rotindex;

		int posetoboneindex;

		int	posscaleindex;

		int	rotscaleindex;

		int	qalignmentindex;

		int unused[6];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiosrcbonetransform_t
	{
		int			sznameindex;
		matrix3x4_t	pretransform;
		matrix3x4_t	posttransform;
	};

	// I have never seen the following structs used in Titanfall 1 but the game definitely has them as seen in datacache.dll
	// these cannot be confirmed for Titanfall 2 and as such will not be listed there
	struct mstudioboneflexdrivercontrol_t
	{
		int m_nBoneComponent;		// Bone component that drives flex, StudioBoneFlexComponent_t
		int m_nFlexControllerIndex;	// Flex controller to drive
		float m_flMin;				// Min value of bone component mapped to 0 on flex controller
		float m_flMax;				// Max value of bone component mapped to 1 on flex controller
	};

	struct mstudioboneflexdriver_t
	{
		int m_nBoneIndex;			// Bone to drive flex controller
		int m_nControlCount;		// Number of flex controllers being driven
		int m_nControlIndex;		// Index into data where controllers are (relative to this)

		int unused[3];
	};

	struct mstudioaxisinterpbone_t
	{
		int				control;// local transformation of this bone used to calc 3 point blend
		int				axis;	// axis to check
		Vector			pos[6];	// X+, X-, Y+, Y-, Z+, Z-
		Quaternion		quat[6];// X+, X-, Y+, Y-, Z+, Z-
	};

	struct mstudioquatinterpbone_t
	{
		int				control;// local transformation to check
		int				numtriggers;
		int				triggerindex;
	};

	struct mstudioaimatbone_t
	{
		int				parent;
		int				aim;		// Might be bone or attach
		Vector			aimvector;
		Vector			upvector;
		Vector			basepos;
	};

	struct mstudiotwistbonetarget_t
	{
		int				m_nBone;
		float			m_flWeight;
		Vector			m_vBaseTranslate;
		Quaternion		m_qBaseRotation;
	};

	struct mstudiotwistbone_t
	{
		bool			m_bInverse;				// False: Apply child rotation to twist targets True: Apply parent rotation to twist targets
		Vector			m_vUpVector;			// In parent space, projected into plane defined by vector between parent & child
		int				m_nParentBone;
		Quaternion		m_qBaseInv;	// The base rotation of the parent, used if m_bInverse is true
		int				m_nChildBone;

		int				m_nTargetCount;
		int				m_nTargetIndex;
	};

	struct mstudioquatinterpinfo_t
	{
		float			inv_tolerance;	// 1 / radian angle of trigger influence
		Quaternion		trigger;	// angle to match
		Vector			pos;		// new position
		Quaternion		quat;		// new angle
	};

	struct mstudiobonecontroller_t
	{
		int					bone;	// -1 == 0
		int					type;	// X, Y, Z, XR, YR, ZR, M
		float				start;
		float				end;
		int					rest;	// char index value at rest
		int					inputfield;	// 0-3 user set controller, 4 mouth
		int					unused[8];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudioattachment_t
	{
		int					sznameindex;
		unsigned int		flags;
		int					localbone;
		matrix3x4_t			local; // attachment point
		int					unused[8];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiohitboxset_t
	{
		int					sznameindex;
		int					numhitboxes;
		int					hitboxindex;
	};

	// unchanged from p2
	struct mstudiobbox_t
	{
		int					bone;
		int					group;				// intersection group
		Vector				bbmin;				// bounding box
		Vector				bbmax;
		int					szhitboxnameindex;	// offset to the name of the hitbox.
		int					unused[8];
	};

	// never seen this used but we have it anyway
	struct mstudiolocalhierarchy_t
	{
		int			iBone;			// bone being adjusted
		int			iNewParent;		// the bones new parent

		float		start;			// beginning of influence
		float		peak;			// start of full influence
		float		tail;			// end of full influence
		float		end;			// end of all influence

		int			iStart;			// first frame 

		int			localanimindex;

		int			unused[4];
	};

	// sequence and autolayer flags
	#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
	#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
	#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
	#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
	#define STUDIO_POST		0x0010		// 
	#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
	#define STUDIO_FRAMEANIM 0x0040		// animation is encoded as by frame x bone instead of RLE bone x frame
	#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
	#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
	#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
	#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
	#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
	#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
	#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index on server
	#define STUDIO_WORLD	0x4000		// sequence blends in worldspace
	#define STUDIO_NOFORCELOOP 0x8000	// do not force the animation loop
	#define STUDIO_EVENT_CLIENT 0x10000	// Has been updated at runtime to event index on client
	#define STUDIO_ANIM_UNK		    0x20000 // actually first in v52, from where??
	#define STUDIO_FRAMEMOVEMENT    0x40000 // framemovements are only read if this flag is present
	#define STUDIO_ANIM_UNK2	    0x80000 // cherry blossom v53, levi in v54

	// similar to p2 struct with some unused slots filled
	struct mstudioanimdesc_t
	{
		int baseptr;

		int sznameindex;

		float fps; // frames per second	
		int flags; // looping/non-looping flags

		int numframes;

		// piecewise movement
		int	nummovements;
		int movementindex;

		int ikrulezeroframeindex;

		int framemovementindex; // new in v52

		int unused1[4]; // remove as appropriate (and zero if loading older versions)	

		int animblock;
		int animindex; // non-zero when anim data isn't in sections

		int numikrules;
		int ikruleindex; // non-zero when IK data is stored in the mdl
		int animblockikruleindex; // non-zero when IK data is stored in animblock file

		int numlocalhierarchy;
		int localhierarchyindex;;

		int sectionindex;
		int sectionframes; // number of frames used in each fast lookup section, zero if not used

		short zeroframespan; // frames per span
		short zeroframecount; // number of spans
		int zeroframeindex;

		float zeroframestalltime; // saved during read stalls
	};

	struct mstudioanimblock_t
	{
		int					datastart;
		int					dataend;
	};

	struct mstudioanimsections_t
	{
		int					animblock;
		int					animindex;
	};

	union mstudioanimvalue_t
	{
		struct
		{
			char	valid;
			char	total;
		} num;
		short		value;
	};

	struct mstudioanim_valueptr_t
	{
		short	offset[3];
	};

	#define STUDIO_ANIM_RAWPOS	0x01 // Vector48
	#define STUDIO_ANIM_RAWROT	0x02 // Quaternion48
	#define STUDIO_ANIM_ANIMPOS	0x04 // mstudioanim_valueptr_t
	#define STUDIO_ANIM_ANIMROT	0x08 // mstudioanim_valueptr_t
	#define STUDIO_ANIM_DELTA	0x10
	#define STUDIO_ANIM_RAWROT2	0x20 // Quaternion64
	#define STUDIO_ANIM_RAWSCALE	0x40 // Vector48
	#define STUDIO_ANIM_ANIMSCALE	0x80 // mstudioanim_valueptr_t 
										 // gravestone_01_animated.mdl and leviathan models use scale in RLE

	struct mstudio_rle_anim_t
	{
		char				bone;
		char				flags;		// weighing options

		// leaving these here as there is no other way to show the data off, there likely should be scale stuff in here
		// I wanna redo these at some point so it's not copy/paste but honestly I am unsure if it can be done better

		// scale comes after pos in both cases

		// valid for animating data only
		inline char* pData(void) const { return (((char*)this) + sizeof(struct mstudio_rle_anim_t)); };
		inline mstudioanim_valueptr_t* pRotV(void) const { return (mstudioanim_valueptr_t*)(pData()); };
		inline mstudioanim_valueptr_t* pPosV(void) const { return (mstudioanim_valueptr_t*)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0); };

		// valid if animation unvaring over timeline
		inline Quaternion48* pQuat48(void) const { return (Quaternion48*)(pData()); };
		inline Quaternion64* pQuat64(void) const { return (Quaternion64*)(pData()); };
		inline Vector48* pPos(void) const { return (Vector48*)(pData() + ((flags & STUDIO_ANIM_RAWROT) != 0) * sizeof(*pQuat48()) + ((flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof(*pQuat64())); };

		// need funcs for scale and to make this less stinky

		// points to next bone in the list
		short				nextoffset;
	};

	#define STUDIO_FRAME_RAWPOS		0x01 // Vector48 in constants
	#define STUDIO_FRAME_RAWROT		0x02 // Quaternion48 in constants
	#define STUDIO_FRAME_RAWSCALE	0x04 // Vector48 in constants, atlas_mp_scripted.mdl at_hotdrop_loop at_hotdrop_01
	#define STUDIO_FRAME_ANIMPOS	0x08 // Quaternion48 in framedata
	#define STUDIO_FRAME_ANIMROT	0x10 // Vector48 in framedata
	#define STUDIO_FRAME_ANIMSCALE	0x20 // Vector48 in framedata

	// per bone
	struct unkframeanimdata_t
	{
		short unkframe; // advances by six when bone has flags for frame data
		short unkconstant; // advances by six when bone has flags for constant data
	};

	struct mstudio_frame_anim_t
	{
		int constantsoffset;

		int frameoffset;
		int framelength;

		int fixedOldBoneflags;

		int unkdataindex; // indexes into array of values that somewhat relate to flags

		int unused;
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

		int	bone;

		int slot;	// iktarget slot.  Usually same as chain.
		float height;
		float radius;
		float floor;
		Vector pos;
		Quaternion q;

		int compressedikerrorindex;

		int unused2;

		int iStart;
		int ikerrorindex;

		float start;	// beginning of influence
		float peak;	// start of full influence
		float tail;	// end of full influence
		float end;	// end of all influence

		float unused3;	// 
		float contact;	// frame footstep makes ground concact
		float drop;		// how far down the foot should drop when reaching for IK
		float top;		// top of the foot box

		int unused6;
		int unused7;
		int unused8;

		int szattachmentindex;		// name of world attachment

		float endHeight; // new in v52

		int unused[6];
	};

	struct mstudioikerror_t
	{
		Vector		pos;
		Quaternion	q;
	};

	struct mstudiocompressedikerror_t
	{
		float	scale[6];
		short	offset[6];
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

	#define STUDIO_LINEAR	0x00001000

	#define STUDIO_TYPES	0x0003FFFF
	#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance

	struct mstudiomovement_t
	{
		int					endframe;
		int					motionflags;
		float				v0;			// velocity at start of block
		float				v1;			// velocity at end of block
		float				angle;		// YAW rotation at end of this blocks movement
		Vector				vector;		// movement vector relative to this blocks initial angle
		Vector				position;	// relative to start of animation???
	};

	// new in Titanfall 1
	// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
	// pos_x, pos_y, pos_z, yaw
	struct mstudioframemovement_t
	{
		float scale[4];
		short offset[4];
	};

	struct mstudioseqdesc_t
	{
		int baseptr;

		int	szlabelindex;

		int szactivitynameindex;

		int flags; // looping/non-looping flags

		int activity; // initialized at loadtime to game DLL values
		int actweight;

		int numevents;
		int eventindex;

		Vector bbmin; // per sequence bounding box
		Vector bbmax;

		int numblends;

		// Index into array of shorts which is groupsize[0] x groupsize[1groupsize[1] in length
		int animindexindex;

		int movementindex; // [blend] float array for blended movement
		int groupsize[2];
		int paramindex[2]; // X, Y, Z, XR, YR, ZR
		float paramstart[2]; // local (0..1) starting value
		float paramend[2]; // local (0..1) ending value
		int paramparent;

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

		int weightlistindex;

		int posekeyindex;

		int numiklocks;
		int iklockindex;

		// Key values
		int	keyvalueindex;
		int keyvaluesize;

		int cycleposeindex; // index of pose parameter to use as cycle index

		int activitymodifierindex;
		int numactivitymodifiers;

		int ikResetMask; // new in v52
		int unk1; // count? STUDIO_ANIMDESC_52_UNK?? ikReset (what above var is masking)

		int unused[3];
	};

	#define NEW_EVENT_STYLE ( 1 << 10 )

	struct mstudioevent_t
	{
		float				cycle;
		int					event;
		int					type;
		char				options[64];

		int					szeventindex;
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
	#define STUDIO_AL_UNK		0x2000		// observed in v52
	#define STUDIO_AL_POSE		0x4000		// layer blends using a pose parameter instead of parent cycle
	#define STUDIO_AL_UNK1		0x8000		// added in v53 (probably)
											// I love this game I love this game I love this game I love this game I love this game I love this game

	struct mstudioautolayer_t
	{
		//private:
		short				iSequence;
		short				iPose;
		//public:
		int					flags;
		float				start;	// beginning of influence
		float				peak;	// start of full influence
		float				tail;	// end of full influence
		float				end;	// end of all influence
	};

	struct mstudioiklock_t
	{
		int			chain;
		float		flPosWeight;
		float		flLocalQWeight;
		int			flags;

		int			unused[4];
	};

	struct mstudioactivitymodifier_t
	{
		int sznameindex;
		bool negate; // negate all other activity modifiers when this one is active?
	};

	struct mstudio_meshvertexloddata_t
	{
		int modelvertexdataUnusedPad; // likely has none of the funny stuff because unused

		int numLODVertexes[MAX_NUM_LODS]; // depreciated starting with rmdl v14(?)
	};

	struct mstudiomesh_t
	{
		int material;

		int modelindex;

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

		int unused[8]; // remove as appropriate
	};
	
	struct mstudiomodel_t
	{
		char name[64];

		int type;

		float boundingradius;

		int nummeshes;
		int meshindex;

		mstudiomesh_t* pMesh(int i)
		{
			return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i;
		}

		// cache purposes
		int numvertices; // number of unique vertices/normals/texcoords
		int vertexindex; // vertex Vector
						 // offset by vertexindex number of chars into vvd verts
		int tangentsindex; // tangents Vector
						   // offset by tangentsindex number of chars into vvd tangents

		int numattachments;
		int attachmentindex;

		int deprecated_numeyeballs;
		int deprecated_eyeballindex;

		int pad[4];

		int colorindex; // vertex color
						// offset by colorindex number of chars into vvc vertex colors
		int uv2index; // vertex second uv map
					  // offset by uv2index number of chars into vvc secondary uv map

		int unused[4];
	};

	struct mstudiobodyparts_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					nummodels;
		int					base;
		int					modelindex; // index into models array

		mstudiomodel_t* pModel(int i)
		{
			return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i;
		};
	};

	struct mstudioiklink_t
	{
		int		bone;
		Vector	kneeDir;	// ideal bending direction (per link, if applicable)
		Vector	unused0;	// unused
	};

	struct mstudioikchain_t
	{
		int				sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int				linktype;
		int				numlinks;
		int				linkindex;
		
		mstudioiklink_t* pLink(int i)
		{
			return reinterpret_cast<mstudioiklink_t*>((char*)this + linkindex) + i;
		}
	};
	
	struct mstudioposeparamdesc_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					flags;	// ????
		float				start;	// starting value
		float				end;	// ending value
		float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
	};
	
	struct mstudiomodelgroup_t
	{
		int					szlabelindex;	// textual name
		char* pszLabel()
		{
			return ((char*)this) + szlabelindex;
		}

		int					sznameindex;	// file name
		inline char* const pszName() const { return ((char*)this + sznameindex); }
	};

	struct mstudiotexture_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int unused_flags;
		int used;
		int unused1;

		// these are turned into 64 bit ints on load and only filled in memory
		//mutable IMaterial		*material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
		//mutable void* clientmaterial;	// gary, replace with client material pointer if used
		int material_RESERVED;
		int clientmaterial_RESERVED;

		int unused[10];
	};
}

// Titanfall 2, version '53'
namespace r2
{
	struct mstudiopertrihdr_t
	{
		short version; // game requires this to be 2 or else it errors

		short unk; // may or may not exist, version gets casted as short in ida

		Vector bbmin;
		Vector bbmax;

		int unused[8];
	};

	struct mstudiopertrivertex_t
	{
		// to get float value:
		// axisValue * ((float)(bbmax.axis - bbmin.axis) * 0.000015259022)
		// where axis is x, y, or z. bbmax and bbmin are from the pertri header.
		short x, y, z;
	};


	#define BONE_CALCULATE_MASK			0x1F
	#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
	#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
	#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
	#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
	#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.

	#define BONE_USED_BY_IKCHAIN		0x20 // bone is influenced by IK chains, added in V52 (Titanfall 1)

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

	#define BONE_FLAG_UNK				0x00080000 // where?

	#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
	#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

	#define BONE_TYPE_MASK				0x00F00000
	#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

	#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Vector48
	#define BONE_HAS_SAVEFRAME_ROT64	0x00400000	// Quaternion64
	#define BONE_HAS_SAVEFRAME_ROT32	0x00800000	// Quaternion32

	#define BONE_FLAG_UNK1				0x01000000 // where?

	struct mstudiobone_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int parent; // parent bone
		int bonecontroller[6]; // bone controller index, -1 == none

		// default values
		Vector pos; // base bone position
		Quaternion quat;
		RadianEuler rot; // base bone rotation
		Vector scale; // base bone scale

		// compression scale
		Vector posscale; // scale muliplier for bone position in animations. depreciated in v53, as the posscale is stored in anim bone headers
		Vector rotscale; // scale muliplier for bone rotation in animations
		Vector scalescale; // scale muliplier for bone scale in animations

		matrix3x4_t poseToBone;
		Quaternion qAlignment;

		int flags;
		int proctype;
		int procindex; // procedural rule offset
		int physicsbone; // index into physically simulated bone
						 // from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
		int surfacepropidx; // index into string tablefor property name
		inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

		int contents; // See BSPFlags.h for the contents flags

		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// unknown phy related section
		short unkindex; // index into this section
		short unkcount; // number of sections for this bone?? see: models\s2s\s2s_malta_gun_animated.mdl

		int unused[7]; // remove as appropriate
	};

	#define JIGGLE_IS_FLEXIBLE				0x01
	#define JIGGLE_IS_RIGID					0x02
	#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
	#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
	#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
	#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20
	#define JIGGLE_HAS_BASE_SPRING			0x40

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiojigglebone_t
	{
		int flags;

		// general params
		float length; // how far from bone base, along bone, is tip
		float tipMass;

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

	// this struct is the same in r1 and r2
	struct mstudiolinearbone_t
	{
		int numbones;

		int flagsindex;
		inline int flags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int>((char*)this + flagsindex) + i; }
		inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }

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

		int	posscaleindex; // unused

		int	rotscaleindex;
		inline const Vector* pRotScale(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<Vector*>((char*)this + rotscaleindex) + i; }

		int	qalignmentindex;
		inline const Quaternion* pQAlignment(int i)
		const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + qalignmentindex) + i;
		}

		int unused[6];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiosrcbonetransform_t
	{
		int			sznameindex;
		matrix3x4_t	pretransform;
		matrix3x4_t	posttransform;
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudioattachment_t
	{
		int					sznameindex;
		unsigned int		flags;
		int					localbone;
		matrix3x4_t			local; // attachment point
		int					unused[8];
	};

	// this struct is the same in r1, r2, and early r5, and unchanged from p2
	struct mstudiohitboxset_t
	{
		int					sznameindex;
		int					numhitboxes;
		int					hitboxindex;
	};

	struct mstudiobbox_t
	{
		int					bone;
		int					group;				// intersection group
		Vector				bbmin;				// bounding box
		Vector				bbmax;
		int					szhitboxnameindex;	// offset to the name of the hitbox.

		int critoverride; // overrides the group to be a crit, 0 or 1. might be group override since group 1 is head.
		int keyvalueindex; // indexes into a kv group if used, mostly for titans.

		int unused[6];
	};
	
	union mstudioanimvalue_t
	{
		struct
		{
			char	valid;
			char	total;
		} num;
		short		value;
	};

	struct mstudioanim_valueptr_t
	{
		short	offset[3];
		inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
	};

	// These work as toggles, flag enabled = raw data, flag disabled = pointers, with rotations
	#define STUDIO_ANIM_DELTA		0x01 // delta animation
	#define STUDIO_ANIM_RAWPOS		0x02 // Vector48
	#define STUDIO_ANIM_RAWROT		0x04 // Quaternion64
	#define STUDIO_ANIM_RAWSCALE	0x08 // Vector48, drone_frag.mdl for scale track usage
	#define STUDIO_ANIM_NOROT		0x10 // this flag is used to check if there is 1. no static rotation and 2. no rotation track
										 // https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L393 used for this
										 // this flag is used when the animation has no rotation data, it exists because it is not possible to check this as there are not separate flags for static/track rotation data

	struct mstudio_rle_anim_t
	{
		float				posscale; // does what posscale is used for

		unsigned char		bone;
		char				flags;		// weighing options

		inline char* pData() const { return ((char*)this + sizeof(mstudio_rle_anim_t)); } // gets start of animation data, this should have a '+2' if aligned to 4
		inline mstudioanim_valueptr_t* pRotV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData()); } // returns rot as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pPosV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 8); } // returns pos as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pScaleV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 14); } // returns scale as mstudioanim_valueptr_t

		inline Quaternion64* pQuat64() const { return reinterpret_cast<Quaternion64*>(pData()); } // returns rot as static Quaternion64
		inline Vector48* pPos() const { return reinterpret_cast<Vector48*>(pData() + 8); } // returns pos as static Vector48
		inline Vector48* pScale() const { return reinterpret_cast<Vector48*>(pData() + 14); } // returns scale as static Vector48

		// points to next bone in the list
		inline int* pNextOffset() const { return reinterpret_cast<int*>(pData() + 20); }
		inline mstudio_rle_anim_t* pNext() const { return pNextOffset() ? reinterpret_cast<mstudio_rle_anim_t*>((char*)this + *pNextOffset()) : nullptr; } // untested
	};

	struct mstudioanimsections_t
	{
		int					animindex;
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

	#define STUDIO_LINEAR	0x00001000

	#define STUDIO_TYPES	0x0003FFFF
	#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance

	struct mstudiomovement_t
	{
		int					endframe;
		int					motionflags;
		float				v0;			// velocity at start of block
		float				v1;			// velocity at end of block
		float				angle;		// YAW rotation at end of this blocks movement
		Vector				vector;		// movement vector relative to this blocks initial angle
		Vector				position;	// relative to start of animation???
	};

	// new in Titanfall 1
	// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
	// pos_x, pos_y, pos_z, yaw
	struct mstudioframemovement_t
	{
		float scale[4];
		short offset[4];
		inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
	};

	// sequence and autolayer flags
	#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
	#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
	#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
	#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
	#define STUDIO_POST		0x0010		// 
	#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
	#define STUDIO_FRAMEANIM 0x0040		// animation is encoded as by frame x bone instead of RLE bone x frame
	#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
	#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
	#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
	#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
	#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
	#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
	#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index on server
	#define STUDIO_WORLD	0x4000		// sequence blends in worldspace
	#define STUDIO_NOFORCELOOP 0x8000	// do not force the animation loop
	#define STUDIO_EVENT_CLIENT 0x10000	// Has been updated at runtime to event index on client
	#define STUDIO_ANIM_UNK		    0x20000 // actually first in v52, from where??
	#define STUDIO_FRAMEMOVEMENT    0x40000 // framemovements are only read if this flag is present
	#define STUDIO_ANIM_UNK2	    0x80000 // cherry blossom v53, levi in v54

	struct mstudioanimdesc_t
	{
		int baseptr;

		int sznameindex;
		inline const char* pszName() const { return ((char*)this + sznameindex); }

		float fps; // frames per second	
		int flags; // looping/non-looping flags

		int numframes;

		// piecewise movement
		int nummovements;
		int movementindex;
		inline mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<mstudiomovement_t*>((char*)this + movementindex) + i; };

		int framemovementindex; // new in v52
		inline const mstudioframemovement_t* pFrameMovement() const { return reinterpret_cast<mstudioframemovement_t*>((char*)this + framemovementindex); }

		int animindex; // non-zero when anim data isn't in sections
		mstudio_rle_anim_t* pAnim(int* piFrame) const; // returns pointer to data and new frame index

		int numikrules;
		int ikruleindex; // non-zero when IK data is stored in the mdl

		int numlocalhierarchy;
		int localhierarchyindex;

		int sectionindex;
		int sectionframes; // number of frames used in each fast lookup section, zero if not used
		inline const mstudioanimsections_t* pSection(int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }

		int unused[8];
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
		int bone;

		int slot; // iktarget slot. Usually same as chain.
		float height;
		float radius;
		float floor;
		Vector pos;
		Quaternion q;

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

		float endHeight; // new in v52
						 // titan_buddy_mp_core.mdl

		int unused[8];
	};

	struct mstudioikerror_t
	{
		Vector		pos;
		Quaternion	q;
	};

	struct mstudiocompressedikerror_t
	{
		float	scale[6];
		short	offset[6];
	};

	struct mstudioseqdesc_t
	{
		int baseptr;

		int	szlabelindex;

		int szactivitynameindex;

		int flags; // looping/non-looping flags

		int activity; // initialized at loadtime to game DLL values
		int actweight;

		int numevents;
		int eventindex;

		Vector bbmin; // per sequence bounding box
		Vector bbmax;

		int numblends;

		// Index into array of shorts which is groupsize[0] x groupsize[1] in length
		int animindexindex;

		int movementindex; // [blend] float array for blended movement
		int groupsize[2];
		int paramindex[2]; // X, Y, Z, XR, YR, ZR
		float paramstart[2]; // local (0..1) starting value
		float paramend[2]; // local (0..1) ending value
		int paramparent;

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

		int weightlistindex;

		int posekeyindex;

		int numiklocks;
		int iklockindex;

		// Key values
		int keyvalueindex;
		int keyvaluesize;

		int cycleposeindex; // index of pose parameter to use as cycle index

		int activitymodifierindex;
		int numactivitymodifiers;

		int ikResetMask; // new in v52
						 // titan_buddy_mp_core.mdl
						 // reset all ikrules with this type???
		int unk1; // count? STUDIO_ANIMDESC_52_UNK??
				  // mayhaps this is the ik type applied to the mask if what's above it true

		int unused[8];
	};

	#define NEW_EVENT_STYLE ( 1 << 10 )

	struct mstudioevent_t
	{
		float				cycle;
		int					event;
		int					type;
		char				options[64];

		int					szeventindex;
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
	#define STUDIO_AL_UNK		0x2000		// observed in v52
	#define STUDIO_AL_POSE		0x4000		// layer blends using a pose parameter instead of parent cycle
	#define STUDIO_AL_UNK1		0x8000		// added in v53 (probably)
											// I love this game I love this game I love this game I love this game I love this game I love this game

	struct mstudioautolayer_t
	{
		//private:
		short				iSequence;
		short				iPose;
		//public:
		int					flags;
		float				start;	// beginning of influence
		float				peak;	// start of full influence
		float				tail;	// end of full influence
		float				end;	// end of all influence
	};

	struct mstudioiklock_t
	{
		int			chain;
		float		flPosWeight;
		float		flLocalQWeight;
		int			flags;

		int			unused[4];
	};

	struct mstudioactivitymodifier_t
	{
		int sznameindex;
		bool negate; // negate all other activity modifiers when this one is active?
	};

	struct mstudio_meshvertexloddata_t
	{
		int modelvertexdataUnusedPad; // likely has none of the funny stuff because unused

		int numLODVertexes[MAX_NUM_LODS]; // depreciated starting with rmdl v14(?)
	};

	struct mstudiomesh_t
	{
		int material;

		int modelindex;

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

		char unk[8]; // set on load

		int unused[6]; // remove as appropriate
	};

	struct mstudiomodel_t
	{
		char name[64];

		int type;

		float boundingradius;

		int nummeshes;
		int meshindex;

		mstudiomesh_t* pMesh(int i)
		{
			return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i;
		}

		// cache purposes
		int numvertices; // number of unique vertices/normals/texcoords
		int vertexindex; // vertex Vector
						 // offset by vertexindex number of chars into vvd verts
		int tangentsindex; // tangents Vector
						   // offset by tangentsindex number of chars into vvd tangents

		int numattachments;
		int attachmentindex;

		int deprecated_numeyeballs;
		int deprecated_eyeballindex;

		int pad[4];

		int colorindex; // vertex color
						// offset by colorindex number of chars into vvc vertex colors
		int uv2index; // vertex second uv map
					  // offset by uv2index number of chars into vvc secondary uv map

		int unused[4];
	};

	struct mstudiobodyparts_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					nummodels;
		int					base;
		int					modelindex; // index into models array

		mstudiomodel_t* pModel(int i)
		{
			return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i;
		};
	};

	struct mstudioiklink_t
	{
		int		bone;
		Vector	kneeDir;	// ideal bending direction (per link, if applicable)
		Vector	unused0;	// unused
	};

	struct mstudioikchain_t
	{
		int				sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int				linktype;
		int				numlinks;
		int				linkindex;

		mstudioiklink_t* pLink(int i)
		{
			return reinterpret_cast<mstudioiklink_t*>((char*)this + linkindex) + i;
		}

		float	unk;		// no clue what this does tbh, tweaking it does nothing
							// default value: 0.707f
							// this value is similar to default source engine ikchain values

		int		unused[3];	// these get cut in apex so I can't imagine this is used
	};

	struct mstudioposeparamdesc_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					flags;	// ????
		float				start;	// starting value
		float				end;	// ending value
		float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
	};

	struct mstudiomodelgroup_t
	{
		int					szlabelindex;	// textual name
		inline char* const pszLabel() const { return ((char*)this + szlabelindex); }

		int					sznameindex;	// file name
		inline char* const pszName() const { return ((char*)this + sznameindex); }
	};

	struct mstudiotexture_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int unused_flags;
		int used;

		int unused[8];
	};

	// This flag is set if no hitbox information was specified
	#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX	0x1

	// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
	// models when we change materials.
	#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP		0x2

	// Use this when there are translucent parts to the model but we're not going to sort it 
	#define STUDIOHDR_FLAGS_FORCE_OPAQUE			0x4

	// Use this when we want to render the opaque parts during the opaque pass
	// and the translucent parts during the translucent pass
	#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS		0x8

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

	// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
	// instead of overriding them with the default one (necessary for translucent shadows)
	#define STUDIOHDR_FLAGS_OBSOLETE				0x200

	#define STUDIOHDR_FLAGS_UNUSED					0x400

	// NOTE:  This flag is set at mdl build time
	#define STUDIOHDR_FLAGS_NO_FORCED_FADE			0x800

	// NOTE:  The npc will lengthen the viseme check to always include two phonemes
	#define STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE	0x1000

	// This flag is set when the .qc has $constantdirectionallight in it
	// If set, we use constantdirectionallightdot to calculate light intensity
	// rather than the normal directional dot product
	// only valid if STUDIOHDR_FLAGS_STATIC_PROP is also set
	#define STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT 0x2000

	// Flag to mark delta flexes as already converted from disk format to memory format
	#define STUDIOHDR_FLAGS_FLEXES_CONVERTED		0x4000

	// Indicates the studiomdl was built in preview mode
	#define STUDIOHDR_FLAGS_BUILT_IN_PREVIEW_MODE	0x8000

	// Ambient boost (runtime flag)
	#define STUDIOHDR_FLAGS_AMBIENT_BOOST			0x10000

	// Don't cast shadows from this model (useful on first-person models)
	#define STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS		0x20000

	// alpha textures should cast shadows in vrad on this model (ONLY prop_static!)
	#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS	0x40000

	// Model has a quad-only Catmull-Clark SubD cage
	#define STUDIOHDR_FLAGS_SUBDIVISION_SURFACE		0x80000

	// flagged on load to indicate no animation events on this model
	// might be a different thing on v54
	#define STUDIOHDR_FLAGS_NO_ANIM_EVENTS			0x100000

	// If flag is set then studiohdr_t.flVertAnimFixedPointScale contains the
	// scale value for fixed point vert anim data, if not set then the
	// scale value is the default of 1.0 / 4096.0.  Regardless use
	// studiohdr_t::VertAnimFixedPointScale() to always retrieve the scale value
	#define STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE	0x200000

	#define STUDIOHDR_FLAGS_RESPAWN_UNK                 0x800000

	// If this flag is present the model has vertex color, and by extension a VVC (IDVC) file.
	#define STUDIOHDR_FLAGS_USES_VERTEX_COLOR	        0x1000000

	// If this flag is present the model has a secondary UV layer, and by extension a VVC (IDVC) file.
	#define STUDIOHDR_FLAGS_USES_UV2			        0x2000000

	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 53 (0x35,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		int sznameindex; // This has been moved from studiohdr2_t to the front of the main header.
		inline char* const pszName() const { return ((char*)this + sznameindex); }
		char name[64]; // The internal name of the model, padding with null chars.
						// Typically "my_model.mdl" will have an internal name of "my_model"
		int length; // Data size of MDL file in chars.

		Vector eyeposition;	// ideal eye position

		Vector illumposition;	// illumination center

		Vector hull_min;		// ideal movement hull size
		Vector hull_max;

		Vector view_bbmin;		// clipping bounding box
		Vector view_bbmax;

		int flags;

		// highest observed: 250
		// max is definitely 256 because 8bit uint limit
		int numbones; // bones
		int boneindex;
		inline mstudiobone_t* pBone(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<mstudiobone_t*>((char*)this + boneindex) + i; }

		int numbonecontrollers; // bone controllers
		int bonecontrollerindex;

		int numhitboxsets;
		int hitboxsetindex;

		int numlocalanim; // animations/poses
		int localanimindex; // animation descriptions

		int numlocalseq; // sequences
		int	localseqindex;

		int activitylistversion; // initialization flag - have the sequences been indexed? set on load
		int eventsindexed;

		// mstudiotexture_t
		// short rpak path
		// raw textures
		int numtextures; // the material limit exceeds 128, probably 256.
		int textureindex;

		// this should always only be one, unless using vmts.
		// raw textures search paths
		int numcdtextures;
		int cdtextureindex;

		// replaceable textures tables
		int numskinref;
		int numskinfamilies;
		int skinindex;

		int numbodyparts;
		int bodypartindex;
		inline mstudiobodyparts_t* pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

		int numlocalattachments;
		int localattachmentindex;

		int numlocalnodes;
		int localnodeindex;
		int localnodenameindex;

		int deprecated_numflexdesc;
		int deprecated_flexdescindex;

		int deprecated_numflexcontrollers;
		int deprecated_flexcontrollerindex;

		int deprecated_numflexrules;
		int deprecated_flexruleindex;

		int numikchains;
		int ikchainindex;

		int numruimeshes;
		int ruimeshindex;

		int numlocalposeparameters;
		int localposeparamindex;

		int surfacepropindex;

		int keyvalueindex;
		int keyvaluesize;

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;

		float mass;
		int contents;

		// external animations, models, etc.
		int numincludemodels;
		int includemodelindex;

		// implementation specific back pointer to virtual data
		int /* mutable void* */ virtualModel;

		int bonetablebynameindex;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		char constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		char rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		char numAllowedRootLODs;

		char unused;

		float defaultFadeDist; // set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
							// player/titan models seem to inherit this value from the first model loaded in menus.
							// works oddly on entities, probably only meant for static props

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale;
		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// this is in most shipped models, probably part of their asset bakery.
		// doesn't actually need to be written pretty sure, only four chars when not present.
		// this is not completely true as some models simply have nothing, such as animation models.
		int sourceFilenameOffset;

		int numsrcbonetransform;
		int srcbonetransformindex;

		int	illumpositionattachmentindex;

		int linearboneindex;
		inline mstudiolinearbone_t* pLinearBones() const { return linearboneindex ? reinterpret_cast<mstudiolinearbone_t*>((char*)this + linearboneindex) : nullptr; }

		int m_nBoneFlexDriverCount;
		int m_nBoneFlexDriverIndex;

		// for static props (and maybe others)
		// Precomputed Per-Triangle AABB data
		int m_nPerTriAABBIndex;
		int m_nPerTriAABBNodeCount;
		int m_nPerTriAABBLeafCount;
		int m_nPerTriAABBVertCount;

		// always "" or "Titan"
		int unkstringindex;

		// ANIs are no longer used and this is reflected in many structs
		// start of interal file data
		int vtxindex; // VTX
		int vvdindex; // VVD / IDSV
		int vvcindex; // VVC / IDCV 
		int vphyindex; // VPHY / IVPS

		int vtxsize; // VTX
		int vvdsize; // VVD / IDSV
		int vvcsize; // VVC / IDCV 
		int vphysize; // VPHY / IVPS

		inline vtx::FileHeader_t* pVTX() const { return vtxsize > 0 ? reinterpret_cast<vtx::FileHeader_t*>((char*)this + vtxindex) : nullptr; }
		inline vvd::vertexFileHeader_t* pVVD() const { return vvdsize > 0 ? reinterpret_cast<vvd::vertexFileHeader_t*>((char*)this + vvdindex) : nullptr; }
		inline vvc::vertexColorFileHeader_t* pVVC() const { return vvcsize > 0 ? reinterpret_cast<vvc::vertexColorFileHeader_t*>((char*)this + vvcindex) : nullptr; }

		// this data block is related to the vphy, if it's not present the data will not be written
		// definitely related to phy, apex phy has this merged into it
		int unkindex; // section between vphy and vtx.?
		int numunk; // only seems to be used when phy has one solid

		// mostly seen on '_animated' suffixed models
		// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
		int numBoneFollowers;
		int boneFollowerIndex; // index only written when numbones > 1, means whatever func writes this likely checks this (would make sense because bonefollowers need more than one bone to even be useful). maybe only written if phy exists

		int unused1[60];

	};
}

// Apex Legends, version '54'
namespace r5
{
	// note: rrigs do not have a checksum
	// note: many things that are normally written on load are saved in file, for example 'surfacepropLookup'
	namespace v8
	{
		

		// some of these are likely different in apex
		#define BONE_CALCULATE_MASK			0x1F
		#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
		#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
		#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
		#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
		#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.

		#define BONE_USED_BY_IKCHAIN		0x20 // bone is influenced by IK chains, added in V52 (Titanfall 1)

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

		#define BONE_FLAG_UNK				0x00080000 // where?

		#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
		#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

		#define BONE_TYPE_MASK				0x00F00000
		#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

		#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Vector48
		#define BONE_HAS_SAVEFRAME_ROT64	0x00400000	// Quaternion64
		#define BONE_HAS_SAVEFRAME_ROT32	0x00800000	// Quaternion32

		#define BONE_FLAG_UNK1				0x01000000 // where?

		struct mstudiobone_t
		{
			int sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

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
			int physicsbone; // index into physically simulated bone
							 // from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
			int surfacepropidx; // index into string tablefor property name
			inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

			int contents; // See BSPFlags.h for the contents flags

			int surfacepropLookup; // this index must be cached by the loader, not saved in the file

			int unk;

			int unkid; // physics index (?)
		};

		#define JIGGLE_IS_FLEXIBLE				0x01
		#define JIGGLE_UNK						0x02
		#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
		#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
		#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
		#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20
		#define JIGGLE_HAS_BASE_SPRING			0x40

		// apex changed this a bit, 'is_rigid' cut
		struct mstudiojigglebone_t
		{
			char flags;

			unsigned char bone; // id of bone, might be single byte

			short pad; // possibly unused, possibly struct packing

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

		// this struct is the same in r1 and r2
		struct mstudiolinearbone_t
		{
			int numbones;

			int flagsindex;
			inline int flags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int>((char*)this + flagsindex) + i; }
			inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }

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
			int			sznameindex;
			matrix3x4_t	pretransform;
			matrix3x4_t	posttransform;
		};

		union mstudioanimvalue_t
		{
			struct
			{
				char	valid;
				char	total;
			} num;
			short		value;
		};

		#define STUDIO_ANIMPTR_Z 0x01
		#define STUDIO_ANIMPTR_Y 0x02
		#define STUDIO_ANIMPTR_X 0x04

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
			short offset : 13;
			short flags : 3;
			unsigned char axisIdx1; // these two are definitely unsigned
			unsigned char axisIdx2;
		};

		// flags for the per bone animation headers
		// "mstudioanim_valueptr_t" indicates it has a set of offsets into anim tracks
		#define STUDIO_ANIM_ANIMSCALE	0x01 // mstudioanim_valueptr_t
		#define STUDIO_ANIM_ANIMROT		0x02 // mstudioanim_valueptr_t
		#define STUDIO_ANIM_ANIMPOS		0x04 // mstudioanim_valueptr_t

		// flags for the per bone array, in 4 bit sections (two sets of flags per char), aligned to two chars
		#define STUDIO_ANIM_POS		0x1 // animation has pos values
		#define STUDIO_ANIM_ROT		0x2	// animation has rot values
		#define STUDIO_ANIM_SCALE	0x4	// animation has scale values

		struct mstudio_rle_anim_t
		{
			short size : 13; // total size of all animation data, not next offset because even the last one has it
			short flags : 3;
		};

		struct mstudioanimsections_t
		{
			int					animindex;
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

		#define STUDIO_LINEAR	0x00001000

		#define STUDIO_TYPES	0x0003FFFF
		#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance

		struct mstudiomovement_t
		{
			int					endframe;
			int					motionflags;
			float				v0;			// velocity at start of block
			float				v1;			// velocity at end of block
			float				angle;		// YAW rotation at end of this blocks movement
			Vector				vector;		// movement vector relative to this blocks initial angle
			Vector				position;	// relative to start of animation???
		};

		// new in Titanfall 1
		// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
		// pos_x, pos_y, pos_z, yaw
		struct mstudioframemovement_t
		{
			float scale[4];
			int sectionframes;
			//inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
		};
		// sequence and autolayer flags
		#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
		#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
		#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
		#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
		#define STUDIO_POST		0x0010		// 
		#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
		#define STUDIO_FRAMEANIM 0x0040		// animation is encoded as by frame x bone instead of RLE bone x frame
		#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
		#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
		#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
		#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
		#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
		#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
		#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index on server
		#define STUDIO_WORLD	0x4000		// sequence blends in worldspace
		#define STUDIO_NOFORCELOOP 0x8000	// do not force the animation loop
		#define STUDIO_EVENT_CLIENT 0x10000	// Has been updated at runtime to event index on client
		#define STUDIO_ANIM_UNK		    0x20000 // actually first in v52, from where??
		#define STUDIO_FRAMEMOVEMENT    0x40000 // framemovements are only read if this flag is present
		#define STUDIO_ANIM_UNK2	    0x80000 // cherry blossom v53, levi in v54

		struct mstudioanimdesc_t
		{
			int baseptr;

			int sznameindex;
			inline const char* pszName() const { return ((char*)this + sznameindex); }

			float fps; // frames per second	
			int flags; // looping/non-looping flags

			int numframes;

			// piecewise movement
			int nummovements;
			int movementindex;
			inline mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<mstudiomovement_t*>((char*)this + movementindex) + i; };

			int framemovementindex; // new in v52
			inline const mstudioframemovement_t* pFrameMovement() const { return reinterpret_cast<mstudioframemovement_t*>((char*)this + framemovementindex); }

			int animindex; // non-zero when anim data isn't in sections
			mstudio_rle_anim_t* pAnim(int* piFrame) const; // returns pointer to data and new frame index

			int numikrules;
			int ikruleindex; // non-zero when IK data is stored in the mdl

			int numlocalhierarchy;
			int localhierarchyindex;

			int sectionindex;
			int sectionframes; // number of frames used in each fast lookup section, zero if not used
			inline const mstudioanimsections_t* pSection(int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }
		};

		struct studiohdr_t
		{
			int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version; // Format version number, such as 48 (0x30,0x00,0x00,0x00)
			int checksum; // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64]; // The internal name of the model, padding with null chars.
							// Typically "my_model.mdl" will have an internal name of "my_model"
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

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			// seemingly unused now, as animations are per sequence
			int numlocalanim; // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int activitylistversion; // initialization flag - have the sequences been indexed?

			// mstudiotexture_t
			// short rpak path
			// raw textures
			int materialtypesindex; // index into an array of char sized material type enums for each material used by the model
			int numtextures; // the material limit exceeds 128, probably 256.
			int textureindex;

			// this should always only be one, unless using vmts.
			// raw textures search paths
			int numcdtextures;
			int cdtextureindex;

			// replaceable textures tables
			int numskinref;
			int numskinfamilies;
			int skinindex;

			int numbodyparts;
			int bodypartindex;

			int numlocalattachments;
			int localattachmentindex;

			int numlocalnodes;
			int localnodeindex;
			int localnodenameindex;

			int numunknodes; // ???
			int nodedataindexindex; // index into an array of int sized offsets that read into the data for each node

			int meshindex; // hard offset to the start of this models meshes

			// all flex related model vars and structs are stripped in respawn source
			int deprecated_numflexcontrollers;
			int deprecated_flexcontrollerindex;

			int deprecated_numflexrules;
			int deprecated_flexruleindex;

			int numikchains;
			int ikchainindex;

			// mesh panels for using rui on models, primarily for weapons
			int numruimeshes;
			int ruimeshindex;

			int numlocalposeparameters;
			int localposeparamindex;

			int surfacepropindex;

			int keyvalueindex;
			int keyvaluesize;

			int numlocalikautoplaylocks;
			int localikautoplaylockindex;

			float mass;
			int contents;

			// unused for packed models
			// technically still functional though I am unsure why you'd want to use it
			int numincludemodels;
			int includemodelindex;

			int /* mutable void* */ virtualModel;

			int bonetablebynameindex;

			// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
			// this value is used to calculate directional components of lighting 
			// on static props
			char constdirectionallightdot;

			// set during load of mdl data to track *desired* lod configuration (not actual)
			// the *actual* clamped root lod is found in studiohwdata
			// this is stored here as a global store to ensure the staged loading matches the rendering
			char rootLOD;

			// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
			// to be set as root LOD:
			//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
			//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
			char numAllowedRootLODs;

			char unused;

			float defaultFadeDist;	// set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
								// player/titan models seem to inherit this value from the first model loaded in menus.
								// works oddly on entities, probably only meant for static props

			float gathersize; // what. from r5r struct. no clue what this does, seemingly for early versions of apex bsp but stripped in release apex (season 0)
							  // bad name, frustum culling

			int deprecated_numflexcontrollerui;
			int deprecated_flexcontrolleruiindex;

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			// this is in most shipped models, probably part of their asset bakery.
			// doesn't actually need to be written pretty sure, only four chars when not present.
			// this is not completely true as some models simply have nothing, such as animation models.
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// unsure what this is for but it exists for jigglbones
			int numprocbones;
			int procbonetableindex;
			int linearprocboneindex;

			// depreciated as they are removed in 12.1
			int deprecated_m_nBoneFlexDriverCount;
			int deprecated_m_nBoneFlexDriverIndex;

			int deprecated_m_nPerTriAABBIndex;
			int deprecated_m_nPerTriAABBNodeCount;
			int deprecated_m_nPerTriAABBLeafCount;
			int deprecated_m_nPerTriAABBVertCount;

			// always "" or "Titan"
			int unkstringindex;

			// this is now used for combined files in rpak, vtx, vvd, and vvc are all combined while vphy is separate.
			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			// as of s2/3 these are no unused except phy
			int vtxindex; // VTX
			int vvdindex; // VVD / IDSV
			int vvcindex; // VVC / IDCV 
			int vphyindex; // VPHY / IVPS

			int vtxsize;
			int vvdsize;
			int vvcsize;
			int vphysize; // still used in models using vg

			// unused in apex, gets cut in 12.1
			int deprecated_unkindex; // deprecated_imposterIndex
			int deprecated_numunk; // deprecated_numImposters

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			int numBoneFollowers; // numBoneFollowers
			int boneFollowerIndex;

			// BVH4 size (?)
			Vector mins;
			Vector maxs; // seem to be the same as hull size

			int unk3_v54[3];

			int bvh4index; // bvh4 tree

			short unk4_v54[2]; // same as unk3_v54_v121, 2nd might be base for other offsets? these are related to bvh4 stuff

			// new in apex vertex weight file for verts that have more than three weights
			// vvw is a 'fake' extension name, we do not know the proper name.
			int vvwindex; // index will come last after other vertex files
			int vvwsize;
		};
	}

	namespace v121
	{
		union mstudioanimvalue_t
		{
			struct
			{
				char	valid;
				char	total;
			} num;
			short		value;
		};

		struct mstudio_rle_anim_t
		{
			short size : 13; // total size of all animation data, not next offset because even the last one has it
			short flags : 3;
		};

		struct mstudioanimsections_t
		{
			int animindex;
			int isExternal; // 0 or 1, if 1 section is not in rseq (I think)
		};

		// sequence and autolayer flags
		#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
		#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
		#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
		#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
		#define STUDIO_POST		0x0010		// 
		#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
		#define STUDIO_ANIM_UNK 0x0040		// this gets used in apex and frameanims are not a thing anymore.
		#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
		#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
		#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
		#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
		#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
		#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
		#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index on server
		#define STUDIO_WORLD	0x4000		// sequence blends in worldspace
		#define STUDIO_NOFORCELOOP 0x8000	// do not force the animation loop
		#define STUDIO_EVENT_CLIENT 0x10000	// Has been updated at runtime to event index on client
		#define STUDIO_ANIM_UNK1		0x20000 // animation requires this for data to be read
		#define STUDIO_FRAMEMOVEMENT    0x40000 // framemovements are only read if this flag is present
		#define STUDIO_ANIM_UNK2	    0x80000 // cherry blossom v53, levi in v54

		struct mstudioanimdesc_t
		{
			int baseptr;

			int sznameindex;

			float fps; // frames per second	
			int flags; // looping/non-looping flags

			int numframes;

			// piecewise movement
			int nummovements;
			int movementindex;

			int framemovementindex; // new in v52

			int animindex; // non-zero when anim data isn't in sections
			char* pAnim(int* piFrame) const; // returns pointer to data and new frame index

			int numikrules;
			int ikruleindex; // non-zero when IK data is stored in the mdl

			int sectionindex;
			int sectionstaticframes; // number of static frames inside the animation, the reset excluding the final frame are stored externally. when external data is not loaded(?)/found(?) it falls back on the last frame of this as a stall
			int sectionframes; // number of frames used in each fast lookup section, zero if not used
			inline const mstudioanimsections_t* pSection(int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }

			// both of these are used in pAnim
			__int64 unk1;
			__int64 unk2; // gets converted to an offset on load?
		};
	}

	// technically not v54 anymore
	namespace v16
	{
		struct mstudioanimdesc_t
		{
			float fps; // frames per second	
			int flags; // looping/non-looping flags

			int numframes;

			unsigned short sznameindex;

			unsigned short framemovementindex; // new in v52

			int animindex; // non-zero when anim data isn't in sections

			unsigned short numikrules;
			unsigned short ikruleindex; // non-zero when IK data is stored in the mdl

			__int64 unk2;
			unsigned short unk1;

			unsigned short sectionindex;
			unsigned short sectionstaticframes; // number of static frames inside the animation, the reset excluding the final frame are stored externally. when external data is not loaded(?)/found(?) it falls back on the last frame of this as a stall
			unsigned short sectionframes; // number of frames used in each fast lookup section, zero if not used
		};

		struct mstudioseqdesc_t
		{
			short szlabelindex;

			short szactivitynameindex;

			int flags; // looping/non-looping flags

			short activity; // initialized at loadtime to game DLL values
			short actweight;

			short numevents;
			short eventindex;

			Vector bbmin; // per sequence bounding box
			Vector bbmax;

			short numblends;

			// Index into array of shorts which is groupsize[0] x groupsize[1] in length
			short animindexindex;

			//short movementindex; // [blend] float array for blended movement
			short paramindex[2]; // X, Y, Z, XR, YR, ZR
			float paramstart[2]; // local (0..1) starting value
			float paramend[2]; // local (0..1) ending value
			//short paramparent;

			float fadeintime; // ideal cross fate in time (0.2 default)
			float fadeouttime; // ideal cross fade out time (0.2 default)

			char fill[4];

			// stuff is different after this
			/*
			short localentrynode; // transition node at entry
			short localexitnode; // transition node at exit
			short nodeflags; // transition rules

			float entryphase; // used to match entry gait
			float exitphase; // used to match exit gait

			float lastframe; // frame that should generation EndOfSequence

			short nextseq; // auto advancing sequences
			short pose; // index of delta animation between end and nextseq*/

			short numikrules;

			short numautolayers;
			unsigned short autolayerindex;

			unsigned short weightlistindex;

			char groupsize[2];

			unsigned short posekeyindex;

			short numiklocks;
			short iklockindex;

			// Key values
			unsigned short keyvalueindex;
			short keyvaluesize;

			//short cycleposeindex; // index of pose parameter to use as cycle index

			short activitymodifierindex;
			short numactivitymodifiers;

			int ikResetMask; // new in v52
			int unk1;

			unsigned short unkindex;
			short unkcount;
		};

		struct mstudiobbox_t
		{
			short bone;
			short group; // intersection group

			Vector bbmin; // bounding box
			Vector bbmax;

			unsigned short szhitboxnameindex; // offset to the name of the hitbox.

			unsigned short keyvalueindex; // used for keyvalues, most for titans.
		};

		struct studiohdr_t
		{
			int flags;
			int checksum; // unsure if this is still checksum, there isn't any other files that have it still
			unsigned short sznameindex; // No longer stored in string block, uses string in header.
			char name[32]; // The internal name of the model, padding with null chars.
						   // Typically "my_model.mdl" will have an internal name of "my_model"
			char unk_v16;

			char surfacepropLookup; // saved in the file

			float mass;

			int version; // time will tell

			unsigned short hitboxsetindex;
			char numhitboxsets;

			char illumpositionattachmentindex;

			Vector illumposition;	// illumination center

			Vector hull_min;		// ideal movement hull size
			Vector hull_max;

			Vector view_bbmin;		// clipping bounding box
			Vector view_bbmax;

			unsigned short numbones; // bones
			unsigned short boneindex;
			unsigned short bonedataindex;

			unsigned short numlocalseq; // sequences
			unsigned short localseqindex;

			//char unkfill[5];

			// needs to be confirmed
			unsigned short unk_v54_v14[2]; // added in v13 -> v14

			// needs to be confirmed
			char activitylistversion; // initialization flag - have the sequences been indexed?

			char numlocalattachments;
			unsigned short localattachmentindex;

			unsigned short numlocalnodes;
			unsigned short localnodenameindex;
			unsigned short nodedataindexindex;

			unsigned short numikchains;
			unsigned short ikchainindex;

			unsigned short numtextures; // the material limit exceeds 128, probably 256.
			unsigned short textureindex;

			// replaceable textures tables
			unsigned short numskinref;
			unsigned short numskinfamilies;
			unsigned short skinindex;

			short numbodyparts;
			unsigned short bodypartindex;

			// this is rui meshes
			unsigned short numruimeshes;
			unsigned short ruimeshindex;

			unsigned short numlocalposeparameters;
			unsigned short localposeparamindex;

			unsigned short surfacepropindex;

			unsigned short keyvalueindex;

			// vg stuff 1
			unsigned short numVGMeshes; // total number of meshes, not including LODs
			unsigned short vgMeshIndex;

			unsigned short bonetablebynameindex;

			// vg stuff 2
			unsigned short boneStateIndex;
			unsigned short numBoneStates;

			unsigned short vgHeaderIndex;
			unsigned short numVGHeaders;

			unsigned short vgLODIndex;
			unsigned short numVGLODs;

			float fadeDistance;

			float gathersize; // what. from r5r struct

			unsigned short numsrcbonetransform;
			unsigned short srcbonetransformindex;

			// asset bakery strings if it has any
			unsigned short sourceFilenameOffset;

			unsigned short linearboneindex;

			// unsure what this is for but it exists for jigglbones
			unsigned short numprocbonesunk;
			unsigned short procbonearrayindex;
			unsigned short procbonemaskindex;

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			unsigned short numBoneFollowers; // numBoneFollowers
			unsigned short boneFollowerIndex;

			unsigned short bvh4index; // same as v54

			char unk5_v16; // unk4_v54[0]
			char unk6_v16; // unk4_v54[1]
			short unk7_v16; // unk4_v54[2]
			short unk8_v16;
			short unk9_v16;

			//unsigned short unkshorts[7];
		};
	}
}
