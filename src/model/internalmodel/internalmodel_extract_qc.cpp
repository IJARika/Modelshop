#include <pch.h>

#include <model/internalmodel/internalmodel.h>
#include <model/internalmodel/internalmodel_extract.h>

extern CmdSettings_t g_CmdSettings;
extern CBufferManager g_BufferManager;


//
// QC
//

const uint32_t ParseQC_GetSkinIndices(const uint32_t numSkinRef, int16_t* const indices)
{
	for (uint32_t i = 0u; i < numSkinRef; i++)
	{
		indices[i] = static_cast<int16_t>(i);
	}

	return numSkinRef;
}

const uint32_t ParseQC_GetUsedSkinIndices(const int16_t* const skins, const int numSkinRef, const int numSkinFamilies, int16_t* const indices)
{
	// [rika]: no actual skins, just base materials
	assertm(numSkinFamilies > 1, "model had no additional skins");

	memset(indices, 0xff, sizeof(int16_t) * numSkinRef);

	const int16_t* skinGroup = skins + numSkinRef;

	// [rika]: parse through each skingroup to see which indices changed
	// todo: setting to skip this and write all materials
	for (int family = 0; family < numSkinFamilies - 1; family++)
	{
		for (int material = 0; material < numSkinRef; material++)
		{
			if (skins[material] == skinGroup[material])
				continue;

			indices[material] = static_cast<int16_t>(material);
		}

		skinGroup += numSkinRef;
	}

	// [rika]: make our indices sequential
	uint32_t curIdx = 0;
	for (int i = 0; i < numSkinRef; i++)
	{
		if (indices[i] == -1)
			continue;

		indices[curIdx] = indices[i];
		curIdx++;
	}

	return curIdx;
}

void IModelExporter::ParseQC_General(qc::QCFile* const file, const IModel* const imodel)
{
	using namespace qc;

	const int contents = imodel->GetContents();
	CmdParse(file, QC_MODELNAME, imodel->GetName());
	CmdParse(file, QC_CONTENTS, &contents);
	CmdParse(file, QC_SURFACEPROP, imodel->GetSurfaceProp());

	// [rika]: flag based options
	const IModelSourceFlags_t flags = imodel->GetFlags();
	if (flags & IMODEL_FLAG_STATIC_PROP)
	{
		CmdParse(file, QC_STATICPROP, nullptr);
	}

	if (flags & IMODEL_FLAG_FORCE_OPAQUE)
	{
		assertm(!(flags & IMODEL_FLAG_TRANSLUCENT_TWOPASS), "translucent and opaque");
		CmdParse(file, QC_OPAQUE, nullptr);
	}

	// removed in r1, parsed from materials. flag replaced in r5, used by arms and loading models a lot!
	if (flags & IMODEL_FLAG_TRANSLUCENT_TWOPASS)
	{
		assertm(!(flags & IMODEL_FLAG_FORCE_OPAQUE), "translucent and opaque");
		CmdParse(file, QC_MOSTLYOPAQUE, nullptr);
	}

	if (flags & IMODEL_FLAG_OBSOLETE)
	{
		CmdParse(file, QC_OBSOLETE, nullptr);
	}

	if (flags & IMODEL_FLAG_NO_FORCED_FADE)
	{
		CmdParse(file, QC_NOFORCEDFADE, nullptr);
	}

	if (flags & IMODEL_FLAG_FORCE_PHONEME_CROSSFADE)
	{
		CmdParse(file, QC_FORCEPHONEMECROSSFADE, nullptr);
	}

	if (flags & IMODEL_FLAG_CONSTANT_DIRECTIONAL_LIGHT_DOT)
	{
		const uint8_t constDirectionalLightDot = imodel->GetConstDirectionalLightDot();
		CmdParse(file, QC_CONSTANTDIRECTIONALLIGHT, &constDirectionalLightDot);
	}

	if (flags & IMODEL_FLAG_USES_EXTRA_BONE_WEIGHTS)
	{
		CmdParse(file, QC_USEDETAILEDWEIGHTS, nullptr);
	}

	// believe this is different later seasons 
	if (flags & IMODEL_FLAG_AMBIENT_BOOST)
	{
		CmdParse(file, QC_AMBIENTBOOST, nullptr);
	}

	if (flags & IMODEL_FLAG_DO_NOT_CAST_SHADOWS)
	{
		CmdParse(file, QC_DONOTCASTSHADOWS, nullptr);
	}

	// believe this is different later seasons (used on non static props)
	if (flags & IMODEL_FLAG_CAST_TEXTURE_SHADOWS)
	{
		CmdParse(file, QC_CASTTEXTURESHADOWS, nullptr);
	}

	if (flags & IMODEL_FLAG_SUBDIVISION_SURFACE)
	{
		CmdParse(file, QC_SUBD, nullptr);
	}

	if (flags & IMODEL_FLAG_USES_VERTEX_COLOR)
	{
		CmdParse(file, QC_USEVERTEXCOLOR, nullptr);
	}

	if (flags & IMODEL_FLAG_USES_UV2)
	{
		CmdParse(file, QC_USEEXTRATEXCOORD, nullptr);
	}

	// never fade (-1.0f) should be the default
	const float fadeDistance = imodel->GetFadeDistance();
	if (fadeDistance != -1.0f)
	{
		CmdParse(file, QC_FADEDISTANCE, &fadeDistance);
	}

	CmdParse(file, QC_EYEPOSITION, imodel->GetEyePos(), 3);

	const float flMaxEyeDeflection = imodel->GetMaxEyeDeflection();
	if (flMaxEyeDeflection != 0.0f)
	{
		CmdParse(file, QC_MAXEYEDEFLECTION, &flMaxEyeDeflection);
	}

	const uint32_t numMaterials = imodel->GetMaterialCount();
	if (numMaterials)
	{
		const char** materials = new const char* [numMaterials] {};
		const char** materialFullPaths = nullptr;

		// if we are going to rename materials
		if (g_CmdSettings.truncateMaterials)
		{
			materialFullPaths = new const char* [numMaterials] {};
		}

		for (uint32_t i = 0u; i < numMaterials; i++)
		{
			materials[i] = imodel->GetMaterial(i)->GetName();

			// are the materials getting shortened ?
			if (g_CmdSettings.truncateMaterials == false)
				continue;

			assertm(materialFullPaths, "material path array was not allocated");

			materialFullPaths[i] = materials[i];
			materials[i] = keepAfterLastSlashOrBackslash(materials[i]);
		}

		if (imodel->GetSkinCount() > 1u)
		{
			const uint32_t numSkinRef = numMaterials;
			const uint32_t numSkinFamilies = imodel->GetSkinCount();

			int16_t* const skins = new int16_t[numSkinRef * numSkinFamilies]{};
			int16_t* const indices = new int16_t[numSkinRef]{};
			const char** names = new const char* [numSkinFamilies] {};

			const size_t skinFamilySize = sizeof(int16_t) * numSkinRef;
			for (uint32_t i = 0u; i < numSkinFamilies; i++)
			{
				const IModelSkin* const skin = imodel->GetSkin(i);

				names[i] = skin->GetName();

				memcpy_s(skins + (numSkinRef * i), skinFamilySize, skin->GetIndices(), skinFamilySize);
			}

			const uint32_t usedIndices = g_CmdSettings.qcUseTrimmedSkins ? ParseQC_GetUsedSkinIndices(skins, numSkinRef, numSkinFamilies, indices) : ParseQC_GetSkinIndices(numSkinRef, indices);

			const TextureGroupData_t texturegroup(materials, skins, indices, numSkinRef, numSkinFamilies, usedIndices, names);
			CmdParse(file, QC_TEXTUREGROUP, &texturegroup);

			FreeAllocArray(skins);
			FreeAllocArray(indices);
			FreeAllocArray(names);
		}

		// check for collision
		if (g_CmdSettings.truncateMaterials)
		{
			for (uint32_t i = 0u; i < numMaterials; i++)
			{
				const CommandOptionPair_t renameData(materials[i], materialFullPaths[i]);
				CmdParse(file, QC_RENAMEMATERIAL, &renameData);
			}
		}

		FreeAllocArray(materials);
		FreeAllocArray(materialFullPaths);
	}

	// [rika]: most models will have at least one (unless it's retail apex), it will be the path prefixing 'mdl' or 'models', in most cases this is just an empty string
	for (uint32_t i = 0u; i < imodel->GetCDTextureCount(); i++)
	{
		CmdParse(file, QC_CDMATERIALS, imodel->GetCDTexture(i));
	}

	if (imodel->KeyValueText())
	{
		CmdParse(file, QC_KEYVALUES, imodel->KeyValueText());
	}

	const uint8_t numAllowedRootLODs = imodel->GetNumAllowedRootLOD();
	if (numAllowedRootLODs > 0)
	{
		CmdParse(file, QC_ALLOWROOTLODS, &numAllowedRootLODs);
	}

	const uint8_t rootLOD = imodel->GetRootLOD();
	if (rootLOD > 0)
	{
		CmdParse(file, QC_MINLOD, &rootLOD);
	}
}

