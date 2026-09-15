#include <pch.h>

#include <model/studio/bone_setup.h>
#include <model/internalmodel/internalmodel.h>

extern CmdSettings_t g_CmdSettings;
extern CBufferManager g_BufferManager;

//
// Model Source File
//

studiosource_t::studiosource_t(const r1::studiohdr_t* const pHdr) : id(pHdr->id), checksum(pHdr->checksum), version(pHdr->version), version_minor(-1)
{

}

studiosource_t::studiosource_t(const r2::studiohdr_t* const pHdr) : id(pHdr->id), checksum(pHdr->checksum), version(pHdr->version), version_minor(-1)
{

}

studiosource_t::studiosource_t(const r5::studiohdr_t* const pHdr, const IModelSourceVersion_t srcVer) : id(pHdr->id), checksum(pHdr->checksum), version(pHdr->version), version_minor(srcVer.minor)
{
	assertm(pHdr->version == srcVer.major, "provided version was not valid");
}

studiosource_t::~studiosource_t()
{

}


//
// Animation Tracks
//

IModelAnimationTrack::IModelAnimationTrack(const int frameCountIn) : frameCount(frameCountIn), frames(nullptr), bone(-1), name(nullptr), flags(0)
{
	frames = new frame_t[frameCount];
}

IModelAnimationTrack::~IModelAnimationTrack()
{
	FreeAllocArray(name);
	FreeAllocArray(frames);
}

void IModelAnimationTrack::GetFrame(const int frameIdx, Vector& pos, Quaternion& rot, Vector& scale) const
{
	pos = frames[frameIdx].position;
	rot = frames[frameIdx].rotation;
	scale = frames[frameIdx].scale;
}

void IModelAnimationTrack::GetFrameLast(Vector& pos, Quaternion& rot, Vector& scale) const
{
	if (GetFrameCount() < 1)
		return;

	GetFrame(GetFrameCount() - 1, pos, rot, scale);
}

void IModelAnimationTrack::SetBoneName(const char* nameIn)
{
	name = AllocStudioString(nameIn);
}

void IModelAnimationTrack::Init(const int frameCountIn, const int boneIndex, const char* nameIn)
{
	name = AllocStudioString(nameIn);
	bone = boneIndex;
	frameCount = frameCountIn;

	frames = new frame_t[frameCount];
}

void IModelAnimationTrack::AddFrame(const int frameIdx, const Vector& pos, const Quaternion& rot, const Vector& scale)
{
	assert(GetFrameCount());

	frames[frameIdx].position = pos;
	frames[frameIdx].rotation = rot;
	frames[frameIdx].scale = scale;
};

IModelMovementTrack::IModelMovementTrack(const int frameCountIn) : frameCount(frameCountIn)
{
	frames = new frame_t[frameCount];
}

IModelMovementTrack::~IModelMovementTrack()
{
	FreeAllocArray(frames);
}

void IModelMovementTrack::GetFrame(const int frameIdx, Vector& pos, QAngle& rot) const
{
	pos = frames[frameIdx].positions;
	rot = frames[frameIdx].rotations;
}

void IModelMovementTrack::AddFrame(const int frameIdx, const Vector& pos, const QAngle& rot)
{
	assert(GetFrameCount());

	frames[frameIdx].positions = pos;
	frames[frameIdx].rotations = rot;
}


//
// Model Animation data
//

const char* s_CurrentAnimBlockModel = nullptr;
IModelAnimation::IModelAnimation(const r1::studiohdr_t* const pHdr, const r1::mstudioanimdesc_t* const pAnimDesc, const StudioLooseData_t* const looseData) : frameCount(pAnimDesc->numframes), frameRate(pAnimDesc->fps), flags(IMODELANIM_FLAG_NONE),
	movementCount(pAnimDesc->nummovements), movements(nullptr), framemovement(nullptr), ikRuleCount(pAnimDesc->numikrules), ikRules(nullptr), localHierarchyCount(pAnimDesc->numlocalhierarchy), localHierarchies(nullptr),
	sectionFrames(pAnimDesc->sectionframes), sectionStallFrames(0), sectionCount(0), localSectionCount(0), zeroFrameSpan(pAnimDesc->zeroframespan), zeroFrameCount(pAnimDesc->zeroframecount),

	animTracks(nullptr), movementTrack(nullptr), animTrackCount(pHdr->numbones)
{
	assertm(localHierarchyCount == 0, "hierarchies should not be present and are not parsed");

	name = AllocStudioString(pAnimDesc->pszName());

	flags |= (pAnimDesc->flags | eIModelAnimFlags::IMODELANIM_FLAG_HAS_ANIM);

	// mark this animation as having valid data, used in apex but we support it
	flags |= eIModelAnimFlags::IMODELANIM_FLAG_HAS_ANIM;

	// set up tracks for parsing
	animTracks = new IModelAnimationTrack[animTrackCount]{};

	for (int track = 0; track < animTrackCount; track++)
	{
		animTracks[track].Init(GetFrameCount(), track, pHdr->pBone(track)->pszName());
	}

	// parse movements
	if (movementCount)
	{
		flags |= eIModelAnimFlags::IMODELANIM_FLAG_MOVEMENTTRACK;
		movementTrack = new IModelMovementTrack(GetFrameCount());
		
		movements = new IModelMovement[movementCount]{};
		for (int i = 0; i < pAnimDesc->nummovements; i++)
		{
			movements[i] = IModelMovement(pAnimDesc->pMovement(i));
		}
	}

	if (flags & IMODELANIM_FLAG_FRAMEMOVEMENT)
	{
		flags |= eIModelAnimFlags::IMODELANIM_FLAG_MOVEMENTTRACK;
		movementTrack = new IModelMovementTrack(GetFrameCount());

		framemovement = new IModelFrameMovement(pAnimDesc->pFrameMovement());
	}

	if (ikRuleCount)
	{
		ikRules = new IModelIKRule[ikRuleCount]{};

		for (int i = 0; i < pAnimDesc->numikrules; i++)
		{
			ikRules[i] = IModelIKRule(pAnimDesc->pIKRule(i));
		}
	}
	
	if (sectionFrames)
	{
		sectionCount = static_cast<int16_t>(SectionCount());
		localSectionCount = 0;
	}

	// parse local section count
	for (int16_t i = 0; i < sectionCount; i++)
	{
		const r1::mstudioanimsections_t* const section = pAnimDesc->pSection(i);

		if (section->animblock > 0)
		{
			continue;
		}

		localSectionCount++;
	}

	s_CurrentAnimBlockModel = looseData->GetANI();

	// parse out animation data
	Vector positions[256]{};
	Quaternion quats[256]{};
	Vector scales[256]{};

	// normally done per frame but due to how we work, it's not needed here
	if (flags & IMODELANIM_FLAG_DELTA)
	{
		for (int i = 0; i < pHdr->numbones; i++)
		{
			positions[i].Init(0.0f, 0.0f, 0.0f);
			quats[i].Init(0.0f, 0.0f, 0.0f, 1.0f);
			scales[i].Init(1.0f, 1.0f, 1.0f);
		}
	}
	else
	{
		for (int i = 0; i < pHdr->numbones; i++)
		{
			const r1::mstudiobone_t* const bone = pHdr->pBone(i);

			positions[i] = bone->pos;
			quats[i] = bone->quat;
			scales[i] = bone->scale;
		}
	}
	
	for (int frame = 0; frame < GetFrameCount(); frame++)
	{
		const float cycle = GetCycle(frame);

		const float fFrame = cycle * static_cast<float>(pAnimDesc->numframes - 1);

		const int iFrame = static_cast<int>(fFrame);
		const float s = (fFrame - static_cast<float>(iFrame));

		int iLocalFrame = iFrame;
		float flStall = 0.0f;

		const r1::mstudio_rle_anim_t* panim = nullptr;
		const r1::mstudio_frame_anim_t* pFrameanim = nullptr;

		const uint8_t* pBoneFlags = nullptr;
		const uint8_t* pConstantData = nullptr;
		const uint8_t* pFrameData = nullptr;
		int framelength = 0;

		if (pAnimDesc->flags & STUDIO_FRAMEANIM)
		{
			pFrameanim = reinterpret_cast<const r1::mstudio_frame_anim_t* const>(pAnimDesc->pAnim(&iLocalFrame, flStall));
			if (pFrameanim)
			{
				pBoneFlags = pFrameanim->pBoneFlags();
				pConstantData = pFrameanim->pConstantData();
				pFrameData = pFrameanim->pFrameData(iLocalFrame);
				framelength = pFrameanim->framelength;
			}
		}
		else
		{
			panim = reinterpret_cast<const r1::mstudio_rle_anim_t* const>(pAnimDesc->pAnim(&iLocalFrame, flStall));
		}

		const bool bIsDelta = pAnimDesc->flags & STUDIO_DELTA ? true : false;

		// animation stalled
		if (panim == nullptr && pFrameanim == nullptr)
		{
			assertm(false, "uh oh!");
		}

		// rle animation
		if (panim)
		{
			for (int bone = 0; bone < pHdr->numbones; bone++)
			{
				Vector pos;
				Quaternion q;
				Vector scale;

				// source parses weight list?
				// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L1100
				if (panim && panim->bone == bone)
				{
					animTracks[bone].SetFlags(eIModelAnimTrackFlags::TRACK_POS | eIModelAnimTrackFlags::TRACK_ROT | eIModelAnimTrackFlags::TRACK_SCL);

					// need to add a bool here, we do not want the interpolated values (inbetween frames)
					r1::CalcBoneQuaternion(iLocalFrame, s, pHdr->pBone(panim->bone), pHdr->pLinearBones(), panim, q);
					r1::CalcBonePosition(iLocalFrame, s, pHdr->pBone(panim->bone), pHdr->pLinearBones(), panim, pos);
					r1::CalcBoneScale(iLocalFrame, s, pHdr->pBone(panim->bone)->scale, pHdr->pBone(panim->bone)->scalescale, panim, scale);

					panim = panim->pNext();
				}
				else
				{
					pos = positions[bone];
					q = quats[bone];
					scale = scales[bone];
				}

				animTracks[bone].AddFrame(frame, pos, q, scale);
			}
		}
		else if (pFrameanim && pFrameanim->boneLookupTableOffset)
		{
			if (s > 0.0f)
			{
				for (int bone = 0; bone < pHdr->numbones; bone++)
				{
					Vector pos;
					Quaternion q;
					Vector scale;

					// bone offsets means we don't have to parse skipped frames!
					const r1::mstudio_frame_anim_boneoffset_t* const pBoneOffset = pFrameanim->pBoneLookup(bone);

					pConstantData = pFrameanim->pConstantData() + pBoneOffset->constOffset;
					pFrameData = pFrameanim->pFrameData(iLocalFrame) + pBoneOffset->frameOffset;

					const uint8_t boneFlags = pBoneFlags[bone];

					if (boneFlags)
					{
						animTracks[bone].SetFlags(eIModelAnimTrackFlags::TRACK_POS | eIModelAnimTrackFlags::TRACK_ROT | eIModelAnimTrackFlags::TRACK_SCL);
						r1::ExtractTwoFrames(pBoneFlags[bone], s, pFrameData, pConstantData, framelength, q, pos, scale, bIsDelta, pHdr->pLinearBones(), bone);
					}
					else
					{
						pos = positions[bone];
						q = quats[bone];
						scale = scales[bone];
					}

					animTracks[bone].AddFrame(frame, pos, q, scale);
				}
			}
			else
			{
				for (int bone = 0; bone < pHdr->numbones; bone++)
				{
					Vector pos;
					Quaternion q;
					Vector scale;

					// bone offsets means we don't have to parse skipped frames!
					const r1::mstudio_frame_anim_boneoffset_t* const pBoneOffset = pFrameanim->pBoneLookup(bone);

					pConstantData = pFrameanim->pConstantData() + pBoneOffset->constOffset;
					pFrameData = pFrameanim->pFrameData(iLocalFrame) + pBoneOffset->frameOffset;

					const uint8_t boneFlags = pBoneFlags[bone];

					if (boneFlags)
					{
						animTracks[bone].SetFlags(eIModelAnimTrackFlags::TRACK_POS | eIModelAnimTrackFlags::TRACK_ROT | eIModelAnimTrackFlags::TRACK_SCL);
						r1::ExtractSingleFrame(pBoneFlags[bone], pFrameData, pConstantData, q, pos, scale, bIsDelta, pHdr->pLinearBones(), bone);
					}
					else
					{
						pos = positions[bone];
						q = quats[bone];
						scale = scales[bone];
					}

					animTracks[bone].AddFrame(frame, pos, q, scale);
				}
				
			}
		}
		else if (pFrameanim)
		{
			if (s > 0.0f)
			{
				for (int bone = 0; bone < pHdr->numbones; bone++)
				{
					Vector pos;
					Quaternion q;
					Vector scale;

					const uint8_t boneFlags = pBoneFlags[bone];

					if (boneFlags)
					{
						animTracks[bone].SetFlags(eIModelAnimTrackFlags::TRACK_POS | eIModelAnimTrackFlags::TRACK_ROT | eIModelAnimTrackFlags::TRACK_SCL);
						pFrameData = r1::ExtractTwoFrames(pBoneFlags[bone], s, pFrameData, pConstantData, framelength, q, pos, scale, bIsDelta, pHdr->pLinearBones(), bone);
					}
					else
					{
						r1::SkipBoneFrame(pBoneFlags[bone], pConstantData, pFrameData);

						pos = positions[bone];
						q = quats[bone];
						scale = scales[bone];
					}

					animTracks[bone].AddFrame(frame, pos, q, scale);
				}
			}
			else
			{
				for (int bone = 0; bone < pHdr->numbones; bone++)
				{
					Vector pos;
					Quaternion q;
					Vector scale;

					const uint8_t boneFlags = pBoneFlags[bone];

					if (boneFlags)
					{
						animTracks[bone].SetFlags(eIModelAnimTrackFlags::TRACK_POS | eIModelAnimTrackFlags::TRACK_ROT | eIModelAnimTrackFlags::TRACK_SCL);
						pFrameData = r1::ExtractSingleFrame(pBoneFlags[bone], pFrameData, pConstantData, q, pos, scale, bIsDelta, pHdr->pLinearBones(), bone);
					}
					else
					{
						r1::SkipBoneFrame(pBoneFlags[bone], pConstantData, pFrameData);

						pos = positions[bone];
						q = quats[bone];
						scale = scales[bone];
					}

					animTracks[bone].AddFrame(frame, pos, q, scale);
				}
			}
		}

		if (movementTrack)
		{
			Vector pos;
			QAngle rot;

			// above comment about interpolated values
			r1::Studio_AnimPosition(pAnimDesc, cycle, pos, rot);

			movementTrack->AddFrame(frame, pos, rot);
		}
	}

	s_CurrentAnimBlockModel = nullptr;
}

IModelAnimation::IModelAnimation(const r2::studiohdr_t* const pHdr, const r2::mstudioanimdesc_t* const pAnimDesc) : frameCount(pAnimDesc->numframes), frameRate(pAnimDesc->fps), flags(IMODELANIM_FLAG_NONE),
	movementCount(pAnimDesc->nummovements), movements(nullptr), framemovement(nullptr), ikRuleCount(pAnimDesc->numikrules), ikRules(nullptr), localHierarchyCount(pAnimDesc->numlocalhierarchy), localHierarchies(nullptr),
	sectionFrames(pAnimDesc->sectionframes), sectionStallFrames(0), sectionCount(0), localSectionCount(0), zeroFrameSpan(0), zeroFrameCount(0),

	animTracks(nullptr), movementTrack(nullptr), animTrackCount(pHdr->numbones)
{
	assertm(localHierarchyCount == 0, "hierarchies should not be present and are not parsed");

	name = AllocStudioString(pAnimDesc->pszName());

	// exclude STUDIO_FRAMEANIM flag, it's not supported for this version
	constexpr int maskDisabledFlags = ~(STUDIO_UNUSED40);
	assertm((pAnimDesc->flags & STUDIO_UNUSED40) == false, "animation had flag 0x40"); // should not have flag 0x40

	flags |= (pAnimDesc->flags & maskDisabledFlags);

	// mark this animation as having valid data, used in apex but we support it
	flags |= eIModelAnimFlags::IMODELANIM_FLAG_HAS_ANIM;

	// set up tracks for parsing
	animTracks = new IModelAnimationTrack[animTrackCount]{};

	for (int track = 0; track < animTrackCount; track++)
	{
		animTracks[track].Init(GetFrameCount(), track, pHdr->pBone(track)->pszName());
	}

	// parse movements
	if (movementCount)
	{
		flags |= eIModelAnimFlags::IMODELANIM_FLAG_MOVEMENTTRACK;
		movementTrack = new IModelMovementTrack(GetFrameCount());
		
		movements = new IModelMovement[movementCount]{};
		for (int i = 0; i < pAnimDesc->nummovements; i++)
		{
			movements[i] = IModelMovement(pAnimDesc->pMovement(i));
		}
	}

	if (flags & IMODELANIM_FLAG_FRAMEMOVEMENT)
	{
		flags |= eIModelAnimFlags::IMODELANIM_FLAG_MOVEMENTTRACK;
		movementTrack = new IModelMovementTrack(GetFrameCount());

		framemovement = new IModelFrameMovement(pAnimDesc->pFrameMovement());
	}

	if (ikRuleCount)
	{
		ikRules = new IModelIKRule[ikRuleCount]{};

		for (int i = 0; i < pAnimDesc->numikrules; i++)
		{
			ikRules[i] = IModelIKRule(pAnimDesc->pIKRule(i));
		}
	}

	if (sectionFrames)
	{
		sectionCount = static_cast<int16_t>(SectionCount());
		localSectionCount = sectionCount;
	}

	// parse out animation data
	Vector positions[256]{};
	Quaternion quats[256]{};
	Vector scales[256]{};

	// normally done per frame but due to how we work, it's not needed here
	if (flags & IMODELANIM_FLAG_DELTA)
	{
		for (int i = 0; i < pHdr->numbones; i++)
		{
			positions[i].Init(0.0f, 0.0f, 0.0f);
			quats[i].Init(0.0f, 0.0f, 0.0f, 1.0f);
			scales[i].Init(1.0f, 1.0f, 1.0f);
		}
	}
	else
	{
		for (int i = 0; i < pHdr->numbones; i++)
		{
			const r2::mstudiobone_t* const bone = pHdr->pBone(i);

			positions[i] = bone->pos;
			quats[i] = bone->quat;
			scales[i] = bone->scale;
		}
	}
	
	for (int frame = 0; frame < GetFrameCount(); frame++)
	{
		const float cycle = GetCycle(frame);

		const float fFrame = cycle * static_cast<float>(pAnimDesc->numframes - 1);

		const int iFrame = static_cast<int>(fFrame);
		const float s = (fFrame - static_cast<float>(iFrame));

		int iLocalFrame = iFrame;

		const r2::mstudio_rle_anim_t* panim = pAnimDesc->pAnim(&iLocalFrame);

		for (int bone = 0; bone < pHdr->numbones; bone++)
		{
			Vector pos;
			Quaternion q;
			Vector scale;

			// source parses weight list?
			// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L1100
			if (panim && panim->bone == bone)
			{
				animTracks[bone].SetFlags(eIModelAnimTrackFlags::TRACK_POS | eIModelAnimTrackFlags::TRACK_ROT | eIModelAnimTrackFlags::TRACK_SCL);

				// need to add a bool here, we do not want the interpolated values (inbetween frames)
				r2::CalcBonePosition(iLocalFrame, s, pHdr->pBone(panim->bone), pHdr->pLinearBones(), panim, pos);
				r2::CalcBoneQuaternion(iLocalFrame, s, pHdr->pBone(panim->bone), pHdr->pLinearBones(), panim, q);
				r2::CalcBoneScale(iLocalFrame, s, pHdr->pBone(panim->bone)->scale, pHdr->pBone(panim->bone)->scalescale, panim, scale);

				panim = panim->pNext();
			}
			else
			{
				pos = positions[bone];
				q = quats[bone];
				scale = scales[bone];
			}			

			animTracks[bone].AddFrame(frame, pos, q, scale);
		}

		if (movementTrack)
		{
			Vector pos;
			QAngle rot;

			// above comment about interpolated values
			r2::Studio_AnimPosition(pAnimDesc, cycle, pos, rot);

			movementTrack->AddFrame(frame, pos, rot);
		}
	}
}

IModelAnimation::~IModelAnimation()
{
	FreeAllocArray(name);

	FreeAllocArray(movements);
	FreeAllocVar(framemovement);
	FreeAllocArray(ikRules);
	FreeAllocArray(localHierarchies);

	FreeAllocArray(animTracks);
	FreeAllocVar(movementTrack);
}


//
// Model Sequence Data
//

// events
IModelEvent::IModelEvent(const r1::mstudioevent_t* const pEvent) : cycle(pEvent->cycle), unk(0.0f), event(pEvent->event), type(pEvent->type)
{
	name = AllocStudioString(pEvent->pszEvent());
	options = AllocStudioString(pEvent->options, 64);
}

IModelEvent::IModelEvent(const r2::mstudioevent_t* const pEvent) : cycle(pEvent->cycle), unk(0.0f), event(pEvent->event), type(pEvent->type)
{
	name = AllocStudioString(pEvent->pszEvent());
	options = AllocStudioString(pEvent->options, 64);
}

IModelEvent::IModelEvent(const IModelEvent& eventIn) : cycle(eventIn.cycle), unk(eventIn.unk), event(eventIn.event), type(eventIn.type)
{
	name = AllocStudioString(eventIn.name);
	options = AllocStudioString(eventIn.options);
}

IModelEvent::~IModelEvent()
{
	FreeAllocArray(name);
	FreeAllocArray(options);
}

