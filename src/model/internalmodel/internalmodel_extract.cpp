#include <pch.h>

#include <model/internalmodel/internalmodel.h>
#include <model/internalmodel/internalmodel_extract.h>

extern CmdSettings_t g_CmdSettings;
extern CBufferManager g_BufferManager;

void IModelExporter::AdjustAnimationOrigin(const IModelAnimation* const anim, const IModelMovementTrack* const moveTrack, const int frameIdx, Vector& pos, Quaternion& quat, const uint32_t bone)
{
	Vector movePos(0.0f);
	QAngle moveRot(0.0f);
	QAngle baseRot(quat);

	// would these other bones need to be updated with movement data as well?
	if (bone == 0u && anim->HasMovementTrack() && g_CmdSettings.ignoreMotion == false)
	{
		moveTrack->GetFrame(frameIdx, movePos, moveRot);
	}

	pos += movePos;
	baseRot.y += (moveRot.y - 90); // !!!! MIGHT NOT BE THE RIGHT AXIS FOR MOVEMENT ROTATION !!!! 90 degree rotation works, likely different than smd for le funny dmx reasons. movement rotation seems to look good on this axis.
	// VectorYawRotate ?
	// might have to change this depending on format?

	//adjust position as we are rotating on the Z axis
	const float x = pos.x;
	const float y = pos.y;

	pos.x = y;
	pos.y = -x;

	AngleQuaternion(baseRot, quat);
}

bool IModelExporter::WriteToFile(const char* const fileBuf, const char* const fileName, const int fileSize)
{
	// file out
	std::filesystem::path tmp(fileName);
	tmp.replace_extension(s_ExportFormatNames[g_CmdSettings.format]);

	std::string fileOutPath = std::filesystem::path(Path()).append(tmp.string()).string();

	std::ofstream out(fileOutPath, std::ios::out | std::ios::binary);
	out.write(fileBuf, static_cast<size_t>(fileSize));

	return true;
}

void IModelExporter::FormNameAnim(char* const buf, const size_t size, const char* const name)
{
	assertm(strnlen_s(name, MAX_PATH_SOURCE) < (size - 16ull), "big animation name");
	snprintf(buf, size, "anims_%s/%s%s", Name().c_str(), name, s_ExportFormatExtensions[g_CmdSettings.format]);
}


//
// DMX
// 

constexpr const char* const s_DmeModelName = "mshop dmxport";

void IModelExporter::ExportSkeletonDMX(CDataModel* const dmx)
{
	const IModel* const imodel = Model();

	CDmeModel* const dmeModel = new CDmeModel(dmx, s_DmeModelName, 0, imodel->GetBoneCount(), false); // this memory is managed by CDataModel

	dmeModel->AddTransform("unnamed", 0);
	dmeModel->SetUpAxis();

	CDmeTransformList* const dmeTransformList = dmeModel->AddBaseState("bind", 0, imodel->GetBoneCount(), false);

	for (uint32_t boneIdx = 0; boneIdx < imodel->GetBoneCount(); boneIdx++)
	{
		const IModelBone* const bone = imodel->GetBone(boneIdx);

		CDmeJoint* const dmeJoint = dmeModel->AddJoint(bone->GetName(), 0);
		dmeJoint->SetUpSurfaceProp(bone->GetSurfaceProp());
		dmeJoint->AddTransform(bone->GetName(), 0, bone->GetPos(), bone->GetQuat(), bone->GetScale());

		dmeTransformList->AddTransform(bone->GetName(), 1, bone->GetPos(), bone->GetQuat(), bone->GetScale());
	}

	// setup bone heirrarchy, should be in same order as model
	for (int jointIdx = 0; jointIdx < dmeModel->GetJointCount(); jointIdx++)
	{
		CDmeJoint* const dmeJoint = dmeModel->GetJoint(jointIdx);
		std::vector<uint32_t> jointChildren = imodel->GetBoneChildren(jointIdx);

		for (const uint32_t childIdx : jointChildren)
		{
			dmeJoint->AddChildJoint(dmeModel->GetJoint(static_cast<int>(childIdx)));
		}
	}

	dmeModel->AddChildJoint();
}