#define SET_QC_BONE_FLAG(src, flagSrc, qc, flagQC) if (src & flagSrc) { qc |= flagQC;}
void IModelExporter::ParseQC_Bone(qc::QCFile* const file, const IModel* const imodel, const IModelBone* const bone)
{
	using namespace qc;

	// [rika]: parse out the data required for a $definebone (and other) command(s)
	const char* const parent = bone->GetParent() > -1 ? imodel->GetBone(bone->GetParent())->GetName() : nullptr;
	const matrix3x4_t* pPostTransform = nullptr;

	// [rika]: get data for fixups if it exits
	if (imodel->GetBoneTransformCount())
	{
		const IModelBoneTransform* const pBoneTransform = IModelBoneTransform::GetSrcBoneTransform(bone->GetName(), imodel->GetBoneTransform(0), imodel->GetBoneTransformCount());

		if (pBoneTransform)
		{
			pPostTransform = pBoneTransform->GetPostTransform();
		}
	}

	const int flags = bone->GetFlags();
	int qcBoneFlags = 0;

	SET_QC_BONE_FLAG(flags, BONE_SCREEN_ALIGN_SPHERE, qcBoneFlags, BoneData_t::QCB_SCREEN_ALIGN_SPHERE);
	SET_QC_BONE_FLAG(flags, BONE_SCREEN_ALIGN_CYLINDER, qcBoneFlags, BoneData_t::QCB_SCREEN_ALIGN_CYLINDER);
	SET_QC_BONE_FLAG(flags, BONE_HAS_SAVEFRAME_POS, qcBoneFlags, BoneData_t::QCB_HAS_SAVEFRAME_POS);
	SET_QC_BONE_FLAG(flags, BONE_HAS_SAVEFRAME_ROT64, qcBoneFlags, BoneData_t::QCB_HAS_SAVEFRAME_ROT64);
	SET_QC_BONE_FLAG(flags, BONE_HAS_SAVEFRAME_ROT32, qcBoneFlags, BoneData_t::QCB_HAS_SAVEFRAME_ROT32);

	const BoneData_t boneData(bone->GetName(), parent/*, bone->pszSurfaceProp()*/, bone->GetContents(), qcBoneFlags, &bone->GetPos(), &bone->GetRot(), pPostTransform);
	const CommandOptionPair_t boneSurface(bone->GetName(), bone->GetSurfaceProp());

	CmdParse(file, QC_DEFINEBONE, &boneData);

	// THESE NEED NAMES, save data in DefineBoneData_t and pass to these funcs
	CmdParse(file, QC_JOINTCONTENTS, &boneData);
	CmdParse(file, QC_JOINTSURFACEPROP, &boneSurface);

	// TODO: IMODELBONE_FLAGS
	if (flags & BONE_USED_BY_BONE_MERGE)
	{
		CmdParse(file, QC_BONEMERGE, bone->GetName());
	}

	if (flags & (BONE_SCREEN_ALIGN_SPHERE | BONE_SCREEN_ALIGN_CYLINDER))
	{
		CmdParse(file, QC_SCREENALIGN, &boneData, 1, true);
	}

	if (flags & BONE_FIXED_ALIGNMENT)
	{
		CmdParse(file, QC_LIMITROTATION, bone->GetName());
	}

	if (flags & (BONE_HAS_SAVEFRAME_POS | BONE_HAS_SAVEFRAME_ROT64 | BONE_HAS_SAVEFRAME_ROT32))
	{
		CmdParse(file, QC_BONESAVEFRAME, &boneData, 1, true);
	}

	// todo: 
	switch (bone->GetProcType())
	{
	case eProcBoneType::NONE:
	{
		// do nothing
		break;
	}
	case eProcBoneType::JIGGLE:
	{
		const IModelJiggleBone* const jigglebone = bone->GetProcBone<IModelJiggleBone>();

		JiggleData_t jiggleData(jigglebone->GetName(), jigglebone->flags);

		// todo: just do this in the constructor (I was lazy tonight)
		if (jigglebone->IsApexLegends())
		{
			jiggleData.FixFlagsApex(); // not ideal but will work for now
		}

		jiggleData.SetGeneral(jigglebone->length, jigglebone->tipMass, jigglebone->tipFriction);
		jiggleData.SetFlexible(jigglebone->yawStiffness, jigglebone->yawDamping, jigglebone->pitchStiffness, jigglebone->pitchDamping, jigglebone->alongStiffness, jigglebone->alongDamping);
		jiggleData.SetAngleConstraint(jigglebone->angleLimit);
		jiggleData.SetYawConstraint(jigglebone->minYaw, jigglebone->maxYaw, jigglebone->yawFriction, jigglebone->yawBounce);
		jiggleData.SetPitchConstraint(jigglebone->minPitch, jigglebone->maxPitch, jigglebone->pitchFriction, jigglebone->pitchBounce);
		jiggleData.SetBaseSpring(jigglebone->baseMass, jigglebone->baseStiffness, jigglebone->baseDamping, jigglebone->baseMinLeft, jigglebone->baseMaxLeft, jigglebone->baseLeftFriction,
			jigglebone->baseMinUp, jigglebone->baseMaxUp, jigglebone->baseUpFriction, jigglebone->baseMinForward, jigglebone->baseMaxForward, jigglebone->baseForwardFriction);

		CmdParse(file, QC_JIGGLEBONE, &jiggleData);

		break;
	}
	default:
	{
		assertm(false, "unsupported proc bone type");
		break;
	}
	}
}