// activity modifier
IModelActMod::IModelActMod(const r1::mstudioactivitymodifier_t* const pActMod) : negate(pActMod->negate)
{
	name = AllocStudioString(pActMod->pszName());
}

IModelActMod::IModelActMod(const r2::mstudioactivitymodifier_t* const pActMod) : negate(pActMod->negate)
{
	name = AllocStudioString(pActMod->pszName());
}

IModelActMod::IModelActMod(const IModelActMod& actModIn) : negate(actModIn.negate)
{
	name = AllocStudioString(actModIn.name);
}

IModelActMod::~IModelActMod()
{
	FreeAllocArray(name);
}

// sequence
IModelSequence::IModelSequence(const r1::studiohdr_t* const pHdr, const r1::mstudioseqdesc_t* const pSeqDesc) : flags(IMODELANIM_FLAG_NONE), actWeight(pSeqDesc->actweight), bbmin(pSeqDesc->bbmin), bbmax(pSeqDesc->bbmax),
	events(nullptr), eventCount(pSeqDesc->numevents), blends(nullptr), blendCount(pSeqDesc->numblends),
	paramParent(pSeqDesc->paramparent), fadeInTime(pSeqDesc->fadeintime), fadeOutTime(pSeqDesc->fadeouttime), localEntryNode(pSeqDesc->localentrynode), localExitNode(pSeqDesc->localexitnode), nodeFlags(pSeqDesc->nodeflags),
	entryPhase(pSeqDesc->entryphase), exitPhase(pSeqDesc->exitphase), lastFrame(pSeqDesc->lastframe), nextSeq(pSeqDesc->nextseq), pose(pSeqDesc->pose),
	ikRuleCount(pSeqDesc->numikrules), autoLayers(nullptr), autoLayerCount(pSeqDesc->numautolayers), weights(nullptr), weightCount(pHdr->numbones), poseKeys(nullptr), ikLocks(nullptr), ikLockCount(pSeqDesc->numiklocks),
	keyvalues(nullptr), activityModifiers(nullptr), activityModifierCount(pSeqDesc->numactivitymodifiers), 
	cyclePoseIndex(pSeqDesc->cycleposeindex), ikResetMask(pSeqDesc->ikResetMask), unk(pSeqDesc->unk_C4)
{
	label = AllocStudioString(pSeqDesc->pszLabel());
	activity = AllocStudioString(pSeqDesc->pszActivityName());

	flags |= pSeqDesc->flags;

	assertm(pSeqDesc->AnimCount() == pSeqDesc->numblends, "blend count mismatch");

	if (eventCount)
	{
		events = new IModelEvent[eventCount]{};

		for (int i = 0; i < pSeqDesc->numevents; i++)
		{
			events[i] = IModelEvent(pSeqDesc->pEvent(i));
		}
	}

	if (blendCount)
	{
		blends = AllocStudioBuffer(pSeqDesc->pAnimIndex(0), pSeqDesc->AnimCount());
	}

	// init some arrays
	groupSize[0] = pSeqDesc->groupsize[0];
	groupSize[1] = pSeqDesc->groupsize[1];
	paramIndex[0] = pSeqDesc->paramindex[0];
	paramIndex[1] = pSeqDesc->paramindex[1];
	paramStart[0] = pSeqDesc->paramstart[0];
	paramStart[1] = pSeqDesc->paramstart[1];
	paramEnd[0] = pSeqDesc->paramend[0];
	paramEnd[1] = pSeqDesc->paramend[1];

	if (autoLayerCount)
	{
		autoLayers = new IModelAutoLayer[autoLayerCount]{};

		for (int i = 0; i < pSeqDesc->numautolayers; i++)
		{
			autoLayers[i] = IModelAutoLayer(pSeqDesc->pAutoLayer(i));
		}
	}

	assertm(pSeqDesc->weightlistindex, "sequence should have a weight list");
	weights = AllocStudioBuffer(pSeqDesc->pBoneweight(0), weightCount);

	if (pSeqDesc->posekeyindex)
	{
		poseKeys = AllocStudioBuffer(pSeqDesc->pPoseKey(0, 0), pSeqDesc->groupsize[0] + pSeqDesc->groupsize[1]);
	}

	if (ikLockCount)
	{
		assertm(false, "sequence had iklocks!");

		ikLocks = new IModelIKLock[ikLockCount]{};

		for (int i = 0; i < pSeqDesc->numiklocks; i++)
		{
			ikLocks[i] = IModelIKLock(pSeqDesc->pIKLock(i));
		}
	}

	if (pSeqDesc->keyvaluesize && pSeqDesc->keyvalueindex)
	{
		keyvalues = AllocStudioString(pSeqDesc->pKeyValues(), static_cast<size_t>(pSeqDesc->keyvaluesize));
	}

	if (activityModifierCount)
	{
		activityModifiers = new IModelActMod[activityModifierCount]{};

		for (int i = 0; i < pSeqDesc->numactivitymodifiers; i++)
		{
			activityModifiers[i] = IModelActMod(pSeqDesc->pActivityModifier(i));
		}
	}
}

IModelSequence::IModelSequence(const r2::studiohdr_t* const pHdr, const r2::mstudioseqdesc_t* const pSeqDesc) : flags(IMODELANIM_FLAG_NONE), actWeight(pSeqDesc->actweight), bbmin(pSeqDesc->bbmin), bbmax(pSeqDesc->bbmax),
	events(nullptr), eventCount(pSeqDesc->numevents), blends(nullptr), blendCount(pSeqDesc->numblends),
	paramParent(pSeqDesc->paramparent), fadeInTime(pSeqDesc->fadeintime), fadeOutTime(pSeqDesc->fadeouttime), localEntryNode(pSeqDesc->localentrynode), localExitNode(pSeqDesc->localexitnode), nodeFlags(pSeqDesc->nodeflags),
	entryPhase(pSeqDesc->entryphase), exitPhase(pSeqDesc->exitphase), lastFrame(pSeqDesc->lastframe), nextSeq(pSeqDesc->nextseq), pose(pSeqDesc->pose),
	ikRuleCount(pSeqDesc->numikrules), autoLayers(nullptr), autoLayerCount(pSeqDesc->numautolayers), weights(nullptr), weightCount(pHdr->numbones), poseKeys(nullptr), ikLocks(nullptr), ikLockCount(pSeqDesc->numiklocks),
	keyvalues(nullptr), activityModifiers(nullptr), activityModifierCount(pSeqDesc->numactivitymodifiers), 
	cyclePoseIndex(pSeqDesc->cycleposeindex), ikResetMask(pSeqDesc->ikResetMask), unk(pSeqDesc->unk_C4)
{
	label = AllocStudioString(pSeqDesc->pszLabel());
	activity = AllocStudioString(pSeqDesc->pszActivityName());

	// exclude STUDIO_FRAMEANIM flag, it's not supported for this version
	constexpr int maskDisabledFlags = ~(STUDIO_UNUSED40);
	assertm((pSeqDesc->flags & STUDIO_UNUSED40) == false, "animation had flag 0x40"); // should not have flag 0x40

	flags |= (pSeqDesc->flags & maskDisabledFlags);

	assertm(pSeqDesc->AnimCount() == pSeqDesc->numblends, "blend count mismatch");

	if (eventCount)
	{
		events = new IModelEvent[eventCount]{};

		for (int i = 0; i < pSeqDesc->numevents; i++)
		{
			events[i] = IModelEvent(pSeqDesc->pEvent(i));
		}
	}

	if (blendCount)
	{
		blends = AllocStudioBuffer(pSeqDesc->pAnimIndex(0), pSeqDesc->AnimCount());
	}

	// init some arrays
	groupSize[0] = pSeqDesc->groupsize[0];
	groupSize[1] = pSeqDesc->groupsize[1];
	paramIndex[0] = pSeqDesc->paramindex[0];
	paramIndex[1] = pSeqDesc->paramindex[1];
	paramStart[0] = pSeqDesc->paramstart[0];
	paramStart[1] = pSeqDesc->paramstart[1];
	paramEnd[0] = pSeqDesc->paramend[0];
	paramEnd[1] = pSeqDesc->paramend[1];

	if (autoLayerCount)
	{
		autoLayers = new IModelAutoLayer[autoLayerCount]{};

		for (int i = 0; i < pSeqDesc->numautolayers; i++)
		{
			autoLayers[i] = IModelAutoLayer(pSeqDesc->pAutoLayer(i));
		}
	}

	assertm(pSeqDesc->weightlistindex, "sequence should have a weight list");
	weights = AllocStudioBuffer(pSeqDesc->pBoneweight(0), weightCount);

	if (pSeqDesc->posekeyindex)
	{
		poseKeys = AllocStudioBuffer(pSeqDesc->pPoseKey(0, 0), pSeqDesc->groupsize[0] + pSeqDesc->groupsize[1]);
	}

	if (ikLockCount)
	{
		assertm(false, "sequence had iklocks!");

		ikLocks = new IModelIKLock[ikLockCount]{};

		for (int i = 0; i < pSeqDesc->numiklocks; i++)
		{
			ikLocks[i] = IModelIKLock(pSeqDesc->pIKLock(i));
		}
	}

	if (pSeqDesc->keyvaluesize && pSeqDesc->keyvalueindex)
	{
		keyvalues = AllocStudioString(pSeqDesc->pKeyValues(), static_cast<size_t>(pSeqDesc->keyvaluesize));
	}

	if (activityModifierCount)
	{
		activityModifiers = new IModelActMod[activityModifierCount]{};

		for (int i = 0; i < pSeqDesc->numactivitymodifiers; i++)
		{
			activityModifiers[i] = IModelActMod(pSeqDesc->pActivityModifier(i));
		}
	}
}

IModelSequence::~IModelSequence()
{
	FreeAllocArray(label);
	FreeAllocArray(activity);

	FreeAllocArray(events);
	FreeAllocArray(blends);
	FreeAllocArray(autoLayers);
	FreeAllocArray(weights);
	FreeAllocArray(poseKeys);
	FreeAllocArray(ikLocks);
	FreeAllocArray(keyvalues);
	FreeAllocArray(activityModifiers);
}


//
// Model Vertex Data
//

IModelFace::IModelFace(const IModelFaceIndice numIndices, const IModelFaceIndice a, const IModelFaceIndice b, const IModelFaceIndice c, const IModelFaceIndice d = 0u) : indiceCount(numIndices), indiceExtra(nullptr)
{
	assertm(indiceCount == 3 || indiceCount == 4, "invalid type");

	indices[0] = a;
	indices[1] = b;
	indices[2] = c;
	indices[3] = d;
};
IModelFace::IModelFace(const IModelFaceIndice numIndices, const IModelFaceIndice* const pIndices) : indiceCount(numIndices), indiceExtra(nullptr)
{
	assertm(numIndices && numIndices <= IMODELFACE_maxIndices, "invailid index count");

	memcpy_s(indices, indiceSize, pIndices, indiceSize);

	if (IMODELFACE_maxLocalIndices >= numIndices)
		return;

	const IModelFaceIndice numExtraIndices = numIndices - IMODELFACE_maxLocalIndices;
	indiceExtra = new IModelFaceIndice[numExtraIndices]{};
	memcpy_s(indiceExtra, sizeof(IModelFaceIndice) * numExtraIndices, pIndices + IMODELFACE_maxLocalIndices, sizeof(IModelFaceIndice) * numExtraIndices);
};
IModelFace::IModelFace(const IModelFace& face) : indiceCount(face.indiceCount), indiceExtra(nullptr)
{
	memcpy_s(indices, indiceSize, face.indices, indiceSize);

	if (face.indiceExtra == nullptr)
		return;

	const IModelFaceIndice numExtraIndices = indiceCount - IMODELFACE_maxLocalIndices;
	indiceExtra = new IModelFaceIndice[numExtraIndices]{};
	memcpy_s(indiceExtra, sizeof(IModelFaceIndice) * numExtraIndices, face.indiceExtra, sizeof(IModelFaceIndice) * numExtraIndices);
}

void IModelFace::FlipOrder()
{
	IModelFaceIndice reordered[IMODELFACE_maxIndices]{};

	// keeps the first indice first
	reordered[0] = GetIndice(0);

	for (IModelFaceIndice i = 1u, j = (indiceCount - 1u); i < indiceCount; i++, j--)
	{
		assertm(j >= 0, "invalid index");

		reordered[i] = GetIndice(j);
	}

	memcpy_s(indices, indiceSize, reordered, indiceSize);

	if (indiceCount > IMODELFACE_maxLocalIndices)
	{
		const IModelFaceIndice extraCount = indiceCount - IMODELFACE_maxLocalIndices;
		const size_t extraSize = sizeof(IModelFaceIndice) * extraCount;
		memcpy_s(indiceExtra, extraSize, reordered + extraCount, extraSize);
	}
}

IModelVertex::~IModelVertex()
{

}

// for normal source
void IModelVertex::ParseWeight(IModelVertex* const vertex, IModelMesh* const mesh, const OptimizedModel::Vertex_t* const pVert, const vvd::mstudiovertex_t* const pSrcVert, const OptimizedModel::BoneStateChangeHeader_t* const pBoneStates, const bool isHwSkinned)
{
	IModelMeshData* const meshData = mesh->GetData();

	vertex->weightIndex = mesh->GetWeightCount();
	vertex->weightCount = static_cast<uint8_t>(pSrcVert->m_BoneWeights.numbones);
	mesh->AddWeights(vertex->GetWeightCount());

	IModelVertexBoneWeight* const pLocalWeights = meshData->weights + vertex->weightIndex;

	for (uint32_t i = 0; i < vertex->weightCount; i++)
	{
		pLocalWeights[i].weight = pSrcVert->m_BoneWeights.weight[pVert->boneWeightIndex[i]];

		// static props can be hardware skinned, but have no bonestates (one bone). this make it skip this, however it's a non issue as the following statement will work fine for this (there is only one bone at idx 0)
		if (isHwSkinned && pBoneStates)
		{
			pLocalWeights[i].bone = pBoneStates[pVert->boneID[i]].newBoneID;
			continue;
		}

		pLocalWeights[i].bone = pVert->boneID[i];
	}
}

// for apex legends
void IModelVertex::ParseWeight(IModelVertex* const vertex, IModelMesh* const mesh, const vvd::mstudiovertex_t* const pSrcVert, const vvw::mstudioboneweightextra_t* const pSrcWeights)
{
	IModelMeshData* const meshData = mesh->GetData();

	vertex->weightIndex = mesh->GetWeightCount();
	vertex->weightCount = static_cast<uint8_t>(pSrcVert->m_BoneWeights.numbones);
	mesh->AddWeights(vertex->GetWeightCount());

	IModelVertexBoneWeight* const pLocalWeights = meshData->weights + vertex->weightIndex;

	if (pSrcWeights)
	{
		const vvw::mstudioboneweightextra_t* const pExtraWeights = pSrcWeights + pSrcVert->m_BoneWeights.weightextra.extraweightindex;

		for (int i = 0; i < pSrcVert->m_BoneWeights.numbones; i++)
		{
			ParseExtraWeight(pSrcVert, pExtraWeights, i, pLocalWeights[i].weight, pLocalWeights[i].bone);
		}
	}
	else
	{
		for (int i = 0; i < pSrcVert->m_BoneWeights.numbones; i++)
		{
			pLocalWeights[i].bone = pSrcVert->m_BoneWeights.bone[i];
			pLocalWeights[i].weight = pSrcVert->m_BoneWeights.weight[i];
		}
	}
}

// get a weight and bone given the index desired, a vertex, and a pointer to the extra weights
void IModelVertex::ParseExtraWeight(const vvd::mstudiovertex_t* const pSrcVert, const vvw::mstudioboneweightextra_t* const pExtraBoneWeights, const int idx, float& weight, int& bone)
{
	if (idx >= 3)
	{
		bone = pExtraBoneWeights[idx - 3].bone;
		weight = pExtraBoneWeights[idx - 3].Weight();

		return;
	}

	bone = pSrcVert->m_BoneWeights.bone[idx];
	weight = pSrcVert->m_BoneWeights.weightextra.Weight(idx);
}

// for general data
void IModelVertex::ParseFromVTX(IModelVertex* const vertex, const uint32_t vertexIndex, IModelMesh* const mesh, const int origId, const vvd::mstudiovertex_t* const pSrcVerts, const Vector4D* const pSrcTangs, const Color32* const pSrcColors, const Vector2D* const pSrcTexcoords)
{
	const vvd::mstudiovertex_t* const pSrcVert = pSrcVerts + origId;

	vertex->position = pSrcVert->m_vecPosition;
	vertex->normal = pSrcVert->m_vecNormal;
	vertex->tangent = pSrcTangs[origId];

	IModelMeshData* const meshData = mesh->GetData();

	if (pSrcColors && meshData->colors)
	{
		meshData->colors[vertexIndex] = pSrcColors[origId];
	}

	Vector2D* const pLocalTexcoords = meshData->texcoords + (vertexIndex * mesh->GetTexcoordCount());
	pLocalTexcoords[0] = pSrcVert->m_vecTexCoord;

	// only supports a second texcoord, technically can have three but not sure if anything shipped like that
	if (pSrcColors && meshData->colors)
	{
		pLocalTexcoords[1] = pSrcTexcoords[origId];
	}
}

// for normal source
void IModelVertex::ParseFromVTX(IModelVertex* const vertex, const uint32_t vertexIndex, IModelMesh* const mesh, const OptimizedModel::Vertex_t* const pVert, const vvd::mstudiovertex_t* const pSrcVerts, const Vector4D* const pSrcTangs, const Color32* const pSrcColors, const Vector2D* const pSrcTexcoords,
	const OptimizedModel::BoneStateChangeHeader_t* const pBoneStates, const bool isHwSkinned)
{
	const int origId = pVert->origMeshVertID;

	ParseFromVTX(vertex, vertexIndex, mesh, origId, pSrcVerts, pSrcTangs, pSrcColors, pSrcTexcoords);
	ParseWeight(vertex, mesh, pVert, pSrcVerts + origId, pBoneStates, isHwSkinned);
}

// for apex legends
void IModelVertex::ParseFromVTX(IModelVertex* const vertex, const uint32_t vertexIndex, IModelMesh* const mesh, const OptimizedModel::Vertex_t* const pVert, const vvd::mstudiovertex_t* const pSrcVerts, const Vector4D* const pSrcTangs, const Color32* const pSrcColors, const Vector2D* const pSrcTexcoords,
	const vvw::mstudioboneweightextra_t* const pSrcWeights)
{
	const int origId = pVert->origMeshVertID;

	ParseFromVTX(vertex, vertexIndex, mesh, origId, pSrcVerts, pSrcTangs, pSrcColors, pSrcTexcoords);
	ParseWeight(vertex, mesh, pSrcVerts + origId, pSrcWeights);
}

// for ivps
void IModelVertex::ParseFromIVPS(IModelVertex* const vertex, IModelMesh* const mesh, const Vector* const pPos, const IModelVertexBoneWeight* const pWeight)
{
	vertex->position = *pPos;
	vertex->normal.Init();
	vertex->tangent.Init();

	if (pWeight == nullptr)
	{
		vertex->weightIndex = 0u;
		vertex->weightCount = 0u;

		return;
	}

	vertex->weightIndex = mesh->GetWeightCount();
	vertex->weightCount = 1u;
	mesh->AddWeights(vertex->GetWeightCount());

	IModelMeshData* const meshData = mesh->GetData();
	IModelVertexBoneWeight* const pLocalWeight = meshData->weights + vertex->weightIndex;

	pLocalWeight->bone = pWeight->bone;
	pLocalWeight->weight = pWeight->weight;
}


//
// Model Mesh Data
//

IModelMeshData::IModelMeshData(const uint32_t vertexCount, const uint32_t indiceCount, const uint16_t weightsPerVertex, const uint16_t texcoordsPerVertex, const bool useVertexColor, char* tempBuf = nullptr) : buffer(nullptr), vertices(nullptr), indices(nullptr), triangles(nullptr),
	weights(nullptr), texcoords(nullptr), colors(nullptr)
{
	if (tempBuf == nullptr)
	{
		const size_t maxRequiredSize = RequiredSize(vertexCount, indiceCount, 0u, vertexCount * weightsPerVertex, texcoordsPerVertex, useVertexColor);
		buffer = new char[maxRequiredSize];

		tempBuf = buffer;
	}

	vertices = reinterpret_cast<IModelVertex*>(tempBuf);
	indices = reinterpret_cast<IModelFace*>(vertices + vertexCount);
	colors = reinterpret_cast<Color32*>(indices + indiceCount);
	texcoords = reinterpret_cast<Vector2D*>(colors + (vertexCount * useVertexColor));
	weights = reinterpret_cast<IModelVertexBoneWeight*>(texcoords + (vertexCount * texcoordsPerVertex));

	if (!useVertexColor)
	{
		colors = nullptr;
	}

	if (texcoordsPerVertex == 0u)
	{
		texcoords = nullptr;
	}

	if (weightsPerVertex == 0u)
	{
		weights = nullptr;
	}
}

IModelMeshData::~IModelMeshData()
{
	FreeAllocArray(buffer);
}

IModelMesh::IModelMesh(const uint32_t lodIndex, const IModelSourceFlags_t sourceFlags, const r1::mstudiomesh_t* const pStudioMesh, const OptimizedModel::MeshHeader_t* const pVertexMesh, const StudioLooseData_t* const pLooseData) : flags(IMODELMESH_FLAG_NONE), material(pStudioMesh->material),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(1u), maxWeightsPerVert(0u), meshid(pStudioMesh->meshid), center(pStudioMesh->center)
{
	if (!pVertexMesh->numStripGroups)
		return;

	flags |= IMODEL_defaultMeshFlags_VTX;

	ParseSourceFlags(sourceFlags);

	const int baseVertexOffset = (pStudioMesh->pModel()->vertexindex / sizeof(vvd::mstudiovertex_t)) + pStudioMesh->vertexoffset;

	FromValveVertex(pVertexMesh, lodIndex, baseVertexOffset, pStudioMesh->numvertices, false, pLooseData->GetVVD(), pLooseData->GetVVC(), pLooseData->GetVVW());
}