// only marginal performance boost due to expensive emplace back, but good to have for later
void IModelExporter::ImportSkeletonDMX(CDataModel* const dmxSource, CDataModel* const dmxDest)
{
	const DmElementHash dmeModelHash(eDmElementClass::DME_MODEL, s_DmeModelName, 0);
	DmElement* const dmeElementSource = dmxSource->GetElement(dmeModelHash);
	assertm(dmeElementSource, "dmx did not have valid skeleton containing element");

	CDmeModel* const dmeModelSource = static_cast<CDmeModel* const>(dmeElementSource->GetElement());
	assertm(dmeModelSource, "element did not have valid class data");

	// new model for our skeleton, should not exist at this point
	CDmeModel* const dmeModel = new CDmeModel(dmxDest, s_DmeModelName, 0, dmeModelSource, false); // this memory is managed by CDataModel

	dmeModel->AddTransform("unnamed", 0);
	dmeModel->SetUpAxis();

	dmeModel->AddChildJoint();

	dmxDest->AddDependent(dmxSource);
}

const bool IModelExporter::ExportModelDMX(const IModelModel* const model, void* const file)
{
	UNUSED(file);

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();
	CDataModel* dmx = new CDataModel(Path(), model->GetName(), &g_CmdSettings.dmxVersion, buf->Buffer(), managedBufferSize);

	// sets up a root element for model/animation
	dmx->InitForModel();

	ExportSkeletonDMX(dmx);

	// gets the DmeModel we created for the skeleton, on dmx model files this also contains the vertex data
	const DmElementHash dmeModelHash(DME_MODEL, s_DmeModelName, 0);
	CDmeModel* const dmeModel = static_cast<CDmeModel* const>(dmx->GetElement(dmeModelHash)->GetElement());
	assertm(dmeModel && dmeModel->GetType() == DME_MODEL, "element did not exist or was invalid");

	// I don't know/remember what Dag stands for
	CDmeDag* const dmeDag = dmeModel->AddChildDag(model->GetName(), 0);
	dmeDag->AddTransform(model->GetName(), 0);

	CDmeMesh* const dmeMesh = dmeDag->AddShape(model->GetName(), 0);

	CDmeVertexData* const dmeVertexData = dmeMesh->AddBaseState(dmx, "bind", 0, model->GetVertexCount(), model->GetMaxVertBoneCount());
	dmeMesh->AddCurrentState(dmeVertexData);

	// hate this, since all faces are local to the single vertex data element we have to do some offset funnies
	int localVertexIndex = 0; // because vertices are stored in one big chunk, we need to adjust indices. use this as the base index for indices in each mesh
	int relativeIndiceIndex = 0;
	for (uint32_t meshIdx = 0u; meshIdx < model->GetMeshCount(); meshIdx++)
	{
		const IModelMesh* const mesh = model->GetMesh(meshIdx);
		const IModelMeshFlags_t flags = mesh->GetFlags();

		if (mesh->GetVertexCount() == 0u || flags == IMODELMESH_FLAG_NONE)
		{
			continue;
		}

		const int materialId = mesh->GetMaterialId();
		const char* const material = Model()->GetMaterial(materialId)->GetName();

		CDmeFaceSet* const dmeFaceSet = dmeMesh->AddFaceSet(dmx, material, mesh->GetMaterialId(), mesh->GetIndiceCount(), true); // needs short name, use material idx as set, as truncating names in rare cases can cause duplicate elements !!! DOESN'T WORK WITH BLENDER AS THE MATERIALS GET MERGED !!!
		dmeFaceSet->AddMaterial(material, materialId, g_CmdSettings.truncateMaterials); // move to constructor?

		for (int vertIdx = 0; vertIdx < static_cast<int>(mesh->GetVertexCount()); vertIdx++)
		{
			// crowbar swaps x/y for static props. this should be done when loading into IModel, not export to avoid duplicate code.

			const IModelVertex* const vert = mesh->GetVertex(vertIdx);

			if (flags & eIModelMeshFlags::IMODELMESH_FLAG_POS)
			{
				dmeVertexData->AddVertexData<Vector>(VertexAttributes_t::AT_VERTEX_POSITION, vert->GetPosition());
			}

			if (flags & eIModelMeshFlags::IMODELMESH_FLAG_NORM)
			{
				dmeVertexData->AddVertexData<Vector>(VertexAttributes_t::AT_VERTEX_NORMAL, vert->GetNormal());
			}

			/*if (flags & eIModelMeshFlags::IMODELMESH_FLAG_TAN)
			{
				dmeVertexData->AddVertexData<Vector4D>(VertexAttributes_t::AT_VERTEX_TANGENT, vert->GetTangent());
			}*/

			if (flags & eIModelMeshFlags::IMODELMESH_FLAG_TEXCOORD0)
			{
				dmeVertexData->AddVertexData<Vector2D>(VertexAttributes_t::AT_VERTEX_TEXCOORD, *mesh->GetTexcoord(vertIdx, 0));
			}

			if (flags & eIModelMeshFlags::IMODELMESH_FLAG_TEXCOORD2)
			{
				dmeVertexData->AddVertexData<Vector2D>(VertexAttributes_t::AT_VERTEX_TEXCOORD2, *mesh->GetTexcoord(vertIdx, 1));
			}

			if (flags & eIModelMeshFlags::IMODELMESH_FLAG_COLOR)
			{
				dmeVertexData->AddVertexData<Color32>(VertexAttributes_t::AT_VERTEX_COLOR, *mesh->GetColor(vertIdx));
			}

			for (uint32_t weightIdx = 0; weightIdx < static_cast<uint32_t>(dmeVertexData->JointCount()); weightIdx++)
			{
				// add empty weights to padd out dmx joint weights
				if (weightIdx >= vert->GetWeightCount())
				{
					dmeVertexData->AddVertexWeight(0.0f, 0);
					continue;
				}

				const IModelVertexBoneWeight* const weight = mesh->GetWeight(vert, weightIdx);
				dmeVertexData->AddVertexWeight(weight->weight, weight->bone);
			}
		}

		int relativeIndices[IMODELFACE_maxIndices]{};
		for (int indiceIdx = 0; indiceIdx < static_cast<int>(mesh->GetIndiceCount()); indiceIdx++)
		{
			const IModelFace* const face = mesh->GetIndice(indiceIdx);
			const IModelFaceIndice indexCount = face->GetIndiceCount();

			// add our indices, making sure to adjust them to no longer be mesh local
			for (IModelFaceIndice i = 0u, j = indexCount; i < indexCount; i++)
			{
				// counter clock wise? do this to get the right face order
				const IModelFaceIndice idx = j % indexCount;
				j--;

				const int indice = static_cast<int>(face->GetIndice(idx)) + localVertexIndex;
				dmeVertexData->AddMappedIndice(indice);

				// the index for each indice
				relativeIndices[i] = relativeIndiceIndex;
				relativeIndiceIndex++;
			}

			dmeFaceSet->AddFace(indexCount, relativeIndices);
		}

		localVertexIndex += mesh->GetVertexCount(); // increment vertex index by this mesh's vertex count
	}

	dmeVertexData->MapVertexIndices(); // map our vertex indice fields after we've filled the map and vertex data fields

	dmx->WriteDataModel(false);

	FreeAllocVar(dmx);
	g_BufferManager.RelieveBuffer(buf);

	return true;
}