constexpr const char* const s_QCModelNameFormat = "%s/%s%s\0"; // file name, model name, extension
void IModelExporter::ParseQC_Bodypart(qc::QCFile* const file, const IModel* const imodel, const IModelBodypart* const bodypart, uint32_t& qcMaxVerts)
{
	using namespace qc;

	const uint16_t numModels = bodypart->GetModelCount();
	if (numModels == 0u)
	{
		assertm(false, "invalid model count");
		return;
	}

	assertm(imodel->GetLODCount() != 0u, "model had bodyparts but no lods");
	const IModelLOD* const lod0 = imodel->GetLOD(0);

	char buf[MAX_PATH]{};

	// single model
	// handle $model (vertex anim) eventually
	if (numModels == 1u)
	{
		const IModelModel* const model = bodypart->GetModel(0u, lod0);

		snprintf(buf, MAX_PATH, s_QCModelNameFormat, s_ExportFormatNames[g_CmdSettings.format], model->GetName(), s_ExportFormatExtensions[g_CmdSettings.format]);

		const CommandOptionPair_t bodyData(bodypart->GetName(), buf);

		CmdParse(file, QC_BODY, &bodyData, 1, true); // save because of strings ?

		const uint32_t numModelVerts = model->GetVertexCount();
		qcMaxVerts = numModelVerts > qcMaxVerts ? numModelVerts : qcMaxVerts;

		return;
	}

	BodyGroupData_t bodyGroupData(bodypart->GetName(), numModels);

	for (uint16_t i = 0; i < numModels; i++)
	{
		const IModelModel* const model = bodypart->GetModel(i, lod0);

		if (model->GetMeshCount() == 0u)
		{
			bodyGroupData.SetBlank(i);
			continue;
		}

		snprintf(buf, MAX_PATH, s_QCModelNameFormat, s_ExportFormatNames[g_CmdSettings.format], model->GetName(), s_ExportFormatExtensions[g_CmdSettings.format]);

		bodyGroupData.SetPart(file, i, buf);

		const uint32_t numModelVerts = model->GetVertexCount();
		qcMaxVerts = numModelVerts > qcMaxVerts ? numModelVerts : qcMaxVerts;
	}

	CmdParse(file, QC_BODYGROUP, &bodyGroupData, 1, true); // save because of strings ?
}

// get the nearest parent used by the LOD
const uint32_t ParseQC_LODBone(const IModel* const imodel, const size_t lod, const uint32_t boneId)
{
	const IModelBone* const bone = imodel->GetBone(boneId);

	// bone is used by this lod, don't replace
	if (bone->GetFlags() & BONE_USED_BY_VERTEX_AT_LOD(lod))
	{
		return boneId;
	}

	// cannot be collasped further
	const int parent = bone->GetParent();
	if (parent == -1)
	{
		// Robots/super_spectre/super_spectre_v1.mdl
		//assertm(false, "bone with no parent unused by LOD");
		return boneId;
	}

	const uint32_t out = ParseQC_LODBone(imodel, lod, parent);
	return out;
}

void IModelExporter::ParseQC_LOD(qc::QCFile* const file, const IModel* const imodel, const uint32_t lod, const bool isShadowLOD, uint32_t& qcMaxVerts)
{
	using namespace qc;

	const IModelLOD* const lod0 = imodel->GetLOD(0u);
	const IModelLOD* const lodX = imodel->GetLOD(lod);

	assertm(lodX->GetModelCount() == lod0->GetModelCount(), "mismatched model count, very bad!");

	LodData_t lodGroupData(lodX->GetModelCount(), imodel->GetBoneCount(), lodX->GetMaterialReplacementCount(), lodX->GetSwitchPoint(), isShadowLOD);

	// models
	char bufBase[MAX_PATH]{};
	char bufReplace[MAX_PATH]{};

	for (uint16_t i = 0u; i < lodX->GetModelCount(); i++)
	{
		const IModelModel* const model0 = lod0->GetModel(i);
		const IModelModel* const modelX = lodX->GetModel(i);

		// blank model
		if (modelX->GetVertexCount() == 0u && model0->GetVertexCount() == 0u)
			continue;

		snprintf(bufBase, MAX_PATH, s_QCModelNameFormat, s_ExportFormatNames[g_CmdSettings.format], model0->GetName(), s_ExportFormatExtensions[g_CmdSettings.format]);

		if (modelX->GetVertexCount() == 0u && model0->GetVertexCount() > 0u)
		{
			lodGroupData.RemoveModel(file, i, bufBase);
			continue;
		}

		snprintf(bufReplace, MAX_PATH, s_QCModelNameFormat, s_ExportFormatNames[g_CmdSettings.format], modelX->GetName(), s_ExportFormatExtensions[g_CmdSettings.format]);

		lodGroupData.ReplaceModel(file, i, bufBase, bufReplace);

		const uint32_t numModelVerts = modelX->GetVertexCount();
		qcMaxVerts = numModelVerts > qcMaxVerts ? numModelVerts : qcMaxVerts;
	}

	// bones
	// [rika]: do we need to check for attachments ? I don't think we can remove a bone if it has attachments
	for (uint32_t i = 0u; i < imodel->GetBoneCount(); i++)
	{
		const uint32_t boneId = ParseQC_LODBone(imodel, lod, i);

		if (boneId == i)
			continue;

		const IModelBone* const pBoneChild = imodel->GetBone(i);
		const IModelBone* const pBoneParent = imodel->GetBone(boneId);

		lodGroupData.ReplaceBone(i, pBoneChild->GetName(), pBoneParent->GetName());
	}

	for (uint32_t i = 0; i < lodX->GetMaterialReplacementCount(); i++)
	{
		const IModelMaterialReplacement* const materialReplacement = lodX->GetMaterialReplacement(i);

		const char* const replacement = materialReplacement->GetReplacement();
		const char* const original = imodel->GetMaterial(materialReplacement->GetOriginal())->GetName();

		lodGroupData.ReplaceMaterial(i, original, replacement);
	}

	if (lodGroupData.isShadowLOD && (imodel->GetFlags() & IMODEL_FLAG_USE_SHADOWLOD_MATERIALS))
	{
		lodGroupData.useShadowLODMaterials = true;
	}

	const CommandList_t lodType = isShadowLOD ? QC_SHADOWLOD : QC_LOD;

	CmdParse(file, lodType, &lodGroupData);
}