IModelMesh::IModelMesh(const uint32_t lodIndex, const IModelSourceFlags_t sourceFlags, const r2::mstudiomesh_t* const pStudioMesh, const OptimizedModel::MeshHeader_t* const pVertexMesh, const StudioLooseData_t* const pLooseData) : flags(IMODELMESH_FLAG_NONE), material(pStudioMesh->material),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(1u), maxWeightsPerVert(0u), meshid(pStudioMesh->meshid), center(pStudioMesh->center)
{
	if (!pVertexMesh->numStripGroups)
		return;

	flags |= IMODEL_defaultMeshFlags_VTX;

	ParseSourceFlags(sourceFlags);

	const int baseVertexOffset = (pStudioMesh->pModel()->vertexindex / sizeof(vvd::mstudiovertex_t)) + pStudioMesh->vertexoffset;

	FromValveVertex(pVertexMesh, lodIndex, baseVertexOffset, pStudioMesh->numvertices, false, pLooseData->GetVVD(), pLooseData->GetVVC(), pLooseData->GetVVW());
}

IModelMesh::IModelMesh(const uint32_t lodIndex, const IModelSourceFlags_t sourceFlags, const r5::mstudiomesh_t* const pStudioMesh, const OptimizedModel::MeshHeader_t* const pVertexMesh, const StudioLooseData_t* const pLooseData) : flags(IMODELMESH_FLAG_NONE), material(pStudioMesh->material),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(1u), maxWeightsPerVert(0u), meshid(pStudioMesh->meshid), center(pStudioMesh->center)
{
	if (!pVertexMesh->numStripGroups)
		return;

	flags |= IMODEL_defaultMeshFlags_VTX;

	ParseSourceFlags(sourceFlags);

	const int baseVertexOffset = (pStudioMesh->pModel()->vertexindex / sizeof(vvd::mstudiovertex_t)) + pStudioMesh->vertexoffset;

	FromValveVertex(pVertexMesh, lodIndex, baseVertexOffset, pStudioMesh->numvertices, false, pLooseData->GetVVD(), pLooseData->GetVVC(), pLooseData->GetVVW());
}

IModelMesh::IModelMesh(const IModel* const imodel, IModelPhysics* const physics, const ivps::phyheader_t* const pPHYS) : flags(IMODELMESH_FLAG_NONE), material(IMODELTEXTURE_PHYSICS),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(0u), maxWeightsPerVert(1u), meshid(0), center(0.0f)
{
	using namespace PhysicsModel;

	flags |= (IMODELMESH_FLAG_TRI | IMODELMESH_FLAG_POS | IMODELMESH_FLAG_WEIGHTS);

	CManagedBuffer* const meshBuf = g_BufferManager.ClaimBuffer();
	data = new IModelMeshData(MAXSTUDIOVERTS, MAXSTUDIOTRIANGLES, maxWeightsPerVert, texcoordCount, false, meshBuf->Buffer());

	const CParsedPhys* const parsedPhys = physics->GetParsedPhys();

	const ivps::compactsurfaceheader_t* pSurface = pPHYS->pFirstSurface();

	for (int i = 0; i < pPHYS->solidCount; i++)
	{
		const Solid* const solid = parsedPhys->GetSolid(static_cast<uint32_t>(i));
		const ivps::legacysurfaceheader_t* const pLegacySurface = pSurface->pLegacySurface();
		const ivps::compactledgenode_t* const pRootNode = pLegacySurface->pRootNode();

		uint32_t numMaxVerts = 0u;
		const int numLeaves = FromIVPSNode(pRootNode, numMaxVerts);
		physics->SetMaxConvexPieces(static_cast<int16_t>(numLeaves));

		assertm(numMaxVerts, "triangle requires more than one vertex");
		numMaxVerts++; // max the max index a count

		const uint32_t boneIndex = imodel->GetBoneIndex(solid->GetName());
		const uint32_t resolvedBoneIndex = boneIndex > imodel->GetBoneCount() ? 0 : boneIndex; // boneIndex is invalid if the name is not a bo

		matrix3x4_t matrix;
		MatrixInvert(*imodel->GetBone(resolvedBoneIndex)->GetPoseToBone(), matrix);

		const ivps::compactledge_t* const pRootLEdge = pRootNode->pLEdge();
		assertm(pRootLEdge, "have to figure something out");

		const IModelVertexBoneWeight boneWeight(resolvedBoneIndex, 1.0f);
		for (int pointIdx = 0; pointIdx < static_cast<int>(numMaxVerts); pointIdx++)
		{
			// breaks on some, what's the deal?
			Vector pos;
			ivps::SourceFromIVPS(pRootLEdge->GetPoint(pointIdx), pos);

			// hack
			// todo: why is this required?
			// fix up similar to crowbar, could not figure out how it works unfortunately
			if (parsedPhys->IsJointed() == false && (imodel->GetFlags() & IMODEL_FLAG_STATIC_PROP) == false)
			{
				ivps::SourceFromIVPS_Silly(pRootLEdge->GetPoint(pointIdx), pos);
			}

			Vector out;
			VectorTransform(pos.Base(), matrix, out.Base());

			AddVertex(&out, &boneWeight);
		}

		pSurface = pSurface->pNextSurface();
	}

	physics->SetConcavePerJoint();

	assertm(MAXSTUDIOVERTS >= vertexCount, "mesh had too many vertices");
	assertm(MAXSTUDIOTRIANGLES >= indiceCount, "mesh had too many triangles"); // number of tris (numIndices / 3)

	GenerateNormals();

	ShrinkData();
	g_BufferManager.RelieveBuffer(meshBuf);
}

IModelMesh::IModelMesh(const IModel* const imodel, IModelMapCollision* const collision) : flags(IMODELMESH_FLAG_NONE), material(IMODELTEXTURE_PHYSICS),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(0u), maxWeightsPerVert(1u), meshid(0), center(0.0f)
{
	flags |= (IMODELMESH_FLAG_POS | IMODELMESH_FLAG_WEIGHTS);

	CManagedBuffer* const meshBuf = g_BufferManager.ClaimBuffer();
	data = new IModelMeshData(MAXSTUDIOVERTS, MAXSTUDIOTRIANGLES, maxWeightsPerVert, texcoordCount, false, meshBuf->Buffer());

	for (int i = 0; i < collision->GetShapeCount(); i++)
	{
		const IModelMapCollisionShape* const shape = collision->GetShape(i);

		const uint32_t boneIndex = shape->GetParent();

		matrix3x4_t matrix;
		MatrixInvert(*imodel->GetBone(boneIndex)->GetPoseToBone(), matrix);

		// index to adjust our side indices by
		const uint16_t baseIndiceIndex = vertexCount;

		// basic bone weight attached to the required bone
		const IModelVertexBoneWeight boneWeight(boneIndex, 1.0f);
		for (int vertIdx = 0; vertIdx < shape->GetVertexCount(); vertIdx++)
		{
			const Vector& pos = shape->GetVertex(vertIdx);

			Vector out;
			VectorTransform(pos.Base(), matrix, out.Base());

			AddVertex(&out, &boneWeight);
		}

		for (int sideIdx = 0; sideIdx < shape->GetSideCount(); sideIdx++)
		{
			const IModelMapCollisionSide* const side = shape->GetSide(sideIdx);

			if (side->GetVertexCount() == 0)
				continue;

			flags |= side->GetFaceType();

			IModelFaceIndice indices[IMODELFACE_maxIndices]{};

			const IModelFaceIndice sideVertCount = side->GetVertexCount();
			for (uint8_t indiceIdx = 0; indiceIdx < sideVertCount; indiceIdx++)
			{
				const IModelFaceIndice indice = side->GetVertexIndice(indiceIdx);
				indices[indiceIdx] = indice + baseIndiceIndex;
			}

			AddIndice(sideVertCount, indices);
		}
	}

	assertm(MAXSTUDIOVERTS >= vertexCount, "mesh had too many vertices");
	assertm(MAXSTUDIOTRIANGLES >= indiceCount, "mesh had too many triangles"); // number of tris (numIndices / 3)

	// for SMD and generating normals!
	TriangulateMesh();
	GenerateNormals();

	ShrinkData();
	g_BufferManager.RelieveBuffer(meshBuf);
}

IModelMesh::IModelMesh(const IModel* const imodel, IModelPerTriAABB* const perTriAABB) : flags(IMODELMESH_FLAG_NONE), material(IMODELTEXTURE_PHYSICS),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(0u), maxWeightsPerVert(1u), meshid(0), center(0.0f)
{
	UNUSED(imodel);

	flags |= (IMODELMESH_FLAG_TRI | IMODELMESH_FLAG_POS | IMODELMESH_FLAG_WEIGHTS);

	CManagedBuffer* const meshBuf = g_BufferManager.ClaimBuffer();
	data = new IModelMeshData(perTriAABB->GetVertCount(), perTriAABB->GetTriCount(), maxWeightsPerVert, texcoordCount, false, meshBuf->Buffer());

	const IModelVertexBoneWeight boneWeight(0, 1.0f);
	for (int i = 0; i < perTriAABB->GetVertCount(); i++)
	{
		AddVertex(&perTriAABB->GetVert(i), &boneWeight);
	}

	for (int i = 0; i < perTriAABB->GetLeafCount(); i++)
	{
		const IModelPerTriAABBLeaf* const pLeaf = perTriAABB->GetLeaf(i);

		for (int triIdx = 0; triIdx < pLeaf->GetTriCount(); triIdx++)
		{
			const IModelPerTriAABBTri* const pTri = pLeaf->GetTriangle(triIdx);

			const IModelFace face(3, pTri->GetIndice(0), pTri->GetIndice(1), pTri->GetIndice(2));
			
			AddIndice(face);
		}
	}

	GenerateNormals();

	ShrinkData();
	g_BufferManager.RelieveBuffer(meshBuf);
}

IModelMesh::IModelMesh(const IModel* const imodel, const r2::UIPanelMesh_s* const pUIMesh) : flags(IMODELMESH_FLAG_NONE), material(IMODELTEXTURE_UIPANEL),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(0u), maxWeightsPerVert(1u), meshid(0), center(0.0f)
{
	flags |= (IMODELMESH_FLAG_POS | IMODELMESH_FLAG_WEIGHTS);

	CManagedBuffer* const meshBuf = g_BufferManager.ClaimBuffer();
	data = new IModelMeshData(r2::maxUIPanelVerts, r2::maxUIPanelFaces, maxWeightsPerVert, texcoordCount, false, meshBuf->Buffer());

	assertm(r2::maxUIPanelFaces >= 32, "exceeded the face limit");

#if 0
	// based on simd psuedo code from various funcions in engine.dll (r2) and r5apex.exe
	// there is basically zero performance difference between this, and the latter one. just what I originally started with and cleaned up.
	__m128 parsedVertices[r2::maxUIPanelVerts]{};

	for (int i = 0; i < pUIMesh->vertexCount; i++)
	{
		const r2::UIPanelVertex_s* const pUIVertex = pUIMesh->pVertex(i);
		const int bone = pUIMesh->Parent(pUIVertex->parent);

		// in code I think this is VectorTransform, but for our cases we need to use VectorITransform
		// TODO: does this have to change for static props?
		VectorITransform(pUIVertex->pos.Base(), *imodel->GetBone(static_cast<uint32_t>(bone))->GetPoseToBone(), parsedVertices[i].m128_f32);

		const IModelVertexBoneWeight weight(bone, 1.0f);
		AddVertex(reinterpret_cast<const Vector* const>(parsedVertices + i), &weight);
	}

	for (int i = 0; i < pUIMesh->faceCount; i++)
	{
		const r2::UIPanelIndices_s* const pUIIndices = pUIMesh->pIndices(i);
		const r2::UIPanelBounds_s* const pUIBounds = pUIMesh->pBounds(i);

		const __m128& vertex0 = parsedVertices[pUIIndices->vertexIndex[0]];
		const __m128& vertex1 = parsedVertices[pUIIndices->vertexIndex[1]];
		const __m128& vertex2 = parsedVertices[pUIIndices->vertexIndex[2]];

		const __m128 vertexDelta_1_0 = SubSIMD(vertex1, vertex0);
		const __m128 vertexDelta_2_1 = SubSIMD(vertex2, vertex1);

		// x
		const __m128 xDelta_2_1 = ReplicateX4(pUIBounds->x[2] - pUIBounds->x[1]);
		const __m128 xDelta_1_0 = ReplicateX4(pUIBounds->x[1] - pUIBounds->x[0]);

		const __m128 x0 = ReplicateX4(pUIBounds->x[0]);

		// y
		const __m128 yDelta_2_1 = ReplicateX4(pUIBounds->y[2] - pUIBounds->y[1]);
		const __m128 yDelta_1_0 = ReplicateX4(pUIBounds->y[1] - pUIBounds->y[0]);

		__m128 y0 = ReplicateX4(pUIBounds->y[0]);

		// what the heck
		const __m128 value0 = SubSIMD(MulSIMD(yDelta_2_1, xDelta_1_0), MulSIMD(yDelta_1_0, xDelta_2_1));
		const __m128 value1 = _mm_rcp_ps(value0);
		const __m128 value2 = SubSIMD(simd_Four_Ones, MulSIMD(value1, value0));

		const __m128 value3 = AddSIMD(MulSIMD(value2, value2), value2);
		const __m128 value4 = AddSIMD(MulSIMD(value3, value1), value1);

		// calc our quirky ahh vertices (quirkyces)
		const __m128 vertexResult2 = MulSIMD(SubSIMD(MulSIMD(vertexDelta_2_1, xDelta_1_0), MulSIMD(vertexDelta_1_0, xDelta_2_1)), value4);
		const __m128 vertexResult1 = MulSIMD(SubSIMD(MulSIMD(vertexDelta_1_0, yDelta_2_1), MulSIMD(vertexDelta_2_1, yDelta_1_0)), value4);
		const __m128 vertexResult0 = SubSIMD(SubSIMD(vertex0, MulSIMD(vertexResult1, x0)), MulSIMD(vertexResult2, y0));

		// it's a quad!
		IModelFaceIndice indices[4]{};

		for (int idx = 0; idx < 4; idx++)
		{
			const __m128 x = ReplicateX4(pUIBounds->x[idx]);
			const __m128 y = ReplicateX4(pUIBounds->y[idx]);

			const __m128 vertexFinal = AddSIMD(AddSIMD(MulSIMD(vertexResult1, x), vertexResult0), MulSIMD(vertexResult2, y));

			// deduplicate vertices and use our base vertices
			bool found = false;
			for (int vertIdx = 0; vertIdx < pUIMesh->vertexCount; vertIdx++)
			{
				const __m128& vertexCandidate = parsedVertices[vertIdx];

				__m128 vertexDelta = SubSIMD(vertexFinal, vertexCandidate);
				vertexDelta = AbsSIMD(vertexDelta);

				const float delta = vertexDelta.m128_f32[0] + vertexDelta.m128_f32[1] + vertexDelta.m128_f32[2];

				if (delta > 0.01f)
				{
					continue;
				}

				indices[idx] = vertIdx;
				found = true;
				break;
			}

			assertm(found, "missed one");
		}

		// check if it was actually a triangle, this seems bad however the duplicated vertex is seemingly always in the fourth slot
		const bool isTri = indices[2] == indices[3];
		IModelFace quad(isTri ? 3 : 4, indices);

		// winding seems to be CCW by default with how we parsed
		if (pUIMesh->IsCCW(i) == false)
		{
			quad.FlipOrder();
		}

		AddIndice(quad);

		// add mesh flags now that we know
		flags |= isTri ? IMODELMESH_FLAG_TRI : IMODELMESH_FLAG_QUAD;
	}
#else
	// rebuilt from pseudo code to be more human readable
	Vector parsedVertices[r2::maxUIPanelVerts]{};

	for (int i = 0; i < pUIMesh->vertexCount; i++)
	{
		const r2::UIPanelVertex_s* const pUIVertex = pUIMesh->pVertex(i);
		const int bone = pUIMesh->Parent(pUIVertex->parent);

		// in code I think this is VectorTransform, but for our cases we need to use VectorITransform
		// TODO: does this have to change for static props?
		VectorITransform(pUIVertex->pos.Base(), *imodel->GetBone(static_cast<uint32_t>(bone))->GetPoseToBone(), parsedVertices[i].Base());

		const IModelVertexBoneWeight weight(bone, 1.0f);
		AddVertex(parsedVertices + i, &weight);
	}

	// only cycle once becuse we don't care about the other matrix
	for (int i = 0; i < pUIMesh->faceCount; i++)
	{
		const r2::UIPanelIndices_s* const pUIIndices = pUIMesh->pIndices(i);
		const r2::UIPanelBounds_s* const pUIBounds = pUIMesh->pBounds(i);

		Vector vertexResult[3]{};

		ModelUIPanel_ParseLocatingVertices(pUIIndices, pUIBounds, parsedVertices, vertexResult);

		ModelUIPanel_ParseForQuad(vertexResult, parsedVertices, pUIMesh, pUIBounds, i);
	}
#endif // 0

	assertm(r2::maxUIPanelVerts >= vertexCount, "mesh had too many vertices");
	assertm(r2::maxUIPanelFaces >= indiceCount, "mesh had too many triangles"); // number of tris (numIndices / 3)

	// for SMD and generating normals!
	TriangulateMesh();
	GenerateNormals();

	ShrinkData();
	g_BufferManager.RelieveBuffer(meshBuf);
}

IModelMesh::IModelMesh(const IModel* const imodel, const r5::UIPanelMesh_s* const pUIMesh) : flags(IMODELMESH_FLAG_NONE), material(IMODELTEXTURE_UIPANEL),
	data(nullptr), vertexCount(0u), indiceCount(0u), triangleCount(0u), weightCount(0u), texcoordCount(0u), maxWeightsPerVert(1u), meshid(0), center(0.0f)
{
	flags |= (IMODELMESH_FLAG_TRI | IMODELMESH_FLAG_POS | IMODELMESH_FLAG_WEIGHTS);

	CManagedBuffer* const meshBuf = g_BufferManager.ClaimBuffer();
	data = new IModelMeshData(r5::maxUIPanelVerts, r5::maxUIPanelFaces, maxWeightsPerVert, texcoordCount, false, meshBuf->Buffer());

	CManagedBuffer* const vertexBuf = g_BufferManager.ClaimBuffer();
	Vector* const parsedVertices = reinterpret_cast<Vector* const>(vertexBuf->Buffer());

	for (int i = 0; i < pUIMesh->vertexCount; i++)
	{
		const r5::UIPanelVertex_s* const pUIVertex = pUIMesh->pVertex(i);
		const int bone = pUIMesh->Parent(pUIVertex->parent);

		// in code I think this is VectorTransform, but for our cases we need to use VectorITransform
		// TODO: does this have to change for static props?
		VectorITransform(pUIVertex->pos.Base(), *imodel->GetBone(static_cast<uint32_t>(bone))->GetPoseToBone(), parsedVertices[i].Base());

		const IModelVertexBoneWeight weight(bone, 1.0f);
		AddVertex(&parsedVertices[i], &weight);
	}

	// only cycle once becuse we don't care about the other matrix
	for (int i = 0; i < pUIMesh->faceCount; i++)
	{
		const r5::UIPanelIndices_s* const pUIIndices = pUIMesh->pIndices(i);
		const r5::UIPanelBounds_s* const pUIBounds = pUIMesh->pBounds(i);

		Vector vertexResult[3]{};

		ModelUIPanel_ParseLocatingVertices(pUIIndices, pUIBounds, parsedVertices, vertexResult);

		ModelUIPanel_ParseForQuad(vertexResult, parsedVertices, pUIMesh, pUIBounds, i);
	}

	assertm(r5::maxUIPanelVerts >= vertexCount, "mesh had too many vertices");
	assertm(r5::maxUIPanelFaces >= indiceCount, "mesh had too many triangles"); // number of tris (numIndices / 3)

	// for SMD and generating normals!
	TriangulateMesh();
	GenerateNormals();

	ShrinkData();
	g_BufferManager.RelieveBuffer(meshBuf);
	g_BufferManager.RelieveBuffer(vertexBuf);
}

IModelMesh::~IModelMesh()
{
	// if we have ngons there is potentially face data allocated
	// I don't like this, do better ?
	if ((flags & IMODELMESH_FLAG_NGON) && data)
	{
		for (uint16_t i = 0; i < indiceCount; i++)
		{
			FreeAllocArray(data->indices[i].indiceExtra);
		}
	}

	FreeAllocVar(data);
}