const bool IModelExporter::ExportAnimDMX(const IModelAnimation* const anim, void* const file)
{
	UNUSED(file);

	if (anim->GetBoneCount() == 0u)
	{
		assertm(false, "cannot animate no bones");
		return false;
	}

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();
	CDataModel* dmx = new CDataModel(Path(), anim->GetName(), &g_CmdSettings.dmxVersion, buf->Buffer(), managedBufferSize);

	// sets up a root element for model/animation
	dmx->InitForModel();

	ExportSkeletonDMX(dmx);

	// animation stuff
	CDmeAnimationList* const animList = new CDmeAnimationList(dmx, "mshop dmxport animations", 0);

	// on paper I guess you could have more than one animation per dmx, sequences? we only do one for now.
	// todo: you can store some very important data for compile (otherwise set via qc) in here
	CDmeChannelsClip* const channelsClip = animList->AddAnimation(anim->GetName(), 0, static_cast<int>(anim->GetFrameRate()));

	channelsClip->AddTimeFrame("unnamed", 0, 0.0f, anim->GetDuration()); // I guess this is the "timeframe" the anim takes place in?

	// [rika]: declare here, pointless to declare over and over in a loop.
	const IModelMovementTrack* const moveTrack = anim->GetMovementTrack();

	for (int boneIdx = 0; boneIdx < static_cast<int>(anim->GetBoneCount()); boneIdx++)
	{
		const IModelAnimationTrack* animTrack = anim->GetAnimTrack(boneIdx);

		assert(boneIdx == animTrack->GetBone());

		const IModelBone* bone = this->Model()->GetBone(boneIdx);

		// add scale when we have a better dmx plugin
		CDmeChannel* const channelPos = channelsClip->AddChannel(bone->GetName(), eDmeChannelType::DME_CH_TYPE_POS);
		CDmeChannel* const channelRot = channelsClip->AddChannel(bone->GetName(), eDmeChannelType::DME_CH_TYPE_ROT);

		CDmeVector3Log* const logPos = static_cast<CDmeVector3Log* const>(channelPos->AddLog("vector3 log", boneIdx));
		CDmeQuaternionLog* const logRot = static_cast<CDmeQuaternionLog* const>(channelRot->AddLog("quaternion log", boneIdx));

		CDmeVector3LogLayer* const layerPos = logPos->AddLayer("vector3 log", anim->GetFrameCount());
		CDmeQuaternionLogLayer* const layerRot = logRot->AddLayer("quaternion log", anim->GetFrameCount());

		for (int frameIdx = 0; frameIdx < anim->GetFrameCount(); frameIdx++)
		{
			Vector pos;
			Quaternion rot;
			Vector scale;

			animTrack->GetFrame(frameIdx, pos, rot, scale);

			// is this the origin bone?
			if (bone->GetParent() == -1)
			{
				AdjustAnimationOrigin(anim, moveTrack, frameIdx, pos, rot, boneIdx);
			}

			layerPos->AddFrame(pos, anim->GetFrameOffset(frameIdx));
			layerRot->AddFrame(rot, anim->GetFrameOffset(frameIdx));
		}
	}

	dmx->WriteDataModel(false);

	FreeAllocVar(dmx);
	g_BufferManager.RelieveBuffer(buf);

	return true;
}