void IModelExporter::ParseQC_Attachments(qc::QCFile* const file, const IModel* const imodel)
{
	using namespace qc;

	const bool staticProp = (imodel->GetFlags() & IMODEL_FLAG_STATIC_PROP);

	const bool useIllumAttachment = IllumPositionData_t::useAttachment(imodel->GetIllumPosAttachmentIndex());
	uint32_t illumpositionattachmentindex = imodel->GetIllumPosAttachmentIndex() - 1;

	bool useAutoCenter = false;
	//uint32_t autoCenterIndex = 0ull;

	const uint32_t numAttachments = imodel->GetAttachmentCount();
	for (uint32_t i = 0; i < numAttachments; i++)
	{
		// skip __illumPosition attachment
		if (useIllumAttachment && illumpositionattachmentindex == i)
			continue;

		const IModelAttachment* const attachment = imodel->GetAttachment(i);

		// should we skip this
		if (staticProp && !useAutoCenter && !strncmp("placementOrigin", attachment->GetName(), 16))
		{
			useAutoCenter = true;
			//autoCenterIndex = i;
		}

		// invalid index in apex (sometimes?), yippee!!!
		// rspn101 always +8
		if (useIllumAttachment && illumpositionattachmentindex >= numAttachments && !strncmp("__illumPosition", attachment->GetName(), 16))
		{
			illumpositionattachmentindex = i;
			continue;
		}

		const AttachmentData_t attachmentData(attachment->GetName(), imodel->GetBone(attachment->GetLocalBone())->GetName(), attachment->GetFlags(), attachment->GetLocal());

		CmdParse(file, QC_ATTACHMENT, &attachmentData);
	}

	// illumposition
	// [rika]: not a fan of this being done outside qc, but also don't wanna put out custom types into qc (supposed to be universial for [redacted])
	{
		const char* localbone = nullptr;
		const matrix3x4_t* localmatrix = nullptr;

		if (useIllumAttachment)
		{
			const IModelAttachment* const attachment = imodel->GetAttachment(illumpositionattachmentindex); // use fixed index incase it is invalid

			localbone = imodel->GetBone(attachment->GetLocalBone())->GetName();
			localmatrix = attachment->GetLocal();
		}

		const IllumPositionData_t illumdata(imodel->GetIllumPos(), imodel->GetIllumPosAttachmentIndex(), localbone, localmatrix);

		CmdParse(file, QC_ILLUMPOSITION, &illumdata);
	}

	if (useAutoCenter)
	{
		CmdParse(file, QC_AUTOCENTER, nullptr);
	}
}

void IModelExporter::ParseQC_Boxes(qc::QCFile* const file, const IModel* const imodel)
{
	using namespace qc;

	const bool autoGeneratedHitbox = (imodel->GetFlags() & IMODEL_FLAG_AUTOGENERATED_HITBOX);
	const CommandFormat_t cmdFmt = autoGeneratedHitbox ? QC_FMT_COMMENT : QC_FMT_NONE;
	bool skipBoneInBBox = false;

	const CommandOptionPair_t bboxData(imodel->GetHullMin()->Base(), imodel->GetHullMax()->Base(), 3, 3);
	CmdParse(file, QC_BBOX, &bboxData);

	const CommandOptionPair_t cboxData(imodel->GetBBMin()->Base(), imodel->GetBBMax()->Base(), 3, 3);
	CmdParse(file, QC_CBOX, &cboxData);

	// [rika]: it's important to parse these in order
	for (uint32_t i = 0; i < imodel->GetHitboxSetCount(); i++)
	{
		const IModelHitboxSet* const hitboxSet = imodel->GetHitboxSet(i);
		CmdParse(file, QC_HBOXSET, hitboxSet->GetName(), 1, false, cmdFmt);

		for (uint32_t boxIdx = 0; boxIdx < hitboxSet->GetHitboxCount(); boxIdx++)
		{
			const IModelHitbox* const hitbox = hitboxSet->GetHitbox(boxIdx);
			const char* const bone = imodel->GetBone(hitbox->GetBone())->GetName();

			const Hitbox_t hboxData(bone, hitbox->GetGroup(), hitbox->GetBBMin(), hitbox->GetBBMax(), hitbox->GetHitboxName(), hitbox->GetHitDataGroup(), hitbox->GetForceCritPoint());

			CmdParse(file, QC_HBOX, &hboxData, 1, false, cmdFmt);

			// hgroup
			if (autoGeneratedHitbox)
			{
				const CommandOptionPair_t hgroupData(hitbox->GetGroup(), bone);
				CmdParse(file, QC_HGROUP, &hgroupData);
			}

			if (autoGeneratedHitbox && !skipBoneInBBox)
			{
				const Vector& bbmin = *hitbox->GetBBMin();
				const Vector& bbmax = *hitbox->GetBBMax();

				for (uint32_t axis = 0; axis < 3; axis++)
				{
					skipBoneInBBox = bbmin[axis] > 0.0f || bbmax[axis] < 0.0f ? true : skipBoneInBBox;
				}
			}
		}
	}

	if (skipBoneInBBox)
	{
		CmdParse(file, QC_SKIPBONEINBBOX, nullptr);
	}
}