void IModelMesh::FromValveVertex( const OptimizedModel::MeshHeader_t* const pVertexMesh, const uint32_t lod, const int baseVertexOffset, const int studioVertCount, const bool isApex,
	const vvd::vertexFileHeader_t* const pVVD, const vvc::vertexColorFileHeader_t* const pVVC, const vvw::vertexBoneWeightsExtraFileHeader_t* const pVVW)
{
	if (!flags)
		return;

	// build fixed up (per lod) buffers of vertex data
	CManagedBuffer* const vertexBuf = g_BufferManager.ClaimBuffer();

	vvd::mstudiovertex_t* pSrcVerts = reinterpret_cast<vvd::mstudiovertex_t*>(vertexBuf->Buffer());
	Vector4D* pSrcTangs = reinterpret_cast<Vector4D*>(pSrcVerts + studioVertCount);
	Color32* pSrcColors = nullptr;
	Vector2D* pSrcTexcoords = nullptr;
	const vvw::mstudioboneweightextra_t* const pSrcWeights = pVVW ? pVVW->GetWeightData(0) : nullptr;

	pVVD->PerLODVertexBuffer(lod, pSrcVerts, pSrcTangs, baseVertexOffset, baseVertexOffset + studioVertCount);

	if (pVVC)
	{
		pSrcColors = reinterpret_cast<Color32*>(pSrcTangs + studioVertCount);
		pSrcTexcoords = reinterpret_cast<Vector2D*>(pSrcColors + studioVertCount);

		pVVC->PerLODVertexBuffer(lod, pVVD->numFixups, pVVD->GetFixupData(0), pSrcColors, pSrcTexcoords, baseVertexOffset, baseVertexOffset + studioVertCount);
	}

	CManagedBuffer* const meshBuf = g_BufferManager.ClaimBuffer();
	data = new IModelMeshData(MAXSTUDIOVERTS, MAXSTUDIOTRIANGLES, isApex ? 16u : 3u, texcoordCount, flags & IMODELMESH_FLAG_COLOR, meshBuf->Buffer()); // temporary, gets allocated to new memory

	// more than one strip group will not work, never seen this in reSource thankfully. it is Very Quirky
	// todo: implement multiple strip groups (for normal source)
	assertm(pVertexMesh->numStripGroups == 1, "more than one strip group");
	for (int stripGrpIdx = 0; stripGrpIdx < pVertexMesh->numStripGroups; stripGrpIdx++)
	{
		const OptimizedModel::StripGroupHeader_t* const pStripGrp = pVertexMesh->pStripGroup(stripGrpIdx);
		const bool isHwSkinned = pStripGrp->IsHWSkinned();

		for (int stripIdx = 0; stripIdx < pStripGrp->numStrips; stripIdx++)
		{
			const OptimizedModel::StripHeader_t* const pStrip = pStripGrp->pStrip(stripIdx);
			const OptimizedModel::BoneStateChangeHeader_t* const pBoneStates = pStrip->pBoneStateChange(0);

			maxWeightsPerVert = pStrip->numBones > maxWeightsPerVert ? pStrip->numBones : maxWeightsPerVert;

			if (isApex)
			{
				for (int vertIdx = 0; vertIdx < pStrip->numVerts; vertIdx++)
				{
					const OptimizedModel::Vertex_t* const pVert = pStripGrp->pVertex(pStrip->vertOffset + vertIdx);
					AddVertex(pVert, pSrcVerts, pSrcTangs, pSrcColors, pSrcTexcoords, pSrcWeights);
				}
			}
			else
			{
				for (int vertIdx = 0; vertIdx < pStrip->numVerts; vertIdx++)
				{
					const OptimizedModel::Vertex_t* const pVert = pStripGrp->pVertex(pStrip->vertOffset + vertIdx);
					AddVertex(pVert, pSrcVerts, pSrcTangs, pSrcColors, pSrcTexcoords, pBoneStates, isHwSkinned);
				}
			}

			// tris or quads
			const bool isQuads = (pStrip->flags & OptimizedModel::StripHeaderFlags_t::STRIP_IS_TRILIST) ? false : true;
			int indiceSize = 0;

			if (isQuads)
			{
				indiceSize = 4;
				flags |= eIModelMeshFlags::IMODELMESH_FLAG_QUAD;
			}
			else
			{
				indiceSize = 3;
				flags |= eIModelMeshFlags::IMODELMESH_FLAG_TRI;
			}

			for (int indiceIdx = 0; indiceIdx < pStrip->numIndices; indiceIdx += indiceSize)
			{
				assertm(!isQuads, "model had quads");

				const IModelFaceIndice a = static_cast<IModelFaceIndice>(pStripGrp->Index(pStrip->indexOffset + indiceIdx));
				const IModelFaceIndice b = static_cast<IModelFaceIndice>(pStripGrp->Index(pStrip->indexOffset + indiceIdx + 1));
				const IModelFaceIndice c = static_cast<IModelFaceIndice>(pStripGrp->Index(pStrip->indexOffset + indiceIdx + 2));
				const IModelFaceIndice d = isQuads ? static_cast<IModelFaceIndice>(pStripGrp->Index(pStrip->indexOffset + indiceIdx + 3)) : 0u;

				IModelFace face(static_cast<IModelFaceIndice>(indiceSize), a, b, c, d);

				AddIndice(face);
			}
		}
	}

	assertm(MAXSTUDIOVERTS >= vertexCount, "mesh had too many vertices");
	assertm(MAXSTUDIOTRIANGLES >= indiceCount, "mesh had too many triangles"); // number of tris (numIndices / 3)

	ShrinkData();

	// add a triangular mesh for SMD if we have quads (never seen but covering our bases)
	if (flags & eIModelMeshFlags::IMODELMESH_FLAG_QUAD)
	{
		TriangulateMesh();
	}

	g_BufferManager.RelieveBuffer(vertexBuf);
	g_BufferManager.RelieveBuffer(meshBuf);
}

const int IModelMesh::FromIVPSNode(const ivps::compactledgenode_t* const pNode, uint32_t& maxVertexIndex)
{
	if (pNode == nullptr)
	{
		assertm(pNode, "node was nullptr");
		return 0;
	}

	int numLeaves = 0;

	if (pNode->HasChildren())
	{
		assertm(pNode->offset_right_node, "had children with no valid offset?");

		const ivps::compactledgenode_t* const pLeftNode = pNode->pLeftNode();
		const ivps::compactledgenode_t* const pRightNode = pNode->pRightNode();

		numLeaves += FromIVPSNode(pLeftNode, maxVertexIndex);
		numLeaves += FromIVPSNode(pRightNode, maxVertexIndex);
	}

	const ivps::compactledge_t* const pLEdge = pNode->pLEdge();

	// skip this node if it has no 'ledge', should just have children
	if (pLEdge == nullptr)
	{
		return numLeaves;
	}

	numLeaves++;

	// convert our ivps bone id to a proper index
	const IModelFaceIndice localIndiceIndex = vertexCount;

	// cycle through all tris for this node
	for (short triIdx = 0; triIdx < pLEdge->n_triangles; triIdx++)
	{
		const ivps::compacttriangle_t* const pTri = pLEdge->pTri(triIdx);

		constexpr IModelFaceIndice numIndices = 3u;

		const uint16_t point0 = pTri->c_three_edges[0].start_point_index;
		const uint16_t point1 = pTri->c_three_edges[2].start_point_index;
		const uint16_t point2 = pTri->c_three_edges[1].start_point_index;

		const IModelFaceIndice a = localIndiceIndex + point0;
		const IModelFaceIndice b = localIndiceIndex + point1;
		const IModelFaceIndice c = localIndiceIndex + point2;

		const IModelFace face(numIndices, a, b, c);

		AddIndice(face);

		maxVertexIndex = point0 > maxVertexIndex ? point0 : maxVertexIndex;
		maxVertexIndex = point1 > maxVertexIndex ? point1 : maxVertexIndex;
		maxVertexIndex = point2 > maxVertexIndex ? point2 : maxVertexIndex;
	}

	return numLeaves;
}

// model ui panel
const bool IModelMesh::ModelUIPanel_MatchVertex(const Vector& targetVertex, const Vector* const vertices, const int numVerts, IModelFaceIndice& indice, const float tolerance)
{
	for (int vertIdx = 0; vertIdx < numVerts; vertIdx++)
	{
		const Vector& vertexCandidate = vertices[vertIdx];

		if (Vector::CompareTolerance(targetVertex, vertexCandidate, tolerance))
		{
			indice = vertIdx;
			return true;
		}
	}

	return false;
}

template<typename Indices, typename Bounds>
void IModelMesh::ModelUIPanel_ParseLocatingVertices(const Indices* const pUIIndices, const Bounds* const pUIBounds, const Vector* const parsedVertices, Vector* const vertexResult)
{
	const Vector& vertex0 = parsedVertices[pUIIndices->vertexIndex[0]];
	const Vector& vertex1 = parsedVertices[pUIIndices->vertexIndex[1]];
	const Vector& vertex2 = parsedVertices[pUIIndices->vertexIndex[2]];

	const Vector vertexDelta_1_0 = vertex1 - vertex0;
	const Vector vertexDelta_2_1 = vertex2 - vertex1;

	// x
	const float xDelta_2_1 = pUIBounds->x[2] - pUIBounds->x[1];
	const float xDelta_1_0 = pUIBounds->x[1] - pUIBounds->x[0];

	// y
	const float yDelta_2_1 = pUIBounds->y[2] - pUIBounds->y[1];
	const float yDelta_1_0 = pUIBounds->y[1] - pUIBounds->y[0];

	// what the heck
	const float value0 = (yDelta_2_1 * xDelta_1_0) - (yDelta_1_0 * xDelta_2_1);
	const float value1 = 1.0f / value0;
	const float value2 = 1.0f - (value1 * value0);

	const float value3 = (value2 * value2) + value2;
	const float value4 = (value3 * value1) + value1;

	// calc our quirky ahh vertices (quirkyces)
	vertexResult[2] = ((vertexDelta_2_1 * xDelta_1_0) - (vertexDelta_1_0 * xDelta_2_1)) * value4;
	vertexResult[1] = ((vertexDelta_1_0 * yDelta_2_1) - (vertexDelta_2_1 * yDelta_1_0)) * value4;
	vertexResult[0] = (vertex0 - (vertexResult[1] * pUIBounds->x[0])) - (vertexResult[2] * pUIBounds->y[0]);
}

template<typename Mesh, typename Bounds>
void IModelMesh::ModelUIPanel_ParseForQuad(const Vector* const vertexResult, const Vector* const parsedVertices, const Mesh* const pUIMesh, const Bounds* const pUIBounds, const int face)
{
	constexpr float defaultUIVertTolerance = 0.01f;

	// it's a quad!
	IModelFaceIndice indices[4]{};
	bool isTri = false;

	for (int idx = 0; idx < 4; idx++)
	{
		const Vector vertexFinal = ((vertexResult[1] * pUIBounds->x[idx]) + vertexResult[0]) + (vertexResult[2] * pUIBounds->y[idx]);

		// deduplicate vertices and use our base vertices
		if (ModelUIPanel_MatchVertex(vertexFinal, parsedVertices, pUIMesh->vertexCount, indices[idx], defaultUIVertTolerance))
		{
			continue;
		}

		// never not the fourth vertex
		assertm(idx == 3, "unmatched vertex was not in the fourth slot");
		float tolerance = defaultUIVertTolerance;

		for (uint8_t level = 0; level < g_CmdSettings.mergeUIVerts; level++)
		{
			tolerance += 0.01f;

			if (ModelUIPanel_MatchVertex(vertexFinal, parsedVertices, pUIMesh->vertexCount, indices[idx], tolerance))
			{
				const Vector& selectedVertex = parsedVertices[indices[idx]];

				Log("ui panel '%s' had a vertex (face %i idx %i) that could not be associated with an existing one, merging (%.4f %.4f %.4f) to (%.4f %.4f %.4f) with %.2f tolerance...\n", pUIMesh->pszPanelName(), face, idx,
					vertexFinal[0], vertexFinal[1], vertexFinal[2], selectedVertex[0], selectedVertex[1], selectedVertex[2], tolerance);

				continue;
			}
		}

		// could not be merged with higher tolerances, complete outlier
		Log("ui panel '%s' had a vertex (face %i idx %i) that could not be associated with an existing one, adding it (%.4f %.4f %.4f) instead...\n", pUIMesh->pszPanelName(), face, idx, vertexFinal[0], vertexFinal[1], vertexFinal[2]);

		indices[idx] = vertexCount;

		const IModelVertexBoneWeight* const pSiblingWeight = GetWeight(GetVertex(indices[2]), 0);

		AddVertex(&vertexFinal, pSiblingWeight);
	}

	// check if it was actually a triangle, this seems bad however the duplicated vertex is seemingly always in the fourth slot
	if (indices[2] == indices[3])
	{
		isTri = true;
	}

	IModelFace quad(isTri ? 3 : 4, indices);

	// winding seems to be CCW by default with how we parsed
	if (pUIMesh->IsCCW(face) == false)
	{
		quad.FlipOrder();
	}

	AddIndice(quad);

	// add mesh flags now that we know
	flags |= isTri ? IMODELMESH_FLAG_TRI : IMODELMESH_FLAG_QUAD;
}

void IModelMesh::GenerateNormals()
{
	assertm(data, "did not have data");
	assertm(data->indices, "did not have faces");
	assertm(data->vertices, "did not have vertices");

	uint32_t localIndiceCount = indiceCount;
	const IModelFace* localIndices = data->indices;

	if ((IMODELMESH_FLAG_QUAD | IMODELMESH_FLAG_NGON) & flags)
	{
		if (triangleCount == 0 || data->triangles == nullptr)
		{
			TriangulateMesh();
		}

		localIndiceCount = triangleCount;
		localIndices = data->triangles;
	}

	// https://iquilezles.org/articles/normals/
	// normals should be set to 0 by default!

	for (uint32_t i = 0u; i < localIndiceCount; i++)
	{
		const IModelFace* const tri = localIndices + i;

		const IModelFaceIndice indexA = tri->GetIndice(0);
		const IModelFaceIndice indexB = tri->GetIndice(1);
		const IModelFaceIndice indexC = tri->GetIndice(2);

		Vector normal = GenerateNormal(GetVertex(indexA)->GetPosition(), GetVertex(indexB)->GetPosition(), GetVertex(indexC)->GetPosition());

		data->vertices[indexA].AccumulateNormal(normal);
		data->vertices[indexB].AccumulateNormal(normal);
		data->vertices[indexC].AccumulateNormal(normal);
	}

	// normalize normals
	for (uint32_t i = 0u; i < GetVertexCount(); i++)
	{
		Vector normalized = GetVertex(i)->GetNormal();
		normalized.NormalizeInPlace();

		data->vertices[i].SetNormal(normalized);
	}

	flags |= IMODELMESH_FLAG_NORM;
}

// adds an indice layout of triangles
void IModelMesh::TriangulateMesh()
{
	// the mesh is only triangles, do not proceed
	if ((flags & (eIModelMeshFlags::IMODELMESH_FLAG_QUAD | eIModelMeshFlags::IMODELMESH_FLAG_NGON)) == 0)
	{
		return;
	}

	// don't do it twice!
	if (triangleCount && data->triangles)
	{
		return;
	}

	CManagedBuffer* const buf = g_BufferManager.ClaimBuffer();

	IModelFace* triangles = reinterpret_cast<IModelFace*>(buf->Buffer());
	IModelFaceIndice numTriangles = 0u;

	for (uint16_t i = 0u; i < indiceCount; i++)
	{
		const IModelFace* const face = data->indices + i;

		if (face->IsTri())
		{
			triangles[numTriangles] = IModelFace(*face);
			numTriangles++;

			continue;
		}

		const IModelFaceIndice numPotentialTriangles = face->GetIndiceCount() - 2u;
		bool flipFlop = true;

		for (IModelFaceIndice triIdx = 0u, front = 0u, back = face->GetIndiceCount() - 1; triIdx < numPotentialTriangles; triIdx++)
		{
			if (flipFlop)
			{
				const IModelFaceIndice a = face->GetIndice(front);
				const IModelFaceIndice b = face->GetIndice(front + 1);
				const IModelFaceIndice c = face->GetIndice(back);

				triangles[numTriangles] = IModelFace(3u, a, b, c);
				numTriangles++;

				flipFlop = false;

				front++;
			}
			else
			{
				const IModelFaceIndice a = face->GetIndice(front);
				const IModelFaceIndice b = face->GetIndice(back - 1);
				const IModelFaceIndice c = face->GetIndice(back);

				triangles[numTriangles] = IModelFace(3u, a, b, c);
				numTriangles++;

				flipFlop = true;

				back--;
			}

			assertm(front < back, "don't cross the beams");
		}
	}

	data->triangles = triangles;
	triangleCount = static_cast<uint16_t>(numTriangles);

	// cheat to realloc data with this new data!
	ShrinkData();

	g_BufferManager.RelieveBuffer(buf);
}

IModelModel::IModelModel(const IModelLOD* const lod, const IModelSourceFlags_t sourceFlags, const r1::mstudiomodel_t* const pStudioModel, const OptimizedModel::ModelLODHeader_t* const pVertexLOD,
	const StudioLooseData_t* const pLooseData, const char* const modelName, const char* const bodypartName, const bool hasLODs) : name(nullptr), unknown(nullptr),
	type(pStudioModel->type), boundingradius(pStudioModel->boundingradius), meshes(nullptr), meshCount(static_cast<uint16_t>(pStudioModel->nummeshes)), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{
	// model is 'blank', a model can have verts but no name, and a name and no verts
	if (!pStudioModel->name[0] && !pStudioModel->nummeshes && !pStudioModel->numvertices)
		return;

	// parse the name out
	name = IModel_ParseModelName(pStudioModel->name, modelName, bodypartName, true, lod, lod->GetModelCount(), hasLODs);

	meshes = new IModelMesh[meshCount]{};

	for (int i = 0; i < pStudioModel->nummeshes; i++)
	{
		const r1::mstudiomesh_t* const pStudioMesh = pStudioModel->pMesh(i);
		const OptimizedModel::MeshHeader_t* const pVertexMesh = pVertexLOD->pMesh(i);

		assertm(!pVertexMesh->flags, "mesh with flags");

		meshes[i] = IModelMesh(lod->GetLODLevel(), sourceFlags, pStudioMesh, pVertexMesh, pLooseData);

		const IModelMesh* const mesh = meshes + i;

		vertexCount += mesh->GetVertexCount();
		indiceCount += mesh->GetIndiceCount();
		weightCount += mesh->GetWeightCount();

		maxVertWeights = mesh->GetMaxVertBoneCount() > maxVertWeights ? mesh->GetMaxVertBoneCount() : maxVertWeights;
		maxVertTexcoords = mesh->GetTexcoordCount() > maxVertTexcoords ? mesh->GetTexcoordCount() : maxVertTexcoords;
	}
}

IModelModel::IModelModel(const IModelLOD* const lod, const IModelSourceFlags_t sourceFlags, const r2::mstudiomodel_t* const pStudioModel, const OptimizedModel::ModelLODHeader_t* const pVertexLOD,
	const StudioLooseData_t* const pLooseData, const char* const modelName, const char* const bodypartName, const bool hasLODs) : name(nullptr), unknown(nullptr),
	type(pStudioModel->type), boundingradius(pStudioModel->boundingradius), meshes(nullptr), meshCount(static_cast<uint16_t>(pStudioModel->nummeshes)), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{
	// model is 'blank', a model can have verts but no name, and a name and no verts
	if (!pStudioModel->name[0] && !pStudioModel->nummeshes && !pStudioModel->numvertices)
		return;

	// parse the name out
	name = IModel_ParseModelName(pStudioModel->name, modelName, bodypartName, true, lod, lod->GetModelCount(), hasLODs);

	meshes = new IModelMesh[meshCount]{};

	for (int i = 0; i < pStudioModel->nummeshes; i++)
	{
		const r2::mstudiomesh_t* const pStudioMesh = pStudioModel->pMesh(i);
		const OptimizedModel::MeshHeader_t* const pVertexMesh = pVertexLOD->pMesh(i);

		assertm(!pVertexMesh->flags, "mesh with flags");

		meshes[i] = IModelMesh(lod->GetLODLevel(), sourceFlags, pStudioMesh, pVertexMesh, pLooseData);

		const IModelMesh* const mesh = meshes + i;

		vertexCount += mesh->GetVertexCount();
		indiceCount += mesh->GetIndiceCount();
		weightCount += mesh->GetWeightCount();

		maxVertWeights = mesh->GetMaxVertBoneCount() > maxVertWeights ? mesh->GetMaxVertBoneCount() : maxVertWeights;
		maxVertTexcoords = mesh->GetTexcoordCount() > maxVertTexcoords ? mesh->GetTexcoordCount() : maxVertTexcoords;
	}
}

IModelModel::IModelModel(const IModelLOD* const lod, const IModelSourceFlags_t sourceFlags, const r5::mstudiomodel_t* const pStudioModel, const OptimizedModel::ModelLODHeader_t* const pVertexLOD,
	const StudioLooseData_t* const pLooseData, const char* const modelName, const char* const bodypartName, const bool hasLODs) : name(nullptr), unknown(pStudioModel->pszUnknown()),
	type(pStudioModel->type), boundingradius(pStudioModel->boundingradius), meshes(nullptr), meshCount(static_cast<uint16_t>(pStudioModel->nummeshes)), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{
	// model is 'blank', a model can have verts but no name, and a name and no verts
	if (!pStudioModel->name[0] && !pStudioModel->nummeshes && !pStudioModel->numvertices)
		return;

	// parse the name out
	name = IModel_ParseModelName(pStudioModel->name, modelName, bodypartName, false, lod, lod->GetModelCount(), hasLODs);

	meshes = new IModelMesh[meshCount]{};

	for (int i = 0; i < pStudioModel->nummeshes; i++)
	{
		const r5::mstudiomesh_t* const pStudioMesh = pStudioModel->pMesh(i);
		const OptimizedModel::MeshHeader_t* const pVertexMesh = pVertexLOD->pMesh(i);

		assertm(!pVertexMesh->flags, "mesh with flags");

		meshes[i] = IModelMesh(lod->GetLODLevel(), sourceFlags, pStudioMesh, pVertexMesh, pLooseData);

		const IModelMesh* const mesh = meshes + i;

		vertexCount += mesh->GetVertexCount();
		indiceCount += mesh->GetIndiceCount();
		weightCount += mesh->GetWeightCount();

		maxVertWeights = mesh->GetMaxVertBoneCount() > maxVertWeights ? mesh->GetMaxVertBoneCount() : maxVertWeights;
		maxVertTexcoords = mesh->GetTexcoordCount() > maxVertTexcoords ? mesh->GetTexcoordCount() : maxVertTexcoords;
	}
}