//
// RMAX
//

void IModelExporter::ExportSkeletonRMAX(rmax::RMAXExporter* const rmax)
{
	rmax->ReserveBones(Model()->GetBoneCount());
	for (uint32_t i = 0; i < Model()->GetBoneCount(); i++)
	{
		const IModelBone* const bone = Model()->GetBone(i);

		rmax->AddBone(bone->GetName(), bone->GetParent(), bone->GetPos(), bone->GetQuat(), bone->GetScale());
	}
}

const bool IModelExporter::ExportModelRMAX(const IModelModel* const model, void* const file)
{
	using namespace rmax;

	RMAXExporter* const rmax = reinterpret_cast<RMAXExporter* const>(file);
	rmax->ResetMeshData();
	rmax->SetName(model->GetName());

	rmax->AllocMeshData(model->GetMeshCount(), model->GetIndiceCount(), model->GetVertexCount(), model->GetMaxTexcoordCount(), model->GetMaxVertBoneCount());

	for (uint32_t meshIdx = 0u; meshIdx < model->GetMeshCount(); meshIdx++)
	{
		const IModelMesh* const mesh = model->GetMesh(meshIdx);
		const IModelMeshFlags_t flags = mesh->GetFlags();

		if (flags == IMODELMESH_FLAG_NONE || mesh->GetVertexCount() == 0u)
		{
			continue;
		}

		int16_t uvIndices = 0;
		uvIndices |= (flags & IMODELMESH_FLAG_TEXCOORD0) ? s_UVIndice0 : 0;
		uvIndices |= (flags & IMODELMESH_FLAG_TEXCOORD2) ? s_UVIndice2 : 0;
		uvIndices |= (flags & IMODELMESH_FLAG_TEXCOORD3) ? s_UVIndice3 : 0;

		rmax->AddMesh(-1, static_cast<int16_t>(mesh->GetMaterialId()), mesh->GetTexcoordCount(), uvIndices, flags & eIModelMeshFlags::IMODELMESH_FLAG_COLOR);
		rmax::RMAXMesh* const rmaxMesh = rmax->GetMeshLast();

		// data parsing
		for (uint32_t i = 0; i < mesh->GetVertexCount(); i++)
		{
			const IModelVertex* const vert = mesh->GetVertex(i);

			rmaxMesh->AddVertex(vert->GetPosition(), vert->GetNormal());

			if (flags & eIModelMeshFlags::IMODELMESH_FLAG_COLOR)
			{
				rmaxMesh->AddColor(*mesh->GetColor(i));
			}

			for (int16_t texcoordIdx = 0; texcoordIdx < mesh->GetTexcoordCount(); texcoordIdx++)
			{
				rmaxMesh->AddTexcoord(*mesh->GetTexcoord(i, texcoordIdx));
			}

			for (uint32_t weightIdx = 0; weightIdx < vert->GetWeightCount(); weightIdx++)
			{
				const IModelVertexBoneWeight* const weight = mesh->GetWeight(vert, weightIdx);
				rmaxMesh->AddWeight(i, weight->bone, weight->weight);
			}
		}

		for (int i = 0; i < static_cast<int>(mesh->GetIndiceCount()); i++)
		{
			const IModelFace* const face = mesh->GetIndice(i);
			assertm(face->IsNGon() == false, "rmax does not support n-gons");

			const uint16_t a = static_cast<uint16_t>(face->GetIndice(0));
			const uint16_t b = static_cast<uint16_t>(face->GetIndice(1));
			const uint16_t c = static_cast<uint16_t>(face->GetIndice(2));
			const uint16_t d = face->IsQuad() ? static_cast<uint16_t>(face->GetIndice(3)) : 0xFFFF;

			rmaxMesh->AddIndice(a, b, c, d);
		}
	}

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();
	rmax->ToFile(buf->Buffer());
	g_BufferManager.RelieveBuffer(buf);

	return true;
}