// assorted animation data outside of $animation and $sequence
void IModelExporter::ParseQC_AnimationTypes(qc::QCFile* const file, const IModel* const imodel, int& animBlockSize)
{
	using namespace qc;

	for (uint32_t i = 0u; i < imodel->GetPoseParameterCount(); i++)
	{
		const IModelPoseParameter* const poseParam = imodel->GetPoseParameter(i);
		const PoseParamData_t poseParamData(poseParam->GetName(), poseParam->GetFlags(), poseParam->GetStart(), poseParam->GetEnd(), poseParam->GetLoop());
		CmdParse(file, QC_POSEPARAMETER, &poseParamData);
	}

	for (uint32_t i = 0u; i < imodel->GetIKChainCount(); i++)
	{
		const IModelIKChain* const ikChain = imodel->GetIKChain(i);
		const IKChainData_t ikChainData(ikChain->GetName(), imodel->GetBone(ikChain->GetIKLink(IModelIKChain::IKLINK_FOOT)->GetBone())->GetName(), ikChain->GetIKLink(IModelIKChain::IKLINK_THIGH)->GetKneeDir(), ikChain->GetUnkAngle());
		CmdParse(file, QC_IKCHAIN, &ikChainData);
	}

	for (uint32_t i = 0u; i < imodel->localIkAutoPlayLockCount; i++)
	{
		const IModelIKLock* const ikLock = imodel->GetIKLock(i);
		assertm(ikLock->GetChain() < imodel->GetIKChainCount(), "invaild ik chain index");

		const IKLockData_t ikLockData(imodel->GetIKChain(ikLock->GetChain())->GetName(), ikLock->GetPosWeight(), ikLock->GetLocalQWeight());
		CmdParse(file, QC_IKAUTOPLAYLOCK, &ikLockData);
	}

	// nodes probably

	for (uint32_t i = 0u; i < imodel->includeModelCount; i++)
	{
		const IModelModelGroup* const includemodel = imodel->GetModelGroup(i);
		CmdParse(file, QC_INCLUDEMODEL, includemodel->GetName());
	}

	for (uint32_t i = 0u; i < imodel->GetAnimBlockCount(); i++)
	{
		const IModelAnimBlock* const animblock = imodel->GetAnimBlock(i);

		animBlockSize = animblock->GetSize() > animBlockSize ? animblock->GetSize() : animBlockSize;
	}
}

void IModelExporter::ParseQC_Animation_OptionsFromFlags(qc::QCFile* const file, const IModel* const imodel, const IModelAnimFlags_t flags, qc::AnimationOptions_t* const animOptions)
{
	UNUSED(file);
	UNUSED(imodel);

	// traditional
	if (flags & IMODELANIM_FLAG_LOOPING)
	{
		animOptions->SetLooping();
	}

	if (flags & IMODELANIM_FLAG_SNAP)
	{
		animOptions->SetSnap();
	}

	if (flags & IMODELANIM_FLAG_DELTA)
	{
		animOptions->SetDelta();
	}

	if (flags & IMODELANIM_FLAG_AUTOPLAY)
	{
		animOptions->SetAutoPlay();
	}

	// if STUDIO_POST can be set via commands, but those commands set other flags too, thankfully
	if (flags & IMODELANIM_FLAG_POST && ((IMODELANIM_FLAG_DELTA | IMODELANIM_FLAG_WORLD) & flags) == false)
	{
		animOptions->SetPost();
	}

	if (flags & IMODELANIM_FLAG_ALLZEROS)
	{
		animOptions->SetNoAnim();
	}

	if (flags & IMODELANIM_FLAG_REALTIME)
	{
		animOptions->SetRealTime();
	}

	if (flags & IMODELANIM_FLAG_HIDDEN)
	{
		animOptions->SetHidden();
	}

	if (flags & IMODELANIM_FLAG_WORLD)
	{
		animOptions->SetWorldSpace();
	}

	if (flags & IMODELANIM_FLAG_NOFORCELOOP)
	{
		animOptions->SetNoForceLoop();
	}

	// respawn specific
	if (flags & IMODELANIM_FLAG_FRAMEMOVEMENT)
	{
		animOptions->SetRootMotion();
	}

	if (flags & IMODELANIM_FLAG_SUPPGEST)
	{
		animOptions->SetSuppressGestures();
	}

	if ((flags & IMODELANIM_FLAG_HAS_ANIM) == false)
	{
		animOptions->SetDefaultPose();
	}
}

void IModelExporter::ParseQC_Animation_IKRules(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim, qc::AnimationData_t* const animData, qc::AnimationOptions_t* const animOptions)
{
	UNUSED(file);
	UNUSED(imodel);

	using namespace qc;

	const uint32_t numIkChains = imodel->GetIKChainCount();
	const int numIkRules = anim->GetIKRuleCount();

	// [rika]: nothing to parse, ikrules require ikchains
	if (numIkChains == 0u)
	{
		return;
	}

	// [rika]: force no ikrules as they would be generated otherwise
	if (numIkRules == 0u)
	{
		animOptions->SetAutoIK(false);
		return;
	}

	int useCount[16]{};

	assertm(16 >= numIkChains, "too many ikchains");

	for (int i = 0; i < numIkRules; i++)
	{
		const IModelIKRule* const ikRule = anim->GetIKRule(i);

		useCount[ikRule->GetChain()]++;
	}

	bool autoIk = false;
	bool ignoredIKRules[MAXSTUDIOIKRULES]{};
	int usedIKRules = numIkRules;

	// auto generated ikrules will be the last ones
	for (int i = numIkRules - numIkChains; i < numIkRules; i++)
	{
		if (i < 0)
		{
			break;
		}

		const IModelIKRule* const ikRule = anim->GetIKRule(i);

		// automatic ikrules are always IK_RELEASE, and only created if there is zero existing rules for that chain
		if (ikRule->GetType() != IModelIKRule::IKRULE_RELEASE || useCount[ikRule->GetChain()] > 1)
		{
			continue;
		}

		const float start = ikRule->GetStart();
		const float peak = ikRule->GetPeak();
		const float tail = ikRule->GetTail();
		const float end = ikRule->GetEnd();

		if (start == 0.0f && peak == 0.0f && tail == 1.0f && end == 1.0f)
		{
			ignoredIKRules[i] = true;
			autoIk = true;
			usedIKRules--;
		}
	}

	animOptions->SetAutoIK(autoIk);

	if (usedIKRules == 0)
		return;

	animData->EnableIKRules(file, usedIKRules);
	for (int i = 0, idx = 0; i < usedIKRules; idx++)
	{
		if (ignoredIKRules[idx])
		{
			continue;
		}

		const IModelIKRule* const ikRule = anim->GetIKRule(idx);

		const int chain = ikRule->GetChain();
		const int bone = ikRule->GetBone();

		// init our ik rule
		animData->SetIKRule(i, ikRule->GetType(), chain, bone, ikRule->GetSlot(), imodel->GetBone(bone)->GetName(), imodel->GetIKChain(chain)->GetName(), ikRule->GetAttachment());
		animData->ikRules[i].SetRange(ikRule->GetStart(), ikRule->GetPeak(), ikRule->GetTail(), ikRule->GetEnd());
		animData->ikRules[i].SetOrigin(ikRule->GetPos(), ikRule->GetQ());
		animData->ikRules[i].SetFoot(ikRule->GetHeight(), ikRule->GetRadius(), ikRule->GetFloor(), ikRule->GetEndHeight());
		animData->ikRules[i].SetFootContact(ikRule->GetContact(), ikRule->GetDrop(), ikRule->GetTop());

		// mark as fixup if it has ikerrors

		i++;
	}
}