IModelModel::IModelModel(const IModel* const imodel, IModelPhysics* const physics, const ivps::phyheader_t* const pPHYS) : name(nullptr), unknown(nullptr),
	type(-1), boundingradius(0.0f), meshes(nullptr), meshCount(1u), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{	
	// static prop saves name (note: this is probably just '$collisionmodel' in general)
	const char* const firstSolidName = physics->GetParsedPhys()->GetSolid(0)->GetName();
	if (imodel->GetBoneIndex(firstSolidName) >= imodel->GetBoneCount())
	{
		assertm(physics->GetParsedPhys()->GetSolidCount() == 1, "jointed model with invalid bones");

		name = AllocStudioString(firstSolidName);

		// model used '$collisionmodel'
		physics->SetJointed(false);
	}
	// form a name
	else
	{
		constexpr size_t bufferSize = 64ull;
		char tmp[bufferSize]{};

		strncpy_s(tmp, bufferSize, keepAfterLastSlashOrBackslash(imodel->GetName()), bufferSize);
		removeExtension(tmp);

		snprintf(tmp, bufferSize, "%s_phys", tmp);
		name = AllocStudioString(tmp);

		// model used '$collisionjoints'
		physics->SetJointed(true);
	}

	meshes = new IModelMesh[meshCount]{};
	meshes[0u] = IModelMesh(imodel, physics, pPHYS);

	// set data from mesh
	const IModelMesh* const mesh = meshes;

	vertexCount = mesh->GetVertexCount();
	indiceCount = mesh->GetIndiceCount();
	weightCount = mesh->GetWeightCount();

	maxVertWeights = mesh->GetMaxVertBoneCount();
	maxVertTexcoords = mesh->GetTexcoordCount();
}

IModelModel::IModelModel(const IModel* const imodel, IModelMapCollision* const collision) : name(nullptr), unknown(nullptr),
	type(-1), boundingradius(0.0f), meshes(nullptr), meshCount(1u), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{	
	// initialize this with basic values so it doesn't explode when we try to deallocate IModelMapCollision
	if (collision->GetShapeCount() == 0)
	{
		return;
	}

	// form a name
	{
		constexpr size_t bufferSize = 64ull;
		char tmp[bufferSize]{};

		strncpy_s(tmp, bufferSize, keepAfterLastSlashOrBackslash(imodel->GetName()), bufferSize);
		removeExtension(tmp);

		snprintf(tmp, bufferSize, "%s_coll", tmp);
		name = AllocStudioString(tmp);
	}

	meshes = new IModelMesh[meshCount]{};
	meshes[0u] = IModelMesh(imodel, collision);

	// set data from mesh
	const IModelMesh* const mesh = meshes;

	vertexCount = mesh->GetVertexCount();
	indiceCount = mesh->GetIndiceCount();
	weightCount = mesh->GetWeightCount();

	maxVertWeights = mesh->GetMaxVertBoneCount();
	maxVertTexcoords = mesh->GetTexcoordCount();
}


IModelModel::IModelModel(const IModel* const imodel, IModelPerTriAABB* const perTriAABB) : name(nullptr), unknown(nullptr),
	type(-1), boundingradius(0.0f), meshes(nullptr), meshCount(1u), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{
	if (perTriAABB->GetVersion() == 0)
	{
		return;
	}

	// form a name
	{
		constexpr size_t bufferSize = 64ull;
		char tmp[bufferSize]{};

		strncpy_s(tmp, bufferSize, keepAfterLastSlashOrBackslash(imodel->GetName()), bufferSize);
		removeExtension(tmp);

		snprintf(tmp, bufferSize, "%s_pertrimesh", tmp);
		name = AllocStudioString(tmp);
	}

	meshes = new IModelMesh[meshCount]{};
	meshes[0u] = IModelMesh(imodel, perTriAABB);

	// set data from mesh
	const IModelMesh* const mesh = meshes;

	vertexCount = mesh->GetVertexCount();
	indiceCount = mesh->GetIndiceCount();
	weightCount = mesh->GetWeightCount();

	maxVertWeights = mesh->GetMaxVertBoneCount();
	maxVertTexcoords = mesh->GetTexcoordCount();
}

IModelModel::IModelModel(const IModel* const imodel, const r2::UIPanelHeader_s* const pUIPanel) : name(nullptr), unknown(nullptr),
	type(-1), boundingradius(0.0f), meshes(nullptr), meshCount(1u), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{	
	const r2::UIPanelMesh_s* const pUIMesh = pUIPanel->pPanelMesh();

	name = AllocStudioString(keepAfterLastSlashOrBackslash(pUIMesh->pszPanelName()));

	meshes = new IModelMesh[meshCount]{};
	meshes[0u] = IModelMesh(imodel, pUIMesh);

	// set data from mesh
	const IModelMesh* const mesh = meshes;

	vertexCount = mesh->GetVertexCount();
	indiceCount = mesh->GetIndiceCount();
	weightCount = mesh->GetWeightCount();

	maxVertWeights = mesh->GetMaxVertBoneCount();
	maxVertTexcoords = mesh->GetTexcoordCount();
}

IModelModel::IModelModel(const IModel* const imodel, const r5::UIPanelHeader_s* const pUIPanel) : name(nullptr), unknown(nullptr),
	type(-1), boundingradius(0.0f), meshes(nullptr), meshCount(1u), vertexCount(0u), indiceCount(0u), weightCount(0u), maxVertWeights(0u), maxVertTexcoords(0u)
{	
	const r5::UIPanelMesh_s* const pUIMesh = pUIPanel->pPanelMesh();

	name = AllocStudioString(keepAfterLastSlashOrBackslash(pUIMesh->pszPanelName()));

	meshes = new IModelMesh[meshCount]{};
	meshes[0u] = IModelMesh(imodel, pUIMesh);

	// set data from mesh
	const IModelMesh* const mesh = meshes;

	vertexCount = mesh->GetVertexCount();
	indiceCount = mesh->GetIndiceCount();
	weightCount = mesh->GetWeightCount();

	maxVertWeights = mesh->GetMaxVertBoneCount();
	maxVertTexcoords = mesh->GetTexcoordCount();
}

IModelModel::~IModelModel()
{
	FreeAllocArray(name);
	FreeAllocArray(meshes);
}

IModelLOD::IModelLOD(const uint32_t lodIndex, const uint16_t numModels, const IModelSourceFlags_t sourceFlags, const r1::studiohdr_t* const pHdr, const StudioLooseData_t* const pLooseData) :
	switchPoint(invalidLODSwitchPoint), lodLevel(lodIndex), models(nullptr), modelCount(0u), maxBonesPerVert(0), materialReplacementCount(0u), materialReplacements(nullptr)
{
	assertm(numModels, "lod with no models");
	models = new IModelModel[numModels]{};

	const OptimizedModel::FileHeader_t* const pVTX = pLooseData->GetVTX();

	for (int partIdx = 0; partIdx < pHdr->numbodyparts; partIdx++)
	{
		const r1::mstudiobodyparts_t* const pStudioBodyPart = pHdr->pBodypart(partIdx);
		const OptimizedModel::BodyPartHeader_t* const pVertexBodypart = pVTX->pBodyPart(partIdx);

		assertm(pStudioBodyPart->nummodels == pVertexBodypart->numModels, "mismatched model counts");

		for (int modelIdx = 0; modelIdx < pStudioBodyPart->nummodels; modelIdx++)
		{
			const r1::mstudiomodel_t* const pStudioModel = pStudioBodyPart->pModel(modelIdx);
			const OptimizedModel::ModelHeader_t* const pVertexModel = pVertexBodypart->pModel(modelIdx);

			const OptimizedModel::ModelLODHeader_t* const pVertexLOD = pVertexModel->pLOD(static_cast<int>(lodLevel));

			if (switchPoint == invalidLODSwitchPoint)
			{
				switchPoint = pVertexLOD->switchPoint;
			}

			const bool useLODs = pVertexModel->numLODs > 1; // model does not have lods if it only has one

			// sanity check that our switch point matches, don't think this can happen but the possibility exists with the file structure
			assertm(switchPoint == pVertexLOD->switchPoint, "LOD had different switch point");
			assertm(pStudioModel->nummeshes == pVertexLOD->numMeshes, "mismatched mesh counts");

			if (numModels == modelCount)
			{
				Error("Models overflowed when parsing lod %u\n", lodLevel);
				break;
			}

			models[modelCount] = IModelModel(this, sourceFlags, pStudioModel, pVertexLOD, pLooseData, pHdr->pszName(), pStudioBodyPart->pszName(), useLODs);

			modelCount++;
		}
	}

	// parse per lod material replacements
	ParseMaterialReplacements(pVTX);
}

IModelLOD::IModelLOD(const uint32_t lodIndex, const uint16_t numModels, const IModelSourceFlags_t sourceFlags, const r2::studiohdr_t* const pHdr, const StudioLooseData_t* const pLooseData) :
	switchPoint(invalidLODSwitchPoint), lodLevel(lodIndex), models(nullptr), modelCount(0u), maxBonesPerVert(0), materialReplacementCount(0u), materialReplacements(nullptr)
{
	assertm(numModels, "lod with no models");
	models = new IModelModel[numModels]{};

	const OptimizedModel::FileHeader_t* const pVTX = pLooseData->GetVTX();

	for (int partIdx = 0; partIdx < pHdr->numbodyparts; partIdx++)
	{
		const r2::mstudiobodyparts_t* const pStudioBodyPart = pHdr->pBodypart(partIdx);
		const OptimizedModel::BodyPartHeader_t* const pVertexBodypart = pVTX->pBodyPart(partIdx);

		assertm(pStudioBodyPart->nummodels == pVertexBodypart->numModels, "mismatched model counts");

		for (int modelIdx = 0; modelIdx < pStudioBodyPart->nummodels; modelIdx++)
		{
			const r2::mstudiomodel_t* const pStudioModel = pStudioBodyPart->pModel(modelIdx);
			const OptimizedModel::ModelHeader_t* const pVertexModel = pVertexBodypart->pModel(modelIdx);

			const OptimizedModel::ModelLODHeader_t* const pVertexLOD = pVertexModel->pLOD(static_cast<int>(lodLevel));

			if (switchPoint == invalidLODSwitchPoint)
			{
				switchPoint = pVertexLOD->switchPoint;
			}

			const bool useLODs = pVertexModel->numLODs > 1; // model does not have lods if it only has one

			// sanity check that our switch point matches, don't think this can happen but the possibility exists with the file structure
			assertm(switchPoint == pVertexLOD->switchPoint, "LOD had different switch point");
			assertm(pStudioModel->nummeshes == pVertexLOD->numMeshes, "mismatched mesh counts");

			if (numModels == modelCount)
			{
				Error("Models overflowed when parsing lod %u\n", lodLevel);
				break;
			}

			models[modelCount] = IModelModel(this, sourceFlags, pStudioModel, pVertexLOD, pLooseData, pHdr->pszName(), pStudioBodyPart->pszName(), useLODs);

			modelCount++;
		}
	}

	// parse per lod material replacements
	ParseMaterialReplacements(pVTX);
}

IModelLOD::IModelLOD(const uint32_t lodIndex, const uint16_t numModels, const IModelSourceFlags_t sourceFlags, const r5::studiohdr_t* const pHdr, const StudioLooseData_t* const pLooseData) :
	switchPoint(invalidLODSwitchPoint), lodLevel(lodIndex), models(nullptr), modelCount(0u), maxBonesPerVert(0), materialReplacementCount(0u), materialReplacements(nullptr)
{
	assertm(numModels, "lod with no models");
	models = new IModelModel[numModels]{};

	const OptimizedModel::FileHeader_t* const pVTX = pLooseData->GetVTX();

	for (int partIdx = 0; partIdx < pHdr->numbodyparts; partIdx++)
	{
		const r5::mstudiobodyparts_t* const pStudioBodyPart = pHdr->pBodypart(partIdx);
		const OptimizedModel::BodyPartHeader_t* const pVertexBodypart = pVTX->pBodyPart(partIdx);

		assertm(pStudioBodyPart->nummodels == pVertexBodypart->numModels, "mismatched model counts");

		for (int modelIdx = 0; modelIdx < pStudioBodyPart->nummodels; modelIdx++)
		{
			const r5::mstudiomodel_t* const pStudioModel = pStudioBodyPart->pModel(modelIdx);
			const OptimizedModel::ModelHeader_t* const pVertexModel = pVertexBodypart->pModel(modelIdx);

			const OptimizedModel::ModelLODHeader_t* const pVertexLOD = pVertexModel->pLOD(static_cast<int>(lodLevel));

			if (switchPoint == invalidLODSwitchPoint)
			{
				switchPoint = pVertexLOD->switchPoint;
			}

			const bool useLODs = pVertexModel->numLODs > 1; // model does not have lods if it only has one

			// sanity check that our switch point matches, don't think this can happen but the possibility exists with the file structure
			assertm(switchPoint == pVertexLOD->switchPoint, "LOD had different switch point");
			assertm(pStudioModel->nummeshes == pVertexLOD->numMeshes, "mismatched mesh counts");

			if (numModels == modelCount)
			{
				Error("Models overflowed when parsing lod %u\n", lodLevel);
				break;
			}

			models[modelCount] = IModelModel(this, sourceFlags, pStudioModel, pVertexLOD, pLooseData, pHdr->pszName(), pStudioBodyPart->pszName(), useLODs);

			modelCount++;
		}
	}

	// parse per lod material replacements
	ParseMaterialReplacements(pVTX);
}

IModelLOD::~IModelLOD()
{
	FreeAllocArray(models);
	FreeAllocArray(materialReplacements);
}

void IModelLOD::ParseMaterialReplacements(const OptimizedModel::FileHeader_t* const pVTX)
{
	if (pVTX->materialReplacementListOffset > 0)
	{
		const OptimizedModel::MaterialReplacementListHeader_t* const pMaterialReplacementList = pVTX->pMaterialReplacementList(lodLevel);

		if (pMaterialReplacementList->numReplacements > 0)
		{
			materialReplacementCount = pMaterialReplacementList->numReplacements;
			materialReplacements = new IModelMaterialReplacement[materialReplacementCount]{};

			for (int i = 0; i < pMaterialReplacementList->numReplacements; i++)
			{
				materialReplacements[i] = IModelMaterialReplacement(pMaterialReplacementList->pMaterialReplacement(i));
			}
		}
	}
}


//
// Model Mesh Other
//

IModelPhysics::IModelPhysics(const IModel* const imodel, const ivps::phyheader_t* const pPHYS) : parsedPhysics(pPHYS)
{
	// if we don't do this, the constructor will get called before physics is parsed
	// calling IModelModel() also does not work, so we use this hack ;p
	*static_cast<IModelModel*>(this) = IModelModel(imodel, this, pPHYS);
}

IModelMapCollisionSide::IModelMapCollisionSide(const r2::mstudiocollside_t* const pCollFace) : normal(pCollFace->normal), normalScale(pCollFace->normalScale), edgeCount(0u), vertCount(0u)
{
	memset(edgeIndices, 0xff, sizeof(edgeIndices));
	memset(vertIndices, 0xff, sizeof(vertIndices));
}



IModelMapCollisionShape::IModelMapCollisionShape(const r2::mstudiocollhdr_t* const pCollHdr, const uint32_t bone) : sides(nullptr), edges(nullptr), vertices(nullptr), sideCount(static_cast<int16_t>(pCollHdr->sideCount)), sideAndQuirkyCount(static_cast<int16_t>(pCollHdr->sideAndUnkCount)),
	edgeCount(static_cast<int16_t>(pCollHdr->edgeCount)), vertCount(static_cast<int16_t>(pCollHdr->vertCount)), parent(bone)
{
	// vertices are clean and easy
	const size_t vertSize = sizeof(Vector) * vertCount;
	vertices = new Vector[vertCount]{};
	memcpy_s(vertices, vertSize, pCollHdr->pVert(0), vertSize);

	sides = new IModelMapCollisionSide[sideAndQuirkyCount]{};

	for (int i = 0; i < sideAndQuirkyCount; i++)
	{
		sides[i] = IModelMapCollisionSide(pCollHdr->pFace(i));
	}

	edges = new IModelMapCollisionEdge[edgeCount]{};

	for (int i = 0; i < edgeCount; i++)
	{
		const r2::mstudiocolledge_t* const pCollEdge = pCollHdr->pEdge(i);

		edges[i] = IModelMapCollisionEdge(pCollEdge);

		// two sides per edge
		for (int idx = 0; idx < 2; idx++)
		{
			const uint8_t sideIdx = pCollEdge->sideIndices[idx];

			IModelMapCollisionSide* const pSide = sides + sideIdx;

			pSide->edgeIndices[pSide->edgeCount] = static_cast<int8_t>(i);
			pSide->edgeCount++;
		}
	}

	for (int i = 0; i < sideCount; i++)
	{
		IModelMapCollisionSide* const pSide = sides + i;

		const r2::mstudiocolledge_t* pEdge = pCollHdr->pEdge(pSide->edgeIndices[0]);

		pSide->vertIndices[0] = pEdge->vertIndices[0];
		pSide->vertIndices[1] = pEdge->vertIndices[1];

		// the vert could should always match the edge count within sides
		pSide->vertCount = 2;

		int currentEdge = 1;
		while (pSide->vertCount < pSide->edgeCount)
		{
			if (currentEdge >= pSide->edgeCount)
				currentEdge = 1;

			pEdge = pCollHdr->pEdge(pSide->edgeIndices[currentEdge]);

			if (pEdge->vertIndices[0] == pSide->vertIndices[pSide->vertCount - 1])
			{
				pSide->vertIndices[pSide->vertCount] = pEdge->vertIndices[1];
				pSide->vertCount++;

				currentEdge++;

				continue;
			}

			if (pEdge->vertIndices[1] == pSide->vertIndices[pSide->vertCount - 1])
			{
				pSide->vertIndices[pSide->vertCount] = pEdge->vertIndices[0];
				pSide->vertCount++;

				currentEdge++;

				continue;
			}

			currentEdge++;
		}

		// correct vertex winding order by calculating a normal and comparing against the original
		Vector normal = GenerateNormal(vertices[pSide->vertIndices[0]], vertices[pSide->vertIndices[1]], vertices[pSide->vertIndices[2]], true);

		// this is a sanity check to make sure our generated normals are not too far out
		if (Vector::CompareToleranceAbs(normal, pSide->normal, 0.1f) == false)
		{
			// move it to a different set of tris, on quads this will base it off the other triangle
			normal = GenerateNormal(vertices[pSide->vertIndices[1]], vertices[pSide->vertIndices[2]], vertices[pSide->vertIndices[3]], true);

			if (Vector::CompareToleranceAbs(normal, pSide->normal, 0.1f) == false)
			{
				assertm(false, "incorrectly generated normal");
			}
		}

		Vector deltaXYZ = normal - pSide->normal;
		deltaXYZ.ABS();
		const float deltaF = deltaXYZ.x + deltaXYZ.y + deltaXYZ.z;

		// normal is the same (or within error)
		if (deltaF < 0.01f)
		{
			continue;
		}

		uint8_t reordered[128]{};

		reordered[0] = pSide->vertIndices[0];

		for (uint8_t idxNewIdx = 1u, idxOldIdx = pSide->vertCount - 1u; idxNewIdx < pSide->vertCount; idxNewIdx++, idxOldIdx--)
		{
			reordered[idxNewIdx] = pSide->vertIndices[idxOldIdx];
		}

		memcpy_s(pSide->vertIndices, sizeof(pSide->vertIndices), reordered, sizeof(reordered));
	}
}

IModelMapCollision::IModelMapCollision(const IModel* const imodel, const r2::studiohdr_t* const pStudioHdr) : shapes(nullptr), shapeCount(pStudioHdr->staticCollisionCount), isRigged(false)
{
	assertm(shapeCount > -1, "bad feature!!!!");

	const uint32_t boneCount = imodel->GetBoneCount();

	// on models without static collsion, the bones have to be parsed
	if (shapeCount == 0)
	{
		isRigged = true;

		for (uint32_t i = 0; i < boneCount; i++)
		{
			const IModelBone* const bone = imodel->GetBone(i);

			shapeCount += bone->GetCollCount();
		}
	}

	if (shapeCount == 0)
	{
		*static_cast<IModelModel*>(this) = IModelModel(imodel, this);
		return;
	}

	shapes = new IModelMapCollisionShape[shapeCount]{};

	if (isRigged)
	{
		for (uint32_t boneIdx = 0; boneIdx < boneCount; boneIdx++)
		{
			const IModelBone* const bone = imodel->GetBone(boneIdx);

			const uint16_t collIndex = bone->GetCollIndex();
			const uint16_t collCount = bone->GetCollCount();

			if (collCount == 0)
				continue;

			for (uint16_t i = 0; i < collCount; i++)
			{
				shapes[collIndex + i] = IModelMapCollisionShape(pStudioHdr->pMapColl(collIndex + i), boneIdx);
			}
		}
	}
	else
	{
		for (int i = 0; i < shapeCount; i++)
		{
			shapes[i] = IModelMapCollisionShape(pStudioHdr->pMapColl(i), 0);
		}
	}

	// see IModelPhysics constructor
	*static_cast<IModelModel*>(this) = IModelModel(imodel, this);
}