static_assert(static_cast<uint16_t>(eIModelAnimTrackFlags::TRACK_POS) == static_cast<uint16_t>(rmax::AnimTrackFlags_t::ANIM_TRACK_POS));
static_assert(static_cast<uint16_t>(eIModelAnimTrackFlags::TRACK_ROT) == static_cast<uint16_t>(rmax::AnimTrackFlags_t::ANIM_TRACK_ROT));
static_assert(static_cast<uint16_t>(eIModelAnimTrackFlags::TRACK_SCL) == static_cast<uint16_t>(rmax::AnimTrackFlags_t::ANIM_TRACK_SCL));

const bool IModelExporter::ExportAnimRMAX(const IModelAnimation* const anim, void* const file)
{
	using namespace rmax;

	if (anim->GetBoneCount() == 0u)
	{
		return false;
	}

	RMAXExporter* const rmax = reinterpret_cast<RMAXExporter* const>(file);
	rmax->ResetData();
	rmax->SetName(anim->GetName());

	const IModelAnimFlags_t flags = anim->GetFlags();
	uint16_t animFlags = 0u;

	// delta flag
	if (flags & eIModelAnimFlags::IMODELANIM_FLAG_DELTA)
	{
		animFlags |= rmax::AnimFlags_t::ANIM_DELTA;
	}

	if ((anim->GetFlags() & eIModelAnimFlags::IMODELANIM_FLAG_HAS_ANIM) == false)
	{
		animFlags |= rmax::AnimFlags_t::ANIM_EMPTY;
	}

	rmax->AddAnim(anim->GetName(), static_cast<uint16_t>(anim->GetFrameCount()), anim->GetFrameRate(), animFlags, anim->GetBoneCount());
	rmax::RMAXAnim* const rmaxAnim = rmax->GetAnimLast();

	if (rmaxAnim->GetFlags() & rmax::AnimFlags_t::ANIM_EMPTY)
	{
		CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();
		rmax->ToFile(buf->Buffer());
		g_BufferManager.RelieveBuffer(buf);

		return true;
	}

	for (uint32_t i = 0u; i < anim->GetBoneCount(); i++)
	{
		const IModelAnimationTrack* const animTrack = anim->GetAnimTrack(i);

		rmaxAnim->SetTrack(animTrack->GetFlags(), static_cast<uint16_t>(i));
		rmax::RMAXAnimTrack* const rmaxTrack = rmaxAnim->GetTrack(i);

		Vector pos;
		Quaternion q;
		Vector scale;

		for (int frameIdx = 0; frameIdx < anim->GetFrameCount(); frameIdx++)
		{
			animTrack->GetFrame(frameIdx, pos, q, scale);

			rmaxTrack->AddFrame(frameIdx, &pos, &q, &scale);
		}
	}

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();
	rmax->ToFile(buf->Buffer());
	g_BufferManager.RelieveBuffer(buf);

	return true;
}


//
// SMD
//

