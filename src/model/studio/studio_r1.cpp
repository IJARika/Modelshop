#include <pch.h>

#include <model/studio/studio_r1.h>

// todo better solution that can be threaded
extern const char* s_CurrentAnimBlockModel;

namespace r1
{
	enum StudioFrameFlagsLegacy_t : uint8_t
	{
		STUDIO_FRAME_OLD_CONST_POS = 0x01, // Vector48 in constants
		STUDIO_FRAME_OLD_CONST_ROT = 0x02, // Quaternion48 in constants
		STUDIO_FRAME_OLD_ANIM_POS = 0x04, // Vector48 in framedata
		STUDIO_FRAME_OLD_ANIM_ROT = 0x08, // Quaternion48 in framedata
		STUDIO_FRAME_OLD_ANIM_POS2 = 0x10, // Vector in framedata
		STUDIO_FRAME_OLD_CONST_POS2 = 0x20, // Vector in constants
		STUDIO_FRAME_OLD_CONST_ROT2 = 0x40, // Quaternion48S in constants
		STUDIO_FRAME_OLD_ANIM_ROT2 = 0x80, // Quaternion48S in framedata

		STUDIO_FRAME_OLD_HAS_PACKED = (STUDIO_FRAME_OLD_CONST_POS | STUDIO_FRAME_OLD_CONST_ROT | STUDIO_FRAME_OLD_ANIM_POS | STUDIO_FRAME_OLD_ANIM_ROT),
		STUDIO_FRAME_OLD_HAS_FULL = (STUDIO_FRAME_OLD_CONST_POS2 | STUDIO_FRAME_OLD_CONST_ROT2 | STUDIO_FRAME_OLD_ANIM_POS2 | STUDIO_FRAME_OLD_ANIM_ROT2),
	};

	inline const char* const FixOldBoneFlags(const studiohdr_t* const pHdr, mstudio_frame_anim_t* const pFrameAnim)
	{
		pFrameAnim->fixedOldBoneflags = s_FrameAnimFixedFlags;
		uint8_t* const pBoneFlags = reinterpret_cast<uint8_t* const>(&pFrameAnim[1]);

		for (int i = 0; i < pHdr->numbones; i++)
		{
			const uint8_t oldBoneFlags = pBoneFlags[i];
			uint8_t newBoneFlags = 0u;

			if ((oldBoneFlags & STUDIO_FRAME_OLD_HAS_FULL) != 0)
			{
				// can't mix packed and unpacked
				if ((oldBoneFlags & STUDIO_FRAME_OLD_HAS_PACKED) != 0)
				{
					assertm(false, "mixed animation types"); // assuming there is an assert here in source because if we mark this as converted and don't complete converting Bad Stuff would happen
					return nullptr;
				}

				newBoneFlags = STUDIO_FRAME_FULLANIM;
			}

			if ((oldBoneFlags & (STUDIO_FRAME_OLD_CONST_POS | STUDIO_FRAME_OLD_CONST_POS2)) != 0)
			{
				newBoneFlags |= STUDIO_FRAME_RAWPOS;
			}

			if ((oldBoneFlags & (STUDIO_FRAME_OLD_CONST_ROT | STUDIO_FRAME_OLD_CONST_ROT2)) != 0)
			{
				newBoneFlags |= STUDIO_FRAME_RAWROT;
			}

			if ((oldBoneFlags & (STUDIO_FRAME_OLD_ANIM_POS | STUDIO_FRAME_OLD_ANIM_POS2)) != 0)
			{
				newBoneFlags |= STUDIO_FRAME_ANIMPOS;
			}

			if ((oldBoneFlags & (STUDIO_FRAME_OLD_ANIM_ROT | STUDIO_FRAME_OLD_ANIM_ROT2)) != 0)
			{
				newBoneFlags |= STUDIO_FRAME_ANIMROT;
			}

			pBoneFlags[i] = newBoneFlags;
		}

		return reinterpret_cast<const char* const>(pFrameAnim);
	}