void IModelExporter::ParseQC_Animation_Movement(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim, qc::AnimationData_t* const animData)
{
	UNUSED(imodel);

	using namespace qc;

	const int numMovements = anim->GetMovementCount();

	if (numMovements == 0)
		return;

	assertm(MAXSTUDIOMOVEKEYS >= numMovements, "animation exceeded maximum movements"); // non vanilla studio mdl

	const IModelMovement* motion = anim->GetMovement(0);
	int prevMotionFlags = motion->GetMotionFlags();
	int prevEndFrame = motion->GetEndFrame();

	AnimationMotion_t motions[MAXSTUDIOMOVEKEYS]{};
	int motionIndex = 0;

	for (int i = 1; i < numMovements; i++)
	{
		motion = anim->GetMovement(i);
		const int motionFlags = motion->GetMotionFlags();
		const int endFrame = motion->GetEndFrame();

		if (motionFlags == prevMotionFlags)
		{
			prevEndFrame = endFrame;
			continue;
		}

		motions[motionIndex].motionflags = prevMotionFlags;
		motions[motionIndex].endframe = prevEndFrame;

		prevMotionFlags = motionFlags;
		prevEndFrame = endFrame;

		motionIndex++;
	}

	// set last motion
	motions[motionIndex].motionflags = prevMotionFlags;
	motions[motionIndex].endframe = prevEndFrame;

	motionIndex++;

	animData->EnableMotion(file, motionIndex);

	for (int i = 0; i < animData->numMotions; i++)
	{
		animData->SetMotion(i, motions[i].endframe, motions[i].motionflags);
	}
}

void IModelExporter::ParseQC_Animation_Sections(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim, int& maxSectionFrames, int& minFramesForSection, int& numZeroFrames, bool& noStallFrames, bool& useLowRes)
{
	UNUSED(file);
	UNUSED(imodel);

	using namespace qc;

	// parse data for section frames
	const int numFrames = anim->GetFrameCount();
	const int sectionFrames = anim->GetSectionFrames();
	if (sectionFrames)
	{
		if (maxSectionFrames == 0)
		{
			maxSectionFrames = sectionFrames;
		}

		assertm(maxSectionFrames >= sectionFrames, "two different section frame counts in animation!");

		minFramesForSection = minFramesForSection < numFrames ? minFramesForSection : numFrames;
	}

	noStallFrames = anim->GetLocalSectionCount() ? true : noStallFrames;
	numZeroFrames = anim->GetZeroFrameCount() > numZeroFrames ? anim->GetZeroFrameCount() : numZeroFrames;
	useLowRes = (anim->GetFlags() & IMODELANIM_FLAG_FRAMEANIM) ? false : useLowRes;
}

void IModelExporter::ParseQC_WeightList(qc::QCFile* const file, const IModel* const imodel, IModelWeightList* const weightLists, uint32_t* const listForSequence)
{
	using namespace qc;

	uint32_t weightListIndex = 0;

	// add the default weight list (all ones)
	weightLists[weightListIndex] = IModelWeightList(s_IModelDefaultWeightList);
	weightListIndex++;

	// cycle through sequences, mark which weightlist they use, and add new ones if found
	for (uint32_t i = 0u; i < imodel->GetSequenceCount(); i++)
	{
		const IModelSequence* const seq = imodel->GetSequence(i);
		const float* const seqWeights = seq->GetWeights();
		const size_t seqWeightSize = sizeof(float) * seq->GetWeightCount();

		bool listExists = false;
		for (uint32_t listIdx = 0; listIdx < weightListIndex; listIdx++)
		{
			IModelWeightList* const weightList = weightLists + listIdx;

			const int result = memcmp(weightList->weights, seqWeights, seqWeightSize);

			if (result)
			{
				continue;
			}

			listForSequence[i] = listIdx;
			weightList->useCount++;
			listExists = true;

			break;
		}

		// we don't have to add a new weight list
		if (listExists)
		{
			continue;
		}

		// weightlist name
		constexpr size_t nameBufSize = 32ull;
		char nameBuf[nameBufSize]{};
		IModelWeightList::NameFromIndex(nameBuf, nameBufSize, weightListIndex);
		const char* const listName = file->WriteString(nameBuf);

		assertm((MAXSTUDIOWEIGHTLIST + 1) > weightListIndex, "exceeded max allowed weight lists");

		// add a new weight list and seq this sequence as using that new weight list
		listForSequence[i] = weightListIndex;
		weightLists[weightListIndex] = IModelWeightList(listName, seqWeights);
		weightListIndex++;
	}

	// build bone name list for weight list commands
	const char** bones = reinterpret_cast<const char**>(file->ReserveData(sizeof(char*) * imodel->GetBoneCount()));
	for (uint32_t i = 0u; i < imodel->GetBoneCount(); i++)
	{
		bones[i] = imodel->GetBone(i)->GetName();
	}

	// add weight list commands
	for (uint32_t i = 0; i < weightListIndex; i++)
	{
		const IModelWeightList* weightList = weightLists + i;
		const WeightListData_t weightListData(weightList->name, bones, weightList->weights, imodel->GetBoneCount(), weightList->markedAsDefault);

		if (weightList->markedAsDefault)
		{
			CmdParse(file, QC_DEFAULTWEIGHTLIST, &weightListData, 1u, false, QC_FMT_ARRAYSTYLE);

			continue;
		}

		CmdParse(file, QC_WEIGHTLIST, &weightListData, 1u, false, QC_FMT_ARRAYSTYLE);
	}
}

void IModelExporter::ParseQC_Animation(qc::QCFile* const file, const IModel* const imodel, const IModelAnimation* const anim)
{
	using namespace qc;

	if (anim->IsOverriden())
	{
		CmdParse(file, QC_DECLAREANIM, anim->GetName());
		return;
	}

	// @ is auto added from sequence
	if (anim->IsSeqDeclared())
	{
		return;
	}

	// I saw what this is for but cannot find it now
	if (anim->IsNullName())
	{
		assertm(false, "funky animation");
		return;
	}

	const char* const name = anim->GetName();

	constexpr size_t filepathLength = 128ull;
	char filepath[filepathLength];
	FormNameAnim(filepath, filepathLength, name);

	AnimationData_t animData(name, filepath, anim->GetFrameRate(), anim->GetFrameCount());
	AnimationOptions_t animOptions(0u, AnimationOptions_t::ANIM_OPT_FEATURE_ANIM);

	ParseQC_Animation_OptionsFromFlags(file, imodel, anim->GetFlags(), &animOptions);
	ParseQC_Animation_IKRules(file, imodel, anim, &animData, &animOptions);
	ParseQC_Animation_Movement(file, imodel, anim, &animData);

	// TEMP, need to store a fixup animation path here
	if (animOptions.IsDelta())
	{
		animData.SetSubtractAnim("");
	}

	animOptions.SetFPS();

	animData.SetOptions(animOptions);

	CmdParse(file, QC_ANIMATION, &animData, 1u, false, QC_FMT_ARRAYSTYLE);
}