inline void ParseVertexIntoSMD(const IModelVertex* const srcVert, const IModelMesh* const srcMesh, smd::Vertex* const vert, const bool isStaticProp, const uint32_t vertexIndex)
{
	vert->position = srcVert->GetPosition();
	vert->normal = srcVert->GetNormal();

	if (isStaticProp)
	{
		StaticPropFlipFlop(vert->position);
		StaticPropFlipFlop(vert->normal);
	}

	vert->numTexcoords = srcMesh->GetTexcoordCount() > smd::maxTexcoords ? smd::maxTexcoords : srcMesh->GetTexcoordCount();
	for (uint16_t texcoordIdx = 0u; texcoordIdx < vert->numTexcoords; texcoordIdx++)
	{
		vert->texcoords[texcoordIdx] = *srcMesh->GetTexcoord(vertexIndex, texcoordIdx);
		IModelVertex::InvertTexcoord(vert->texcoords[texcoordIdx]); // [rika]: the texcoord has to be inverted for proper recompile
	}

	// model does not have weights
	if ((srcMesh->GetFlags() & IMODELMESH_FLAG_WEIGHTS) == false)
	{
		vert->numBones = 1;

		vert->bone[0] = 0;
		vert->weight[0] = 1.0f;

		return;
	}

	vert->numBones = srcVert->GetWeightCount() > smd::maxBoneWeights ? smd::maxBoneWeights : srcVert->GetWeightCount();
	for (uint32_t weightIdx = 0u; weightIdx < vert->numBones; weightIdx++)
	{
		const IModelVertexBoneWeight* const weight = srcMesh->GetWeight(srcVert, weightIdx);

		vert->bone[weightIdx] = weight->bone;
		vert->weight[weightIdx] = weight->weight;
	}
}

const bool IModelExporter::ExportModelSMD(const IModelModel* const model, void* const file)
{
	const IModel* const imodel = Model();

	smd::CStudioModelData* const smd = reinterpret_cast<smd::CStudioModelData* const>(file);

	smd->ResetMeshData(); // new model, reset the data
	smd->SetName(model->GetName());

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();

	// [rika]: model is skin and bones, no meat
	if (model->GetMeshCount() == 0u)
	{
		smd->Write(buf->Buffer(), managedBufferSize);

		g_BufferManager.RelieveBuffer(buf);

		return true;
	}

	const bool isStaticProp = imodel->GetFlags() & IMODEL_FLAG_STATIC_PROP ? true : false;

	for (uint32_t meshIdx = 0; meshIdx < model->GetMeshCount(); meshIdx++)
	{
		const IModelMesh* const mesh = model->GetMesh(meshIdx);
		const IModelMeshFlags_t flags = mesh->GetFlags();

		// [rika]: add more triangles
		smd->AddMeshCapacity(mesh->GetVertexCount(), mesh->GetIndiceCount());

		const IModelTexture* const texture = imodel->GetMaterial(mesh->GetMaterialId());
		assertm(texture, "texture should always be valid");

		// [rika]: making the choice to use the stored rmdl name here when possible, as that is what it was likely compiled with
		const char* material = texture->GetName();
		assertm(material, "material name should always be valid");

		material = g_CmdSettings.truncateMaterials ? keepAfterLastSlashOrBackslash(material) : material;

		for (uint32_t vertexIdx = 0; vertexIdx < mesh->GetVertexCount(); vertexIdx++)
		{
			smd::Vertex vertex;
			ParseVertexIntoSMD(mesh->GetVertex(vertexIdx), mesh, &vertex, isStaticProp, vertexIdx);

			smd->InitVertex(&vertex);
		}

		if (flags & (IMODELMESH_FLAG_QUAD | IMODELMESH_FLAG_NGON))
		{
			assertm(mesh->EnsureTriangulation(), "no triangles ?");

			for (uint32_t indiceIdx = 0; indiceIdx < mesh->GetTriangleCount(); indiceIdx++)
			{
				const IModelFace* const face = mesh->GetTriangle(indiceIdx);

				const IModelFaceIndice indice0 = face->GetIndice(0);
				const IModelFaceIndice indice1 = face->GetIndice(1);
				const IModelFaceIndice indice2 = face->GetIndice(2);

				// order of indices is odd
				smd->InitLocalTriangle(material, indice0, indice2, indice1);
			}

			continue;
		}

		for (uint32_t indiceIdx = 0; indiceIdx < mesh->GetIndiceCount(); indiceIdx++)
		{
			const IModelFace* const face = mesh->GetIndice(indiceIdx);

			const IModelFaceIndice indice0 = face->GetIndice(0);
			const IModelFaceIndice indice1 = face->GetIndice(1);
			const IModelFaceIndice indice2 = face->GetIndice(2);

			// order of indices is odd
			smd->InitLocalTriangle(material, indice0, indice2, indice1);
		}
	}

	smd->Write(buf->Buffer(), managedBufferSize);

	g_BufferManager.RelieveBuffer(buf);

	return true;
}