IModelPerTriAABBLeaf::IModelPerTriAABBLeaf(const r1::mstudiopertrileaf_t* const pLeaf, IModelPerTriAABB* const pPerTriAABB) : triangleCount(0), triangles()
{
	const Vector& baseBBMin = pPerTriAABB->GetBBMin();
	const Vector& volumeScaled = pPerTriAABB->GetScaledVolume();

	bbmin.x = baseBBMin.x + (volumeScaled.x * static_cast<float>(pLeaf->bbmin[0]));
	bbmin.y = baseBBMin.y + (volumeScaled.y * static_cast<float>(pLeaf->bbmin[1]));
	bbmin.z = baseBBMin.z + (volumeScaled.z * static_cast<float>(pLeaf->bbmin[2]));

	bbmax.x = baseBBMin.x + (volumeScaled.x * static_cast<float>(pLeaf->bbmax[0]));
	bbmax.y = baseBBMin.y + (volumeScaled.y * static_cast<float>(pLeaf->bbmax[1]));
	bbmax.z = baseBBMin.z + (volumeScaled.z * static_cast<float>(pLeaf->bbmax[2]));

	for (int i = 0; i < r2::MAX_PER_TRI_LEAF_TRIANGLES; i++)
	{
		const r1::mstudiopertritri_t* const pTri = &pLeaf->triangles[i];

		const bool hasIndices = pTri->vertIndices[0] || pTri->vertIndices[1] || pTri->vertIndices[2];

		if (hasIndices == false)
		{
			break;
		}

		triangleCount++;

		triangles[i] = IModelPerTriAABBTri(pLeaf, pTri);
	}
}

IModelPerTriAABBLeaf::IModelPerTriAABBLeaf(const r1::mstudiopertrileaf_v1_t* const pLeaf, IModelPerTriAABB* const pPerTriAABB) : triangleCount(0), triangles()
{
	const Vector& baseBBMin = pPerTriAABB->GetBBMin();
	const Vector& volumeScaled = pPerTriAABB->GetScaledVolume();

	bbmin.x = baseBBMin.x + (volumeScaled.x * static_cast<float>(pLeaf->bbmin[0]));
	bbmin.y = baseBBMin.y + (volumeScaled.y * static_cast<float>(pLeaf->bbmin[1]));
	bbmin.z = baseBBMin.z + (volumeScaled.z * static_cast<float>(pLeaf->bbmin[2]));

	bbmax.x = baseBBMin.x + (volumeScaled.x * static_cast<float>(pLeaf->bbmax[0]));
	bbmax.y = baseBBMin.y + (volumeScaled.y * static_cast<float>(pLeaf->bbmax[1]));
	bbmax.z = baseBBMin.z + (volumeScaled.z * static_cast<float>(pLeaf->bbmax[2]));

	for (int i = 0; i < r2::MAX_PER_TRI_LEAF_TRIANGLES; i++)
	{
		const r1::mstudiopertritri_v1_t* const pTri = &pLeaf->triangles[i];

		const bool hasIndices = pTri->vertIndices[0] || pTri->vertIndices[1] || pTri->vertIndices[2];

		if (hasIndices == false)
		{
			break;
		}

		triangleCount++;

		triangles[i] = IModelPerTriAABBTri(pLeaf, pTri);
	}
}

IModelPerTriAABBNode::IModelPerTriAABBNode(const r1::mstudiopertrinode_t* const pNode, IModelPerTriAABB* const pPerTriAABB)
{
	const Vector& baseBBMin = pPerTriAABB->GetBBMin();
	const Vector& volumeScaled = pPerTriAABB->GetScaledVolume();

	bbmin.x = baseBBMin.x + (volumeScaled.x * static_cast<float>(pNode->bbmin[0]));
	bbmin.y = baseBBMin.y + (volumeScaled.y * static_cast<float>(pNode->bbmin[1]));
	bbmin.z = baseBBMin.z + (volumeScaled.z * static_cast<float>(pNode->bbmin[2]));

	bbmax.x = baseBBMin.x + (volumeScaled.x * static_cast<float>(pNode->bbmax[0]));
	bbmax.y = baseBBMin.y + (volumeScaled.y * static_cast<float>(pNode->bbmax[1]));
	bbmax.z = baseBBMin.z + (volumeScaled.z * static_cast<float>(pNode->bbmax[2]));

	for (int i = 0; i < 2; i++)
	{
		childIndex[i] = pNode->childIndex[i];
		children[i] = IsLeaf(i) ? reinterpret_cast<const void*>(pPerTriAABB->GetLeaf(~childIndex[i])) : reinterpret_cast<const void*>(pPerTriAABB->GetNode(childIndex[i]));
	}
}

IModelPerTriAABB::IModelPerTriAABB(const IModel* const imodel, const r1::studiohdr_t* const pHdr, const r1::mstudiopertrihdr_t* const pPerTriHdr) : bbmin(pPerTriHdr->bbmin), bbmax(pPerTriHdr->bbmax), volumeScaled((bbmax - bbmin) * r1::perTriAABBScale), version(pPerTriHdr->version),
	nodeCount(pHdr->pStudioHdr2()->m_nPerTriAABBNodeCount), nodes(nullptr), leafCount(pHdr->pStudioHdr2()->m_nPerTriAABBLeafCount), leaves(nullptr), vertCount(pHdr->pStudioHdr2()->m_nPerTriAABBVertCount), vertices(nullptr), triangleCount(0)
{
	switch (version)
	{
	case 0:
	{
		// no per tri data
		*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

		break;
	}
	case 1:
	{
		// never seen in the wild, be cool to actually find one though!
		// on models that do not have collision but are static prop (plant)
		if (leafCount == 0)
		{
			*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

			break;
		}

		leaves = new IModelPerTriAABBLeaf[leafCount]{};

		if (nodeCount)
		{
			nodes = new IModelPerTriAABBNode[nodeCount]{};

			for (int i = 0; i < nodeCount; i++)
			{
				nodes[i] = IModelPerTriAABBNode(pHdr->pPerTriNode(i), this);
			}
		}

		for (int i = 0; i < leafCount; i++)
		{
			leaves[i] = IModelPerTriAABBLeaf(pHdr->pPerTriLeaf_V1(i), this);

			triangleCount += leaves[i].GetTriCount();
		}

		vertices = new Vector[vertCount]{};

		for (int i = 0; i < vertCount; i++)
		{
			const r1::mstudiopertrivert_t* const pVert = pHdr->pPerTriVert_V1(i);

			Vector& pos = vertices[i];

			pos.x = bbmin.x + (volumeScaled.x * static_cast<float>(pVert->x));
			pos.y = bbmin.y + (volumeScaled.y * static_cast<float>(pVert->y));
			pos.z = bbmin.z + (volumeScaled.z * static_cast<float>(pVert->z));
		}

		*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

		break;
	}
	case 2:
	{
		// on models that do not have collision but are static prop (plant)
		if (leafCount == 0)
		{
			*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

			break;
		}

		leaves = new IModelPerTriAABBLeaf[leafCount]{};

		if (nodeCount)
		{
			nodes = new IModelPerTriAABBNode[nodeCount]{};

			for (int i = 0; i < nodeCount; i++)
			{
				nodes[i] = IModelPerTriAABBNode(pHdr->pPerTriNode(i), this);
			}
		}

		for (int i = 0; i < leafCount; i++)
		{
			leaves[i] = IModelPerTriAABBLeaf(pHdr->pPerTriLeaf(i), this);

			triangleCount += leaves[i].GetTriCount();
		}

		vertices = new Vector[vertCount]{};

		for (int i = 0; i < vertCount; i++)
		{
			const r1::mstudiopertrivert_t* const pVert = pHdr->pPerTriVert(i);

			Vector& pos = vertices[i];

			pos.x = bbmin.x + (volumeScaled.x * static_cast<float>(pVert->x));
			pos.y = bbmin.y + (volumeScaled.y * static_cast<float>(pVert->y));
			pos.z = bbmin.z + (volumeScaled.z * static_cast<float>(pVert->z));
		}

		*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

		break;
	}
	default:
	{
		assertm(false, "invalid version");
		break;
	}
	}
}

IModelPerTriAABB::IModelPerTriAABB(const IModel* const imodel, const r2::studiohdr_t* const pHdr, const r2::mstudiopertrihdr_t* const pPerTriHdr) : bbmin(pPerTriHdr->bbmin), bbmax(pPerTriHdr->bbmax), volumeScaled((bbmax - bbmin) * r2::perTriAABBScale), version(pPerTriHdr->version),
	nodeCount(pHdr->m_nPerTriAABBNodeCount), nodes(nullptr), leafCount(pHdr->m_nPerTriAABBLeafCount), leaves(nullptr), vertCount(pHdr->m_nPerTriAABBVertCount), vertices(nullptr), triangleCount(0)
{
	switch (version)
	{
	case 0:
	{
		// no per tri data
		*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

		break;
	}
	case 1:
	{
		// never seen in the wild, be cool to actually find one though!
		assertm(false, "unsupported version");
		break;
	}
	case 2:
	{
		// on models that do not have collision but are static prop (plant)
		if (leafCount == 0)
		{
			*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

			break;
		}

		leaves = new IModelPerTriAABBLeaf[leafCount]{};

		if (nodeCount)
		{
			nodes = new IModelPerTriAABBNode[nodeCount]{};

			for (int i = 0; i < nodeCount; i++)
			{
				nodes[i] = IModelPerTriAABBNode(reinterpret_cast<const r1::mstudiopertrinode_t* const>(pHdr->pPerTriNode(i)), this);
			}
		}

		for (int i = 0; i < leafCount; i++)
		{
			leaves[i] = IModelPerTriAABBLeaf(reinterpret_cast<const r1::mstudiopertrileaf_t* const>(pHdr->pPerTriLeaf(i)), this);

			triangleCount += leaves[i].GetTriCount();
		}

		vertices = new Vector[vertCount]{};

		for (int i = 0; i < vertCount; i++)
		{
			const r2::mstudiopertrivert_t* const pVert = pHdr->pPerTriVert(i);

			Vector& pos = vertices[i];

			pos.x = bbmin.x + (volumeScaled.x * static_cast<float>(pVert->x));
			pos.y = bbmin.y + (volumeScaled.y * static_cast<float>(pVert->y));
			pos.z = bbmin.z + (volumeScaled.z * static_cast<float>(pVert->z));
		}

		*static_cast<IModelModel*>(this) = IModelModel(imodel, this);

		break;
	}
	default:
	{
		assertm(false, "invalid version");
		break;
	}
	}
}

IModelUIPanel::IModelUIPanel(const IModel* const imodel, const r2::UIPanelHeader_s* const pUIPanel) : path(nullptr), hash(pUIPanel->meshHash)
{
	path = AllocStudioString(pUIPanel->pPanelMesh()->pszPanelName());

	*static_cast<IModelModel*>(this) = IModelModel(imodel, pUIPanel);
}

IModelUIPanel::IModelUIPanel(const IModel* const imodel, const r5::UIPanelHeader_s* const pUIPanel) : path(nullptr), hash(pUIPanel->meshHash)
{
	path = AllocStudioString(pUIPanel->pPanelMesh()->pszPanelName());

	*static_cast<IModelModel*>(this) = IModelModel(imodel, pUIPanel);
}


//
// Model Bone Data
//

// these suck
// should be shared for everything before apex I think? also what about boing
IModelJiggleBone::IModelJiggleBone(const r1::mstudiojigglebone_t* const pJiggle, const char* boneName, const uint32_t boneIndex) : IModelProcBone(boneName, boneIndex, eProcBoneType::JIGGLE), flags(IMODELJIGGLE_NONE), length(pJiggle->length), tipMass(pJiggle->tipMass), tipFriction(0.0f),
	yawStiffness(pJiggle->yawStiffness), yawDamping(pJiggle->yawDamping), pitchStiffness(pJiggle->pitchStiffness), pitchDamping(pJiggle->pitchDamping), alongStiffness(pJiggle->alongStiffness), alongDamping(pJiggle->alongDamping),
	angleLimit(pJiggle->angleLimit),
	minYaw(pJiggle->minYaw), maxYaw(pJiggle->maxYaw), yawFriction(pJiggle->yawFriction), yawBounce(pJiggle->yawBounce),
	minPitch(pJiggle->minPitch), maxPitch(pJiggle->maxPitch), pitchFriction(pJiggle->pitchFriction), pitchBounce(pJiggle->pitchBounce),
	baseMass(pJiggle->baseMass), baseStiffness(pJiggle->baseStiffness), baseDamping(pJiggle->baseDamping), baseMinLeft(pJiggle->baseMinLeft), baseMaxLeft(pJiggle->baseMaxLeft), baseLeftFriction(pJiggle->baseLeftFriction), baseMinUp(pJiggle->baseMinUp), baseMaxUp(pJiggle->baseMaxUp), baseUpFriction(pJiggle->baseUpFriction),
	baseMinForward(pJiggle->baseMinForward), baseMaxForward(pJiggle->baseMaxForward), baseForwardFriction(pJiggle->baseForwardFriction)
{
	flags |= (pJiggle->flags & IMODELJIGGLE_FLAG_MASK);
}

// not ideal but works for now
IModelJiggleBone::IModelJiggleBone(const r2::mstudiojigglebone_t* const pJiggle, const char* boneName, const uint32_t boneIndex) : IModelJiggleBone(reinterpret_cast<const r1::mstudiojigglebone_t* const>(pJiggle), boneName, boneIndex)
{

}

IModelJiggleBone::IModelJiggleBone(const r5::mstudiojigglebone_t* const pJiggle, const char* boneName) : IModelProcBone(boneName, pJiggle->bone, eProcBoneType::JIGGLE), flags(pJiggle->flags), length(pJiggle->length), tipMass(pJiggle->tipMass), tipFriction(pJiggle->tipFriction),
yawStiffness(pJiggle->yawStiffness), yawDamping(pJiggle->yawDamping), pitchStiffness(pJiggle->pitchStiffness), pitchDamping(pJiggle->pitchDamping), alongStiffness(pJiggle->alongStiffness), alongDamping(pJiggle->alongDamping),
angleLimit(pJiggle->angleLimit),
minYaw(pJiggle->minYaw), maxYaw(pJiggle->maxYaw), yawFriction(pJiggle->yawFriction), yawBounce(pJiggle->yawBounce),
minPitch(pJiggle->minPitch), maxPitch(pJiggle->maxPitch), pitchFriction(pJiggle->pitchFriction), pitchBounce(pJiggle->pitchBounce),
baseMass(pJiggle->baseMass), baseStiffness(pJiggle->baseStiffness), baseDamping(pJiggle->baseDamping), baseMinLeft(pJiggle->baseMinLeft), baseMaxLeft(pJiggle->baseMaxLeft), baseLeftFriction(pJiggle->baseLeftFriction), baseMinUp(pJiggle->baseMinUp), baseMaxUp(pJiggle->baseMaxUp), baseUpFriction(pJiggle->baseUpFriction),
baseMinForward(pJiggle->baseMinForward), baseMaxForward(pJiggle->baseMaxForward), baseForwardFriction(pJiggle->baseForwardFriction)
{
	flags |= (pJiggle->flags & IMODELJIGGLE_FLAG_MASK);
	flags |= IMODELJIGGLE_IS_APEX_LEGENDS;
}

IModelBone::IModelBone(const r1::mstudiobone_t* const pBone, const uint32_t indexForProc) : parent(pBone->parent), pos(pBone->pos), rot(pBone->rot), quat(pBone->quat), scale(pBone->scale), poseToBone(pBone->poseToBone),
qAlignment(pBone->qAlignment), flags(pBone->flags), contents(pBone->contents), surfacepropLookup(pBone->surfacepropLookup), physicsbone(pBone->physicsbone), procType(eProcBoneType::NONE),
// version 53 and above
collisionIndex(0xFFFF), collisionCount(0x0)
{
	name = AllocStudioString(pBone->pszName());
	surfaceProp = AllocStudioString(pBone->pszSurfaceProp());

	procType = static_cast<eProcBoneType>(pBone->proctype);

	switch (procType)
	{
	case eProcBoneType::NONE:
	{
		procBone.invalid = nullptr;
		break;
	}
	case eProcBoneType::AXISINTERP:
	case eProcBoneType::QUATINTERP:
	case eProcBoneType::AIMATBONE:
	case eProcBoneType::AIMATATTACH:
	{
		procBone.invalid = nullptr;
		Error("Bone %s had unsupported proctype %i, exiting...\n", name, pBone->proctype);
		break;
	}
	case eProcBoneType::JIGGLE:
	{
		IModelJiggleBone* jiggleBone = new IModelJiggleBone(reinterpret_cast<const r1::mstudiojigglebone_t* const>(pBone->pProcedure()), name, indexForProc); // NOTE: THIS NEEDS TO BE CLEANED UP!!!! check type for this??

		procBone.jiggle = jiggleBone;
		break;
	}
	case eProcBoneType::TWIST_MASTER:
	case eProcBoneType::TWIST_SLAVE:
	default:
	{
		procBone.invalid = nullptr;
		Error("Bone %s had unsupported proctype %i, exiting...\n", name, pBone->proctype);
		break;
	}
	}
}

IModelBone::IModelBone(const r2::mstudiobone_t* const pBone, const uint32_t indexForProc) : parent(pBone->parent), pos(pBone->pos), rot(pBone->rot), quat(pBone->quat), scale(pBone->scale), poseToBone(pBone->poseToBone),
qAlignment(pBone->qAlignment), flags(pBone->flags), contents(pBone->contents), surfacepropLookup(pBone->surfacepropLookup), physicsbone(pBone->physicsbone), procType(eProcBoneType::NONE),
// version 53 and above
collisionIndex(pBone->collisionIndex), collisionCount(pBone->collisionCount)
{
	name = AllocStudioString(pBone->pszName());
	surfaceProp = AllocStudioString(pBone->pszSurfaceProp());

	procType = static_cast<eProcBoneType>(pBone->proctype);

	switch (procType)
	{
	case eProcBoneType::NONE:
	{
		procBone.invalid = nullptr;
		break;
	}
	case eProcBoneType::AXISINTERP:
	case eProcBoneType::QUATINTERP:
	case eProcBoneType::AIMATBONE:
	case eProcBoneType::AIMATATTACH:
	{
		procBone.invalid = nullptr;
		Error("Bone %s had unsupported proctype %i, exiting...\n", name, pBone->proctype);
		break;
	}
	case eProcBoneType::JIGGLE:
	{
		IModelJiggleBone* jiggleBone = new IModelJiggleBone(reinterpret_cast<const r2::mstudiojigglebone_t* const>(pBone->pProcedure()), name, indexForProc); // NOTE: THIS NEEDS TO BE CLEANED UP!!!! check type for this??

		procBone.jiggle = jiggleBone;
		break;
	}
	case eProcBoneType::TWIST_MASTER:
	case eProcBoneType::TWIST_SLAVE:
	default:
	{
		procBone.invalid = nullptr;
		Error("Bone %s had unsupported proctype %i, exiting...\n", name, pBone->proctype);
		break;
	}
	}
}

IModelBone::IModelBone(const r5::mstudiobone_t* const pBone) : parent(pBone->parent), pos(pBone->pos), rot(pBone->rot), quat(pBone->quat), scale(pBone->scale), poseToBone(pBone->poseToBone),
qAlignment(pBone->qAlignment), flags(pBone->flags), contents(pBone->contents), surfacepropLookup(pBone->surfacepropLookup), physicsbone(pBone->physicsbone), procType(eProcBoneType::NONE),
// version 53 and above
collisionIndex(pBone->collisionIndex == -1 ? 0xFFFF : static_cast<uint16_t>(pBone->collisionIndex)), collisionCount(pBone->collisionIndex == -1 ? 0 : 1)
{
	name = AllocStudioString(pBone->pszName());
	surfaceProp = AllocStudioString(pBone->pszSurfaceProp());

	procType = static_cast<eProcBoneType>(pBone->proctype);

	switch (procType)
	{
	case eProcBoneType::NONE:
	{
		procBone.invalid = nullptr;
		break;
	}
	case eProcBoneType::AXISINTERP:
	case eProcBoneType::QUATINTERP:
	case eProcBoneType::AIMATBONE:
	case eProcBoneType::AIMATATTACH:
	{
		procBone.invalid = nullptr;
		Error("Bone %s had unsupported proctype %i, exiting...\n", name, pBone->proctype);
		break;
	}
	case eProcBoneType::JIGGLE:
	{
		IModelJiggleBone* jiggleBone = new IModelJiggleBone(reinterpret_cast<const r5::mstudiojigglebone_t* const>(pBone->pProcedure()), name);

		procBone.jiggle = jiggleBone;
		break;
	}
	case eProcBoneType::TWIST_MASTER:
	case eProcBoneType::TWIST_SLAVE:
	default:
	{
		procBone.invalid = nullptr;
		Error("Bone %s had unsupported proctype %i, exiting...\n", name, pBone->proctype);
		break;
	}
	}
}

IModelBone::~IModelBone()
{
	FreeAllocArray(name);
	FreeAllocArray(surfaceProp);

	switch (procType)
	{
	case eProcBoneType::NONE:
	{
		break;
	}
	case eProcBoneType::AXISINTERP:
	case eProcBoneType::QUATINTERP:
	case eProcBoneType::AIMATBONE:
	case eProcBoneType::AIMATATTACH:
	{
		assertm(false, "invalid procbone should not be allocated");
		FreeAllocVar(procBone.invalid);
		break;
	}
	case eProcBoneType::JIGGLE:
	{
		FreeAllocVar(procBone.jiggle);
		break;
	}
	case eProcBoneType::TWIST_MASTER:
	case eProcBoneType::TWIST_SLAVE:
	default:
	{
		assertm(false, "invalid procbone should not be allocated");
		FreeAllocVar(procBone.invalid);
		break;
	}
	}
}