void IModelExporter::ParseQC_Sequence(qc::QCFile* const file, const IModel* const imodel, const IModelSequence* const seq, const char* const weightlist)
{
	using namespace qc;

	if (seq->IsOverriden())
	{
		CmdParse(file, QC_DECLARESEQ, seq->GetLabel());
		return;
	}

	const int numBlends = seq->GetBlendCount();
	assertm(numBlends, "sequence without animations");

	// frame data is based off of the first animation
	const IModelAnimation* const firstAnim = imodel->GetAnimation(seq->GetBlend(0));
	const float frameRate = firstAnim ? firstAnim->GetFrameRate() : 0.0f;
	const int frameCount = firstAnim ? firstAnim->GetFrameCount() : 0;

	SequenceData_t seqData(file, seq->GetLabel(), seq->GetActivity(), seq->GetActWeight(), frameRate, frameCount, weightlist, seq->GetBlendWidth(), seq->GetBlendHeight(), numBlends, seq->GetFadeInTime(), seq->GetFadeOutTime(),
		seq->GetEntryPhase(), seq->GetExitPhase(), seq->GetKeyValues(), seq->GetIKResetMask());

	// cycle through all animations and add their name or path, then parse the first animation to be implied, if there is one
	bool impliedAnimation = false;
	int firstImpliedAnim = -1;
	for (int i = 0; i < numBlends; i++)
	{
		const IModelAnimation* const anim = imodel->GetAnimation(seq->GetBlend(i));

		// not a local animation, we don't (and probably cannot) handle external animations
		// goblin_dropship_fleetscale_1000x.mdl in r1 does this
		if (anim == nullptr)
		{
			printf("sequence '%s' had an external animation in blend slot %i!\n", seq->GetLabel(), i);
			seqData.SetBlend(i, "external_anim");
			continue;
		}

		const char* name = anim->GetName();

		if (anim->IsSeqDeclared())
		{
			constexpr size_t filepathLength = 128ull;
			char filepath[filepathLength];
			FormNameAnim(filepath, filepathLength, name);

			name = file->WriteString(filepath);

			if (impliedAnimation == false)
			{
				impliedAnimation = true;
				firstImpliedAnim = i;
			}
		}

		seqData.SetBlend(i, name);
	}

	AnimationOptions_t animOptions(0u, impliedAnimation ? AnimationOptions_t::ANIM_OPT_FEATURE_ALL : AnimationOptions_t::ANIM_OPT_FEATURE_SEQ);

	// parse animation data since we won't be writing an $animation command for this one
	if (impliedAnimation)
	{
		assertm(firstImpliedAnim == 0, "not ideal but someone could do this");

		const IModelAnimation* const anim = imodel->GetAnimation(seq->GetBlend(firstImpliedAnim));

		ParseQC_Animation_OptionsFromFlags(file, imodel, anim->GetFlags(), &animOptions);
		ParseQC_Animation_IKRules(file, imodel, anim, &seqData, &animOptions);
		ParseQC_Animation_Movement(file, imodel, anim, &seqData);

		// TEMP, need to store a fixup animation path here
		// should work with multiple animations?
		if (animOptions.IsDelta())
		{
			seqData.SetSubtractAnim("");
		}

		animOptions.SetFPS();
	}

	// parse flags from sequence now that we've potentially set options from the implied animation
	ParseQC_Animation_OptionsFromFlags(file, imodel, seq->GetFlags(), &animOptions);
	seqData.SetOptions(animOptions);

	// parse nodes
	// if either are set, both should be set
	const int localEntryNode = seq->GetEntryNode();
	const int localExitNode = seq->GetExitNode();
	if (localEntryNode || localExitNode)
	{
		// apex legends can have either or node set

		const char* const entryNode = localEntryNode ? imodel->GetNodeName(localEntryNode - 1) : nullptr;
		const char* const exitNode = localExitNode ? imodel->GetNodeName(localExitNode - 1) : nullptr;

		seqData.SetupNodes(entryNode, exitNode, localEntryNode, localExitNode, seq->GetNodeFlags());
	}

	// parse poseparms if the sequence has any
	for (int i = 0; i < 2; i++)
	{
		const int paramIndex = seq->GetParamIndex(i);

		// [rika]: apex legends seems to support setting these via independent blend options, making them not linear (for vertical blends)
		if (paramIndex == -1)
		{
			seqData.SetParam(i, nullptr, seq->GetParamStart(i), seq->GetParamEnd(i));
			continue;
		}

		seqData.SetParam(i, imodel->GetPoseParameter(paramIndex)->GetName(), seq->GetParamStart(i), seq->GetParamEnd(i));
	}

	// setup events
	const int numEvents = seq->GetEventCount();
	if (numEvents)
	{
		seqData.EnableEvents(file, numEvents);

		for (int i = 0; i < numEvents; i++)
		{
			const IModelEvent* const event = seq->GetEvent(i);
			seqData.SetEvent(i, event->GetName(), event->GetEventId(), event->IsNewEventStyle(), event->GetCycle(), event->GetUnk(), event->GetOptions());
		}
	}

	// setup autolayers
	const int numLayers = seq->GetAutoLayerCount();
	if (numLayers)
	{
		seqData.EnableLayers(file, numLayers);

		for (int i = 0; i < numLayers; i++)
		{
			const IModelAutoLayer* const layer = seq->GetAutoLayer(i);

			const int layerFlags = layer->GetFlags();

			const char* const sequence = imodel->GetSequence(layer->GetSequenceIndex())->GetLabel();
			const char* const pose = layerFlags & STUDIO_AL_POSE ? imodel->GetPoseParameter(layer->GetPoseIndex())->GetName() : nullptr;

			seqData.SetLayer(i, sequence, pose, layerFlags, layer->GetStart(), layer->GetPeak(), layer->GetTail(), layer->GetEnd());
		}
	}

	// setup ik locks
	const int numIKLocks = seq->GetIKLockCount();
	if (numIKLocks)
	{
		seqData.EnableIKLocks(file, numIKLocks);

		for (int i = 0; i < numIKLocks; i++)
		{
			const IModelIKLock* const ikLock = seq->GetIKLock(i);
			seqData.SetIKLock(i, imodel->GetIKChain(ikLock->GetChain())->GetName(), ikLock->GetPosWeight(), ikLock->GetLocalQWeight());
		}
	}

	if (seq->GetFlags() & IMODELANIM_FLAG_CYCLEPOSE)
	{
		seqData.SetCyclePose(imodel->GetPoseParameter(seq->GetCyclePoseIndex())->GetName());
	}

	// setup activity modifiers
	const int numActMods = seq->GetActModCount();
	if (numActMods)
	{
		seqData.EnableActMods(file, numActMods);

		for (int i = 0; i < numActMods; i++)
		{
			const IModelActMod* const actmod = seq->GetActMod(i);
			seqData.SetActMod(i, actmod->GetName(), actmod->IsNegated());
		}
	}

	CmdParse(file, QC_SEQUENCE, &seqData, 1u, false, QC_FMT_ARRAYSTYLE);
}