	const char* const mstudioanimdesc_t::pAnimBlock(int block, int index) const
	{
		if (block == -1)
		{
			return nullptr;
		}
		if (block == 0)
		{
			const char* const panim = ((char*)this + index);

			// VERY ILLEGAL
			mstudio_frame_anim_t* const pFrameAnim = reinterpret_cast<mstudio_frame_anim_t* const>((char*)panim);
			if ((pStudiohdr()->flags & STUDIOHDR_FLAGS_LEGACY_MODEL) && (flags & STUDIO_FRAMEANIM) && pFrameAnim->fixedOldBoneflags != s_FrameAnimFixedFlags)
			{
				return FixOldBoneFlags(pStudiohdr(), pFrameAnim);
			}

			return panim;
		}

		const char* const pAnimBlock = pStudiohdr()->GetAnimBlock(block);
		if (pAnimBlock)
		{
			const char* const panim = ((char*)pAnimBlock + index);

			// VERY ILLEGAL
			mstudio_frame_anim_t* const pFrameAnim = reinterpret_cast<mstudio_frame_anim_t* const>((char*)panim);
			if ((pStudiohdr()->flags & STUDIOHDR_FLAGS_LEGACY_MODEL) && (flags & STUDIO_FRAMEANIM) && pFrameAnim->fixedOldBoneflags != s_FrameAnimFixedFlags)
			{
				return FixOldBoneFlags(pStudiohdr(), pFrameAnim);
			}

			return panim;
		}

		return nullptr;
	}

	const char* const mstudioanimdesc_t::pAnim(int* piFrame, float& flStall) const
	{
		const char* panim = nullptr;

		int block = animblock;
		int index = animindex;
		int section = 0;

		if (sectionframes)
		{
			if (numframes > sectionframes && *piFrame == numframes - 1)
			{
				// last frame on long anims is stored separately
				*piFrame = 0;
				section = (numframes / sectionframes) + 1;
			}
			else
			{
				section = *piFrame / sectionframes;
				*piFrame -= section * sectionframes;
			}

			const mstudioanimsections_t* const sections = pSection(section);
			block = sections->animblock;
			index = sections->animindex;
		}

		if (block == -1)
		{
			// model needs to be recompiled
			return nullptr;
		}

		panim = pAnimBlock(block, index);

		// force a preload on the next block
		if (sectionframes)
		{
			int count = (numframes / sectionframes) + 2;
			for (int i = section + 1; i < count; i++)
			{
				if (pSection(i)->animblock != block)
				{
					pAnimBlock(pSection(i)->animblock, pSection(i)->animindex);
					break;
				}
			}
		}

		if (panim == nullptr)
		{
			// back up until a previously loaded block is found
			while (--section >= 0)
			{
				block = pSection(section)->animblock;
				index = pSection(section)->animindex;
				panim = pAnimBlock(block, index);
				if (panim)
				{
					// set it to the last frame in the last valid section
					*piFrame = sectionframes - 1;
					break;
				}
			}
		}

		// try to guess a valid stall time interval (tuned for the X360)
		flStall = 0.0f;
		if (panim == nullptr && section <= 0)
		{
			//zeroframestalltime = Plat_FloatTime();
			flStall = 1.0f;
		}
		else if (panim != nullptr && zeroframestalltime != 0.0f)
		{
			float dt = Plat_FloatTime() - zeroframestalltime;
			if (dt >= 0.0)
			{
				flStall = SimpleSpline(clamp((0.200f - dt) * 5.0f, 0.0f, 1.0f));
			}

			if (flStall == 0.0f)
			{
				// disable stalltime
				//zeroframestalltime = 0.0f;
			}
		}

		return panim;
	}

	const char* const mstudioanimdesc_t::pAnim(int* piFrame) const
	{
		float flStall = 0.0f;
		return pAnim(piFrame, flStall);
	}

	const mstudioikrule_t* const mstudioanimdesc_t::pIKRule(const int i) const
	{
		if (numikrules == 0)
		{
			return nullptr;
		}

		if (ikruleindex)
		{
			return reinterpret_cast<const mstudioikrule_t* const>((char*)this + ikruleindex) + i;
		}

		if (animblock == 0)
		{
			return reinterpret_cast<const mstudioikrule_t* const>((char*)this + animblockikruleindex) + i;
		}

		const char* const pAnimBlock = pStudiohdr()->GetAnimBlock(animblock);
		if (pAnimBlock)
		{
			return reinterpret_cast<const mstudioikrule_t* const>(pAnimBlock + animblockikruleindex) + i;
		}

		return nullptr;
	}

	inline const char* const studiohdr_t::GetAnimBlock(const int i) const
	{
		if (s_CurrentAnimBlockModel == nullptr)
		{
			assertm(false, "required animblocks but none were set");
			return nullptr;
		}

#ifdef _DEBUG
		const studiohdr_t* const aniFile = reinterpret_cast<const studiohdr_t* const>(s_CurrentAnimBlockModel);
		assertm(aniFile->id == IDSTUDIOANIMGROUPHEADER, "invalid ani file");
#endif // _DEBUG

		const mstudioanimblock_t* const animBlock = pAnimBlock(i);
		return s_CurrentAnimBlockModel + animBlock->datastart;
	}
}