// hitbox
IModelHitbox::IModelHitbox(const r1::mstudiobbox_t* const pBBox) : bone(pBBox->bone), group(pBBox->group), bbmin(pBBox->bbmin), bbmax(pBBox->bbmax), hitboxName(nullptr), forceCritPoint(0), hitdataGroup(nullptr)
{
	// don't alloc for a null names
	const char* const name = pBBox->pszHitboxName();
	if (name[0] != '\0')
	{
		hitboxName = AllocStudioString(name);
	}
}

IModelHitbox::IModelHitbox(const r2::mstudiobbox_t* const pBBox) : bone(pBBox->bone), group(pBBox->group), bbmin(pBBox->bbmin), bbmax(pBBox->bbmax), hitboxName(nullptr), forceCritPoint(pBBox->forceCritPoint), hitdataGroup(nullptr)
{
	// don't alloc for a null names
	const char* const name = pBBox->pszHitboxName();
	if (name[0] != '\0')
	{
		hitboxName = AllocStudioString(name);
	}

	const char* const hitdata = pBBox->pszHitDataGroup();
	if (hitdata[0] != '\0')
	{
		hitdataGroup = AllocStudioString(hitdata);
	}
}

IModelHitbox::IModelHitbox(const r5::mstudiobbox_t* const pBBox) : bone(pBBox->bone), group(pBBox->group), bbmin(pBBox->bbmin), bbmax(pBBox->bbmax), hitboxName(nullptr), forceCritPoint(pBBox->forceCritPoint), hitdataGroup(nullptr)
{
	// don't alloc for a null names
	const char* const name = pBBox->pszHitboxName();
	if (name[0] != '\0')
	{
		hitboxName = AllocStudioString(name);
	}

	const char* const hitdata = pBBox->pszHitDataGroup();
	if (hitdata[0] != '\0')
	{
		hitdataGroup = AllocStudioString(hitdata);
	}
}


//
// IModel
//

IModel::IModel(const std::filesystem::path path, const r1::studiohdr_t* const pHdr) :  sourcePath(path), source(nullptr), sourceFlags(IMODELFLAGS_NONE), sourceType(IModelSourceType_t::SOURCE_STUDIO),
	// general
	namedebug(), name(nullptr), eyeposition(pHdr->eyeposition), illumposition(pHdr->illumposition), hull_min(pHdr->hull_min), hull_max(pHdr->hull_max), view_bbmin(pHdr->view_bbmin), view_bbmax(pHdr->view_bbmax),
	surfaceProp(nullptr), keyValues(nullptr), mass(pHdr->mass), contents(pHdr->contents), constdirectionallightdot(pHdr->constdirectionallightdot), fadeDistance(pHdr->fadeDistance), gatherSize(0.0f),
	illumpositionattachmentindex(pHdr->IllumPositionAttachmentIndex()), flMaxEyeDeflection(pHdr->MaxEyeDeflection()),

	// bone
	bones(nullptr), boneCount(0u), procBones(nullptr), procBoneCount(0u), boneTransforms(nullptr), boneTransformCount(0u), boneFollowers(nullptr), boneFollowerCount(0u), hitboxSets(nullptr), hitboxSetCount(0u),
	localAttachments(nullptr), localAttachmentCount(0u),

	// mesh
	lods(nullptr), lodCount(0u), bodyparts(nullptr), bodypartCount(0u), rootLOD(pHdr->rootLOD), numAllowedRootLODs(pHdr->numAllowedRootLODs),
	uiPanels(nullptr), uiPanelCount(0u),
	
	// material
	textures(nullptr), textureCount(0u), cdTextures(nullptr), cdTexturesCount(0u), skins(nullptr), skinCount(0u),

	// animation
	animations(nullptr), animationCount(0u), sequences(nullptr), sequenceCount(0u), localNodeNames(nullptr), localNodeCount(0u), ikChains(nullptr), ikChainCount(0u),
	localPoseParameters(nullptr), localPoseParameterCount(0u), localIkAutoPlayLocks(nullptr), localIkAutoPlayLockCount(0u), includeModels(nullptr), includeModelCount(0u), animBlockName(nullptr), animBlocks(nullptr), animBlockCount(0u),

	// collision
	physics(nullptr), mapCollision(nullptr)
{
	// get loose files
	CManagedBuffer* looseBuf = g_BufferManager.ClaimBuffer();
	const StudioLooseData_t looseData(path, keepAfterLastSlashOrBackslash(pHdr->pszName()), looseBuf->Buffer(), managedBufferSize, pHdr->flags & (STUDIOHDR_FLAGS_USES_VERTEX_COLOR | STUDIOHDR_FLAGS_USES_UV2), pHdr->numanimblocks ? keepAfterLastSlashOrBackslash(pHdr->pszAnimBlockName()) : nullptr);

	if (looseData.VerifyFileIntegrity(reinterpret_cast<const studiohdr_short_t* const>(pHdr)) == false)
	{
		sourceType = IModelSourceType_t::SOURCE_FAILED;
		Log("failed to parse source %s, skipping...\n", sourcePath.c_str());
		return;
	}

	// general
	memcpy_s(namedebug, 64, pHdr->name, 64);
	name = AllocStudioString(pHdr->pszName());
	surfaceProp = AllocStudioString(pHdr->pszSurfaceProp());
	
	if (pHdr->keyvaluesize)
	{
		keyValues = AllocStudioString(pHdr->KeyValueText(), static_cast<size_t>(pHdr->keyvaluesize));
	}

	source.pStudioSource = new studiosource_t(pHdr);

	SetFlags_R1(pHdr->flags); // set our flags

	// [rika]: a lot of this can be threaded in the future

	// bones (bones, attachments, hitboxes, etc)
	uint32_t numProcBones = 0u;
	for (int i = 0; i < pHdr->numbones; i++)
	{
		// proctype should be 0 if there is no procbone
		if (pHdr->pBone(i)->proctype)
		{
			numProcBones++;
		}
	}

	IMODEL_ALLOC_DATA_LIMIT(IModelBone, bones, boneCount, pHdr->numbones, MAXSTUDIOBONES);
	IMODEL_ALLOC_DATA_LIMIT(IModelProcBone*, procBones, procBoneCount, numProcBones, MAXSTUDIOBONES);
	for (uint32_t i = 0u, procIdx = 0u; i < boneCount; i++)
	{
		bones[i] = IModelBone(pHdr->pBone(static_cast<int>(i)), i);

		const IModelBone* const bone = GetBone(i);
		if (bone->GetProcType() == eProcBoneType::NONE)
		{
			continue;
		}

		procBones[procIdx] = bone->GetProcBone<IModelProcBone>();
		procIdx++;
	}

	IMODEL_ALLOC_DATA_LIMIT(IModelBoneTransform, boneTransforms, boneTransformCount, pHdr->NumSrcBoneTransforms(), MAXSTUDIOBONES);
	for (int i = 0; i < pHdr->NumSrcBoneTransforms(); i++)
	{
		boneTransforms[i] = IModelBoneTransform(pHdr->SrcBoneTransform(i));
	}

	IMODEL_ALLOC_DATA(IModelHitboxSet, hitboxSets, hitboxSetCount, pHdr->numhitboxsets);
	for (int i = 0; i < pHdr->numhitboxsets; i++)
	{
		hitboxSets[i] = IModelHitboxSet(pHdr->pHitboxSet(i));
	}

	IMODEL_ALLOC_DATA(IModelAttachment, localAttachments, localAttachmentCount, pHdr->numlocalattachments);
	for (int i = 0; i < pHdr->numlocalattachments; i++)
	{
		localAttachments[i] = IModelAttachment(pHdr->pLocalAttachment(i));
	}

	// meshes (bodyparts, lods, models, etc)
	if (pHdr->numbodyparts)
	{
		const OptimizedModel::FileHeader_t* const pVTX = looseData.GetVTX();
		const vvd::vertexFileHeader_t* const pVVD = looseData.GetVVD();

		assertm(pVTX && pVVD, "missing vertex files");
		assertm(pVTX->numLODs == pVVD->numLODs, "number of lods across files mismatched");

		IMODEL_ALLOC_DATA(IModelBodypart, bodyparts, bodypartCount, pHdr->numbodyparts);
		uint16_t modelCount = 0u;
		for (uint32_t i = 0; i < bodypartCount; i++)
		{
			bodyparts[i] = IModelBodypart(pHdr->pBodypart(static_cast<int>(i)), modelCount);
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelLOD, lods, lodCount, pVTX->numLODs, MAX_NUM_LODS);
		for (uint32_t i = 0; i < lodCount; i++)
		{
			lods[i] = IModelLOD(i, modelCount, sourceFlags, pHdr, &looseData);
		}
	}

	// materials (textures, skins, etc..)
	if (pHdr->numtextures)
	{
		assertm(pHdr->numtextures == pHdr->numskinref, "these should always match");

		IMODEL_ALLOC_DATA_LIMIT(IModelTexture, textures, textureCount, pHdr->numtextures, MAXSTUDIOSKINS);
		for (int i = 0; i < pHdr->numtextures; i++)
		{
			textures[i] = IModelTexture(pHdr->pTexture(i));
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelSkin, skins, skinCount, pHdr->numskinfamilies, MAXSTUDIOSKINS);
		for (int i = 0; i < pHdr->numskinfamilies; i++)
		{
			skins[i] = IModelSkin(pHdr->pSkinFamily(i), pHdr->numskinref);
		}
	}

	IMODEL_ALLOC_DATA(char*, cdTextures, cdTexturesCount, pHdr->numcdtextures);
	for (int i = 0; i < pHdr->numcdtextures; i++)
	{
		cdTextures[i] = AllocStudioString(pHdr->pCdtexture(i));
	}

	// animations (animations, sequences, nodes, ik, etc..)
	if (pHdr->numlocalanim || pHdr->numlocalseq)
	{
		IMODEL_ALLOC_DATA_LIMIT(IModelAnimation, animations, animationCount, pHdr->numlocalanim, MAXSTUDIOANIMS);
		for (int i = 0; i < pHdr->numlocalanim; i++)
		{
			animations[i] = IModelAnimation(pHdr, pHdr->pAnimdesc(i), &looseData);
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelSequence, sequences, sequenceCount, pHdr->numlocalseq, MAXSTUDIOSEQUENCES);
		for (int i = 0; i < pHdr->numlocalseq; i++)
		{
			sequences[i] = IModelSequence(pHdr, pHdr->pSeqdesc(i));
		}
	}

	// todo transitions
	IMODEL_ALLOC_DATA(char*, localNodeNames, localNodeCount, pHdr->numlocalnodes);
	for (int i = 0; i < pHdr->numlocalnodes; i++)
	{
		localNodeNames[i] = AllocStudioString(pHdr->pszLocalNodeName(i));
	}

	IMODEL_ALLOC_DATA(IModelIKChain, ikChains, ikChainCount, pHdr->numikchains);
	for (int i = 0; i < pHdr->numikchains; i++)
	{
		ikChains[i] = IModelIKChain(pHdr->pIKChain(i));
	}

	IMODEL_ALLOC_DATA(IModelPoseParameter, localPoseParameters, localPoseParameterCount, pHdr->numlocalposeparameters);
	for (int i = 0; i < pHdr->numlocalposeparameters; i++)
	{
		localPoseParameters[i] = IModelPoseParameter(pHdr->pLocalPoseParameter(i));
	}

	IMODEL_ALLOC_DATA(IModelIKLock, localIkAutoPlayLocks, localIkAutoPlayLockCount, pHdr->numlocalikautoplaylocks);
	for (int i = 0; i < pHdr->numlocalikautoplaylocks; i++)
	{
		localIkAutoPlayLocks[i] = IModelIKLock(pHdr->pLocalIKAutoplayLock(i));
	}

	IMODEL_ALLOC_DATA(IModelModelGroup, includeModels, includeModelCount, pHdr->numincludemodels);
	for (int i = 0; i < pHdr->numincludemodels; i++)
	{
		includeModels[i] = IModelModelGroup(pHdr->pModelGroup(i));
	}

	animBlockName = AllocStudioString(pHdr->pszAnimBlockName());

	IMODEL_ALLOC_DATA(IModelAnimBlock, animBlocks, animBlockCount, pHdr->numanimblocks);
	for (int i = 0; i < pHdr->numanimblocks; i++)
	{
		animBlocks[i] = IModelAnimBlock(pHdr->pAnimBlock(i));
	}

	// collision
	const ivps::phyheader_t* const pPHYS = looseData.GetPHYS_VALVE();
	if (pPHYS)
	{
		physics = new IModelPhysics(this, pPHYS);
	}

	if (pHdr->pPerTriHdr())
	{
		perTriAABB = new IModelPerTriAABB(this, pHdr, pHdr->pPerTriHdr());
	}

	g_BufferManager.RelieveBuffer(looseBuf);
}

IModel::IModel(const std::filesystem::path path, const r2::studiohdr_t* const pHdr) :  sourcePath(path), source(nullptr), sourceFlags(IMODELFLAGS_NONE), sourceType(IModelSourceType_t::SOURCE_STUDIO),
	// general
	namedebug(), name(nullptr), eyeposition(pHdr->eyeposition), illumposition(pHdr->illumposition), hull_min(pHdr->hull_min), hull_max(pHdr->hull_max), view_bbmin(pHdr->view_bbmin), view_bbmax(pHdr->view_bbmax),
	surfaceProp(nullptr), keyValues(nullptr), mass(pHdr->mass), contents(pHdr->contents), constdirectionallightdot(pHdr->constdirectionallightdot), fadeDistance(pHdr->fadeDistance), gatherSize(0.0f),
	illumpositionattachmentindex(pHdr->illumpositionattachmentindex), flMaxEyeDeflection(0.0f),

	// bone
	bones(nullptr), boneCount(0u), procBones(nullptr), procBoneCount(0u), boneTransforms(nullptr), boneTransformCount(0u), boneFollowers(nullptr), boneFollowerCount(0u), hitboxSets(nullptr), hitboxSetCount(0u),
	localAttachments(nullptr), localAttachmentCount(0u),

	// mesh
	lods(nullptr), lodCount(0u), bodyparts(nullptr), bodypartCount(0u), rootLOD(pHdr->rootLOD), numAllowedRootLODs(pHdr->numAllowedRootLODs),
	uiPanels(nullptr), uiPanelCount(0u),
	
	// material
	textures(nullptr), textureCount(0u), cdTextures(nullptr), cdTexturesCount(0u), skins(nullptr), skinCount(0u),

	// animation
	animations(nullptr), animationCount(0u), sequences(nullptr), sequenceCount(0u), localNodeNames(nullptr), localNodeCount(0u), ikChains(nullptr), ikChainCount(0u),
	localPoseParameters(nullptr), localPoseParameterCount(0u), localIkAutoPlayLocks(nullptr), localIkAutoPlayLockCount(0u), includeModels(nullptr), includeModelCount(0u), animBlockName(nullptr), animBlocks(nullptr), animBlockCount(0u),

	// collision
	physics(nullptr), mapCollision(nullptr)
{
	// get loose files
	const StudioLooseData_t looseData(reinterpret_cast<const char* const>(pHdr));

	if (looseData.VerifyFileIntegrity(reinterpret_cast<const studiohdr_short_t* const>(pHdr)) == false)
	{
		sourceType = IModelSourceType_t::SOURCE_FAILED;
		Log("failed to parse source %s, skipping...\n", sourcePath.c_str());
		return;
	}

	// general
	memcpy_s(namedebug, 64, pHdr->name, 64);
	name = AllocStudioString(pHdr->pszName());
	surfaceProp = AllocStudioString(pHdr->pszSurfaceProp());
	
	if (pHdr->keyvaluesize)
	{
		keyValues = AllocStudioString(pHdr->KeyValueText(), static_cast<size_t>(pHdr->keyvaluesize));
	}

	source.pStudioSource = new studiosource_t(pHdr);

	SetFlags_R2(pHdr->flags); // set our flags

	// [rika]: a lot of this can be threaded in the future

	// bones (bones, attachments, hitboxes, etc)
	uint32_t numProcBones = 0u;
	for (int i = 0; i < pHdr->numbones; i++)
	{
		// proctype should be 0 if there is no procbone
		if (pHdr->pBone(i)->proctype)
		{
			numProcBones++;
		}
	}

	IMODEL_ALLOC_DATA_LIMIT(IModelBone, bones, boneCount, pHdr->numbones, MAXSTUDIOBONES);
	IMODEL_ALLOC_DATA_LIMIT(IModelProcBone*, procBones, procBoneCount, numProcBones, MAXSTUDIOBONES);
	for (uint32_t i = 0u, procIdx = 0u; i < boneCount; i++)
	{
		bones[i] = IModelBone(pHdr->pBone(static_cast<int>(i)), i);

		const IModelBone* const bone = GetBone(i);
		if (bone->GetProcType() == eProcBoneType::NONE)
		{
			continue;
		}

		procBones[procIdx] = bone->GetProcBone<IModelProcBone>();
		procIdx++;
	}

	IMODEL_ALLOC_DATA_LIMIT(IModelBoneTransform, boneTransforms, boneTransformCount, pHdr->numsrcbonetransform, MAXSTUDIOBONES);
	for (int i = 0; i < pHdr->numsrcbonetransform; i++)
	{
		boneTransforms[i] = IModelBoneTransform(pHdr->SrcBoneTransform(i));
	}

	IMODEL_ALLOC_DATA_LIMIT(int, boneFollowers, boneFollowerCount, pHdr->boneFollowerCount, MAXSTUDIOBONES);
	for (int i = 0u; i < pHdr->boneFollowerCount; i++)
	{
		boneFollowers[i] = pHdr->BoneFollower(i);
	}

	IMODEL_ALLOC_DATA(IModelHitboxSet, hitboxSets, hitboxSetCount, pHdr->numhitboxsets);
	for (int i = 0; i < pHdr->numhitboxsets; i++)
	{
		hitboxSets[i] = IModelHitboxSet(pHdr->pHitboxSet(i));
	}

	IMODEL_ALLOC_DATA(IModelAttachment, localAttachments, localAttachmentCount, pHdr->numlocalattachments);
	for (int i = 0; i < pHdr->numlocalattachments; i++)
	{
		localAttachments[i] = IModelAttachment(pHdr->pLocalAttachment(i));
	}

	// meshes (bodyparts, lods, models, etc)
	if (pHdr->numbodyparts)
	{
		const OptimizedModel::FileHeader_t* const pVTX = looseData.GetVTX();
		const vvd::vertexFileHeader_t* const pVVD = looseData.GetVVD();

		assertm(pVTX && pVVD, "missing vertex files");
		assertm(pVTX->numLODs == pVVD->numLODs, "number of lods across files mismatched");

		IMODEL_ALLOC_DATA(IModelBodypart, bodyparts, bodypartCount, pHdr->numbodyparts);
		uint16_t modelCount = 0u;
		for (uint32_t i = 0; i < bodypartCount; i++)
		{
			bodyparts[i] = IModelBodypart(pHdr->pBodypart(static_cast<int>(i)), modelCount);
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelLOD, lods, lodCount, pVTX->numLODs, MAX_NUM_LODS);
		for (uint32_t i = 0; i < lodCount; i++)
		{
			lods[i] = IModelLOD(i, modelCount, sourceFlags, pHdr, &looseData);
		}
	}

	if (pHdr->uiPanelCount)
	{
		IMODEL_ALLOC_DATA(IModelUIPanel, uiPanels, uiPanelCount, pHdr->uiPanelCount);

		for (uint32_t i = 0u; i < uiPanelCount; i++)
		{
			uiPanels[i] = IModelUIPanel(this, pHdr->pUiPanel(static_cast<int>(i)));
		}
	}

	// materials (textures, skins, etc..)
	if (pHdr->numtextures)
	{
		assertm(pHdr->numtextures == pHdr->numskinref, "these should always match");

		IMODEL_ALLOC_DATA_LIMIT(IModelTexture, textures, textureCount, pHdr->numtextures, MAXSTUDIOSKINS);
		for (int i = 0; i < pHdr->numtextures; i++)
		{
			textures[i] = IModelTexture(pHdr->pTexture(i));
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelSkin, skins, skinCount, pHdr->numskinfamilies, MAXSTUDIOSKINS);
		for (int i = 0; i < pHdr->numskinfamilies; i++)
		{
			skins[i] = IModelSkin(pHdr->pSkinFamily(i), pHdr->numskinref);
		}
	}

	IMODEL_ALLOC_DATA(char*, cdTextures, cdTexturesCount, pHdr->numcdtextures);
	for (int i = 0; i < pHdr->numcdtextures; i++)
	{
		cdTextures[i] = AllocStudioString(pHdr->pCdtexture(i));
	}

	// animations (animations, sequences, nodes, ik, etc..)
	if (pHdr->numlocalanim || pHdr->numlocalseq)
	{
		IMODEL_ALLOC_DATA_LIMIT(IModelAnimation, animations, animationCount, pHdr->numlocalanim, MAXSTUDIOANIMS);
		for (int i = 0; i < pHdr->numlocalanim; i++)
		{
			animations[i] = IModelAnimation(pHdr, pHdr->pAnimdesc(i));
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelSequence, sequences, sequenceCount, pHdr->numlocalseq, MAXSTUDIOSEQUENCES);
		for (int i = 0; i < pHdr->numlocalseq; i++)
		{
			sequences[i] = IModelSequence(pHdr, pHdr->pSeqdesc(i));
		}
	}

	// todo transitions
	IMODEL_ALLOC_DATA(char*, localNodeNames, localNodeCount, pHdr->numlocalnodes);
	for (int i = 0; i < pHdr->numlocalnodes; i++)
	{
		localNodeNames[i] = AllocStudioString(pHdr->pszLocalNodeName(i));
	}

	IMODEL_ALLOC_DATA(IModelIKChain, ikChains, ikChainCount, pHdr->numikchains);
	for (int i = 0; i < pHdr->numikchains; i++)
	{
		ikChains[i] = IModelIKChain(pHdr->pIKChain(i));
	}

	IMODEL_ALLOC_DATA(IModelPoseParameter, localPoseParameters, localPoseParameterCount, pHdr->numlocalposeparameters);
	for (int i = 0; i < pHdr->numlocalposeparameters; i++)
	{
		localPoseParameters[i] = IModelPoseParameter(pHdr->pLocalPoseParameter(i));
	}

	IMODEL_ALLOC_DATA(IModelIKLock, localIkAutoPlayLocks, localIkAutoPlayLockCount, pHdr->numlocalikautoplaylocks);
	for (int i = 0; i < pHdr->numlocalikautoplaylocks; i++)
	{
		localIkAutoPlayLocks[i] = IModelIKLock(pHdr->pLocalIKAutoplayLock(i));
	}

	IMODEL_ALLOC_DATA(IModelModelGroup, includeModels, includeModelCount, pHdr->numincludemodels);
	for (int i = 0; i < pHdr->numincludemodels; i++)
	{
		includeModels[i] = IModelModelGroup(pHdr->pModelGroup(i));
	}

	// collision
	const ivps::phyheader_t* const pPHYS = looseData.GetPHYS_VALVE();
	if (pPHYS)
	{
		physics = new IModelPhysics(this, pPHYS);
	}

	if (pHdr->collisionOffset)
	{
		mapCollision = new IModelMapCollision(this, pHdr);

		// discard as it is unused... no sure how to better check for this
		if (mapCollision->GetShapeCount() == 0)
		{
			FreeAllocVar(mapCollision);
			mapCollision = nullptr;
		}
	}

	if (pHdr->pPerTriHdr())
	{
		perTriAABB = new IModelPerTriAABB(this, pHdr, pHdr->pPerTriHdr());
	}
}

