#pragma once

extern CmdSettings_t g_CmdSettings;

struct IModelWeightList
{
	IModelWeightList() = default;
	IModelWeightList(const float* const listWeights) : name(nullptr), weights(listWeights), useCount(1), markedAsDefault(true) {}
	IModelWeightList(const char* const listName, const float* const listWeights) : name(listName), weights(listWeights), useCount(1), markedAsDefault(false)
	{

	}

	const char* name;
	const float* weights;
	int useCount;
	bool markedAsDefault;

	static void NameFromIndex(char* const buf, const size_t size, const uint32_t index)
	{
		snprintf(buf, size, "weightlist_%u", index);
	}
};

static const float s_IModelDefaultWeightList[256] = {
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
};

class IModelExporter
{
public:
	IModelExporter(IModel* const imodel, const std::filesystem::path& dir, const std::string& name) : base(imodel), exportDir(dir), exportPath(dir), exportName(name) {};
	~IModelExporter() {};

	static constexpr uint32_t maxFileSize = 32 * 1024 * 1024;

	inline const IModel* const Model() const { return base; }
	inline const std::filesystem::path& Path() const { return exportPath; }
	inline const std::string& Name() const { return exportName; }
	
	// update path in our file if needed
	inline void UpdatePathFile(void* const file)
	{
		switch (g_CmdSettings.format)
		{
		case MODEL_FMT_RMAX:
		{
			rmax::RMAXExporter* const rmax = reinterpret_cast<rmax::RMAXExporter* const>(file);
			rmax->SetPath(exportPath);

			break;
		}
		case MODEL_FMT_DMX:
		{
			break;
		}
		case MODEL_FMT_SMD:
		{
			smd::CStudioModelData* const smd = reinterpret_cast<smd::CStudioModelData* const>(file);
			smd->SetPath(exportPath);

			break;
		}
		default:
		{
			break;
		}
		}
	}

	// make our export path for qc exporting, adds name to file directory
	inline void UpdatePathNamed()
	{
		exportPath = exportDir;
		exportPath /= exportName;

		if (!CreateDirectories(exportPath.parent_path()))
		{
			assertm(false, "failed to create directories");
		}
	}

	// make our export path for physics & other exporting
	inline void UpdatePathRoot(void* const file)
	{
		exportPath = exportDir;

		if (!CreateDirectories(exportPath.parent_path()))
		{
			assertm(false, "failed to create directories");
		}

		UpdatePathFile(file);
	}

	// make our export path for model exporting
	inline void UpdatePathModel(void* const file)
	{
		exportPath = exportDir;
		exportPath /= s_ExportFormatNames[g_CmdSettings.format];

		if (!CreateDirectories(exportPath))
		{
			assertm(false, "failed to create directories");
		}

		UpdatePathFile(file);
	}

	// make our export path for model exporting
	inline void UpdatePathAnim(void* const file)
	{
		exportPath = exportDir;

		std::string tmp("anims_");
		tmp.append(Name());
		exportPath /= tmp;

		if (!CreateDirectories(exportPath))
		{
			assertm(false, "failed to create directories");
		}

		UpdatePathFile(file);
	}

	// sort through stored IModel and export it
	bool ParseForExport();

private:
	std::filesystem::path exportDir;
	std::filesystem::path exportPath;
	std::string exportName;
	const IModel* base;

	void AdjustAnimationOrigin(const IModelAnimation* const anim, const IModelMovementTrack* const moveTrack, const int frameIdx, Vector& pos, Quaternion& quat, const uint32_t bone); // adjust our origin bone
	bool WriteToFile(const char* const fileBuf, const char* const fileName, const int fileSize);
	void FormNameAnim(char* const buf, const size_t size, const char* const name);

	// export an IModelModel based on input format (stored in this class)
	const bool ExportIModel(const eModelExportFormat format, void* const file);

	typedef const bool (IModelExporter::* ExportModelFunc_t)(const IModelModel* const, void* const);
	typedef const bool (IModelExporter::* ExportAnimFunc_t)(const IModelAnimation* const, void* const);

	// rmax
	void ExportSkeletonRMAX(rmax::RMAXExporter* const rmax);
	const bool ExportModelRMAX(const IModelModel* const model, void* const file);
	const bool ExportAnimRMAX(const IModelAnimation* const anim, void* const file);

	// dmx
	void ExportSkeletonDMX(CDataModel* const dmx);
	void ImportSkeletonDMX(CDataModel* const dmxSource, CDataModel* const dmxDest);
	const bool ExportModelDMX(const IModelModel* const model, void* const file);
	const bool ExportAnimDMX(const IModelAnimation* const anim, void* const file);

	// smd
	const bool ExportModelSMD(const IModelModel* const model, void* const file);
	const bool ExportAnimSMD(const IModelAnimation* const anim, void* const file);

	// qc
	void ParseQC_General(qc::QCFile* const file, const IModel* const imodel);
	void ParseQC_Bone(qc::QCFile* const file, const IModel* const imodel, const IModelBone* const bone);
	void ParseQC_Bodypart(qc::QCFile* const file, const IModel* const imodel, const IModelBodypart* const bodypart, uint32_t& qcMaxVerts);
	void ParseQC_LOD(qc::QCFile* const file, const IModel* const imodel, const uint32_t lod, const bool isShadowLOD, uint32_t& qcMaxVerts);
	void ParseQC_Attachments(qc::QCFile* const file, const IModel* const imodel);
	void ParseQC_Boxes(qc::QCFile* const file, const IModel* const imodel);
	void ParseQC_AnimationTypes(qc::QCFile* const file, const IModel* const imodel, int& animBlockSize);
	void ParseQC_Animation_OptionsFromFlags(qc::QCFile* const file, const IModel* const imodel, const IModelAnimFlags_t flags, qc::AnimationOptions_t* const animOptions);
	void ParseQC_Animation_IKRules(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim, qc::AnimationData_t* const animData, qc::AnimationOptions_t* const animOptions);
	void ParseQC_Animation_Movement(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim, qc::AnimationData_t* const animData);
	void ParseQC_Animation_Sections(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim, int& maxSectionFrames, int& minFramesForSection, int& numZeroFrames, bool& noStallFrames, bool& useLowRes);
	void ParseQC_WeightList(qc::QCFile* const file, const IModel* const imodel, IModelWeightList* const weightLists, uint32_t* const listForSequence);
	void ParseQC_Animation(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim);
	void ParseQC_Sequence(qc::QCFile* const file, const IModel* const imodel, const IModelSequence* const seq, const char* const weightlist);
	const bool ExportQC();

	ExportModelFunc_t s_ExportModelFuncs[eModelExportFormat::MODEL_FMT_COUNT] =
	{
		&IModelExporter::ExportModelRMAX,
		&IModelExporter::ExportModelDMX,
		&IModelExporter::ExportModelSMD,
	};

	ExportAnimFunc_t s_ExportAnimFuncs[eModelExportFormat::MODEL_FMT_COUNT] =
	{
		&IModelExporter::ExportAnimRMAX,
		&IModelExporter::ExportAnimDMX,
		&IModelExporter::ExportAnimSMD,
	};
};