const bool IModelExporter::ExportQC()
{
	using namespace qc;

	const IModel* const imodel = Model();

	CManagedBuffer* buf = g_BufferManager.ClaimBuffer();

	QCFile qcFile(Path(), buf->Buffer(), managedBufferSize);

	ParseQC_General(&qcFile, imodel);

	bool useHighResSaveFrame = false; // $animblocksize
	for (uint32_t i = 0u; i < imodel->GetBoneCount(); i++)
	{
		const IModelBone* const bone = imodel->GetBone(i);

		ParseQC_Bone(&qcFile, imodel, bone);

		useHighResSaveFrame = bone->GetFlags() & BONE_HAS_SAVEFRAME_ROT64 ? true : useHighResSaveFrame;
	}

	uint32_t qcMaxVerts = 0u;
	for (uint32_t i = 0u; i < imodel->GetBodyPartCount(); i++)
	{
		ParseQC_Bodypart(&qcFile, imodel, imodel->GetBodyPart(i), qcMaxVerts);
	}

	// skips if we have one LOD
	for (uint32_t i = 1u; i < imodel->GetLODCount(); i++)
	{
		const bool isShadowLOD = (imodel->GetFlags() & IMODEL_FLAG_HASSHADOWLOD) && (imodel->GetLOD(i)->GetSwitchPoint() == -1.0f);

		ParseQC_LOD(&qcFile, imodel, i, isShadowLOD, qcMaxVerts);
	}

	// check if we need $maxverts command
	// todo: doesn't seem quite right to add here
	qcMaxVerts++;
	assertm(qcMaxVerts < MAXSTUDIOVERTS, "invalid max vert value");
	if (qcMaxVerts > (MAXSTUDIOVERTS / 3))
	{
		CmdParse(&qcFile, QC_MAXVERTS, &qcMaxVerts);
	}

	// attachments
	ParseQC_Attachments(&qcFile, imodel);

	// boxes and boxes and boxes and
	ParseQC_Boxes(&qcFile, imodel);

	// animation and sequences
	int animBlockSize = 0; // $animblocksize
	ParseQC_AnimationTypes(&qcFile, imodel, animBlockSize);

	const uint32_t numSeqs = imodel->GetSequenceCount();
	if (numSeqs)
	{
		// pre parse weight lists
		constexpr size_t maxWeightListSize = sizeof(IModelWeightList) * (MAXSTUDIOWEIGHTLIST + 1); // 128 weight lists max plus the default one
		IModelWeightList* const weightLists = reinterpret_cast<IModelWeightList*>(qcFile.ReserveData(maxWeightListSize));
		uint32_t* const listForSequence = reinterpret_cast<uint32_t*>(qcFile.ReserveData(sizeof(uint32_t) * numSeqs));

		ParseQC_WeightList(&qcFile, imodel, weightLists, listForSequence);

		const uint32_t numAnims = imodel->GetAnimationCount();
		if (numAnims)
		{
			// $sectionframes
			int sectionFrameCount = 0;
			int minFrameCount = s_SectionFrames_DefaultMinFrame;

			// $animblocksize
			bool useNoStall = false;
			bool useLowResAnim = true;
			int numZeroFrames = 0;

			for (uint32_t i = 0u; i < numAnims; i++)
			{
				const IModelAnimation* const anim = imodel->GetAnimation(i);

				ParseQC_Animation_Sections(&qcFile, imodel, anim, sectionFrameCount, minFrameCount, numZeroFrames, useNoStall, useLowResAnim);
				ParseQC_Animation(&qcFile, imodel, anim);
			}

			if (sectionFrameCount > 0 && (sectionFrameCount != s_SectionFrames_DefaultFrameCount || minFrameCount != s_SectionFrames_DefaultMinFrame))
			{
				const SectionFrameData_t sectionFrames(sectionFrameCount, minFrameCount);
				CmdParse(&qcFile, QC_SECTIONFRAMES, &sectionFrames);
			}

			if (imodel->GetAnimBlockCount())
			{
				useNoStall = numZeroFrames == 0 ? true : useNoStall;

				const AnimBlockData_t animBlockData(animBlockSize, numZeroFrames, useNoStall, false, useLowResAnim, useHighResSaveFrame);
				CmdParse(&qcFile, QC_ANIMBLOCKSIZE, &animBlockData);
			}
		}

		for (uint32_t i = 0u; i < numSeqs; i++)
		{
			ParseQC_Sequence(&qcFile, imodel, imodel->GetSequence(i), weightLists[listForSequence[i]].name);
		}
	}

	// physics
#ifdef HAS_PHYSICSMODEL_PARSER
	const IModelPhysics* const physics = imodel->GetPhysics();
	if (physics)
	{
		const PhysicsModel::CParsedPhys* const parsedPhys = physics->GetParsedPhys();

		const size_t nameLength = strnlen_s(physics->GetName(), MAX_PATH_SOURCE);
		const size_t bufSize = nameLength + 8; // for extension and null terminator
		char* tmp = new char[bufSize] {};
		snprintf(tmp, bufSize, "%s%s", physics->GetName(), s_ExportFormatExtensions[g_CmdSettings.format]);

		const PhysicsData_t physData(tmp, parsedPhys);
		if (parsedPhys->IsJointed())
		{
			CmdParse(&qcFile, QC_COLLISIONJOINTS, &physData);
		}
		else
		{
			CmdParse(&qcFile, QC_COLLISIONMODEL, &physData);
		}

		FreeAllocArray(tmp);

		const char* const collisionText = parsedPhys->GetCollisionText();
		if (collisionText)
		{
			CmdParse(&qcFile, QC_COLLISIONTEXT, collisionText);
		}
	}
#endif // HAS_PHYSICSMODEL_PARSER

	// export qc file
	QCFile::SetExportVersion(g_CmdSettings.qcVersion);
	qcFile.ParseToText(g_CmdSettings.qcUseIncludes);

	g_BufferManager.RelieveBuffer(buf);

	return true;
}