const bool IModelExporter::ExportAnimSMD(const IModelAnimation* const anim, void* const file)
{
	const IModel* const imodel = Model();

	assertm(imodel->GetBoneCount() == anim->GetBoneCount(), "animation's bone count did not match the model's bone count");

	const uint32_t boneCount = imodel->GetBoneCount();

	smd::CStudioModelData* const smd = reinterpret_cast<smd::CStudioModelData* const>(file);

	smd->ResetMeshData(); // clear any mesh data, won't be needing it
	smd->SetName(anim->GetName());

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();

	// TODO: these should be used so we can store less, anim rework is next!
	const Vector deltaPos(0.0f, 0.0f, 0.0f);
	const Quaternion deltaQuat(0.0f, 0.0f, 0.0f, 1.0f);
	const RadianEuler deltaRot(0.0f, 0.0f, 0.0f);
	const bool isDeltaAnim = anim->GetFlags() & IMODELANIM_FLAG_DELTA;

	if (anim->GetFrameCount() == 0)
	{
		smd->ResetFrameData(1ull);

		// init a static pose
		for (uint32_t i = 0; i < imodel->GetBoneCount(); i++)
		{
			const int ibone = static_cast<int>(i);

			if (isDeltaAnim)
			{
				smd->InitFrameBone(0, ibone, deltaPos, deltaRot);
				continue;
			}

			const IModelBone* const bone = imodel->GetBone(i);
			smd->InitFrameBone(0, ibone, bone->GetPos(), bone->GetRot());
		}

		smd->Write(buf->Buffer(), managedBufferSize);

		g_BufferManager.RelieveBuffer(buf);

		return true;
	}

	smd->ResetFrameData(anim->GetFrameCount_UINT());

	// [rika]: declare here, pointless to declare over and over in a loop.
	const IModelMovementTrack* const moveTrack = anim->GetMovementTrack();

	for (uint32_t frame = 0; frame < anim->GetFrameCount_UINT(); frame++)
	{
		for (uint32_t bone = 0; bone < boneCount; bone++)
		{
			const IModelAnimationTrack* const animTrack = anim->GetAnimTrack(bone);

			Vector pos;
			Quaternion q;
			Vector scale;

			animTrack->GetFrame(frame, pos, q, scale);

			// bones with no parent need to be adjusted
			if (bone == 0 || imodel->GetBone(bone)->GetParent() == -1)
			{
				AdjustAnimationOrigin(anim, moveTrack, frame, pos, q, bone);
			}

			const RadianEuler rot(q);
			smd->InitFrameBone(frame, bone, pos, rot);
		}
	}

	smd->Write(buf->Buffer(), managedBufferSize);

	g_BufferManager.RelieveBuffer(buf);

	return true;
}


//
// Export Handling
//

