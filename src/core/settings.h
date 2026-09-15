#pragma once

#include <pch.h>

// export format
enum eModelExportFormat : uint32_t
{
	MODEL_FMT_RMAX = 0, // Respawn Model Asset eXport
	MODEL_FMT_DMX = 1,
	MODEL_FMT_SMD = 2,

	MODEL_FMT_COUNT,

	MODEL_FMT_INVALID = 0xFFFFFFFF,
};

constexpr const char* const s_ExportFormatNames[eModelExportFormat::MODEL_FMT_COUNT]
{
	"rmax",
	"dmx",
	"smd",
};

constexpr const char* const s_ExportFormatExtensions[eModelExportFormat::MODEL_FMT_COUNT]
{
	".rmax",
	".dmx",
	".smd",
};

// command line settings
enum eCmdArgs : int
{
	CMD_EXTRACT,
	CMD_OUTPATH,
	CMD_FORMAT,
	CMD_VERSION,

	CMD_TRUNCATEMATERIALS,
	CMD_UPAXIS,
	CMD_IGNOREMOTION,
	CMD_IGNOREMESH,
	CMD_IGNOREANIM,
	CMD_MERGEUIVERTS,

	CMD_QC_VERSION,
	CMD_QC_WRITE,
	CMD_QC_INCLUDES,
	CMD_QC_TRIMSKINS,

	CMD_SMD_VERSION,
	CMD_DMX_VERSION,

	_CMD_COUNT,
};

static const char* s_CommandArgs[eCmdArgs::_CMD_COUNT]
{
	"-extract",				// extracts a provided model file
	"-outpath",				// path to write any files into
	"-format",				// desired extraction format
	"-version",				// input version of model

	"-truncate_materials",	// truncate any material paths (thanks blender)
	"-upaxis",				// the upaxis for dmx
	"-ignoremotion",		// ignores motion track for the root bone
	"-ignoremesh",			// ignores mesh data
	"-ignoreanim",			// ignores anim data
	"-mergeuiverts",		// set ui vertex merging to be aggressive

	"-qc_version",			// target version for qc
	"-qc_write",			// a qc file will be written
	"-qc_use_includes",		// qc will be split into include files
	"-qc_use_trim_skins",	// qc texturegroup will have the skin indices trimmed down

	"-smd_version",			// version for the smd
	"-dmx_version",			// dmx encoding and format
};

/*
-extract:
	provides a path to a model, or directory with models in them.
-outpath:
	provides a path to a unique output directory
-format:
	rmax 0, dmx 1, smd 2
-version:
	sets the version to be used on files it cannot be determined from (apex legends)
-truncate_materials:
	truncate material paths if used
-upaxis:
	sets the dmx up axis
-ignoremotion:
	does not apply motion track to root bone of animations
-ignoremesh:
	skip mesh data parsing
-ignoreanim:
	skip anim data parsing
-mergeuiverts:
	merge down ui panel vertices more aggressively

-qc_version:
	sets the target version for qc files, should be in format "%hu %hu", "%hu" works but might be undefined behaviour
-qc_write:
	qc file will be exported
-qc_use_includes:
	makes qc export with include (qci) files
-qc_use_trim_skins:
	texture group will be trimmed to only changed materials

-smd_version:
	1 to 3, sets the feature set for smd
-dmx_version:
	sets the feature set and formating on dmx, this should follow exactly as what is in a dmx file header
*/

// settings from the command line
struct CmdSettings_t
{
	CmdSettings_t() : dmxVersion(DmVersion_t::DMX_ENCODE_BINARY, DmVersion_t::DMX_FORMAT_MODEL, 5, 18)
	{
		outpath = nullptr;
		version = nullptr;
		
		format = eModelExportFormat::MODEL_FMT_DMX;
		truncateMaterials = false;
		ignoreMotion = false;
		ignoreMesh = false;
		ignoreAnim = false;
		mergeUIVerts = false;

		qcVersion = qc::s_QCVersion_P2;
		qcWrite = false;
		qcUseIncludes = false;
		qcUseTrimmedSkins = false;

		smdVersion = smd::SMD_V1;

		dmxVersionSource = nullptr;
	}

	// todo: bitfields for bools?

	// general params
	const char* outpath;	// where to dump output
	const char* version;	// what version is the model
	uint32_t format;
	bool truncateMaterials;
	bool ignoreMotion;		// ignore motion extracted from root bone
	bool ignoreMesh;
	bool ignoreAnim;
	uint8_t mergeUIVerts;		// will merge down ui panel vertices with a much larger margin

	// qc
	bool qcWrite;
	bool qcUseIncludes;
	bool qcUseTrimmedSkins;
	qc::Version_t qcVersion;

	// smd
	int smdVersion;

	// dmx
	DmVersion_t dmxVersion;
	const char* dmxVersionSource;
};