IModel::IModel(const std::filesystem::path path, const r5::studiohdr_t* const pHdr, const IModelSourceVersion_t version) :  sourcePath(path), source(nullptr), sourceFlags(IMODELFLAGS_NONE), sourceType(IModelSourceType_t::SOURCE_STUDIO),
	// general
	namedebug(), name(nullptr), eyeposition(pHdr->eyeposition), illumposition(pHdr->illumposition), hull_min(pHdr->hull_min), hull_max(pHdr->hull_max), view_bbmin(pHdr->view_bbmin), view_bbmax(pHdr->view_bbmax),
	surfaceProp(nullptr), keyValues(nullptr), mass(pHdr->mass), contents(pHdr->contents), constdirectionallightdot(pHdr->constdirectionallightdot), fadeDistance(pHdr->fadeDistance), gatherSize(0.0f),
	illumpositionattachmentindex(pHdr->illumpositionattachmentindex), flMaxEyeDeflection(0.0f),

	// bone
	bones(nullptr), boneCount(0u), procBones(nullptr), procBoneCount(0u), boneTransforms(nullptr), boneTransformCount(0u), boneFollowers(nullptr), boneFollowerCount(0u), hitboxSets(nullptr), hitboxSetCount(0u),
	localAttachments(nullptr), localAttachmentCount(0u),

	// mesh
	lods(nullptr), lodCount(0u), bodyparts(nullptr), bodypartCount(0u), rootLOD(pHdr->rootLOD), numAllowedRootLODs(pHdr->numAllowedRootLODs),
	uiPanels(nullptr), uiPanelCount(0u),
	
	// material
	textures(nullptr), textureCount(0u), cdTextures(nullptr), cdTexturesCount(0u), skins(nullptr), skinCount(0u),

	// animation
	animations(nullptr), animationCount(0u), sequences(nullptr), sequenceCount(0u), localNodeNames(nullptr), localNodeCount(0u), ikChains(nullptr), ikChainCount(0u),
	localPoseParameters(nullptr), localPoseParameterCount(0u), localIkAutoPlayLocks(nullptr), localIkAutoPlayLockCount(0u), includeModels(nullptr), includeModelCount(0u), animBlockName(nullptr), animBlocks(nullptr), animBlockCount(0u),

	// collision
	physics(nullptr), mapCollision(nullptr)
{
	// get loose files
	CManagedBuffer* looseBuf = g_BufferManager.ClaimBuffer();
	const StudioLooseData_t looseData(path, keepAfterLastSlashOrBackslash(pHdr->pszName()), looseBuf->Buffer(), managedBufferSize, pHdr->flags & (STUDIOHDR_FLAGS_USES_VERTEX_COLOR | STUDIOHDR_FLAGS_USES_UV2), nullptr);

	if (looseData.VerifyFileIntegrity(reinterpret_cast<const studiohdr_short_t* const>(pHdr)) == false)
	{
		sourceType = IModelSourceType_t::SOURCE_FAILED;
		Log("failed to parse source %s, skipping...\n", sourcePath.c_str());
		return;
	}

	// general
	memcpy_s(namedebug, 64, pHdr->name, 64);
	name = AllocStudioString(pHdr->pszName());
	surfaceProp = AllocStudioString(pHdr->pszSurfaceProp());
	
	if (pHdr->keyvaluesize)
	{
		keyValues = AllocStudioString(pHdr->KeyValueText(), static_cast<size_t>(pHdr->keyvaluesize));
	}

	source.pStudioSource = new studiosource_t(pHdr, version);

	// TODO r5 flags
	assertm(source.pStudioSource->version_minor == 8, "not supported");
	SetFlags_R5(pHdr->flags); // set our flags

	// [rika]: a lot of this can be threaded in the future

	// bones (bones, attachments, hitboxes, etc)
	IMODEL_ALLOC_DATA_LIMIT(IModelBone, bones, boneCount, pHdr->numbones, MAXSTUDIOBONES);
	IMODEL_ALLOC_DATA_LIMIT(IModelProcBone*, procBones, procBoneCount, pHdr->procBoneCount, MAXSTUDIOBONES);
	for (uint32_t i = 0u, procIdx = 0u; i < boneCount; i++)
	{
		bones[i] = IModelBone(pHdr->pBone(static_cast<int>(i)));

		const IModelBone* const bone = GetBone(i);
		if (bone->GetProcType() == eProcBoneType::NONE)
		{
			continue;
		}

		procBones[procIdx] = bone->GetProcBone<IModelProcBone>();
		procIdx++;
	}

	IMODEL_ALLOC_DATA_LIMIT(IModelBoneTransform, boneTransforms, boneTransformCount, pHdr->numsrcbonetransform, MAXSTUDIOBONES);
	for (int i = 0; i < pHdr->numsrcbonetransform; i++)
	{
		boneTransforms[i] = IModelBoneTransform(pHdr->SrcBoneTransform(i));
	}

	IMODEL_ALLOC_DATA_LIMIT(int, boneFollowers, boneFollowerCount, pHdr->boneFollowerCount, MAXSTUDIOBONES);
	for (int i = 0u; i < pHdr->boneFollowerCount; i++)
	{
		boneFollowers[i] = pHdr->BoneFollower(i);
	}

	IMODEL_ALLOC_DATA(IModelHitboxSet, hitboxSets, hitboxSetCount, pHdr->numhitboxsets);
	for (int i = 0; i < pHdr->numhitboxsets; i++)
	{
		hitboxSets[i] = IModelHitboxSet(pHdr->pHitboxSet(i));
	}

	IMODEL_ALLOC_DATA(IModelAttachment, localAttachments, localAttachmentCount, pHdr->numlocalattachments);
	for (int i = 0; i < pHdr->numlocalattachments; i++)
	{
		localAttachments[i] = IModelAttachment(pHdr->pLocalAttachment(i));
	}

	// meshes (bodyparts, lods, models, etc)
	if (pHdr->numbodyparts)
	{
		const OptimizedModel::FileHeader_t* const pVTX = looseData.GetVTX();
		const vvd::vertexFileHeader_t* const pVVD = looseData.GetVVD();

		assertm(pVTX && pVVD, "missing vertex files");
		assertm(pVTX->numLODs == pVVD->numLODs, "number of lods across files mismatched");

		IMODEL_ALLOC_DATA(IModelBodypart, bodyparts, bodypartCount, pHdr->numbodyparts);
		uint16_t modelCount = 0u;
		for (uint32_t i = 0; i < bodypartCount; i++)
		{
			bodyparts[i] = IModelBodypart(pHdr->pBodypart(static_cast<int>(i)), modelCount);
		}

		assertm(source.pStudioSource->version_minor == 8, "unsupported"); // handle hwData checkin IModelLOD()

		IMODEL_ALLOC_DATA_LIMIT(IModelLOD, lods, lodCount, pVTX->numLODs, MAX_NUM_LODS);
		for (uint32_t i = 0; i < lodCount; i++)
		{
			lods[i] = IModelLOD(i, modelCount, sourceFlags, pHdr, &looseData);
		}
	}

	if (pHdr->uiPanelCount)
	{
		IMODEL_ALLOC_DATA(IModelUIPanel, uiPanels, uiPanelCount, pHdr->uiPanelCount);

		for (uint32_t i = 0u; i < uiPanelCount; i++)
		{
			uiPanels[i] = IModelUIPanel(this, pHdr->pUiPanel(static_cast<int>(i)));
		}
	}

	// materials (textures, skins, etc..)
	if (pHdr->numtextures)
	{
		assertm(pHdr->numtextures == pHdr->numskinref, "these should always match");

		IMODEL_ALLOC_DATA_LIMIT(IModelTexture, textures, textureCount, pHdr->numtextures, MAXSTUDIOSKINS);
		for (int i = 0; i < pHdr->numtextures; i++)
		{
			textures[i] = IModelTexture(pHdr->pTexture(i), pHdr->pMatShaderType(i));
		}

		IMODEL_ALLOC_DATA_LIMIT(IModelSkin, skins, skinCount, pHdr->numskinfamilies, MAXSTUDIOSKINS);
		for (int i = 0; i < pHdr->numskinfamilies; i++)
		{
			skins[i] = IModelSkin(pHdr->pSkinFamily(i), pHdr->numskinref);
		}
	}

	IMODEL_ALLOC_DATA(char*, cdTextures, cdTexturesCount, pHdr->numcdtextures);
	for (int i = 0; i < pHdr->numcdtextures; i++)
	{
		cdTextures[i] = AllocStudioString(pHdr->pCdtexture(i));
	}

	// animations (animations, sequences, nodes, ik, etc..)
	//if (pHdr->numlocalanim || pHdr->numlocalseq)
	//{
	//	IMODEL_ALLOC_DATA_LIMIT(IModelAnimation, animations, animationCount, pHdr->numlocalanim, MAXSTUDIOANIMS);
	//	for (int i = 0; i < pHdr->numlocalanim; i++)
	//	{
	//		animations[i] = IModelAnimation(pHdr, pHdr->pAnimdesc(i));
	//	}

	//	IMODEL_ALLOC_DATA_LIMIT(IModelSequence, sequences, sequenceCount, pHdr->numlocalseq, MAXSTUDIOSEQUENCES);
	//	for (int i = 0; i < pHdr->numlocalseq; i++)
	//	{
	//		sequences[i] = IModelSequence(pHdr, pHdr->pSeqdesc(i));
	//	}
	//}

	//// todo transitions
	//IMODEL_ALLOC_DATA(char*, localNodeNames, localNodeCount, pHdr->numlocalnodes);
	//for (int i = 0; i < pHdr->numlocalnodes; i++)
	//{
	//	localNodeNames[i] = AllocStudioString(pHdr->pszLocalNodeName(i));
	//}

	//IMODEL_ALLOC_DATA(IModelIKChain, ikChains, ikChainCount, pHdr->numikchains);
	//for (int i = 0; i < pHdr->numikchains; i++)
	//{
	//	ikChains[i] = IModelIKChain(pHdr->pIKChain(i));
	//}

	//IMODEL_ALLOC_DATA(IModelPoseParameter, localPoseParameters, localPoseParameterCount, pHdr->numlocalposeparameters);
	//for (int i = 0; i < pHdr->numlocalposeparameters; i++)
	//{
	//	localPoseParameters[i] = IModelPoseParameter(pHdr->pLocalPoseParameter(i));
	//}

	//IMODEL_ALLOC_DATA(IModelIKLock, localIkAutoPlayLocks, localIkAutoPlayLockCount, pHdr->numlocalikautoplaylocks);
	//for (int i = 0; i < pHdr->numlocalikautoplaylocks; i++)
	//{
	//	localIkAutoPlayLocks[i] = IModelIKLock(pHdr->pLocalIKAutoplayLock(i));
	//}

	//IMODEL_ALLOC_DATA(IModelModelGroup, includeModels, includeModelCount, pHdr->numincludemodels);
	//for (int i = 0; i < pHdr->numincludemodels; i++)
	//{
	//	includeModels[i] = IModelModelGroup(pHdr->pModelGroup(i));
	//}

	// collision
	// needs check for v10 edges
	//const irps::phyheader_t* const pPHYS = looseData.GetPHYS_RESPAWN();
	//if (pPHYS)
	//{
	//	physics = new IModelPhysics(this, pPHYS);
	//}

	//if (pHdr->collisionOffset)
	//{
	//	mapCollision = new IModelMapCollision(this, pHdr);

	//	// discard as it is unused... no sure how to better check for this
	//	if (mapCollision->GetShapeCount() == 0)
	//	{
	//		FreeAllocVar(mapCollision);
	//		mapCollision = nullptr;
	//	}
	//}

	g_BufferManager.RelieveBuffer(looseBuf);
}

IModel::~IModel()
{
	// whole lot of memory cleanup
	
	switch (sourceType)
	{
	case IModelSourceType_t::SOURCE_STUDIO:
	{
		FreeAllocVar(source.pStudioSource);
		break;
	}
	case IModelSourceType_t::SOURCE_QC:
	{
		FreeAllocVar(source.pQCSource);
		break;
	}
	case IModelSourceType_t::SOURCE_INVALID:
	{
		FreeAllocVar(source.pInvalid);
		break;
	}
	}

	// free parsed data
	FreeAllocArray(name);
	FreeAllocArray(surfaceProp);
	FreeAllocArray(keyValues);

	FreeAllocArray(bones);
	FreeAllocArray(procBones); // the data these pointers point to should be freed in the bone deconstructor
	FreeAllocArray(boneTransforms);
	FreeAllocArray(boneFollowers);
	FreeAllocArray(localAttachments);
	FreeAllocArray(hitboxSets);

	FreeAllocArray(lods);
	FreeAllocArray(bodyparts);
	FreeAllocArray(uiPanels);

	FreeAllocArray(textures);
	FreeAllocPtrArray(cdTextures, cdTexturesCount);
	FreeAllocArray(skins);

	FreeAllocArray(animations);
	FreeAllocArray(sequences);
	FreeAllocPtrArray(localNodeNames, localNodeCount);
	FreeAllocArray(ikChains);
	FreeAllocArray(localPoseParameters);
	FreeAllocArray(localIkAutoPlayLocks);
	FreeAllocArray(includeModels);
	FreeAllocArray(animBlockName);
	FreeAllocArray(animBlocks);

	FreeAllocVar(physics);
	FreeAllocVar(mapCollision);
	FreeAllocVar(perTriAABB);
}

// parse on import?
std::vector<uint32_t> IModel::GetBoneChildren(const uint32_t bone) const
{
	std::vector<uint32_t> children;
	children.reserve(boneCount - bone);

	for (uint32_t i = bone; i < boneCount; i++)
	{
		const int parent = GetBone(i)->GetParent();
		if (parent != static_cast<int>(bone))
			continue;

		children.push_back(i);
	}

	children.shrink_to_fit();
	return children;
}

// returns allocated memory
IModel* const IModel::LoadStudioModel(const std::filesystem::path path)
{
	const size_t fileSize = std::filesystem::file_size(path);
	char* fileBuf = new char[fileSize];

	std::ifstream file(path, std::ios::in | std::ios::binary);
	file.read(fileBuf, fileSize);

	const int version = GetStudioVersion(fileBuf);
	IModel* out = nullptr;

	switch (version)
	{
	case STUDIO_VERSION_TITANFALL:
	{
		const r1::studiohdr_t* const pStudioHdr = reinterpret_cast<const r1::studiohdr_t* const>(fileBuf);
		out = new IModel(path, pStudioHdr);

		break;
	}
	case STUDIO_VERSION_TITANFALL2:
	{
		const r2::studiohdr_t* const pStudioHdr = reinterpret_cast<const r2::studiohdr_t* const>(fileBuf);
		out = new IModel(path, pStudioHdr);

		break;
	}
	case STUDIO_VERSION_APEX_LEGENDS:
	{
		if (g_CmdSettings.version == nullptr)
		{
			Log("model requires '%s' command!\n", s_CommandArgs[eCmdArgs::CMD_VERSION]);
			break;
		}

		const IModelSourceVersion_t srcVer(g_CmdSettings.version);

		if (srcVer.major != 54)
		{
			Log("invalid usage of '%s' command, expected major version of %i and got %i...\n", s_CommandArgs[eCmdArgs::CMD_VERSION], STUDIO_VERSION_APEX_LEGENDS, srcVer.major);

			break;
		}

		switch (srcVer.minor)
		{
		case 8:
		case 9:
		case 10:
		case 12:
		{
			const r5::studiohdr_t* const pStudioHdr = reinterpret_cast<const r5::studiohdr_t* const>(fileBuf);
			out = new IModel(path, pStudioHdr, srcVer);

			break;
		}
		default:
		{
			break;
		}
		}

		break;
	}
	default:
		break;
	}

	FreeAllocArray(fileBuf); // be careful
	file.close();

	return out;
}


//
// Utils Functions
// 

// get a valid lod name from the provided name
// this model has an lod suffix, it will be replicated regardless of it uses lods or not
const char* const IModel_ParseLodName(const char* const source, const uint32_t lod, const bool hasLODs)
{
	char buffer[maxLODNameSize]{};
	strncpy_s(buffer, maxLODNameSize, source, maxModelNameSize);

	const char* tmp = keepAfterLastSlashOrBackslash(buffer);
	strncpy_s(buffer, maxLODNameSize, tmp, maxLODNameSize);

	removeExtension(buffer);

	char* _LOD = strstr(buffer, "_LOD0");
	char* _lod = strstr(buffer, "_lod0");

	if (_LOD || _lod)
	{
		char* suffix = _LOD ? _LOD : _lod;

		// isolate the suffix
		suffix[0] = '\0';
		suffix[4] = '\0';
		suffix++;

		snprintf(buffer, maxLODNameSize, lodNameFmt, buffer, suffix, lod);
	}
	// this model uses lod suffixes and does not have one
	else if(hasLODs)
	{
		snprintf(buffer, maxLODNameSize, lodNameFmt, buffer, "LOD", lod);
	}
	// don't do anything if it doesn't have a lod suffix, and does not need one

	return AllocStudioString(buffer);
}

const char* const IModel_ParseClampedName(const char* const bodypartName, const IModelLOD* const lod, const uint32_t lodModelIndex)
{
	assertm(lodModelIndex, "should not ever clamp vertices on the first model");

	const char* name = nullptr;
	for (int i = static_cast<int>(lodModelIndex); i >= 0; i--)
	{
		const IModelModel* const model = lod->GetModel(static_cast<uint16_t>(i));
		name = model->GetName();

		// model is blank
		if (name == nullptr)
			continue;

		// another clamped model before it, skip
		if (strstr(name, "clamped"))
			continue;

		break;
	}

	char buffer[maxLODNameSize];

	uint32_t clampIndex = 0u;
	sscanf_s(bodypartName, "clamped%u", &clampIndex);
	snprintf(buffer, maxLODNameSize, "%s_clamped%u", name, clampIndex);

	return AllocStudioString(buffer);
}

// parse name from mstudiomodel_t, returns allocated memory
const char* const IModel_ParseModelName(const char* const source, const char* const model, const char* const bodypartName, const bool hasValidNames, const IModelLOD* const lod, const uint32_t lodModelIndex, const bool hasLODs)
{
	bool validName = true;
	
	if (source[63])
	{
		validName = false;

		// check if any form of extension (even if cut off) is in the last four characters, realistically this should only be '.dmx'
		for (int i = 60; i < 64; i++)
		{
			if (validName)
			{
				break;
			}

			validName = source[i] == '.';
		}
	}

	if (source[0] && validName)
	{
		return IModel_ParseLodName(source, lod->GetLODLevel(), hasLODs);
	}

	if (strstr(bodypartName, "clamped") && validName)
	{
		return IModel_ParseClampedName(bodypartName, lod, lodModelIndex);
	}

#ifdef _DEBUG
	if (hasValidNames)
	{
		Log("model had a model (dmx) with truncated name, when a valid name was expected, it will fall back onto a completely auto generated name!\n");
	}
#endif // _DEBUG

	char name[maxLODNameSize];
	char buffer[maxLODNameSize];

	memcpy_s(name, sizeof(name), source, maxModelNameSize);
	name[maxModelNameSize] = '\0';

	const char* fileName = keepAfterLastSlashOrBackslash(name);
	const char* const underscore = strrchr(fileName, '_');
	const size_t length = maxModelNameSize - static_cast<size_t>(fileName - name);

	if (underscore == nullptr && length < 5)
	{
		strcpy_s(name, maxModelNameSize, keepAfterLastSlashOrBackslash(model));
		removeExtension(name);

		fileName = name;

		snprintf(buffer, maxLODNameSize, "%s_%s_%u_LOD%u", fileName, bodypartName, lodModelIndex, lod->GetLODLevel());
	}
	else
	{
		snprintf(buffer, maxLODNameSize, "%s__%s_%u_LOD%u", fileName, bodypartName, lodModelIndex, lod->GetLODLevel());
	}

	return AllocStudioString(buffer);
}