const bool IModelExporter::ExportIModel(const eModelExportFormat format, void* const file)
{
	const IModel* const imodel = Model();

	// exporting qc
	if (g_CmdSettings.qcWrite)
	{
		UpdatePathNamed();

		const bool exported = ExportQC();

		if (exported == false)
		{
			return false;
		}
	}

	UpdatePathRoot(file);

	// export phys file
	if (imodel->GetPhysics() && g_CmdSettings.ignoreMesh == false)
	{
		const IModelPhysics* const phys = imodel->GetPhysics();

		const bool exported = (this->*s_ExportModelFuncs[format])(static_cast<const IModelModel* const>(phys), file);

		// should probably do some log thing eventually
		if (!exported)
		{
			Log("IModelExporter: failed to export model %s from IModel %s\n", phys->GetName(), imodel->GetName());
		}
	}

	// export map collision file
	if (imodel->GetMapColl() && g_CmdSettings.ignoreMesh == false)
	{
		const IModelMapCollision* const coll = imodel->GetMapColl();

		const bool exported = (this->*s_ExportModelFuncs[format])(static_cast<const IModelModel* const>(coll), file);

		// should probably do some log thing eventually
		if (!exported)
		{
			Log("IModelExporter: failed to export model %s from IModel %s\n", coll->GetName(), imodel->GetName());
		}
	}

	// export per tri collision as a triangle mesh (no bounding boxes)
	if (imodel->GetPerTriAABB() && imodel->GetPerTriAABB()->GetVersion() > 0 && g_CmdSettings.ignoreMesh == false)
	{
		const IModelPerTriAABB* const perTri = imodel->GetPerTriAABB();

		const bool exported = (this->*s_ExportModelFuncs[format])(static_cast<const IModelModel* const>(perTri), file);

		// should probably do some log thing eventually
		if (!exported)
		{
			Log("IModelExporter: failed to export model %s from IModel %s\n", perTri->GetName(), imodel->GetName());
		}
	}

	// export ui panels
	if (imodel->GetUIPanelCount() > 0u && g_CmdSettings.ignoreMesh == false)
	{
		for (uint32_t i = 0u; i < imodel->GetUIPanelCount(); i++)
		{
			const IModelUIPanel* const uiPanel = imodel->GetUIPanel(i);

			const bool exported = (this->*s_ExportModelFuncs[format])(static_cast<const IModelModel* const>(uiPanel), file);

			// should probably do some log thing eventually
			if (!exported)
			{
				Log("IModelExporter: failed to export model %s from IModel %s\n", uiPanel->GetName(), imodel->GetName());
			}
		}
	}

	// export model lods
	if (imodel->GetLODCount() > 0u && g_CmdSettings.ignoreMesh == false)
	{
		UpdatePathModel(file);

		for (uint32_t lodIdx = 0u; lodIdx < imodel->GetLODCount(); lodIdx++)
		{
			const IModelLOD* const lod = imodel->GetLOD(lodIdx);

			for (uint16_t modelIdx = 0u; modelIdx < lod->GetModelCount(); modelIdx++)
			{
				const IModelModel* const model = lod->GetModel(modelIdx);

				// blank model
				if (!model->GetName())
					continue;

				const bool exported = (this->*s_ExportModelFuncs[format])(model, file);

				// should probably do some log thing eventually
				if (!exported)
				{
					Log("IModelExporter: failed to export model %s from IModel %s\n", model->GetName(), imodel->GetName());
				}
			}
		}
	}

	// export animations
	if (imodel->GetAnimationCount() > 0u && g_CmdSettings.ignoreAnim == false)
	{
		UpdatePathAnim(file);

		for (uint32_t animIdx = 0u; animIdx < imodel->GetAnimationCount(); animIdx++)
		{
			const IModelAnimation* const anim = imodel->GetAnimation(animIdx);

			const bool exported = (this->*s_ExportAnimFuncs[format])(anim, file);

			if (!exported)
			{
				Log("IModelExporter: failed to export anim %s from IModel %s\n", anim->GetName(), imodel->GetName());
			}
		}
	}

	// other stuff here in the future, like phys, uipanel, coll, anims, etc

	return true;
}

bool IModelExporter::ParseForExport()
{
	const IModel* const imodel = Model();

	switch (g_CmdSettings.format)
	{
	case eModelExportFormat::MODEL_FMT_RMAX:
	{
		rmax::RMAXExporter* const rmax = new rmax::RMAXExporter(Path(), nullptr, Name().c_str());
		ExportSkeletonRMAX(rmax);

		rmax->ReserveMaterials(imodel->GetMaterialCount());
		for (uint32_t i = 0u; i < imodel->GetMaterialCount(); i++)
		{
			rmax->AddMaterial(imodel->GetMaterial(i)->GetName());
		}

		const bool success = ExportIModel(eModelExportFormat::MODEL_FMT_RMAX, rmax);

		FreeAllocVar(rmax);
		return success;
	}
	case eModelExportFormat::MODEL_FMT_DMX:
	{
		//CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();
		//CDataModel* const dmx = new CDataModel(Path(), imodel->GetName(), &g_CmdSettings.dmxVersion, buf->Buffer(), managedBufferSize);

		//dmx->InitForModel();
		//ExportSkeletonDMX(dmx);

		const bool success = ExportIModel(eModelExportFormat::MODEL_FMT_DMX, nullptr);

		//FreeAllocVar(dmx);
		//g_BufferManager.RelieveBuffer(buf);
		return success;
	}
	case eModelExportFormat::MODEL_FMT_SMD:
	{
		const uint32_t numBones = imodel->GetBoneCount();

		// this should never get used if we don't have bones! (pov_male_anims.mdl)
		smd::CStudioModelData* const smd = numBones ? new smd::CStudioModelData(Path(), numBones, 1ull) : nullptr;

		// [rika]: initialize the nodes, and only one frame, animations should be parsed after as they will reset frames
		for (uint32_t i = 0; i < numBones; i++)
		{
			const IModelBone* const bone = imodel->GetBone(i);
			const int ibone = static_cast<int>(i);

			smd->InitNode(bone->GetName(), ibone, bone->GetParent());
			smd->InitFrameBone(0, ibone, bone->GetPos(), bone->GetRot());
		}

		const bool success = ExportIModel(eModelExportFormat::MODEL_FMT_SMD, smd);

		FreeAllocVar(smd);
		return success;
	}
	default:
	{
		return false;
	}
	}
}