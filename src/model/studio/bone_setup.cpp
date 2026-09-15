#include <pch.h>

//#include <model/studio/studio.h>
#include <model/studio/bone_setup.h>

namespace r1
{
	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame rotation for a single bone
	//-----------------------------------------------------------------------------
	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1, float& v2)
	{
		if (!panimvalue)
		{
			v1 = v2 = 0;
			return;
		}

		// Avoids a crash reading off the end of the data
		// There is probably a better long-term solution; Ken is going to look into it.
		if ((panimvalue->num.total == 1) && (panimvalue->num.valid == 1))
		{
			v1 = v2 = panimvalue[1].value * scale;
			return;
		}

		int k = frame;

		// find the data list that has the frame
		while (panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += panimvalue->num.valid + 1;
			if (panimvalue->num.total == 0)
			{
				//assert(0); // running off the end of the animation stream is bad
				v1 = v2 = 0;
				return;
			}
		}
		if (panimvalue->num.valid > k)
		{
			// has valid animation data
			v1 = panimvalue[k + 1].value * scale;

			if (panimvalue->num.valid > k + 1)
			{
				// has valid animation blend data
				v2 = panimvalue[k + 2].value * scale;
			}
			else
			{
				if (panimvalue->num.total > k + 1)
				{
					// data repeats, no blend
					v2 = v1;
				}
				else
				{
					// pull blend from first data block in next list
					v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
				}
			}
		}
		else
		{
			// get last valid data block
			v1 = panimvalue[panimvalue->num.valid].value * scale;
			if (panimvalue->num.total > k + 1)
			{
				// data repeats, no blend
				v2 = v1;
			}
			else
			{
				// pull blend from first data block in next list
				v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
			}
		}
	}

	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1)
	{
		if (!panimvalue)
		{
			v1 = 0;
			return;
		}

		int k = frame;

		while (panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += panimvalue->num.valid + 1;
			if (panimvalue->num.total == 0)
			{
				//assert(0); // running off the end of the animation stream is bad
				v1 = 0;
				return;
			}
		}
		if (panimvalue->num.valid > k)
		{
			v1 = panimvalue[k + 1].value * scale;
		}
		else
		{
			// get last valid data block
			v1 = panimvalue[panimvalue->num.valid].value * scale;
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame rotation for a single bone
	//-----------------------------------------------------------------------------
	void CalcBoneQuaternion(int frame, float s, const Quaternion& baseQuat, const RadianEuler& baseRot, const Vector& baseRotScale, int iBaseFlags, const Quaternion& baseAlignment, const mstudio_rle_anim_t* panim, Quaternion& q)
	{
		if (panim->flags & STUDIO_ANIM_RAWROT)
		{
			q = *(panim->pQuat48());
			assert(q.IsValid());
			return;
		}

		if (panim->flags & STUDIO_ANIM_RAWROT2)
		{
			q = *(panim->pQuat64());
			assert(q.IsValid());
			return;
		}

		if ((panim->flags & STUDIO_ANIM_ANIMROT) == false)
		{
			if (panim->flags & STUDIO_ANIM_DELTA)
			{
				q.Init(0.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				q = baseQuat;
			}
			return;
		}

		mstudioanim_valueptr_t* pValuesPtr = panim->pRotV();

		if (s > 0.001f)
		{
			Quaternion	q1, q2; // QuaternionAligned
			RadianEuler			angle1, angle2;

			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(0), baseRotScale.x, angle1.x, angle2.x);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(1), baseRotScale.y, angle1.y, angle2.y);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(2), baseRotScale.z, angle1.z, angle2.z);

			if (!(panim->flags & STUDIO_ANIM_DELTA))
			{
				angle1.x = angle1.x + baseRot.x;
				angle1.y = angle1.y + baseRot.y;
				angle1.z = angle1.z + baseRot.z;
				angle2.x = angle2.x + baseRot.x;
				angle2.y = angle2.y + baseRot.y;
				angle2.z = angle2.z + baseRot.z;
			}

			assert(angle1.IsValid() && angle2.IsValid());
			if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
			{
				AngleQuaternion(angle1, q1);
				AngleQuaternion(angle2, q2);

				QuaternionBlend(q1, q2, s, q);
			}
			else
			{
				AngleQuaternion(angle1, q);
			}
		}
		else
		{
			RadianEuler angle;

			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(0), baseRotScale.x, angle.x);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(1), baseRotScale.y, angle.y);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(2), baseRotScale.z, angle.z);

			if (!(panim->flags & STUDIO_ANIM_DELTA))
			{
				angle.x = angle.x + baseRot.x;
				angle.y = angle.y + baseRot.y;
				angle.z = angle.z + baseRot.z;
			}

			assert(angle.IsValid());
			AngleQuaternion(angle, q);
		}

		assert(q.IsValid());

		// align to unified bone
		if (!(panim->flags & STUDIO_ANIM_DELTA) && (iBaseFlags & BONE_FIXED_ALIGNMENT))
		{
			QuaternionAlign(baseAlignment, q, q);
		}
	}

	void CalcBoneQuaternion(int frame, float s, const mstudiobone_t* pBone, const mstudiolinearbone_t* pLinearBones, const mstudio_rle_anim_t* panim, Quaternion& q)
	{
		if (pLinearBones)
		{
			CalcBoneQuaternion(frame, s, *pLinearBones->pQuat(panim->bone), *pLinearBones->pRot(panim->bone), *pLinearBones->pRotScale(panim->bone), pLinearBones->flags(panim->bone), *pLinearBones->pQAlignment(panim->bone), panim, q);
		}
		else
		{
			CalcBoneQuaternion(frame, s, pBone->quat, pBone->rot, pBone->rotscale, pBone->flags, pBone->qAlignment, panim, q);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame position for a single bone
	//-----------------------------------------------------------------------------
	void CalcBonePosition(int frame, float s, const Vector& basePos, const Vector& baseBoneScale, const mstudio_rle_anim_t* panim, Vector& pos)
	{
		if (panim->flags & STUDIO_ANIM_RAWPOS)
		{
			pos = *(panim->pPos());
			assert(pos.IsValid());

			return;
		}

		if ((panim->flags & STUDIO_ANIM_ANIMPOS) == false)
		{
			if (panim->flags & STUDIO_ANIM_DELTA)
			{
				pos.Init(0.0f, 0.0f, 0.0f);
			}
			else
			{
				pos = basePos;
			}

			return;
		}

		mstudioanim_valueptr_t* pPosV = panim->pPosV();
		int j;

		if (s > 0.001f)
		{
			float v1, v2;
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pPosV->pAnimvalue(j), baseBoneScale[j], v1, v2);
				pos[j] = v1 * (1.0f - s) + v2 * s;
			}
		}
		else
		{
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pPosV->pAnimvalue(j), baseBoneScale[j], pos[j]);
			}
		}

		if ((panim->flags & STUDIO_ANIM_DELTA) == false)
		{
			pos.x = pos.x + basePos.x;
			pos.y = pos.y + basePos.y;
			pos.z = pos.z + basePos.z;
		}

		assert(pos.IsValid());
	}

	void CalcBonePosition(int frame, float s, const mstudiobone_t* pBone, const mstudiolinearbone_t* pLinearBones, const mstudio_rle_anim_t* panim, Vector& pos)
	{
		if (pLinearBones)
		{
			CalcBonePosition(frame, s, *pLinearBones->pPos(panim->bone), *pLinearBones->pPosScale(panim->bone), panim, pos);
		}
		else
		{
			CalcBonePosition(frame, s, pBone->pos, pBone->posscale, panim, pos);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame scale for a single bone
	//-----------------------------------------------------------------------------
	void CalcBoneScale(int frame, float s, const Vector& baseScale, const Vector& baseScaleScale, const mstudio_rle_anim_t* panim, Vector& scale)
	{
		if (panim->flags & STUDIO_ANIM_RAWSCALE)
		{
			scale = *(panim->pScale());
			assert(scale.IsValid());

			return;
		}

		if ((panim->flags & STUDIO_ANIM_ANIMSCALE) == false)
		{
			if (panim->flags & STUDIO_ANIM_DELTA)
			{
				scale = baseScale;
			}
			else
			{
				scale.Init(1.0f, 1.0f, 1.0f);
			}

			return;
		}

		mstudioanim_valueptr_t* pScaleV = panim->pScaleV();
		int j;

		if (s > 0.001f)
		{
			float v1, v2;
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pScaleV->pAnimvalue(j), baseScaleScale[j], v1, v2);
				scale[j] = v1 * (1.0f - s) + v2 * s;
			}
		}
		else
		{
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pScaleV->pAnimvalue(j), baseScaleScale[j], scale[j]);
			}
		}

		if (!(panim->flags & STUDIO_ANIM_DELTA))
		{
			scale.x = scale.x + baseScale.x;
			scale.y = scale.y + baseScale.y;
			scale.z = scale.z + baseScale.z;
		}

		assert(scale.IsValid());
	}

	void SkipBoneFrame(uint8_t flags, const uint8_t*& pConstData, const uint8_t*& pFrameData)
	{
		uint8_t boneFlagsConst = flags & 7; // mask to constant data flags

		flags >>= 3; // shift to access per frame flags.

		uint8_t boneFlagsFrame = flags & 7; // mask to per frame data flags
		uint8_t fullAnim = flags & 8; // use this as a base offset into the lut, everything starting at idx 8 is for full sized vectors.

		pConstData += s_FrameAnimBoneSizeLUT[fullAnim | boneFlagsConst];
		pFrameData += s_FrameAnimBoneSizeLUT[fullAnim | boneFlagsFrame];
	}

	const uint8_t* const ExtractSingleFrame(const uint8_t flags, const uint8_t* pFrameData, const uint8_t*& pConstantData, Quaternion& q, Vector& pos, Vector& scale, bool bIsDelta, const mstudiolinearbone_t* const pLinearBones, int bone)
	{
		if (flags & STUDIO_FRAME_FULLANIM)
		{
			// rotation
			if (flags & STUDIO_FRAME_ANIMROT)
			{
				q = *reinterpret_cast<const Quaternion48S* const>(pFrameData);
				assert(q.IsValid());
				pFrameData += sizeof(Quaternion48S);
			}
			else if (flags & STUDIO_FRAME_RAWROT)
			{
				q = *reinterpret_cast<const Quaternion48S* const>(pConstantData);
				assert(q.IsValid());
				pConstantData += sizeof(Quaternion48S);
			}
			else if (pLinearBones)
			{
				if (bIsDelta)
				{
					q.Init(0.0f, 0.0f, 0.0f, 1.0f);
				}
				else
				{
					q = *pLinearBones->pQuat(bone);
				}
			}

			// position
			if (flags & STUDIO_FRAME_ANIMPOS)
			{
				memcpy_s(&pos, sizeof(Vector), pFrameData, sizeof(Vector));
				assert(pos.IsValid());
				pFrameData += sizeof(Vector);
			}
			else if (flags & STUDIO_FRAME_RAWPOS)
			{
				memcpy_s(&pos, sizeof(Vector), pConstantData, sizeof(Vector));
				assert(pos.IsValid());
				pConstantData += sizeof(Vector);
			}
			else if (pLinearBones)
			{
				if (bIsDelta)
				{
					pos.Init(0.0f, 0.0f, 0.0f);
				}
				else
				{
					pos = *pLinearBones->pPos(bone);
				}
			}

			// scale
			if (flags & STUDIO_FRAME_ANIMSCALE)
			{
				memcpy_s(&scale, sizeof(Vector), pFrameData, sizeof(Vector));
				assert(scale.IsValid());
				pFrameData += sizeof(Vector);
			}
			else if (flags & STUDIO_FRAME_RAWSCALE)
			{
				memcpy_s(&scale, sizeof(Vector), pConstantData, sizeof(Vector));
				assert(scale.IsValid());
				pConstantData += sizeof(Vector);
			}
			else if (pLinearBones)
			{
				scale.Init(1.0f, 1.0f, 1.0f);
			}

			return pFrameData;
		}

		// rotation
		if (flags & STUDIO_FRAME_ANIMROT)
		{
			q = *reinterpret_cast<const Quaternion48* const>(pFrameData);
			assert(q.IsValid());
			pFrameData += sizeof(Quaternion48);
		}
		else if (flags & STUDIO_FRAME_RAWROT)
		{
			q = *reinterpret_cast<const Quaternion48* const>(pConstantData);
			assert(q.IsValid());
			pConstantData += sizeof(Quaternion48);
		}
		else if (pLinearBones)
		{
			if (bIsDelta)
			{
				q.Init(0.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				q = *pLinearBones->pQuat(bone);
			}
		}

		// position
		if (flags & STUDIO_FRAME_ANIMPOS)
		{
			pos = *reinterpret_cast<const Vector48* const>(pFrameData);
			assert(pos.IsValid());
			pFrameData += sizeof(Vector48);
		}
		else if (flags & STUDIO_FRAME_RAWPOS)
		{
			pos = *reinterpret_cast<const Vector48* const>(pConstantData);
			assert(pos.IsValid());
			pConstantData += sizeof(Vector48);
		}
		else if (pLinearBones)
		{
			if (bIsDelta)
			{
				pos.Init(0.0f, 0.0f, 0.0f);
			}
			else
			{
				pos = *pLinearBones->pPos(bone);
			}
		}

		// scale
		if (flags & STUDIO_FRAME_ANIMSCALE)
		{
			scale = *reinterpret_cast<const Vector48* const>(pFrameData);
			assert(scale.IsValid());
			pFrameData += sizeof(Vector48);
		}
		else if (flags & STUDIO_FRAME_RAWSCALE)
		{
			scale = *reinterpret_cast<const Vector48* const>(pConstantData);
			assert(scale.IsValid());
			pConstantData += sizeof(Vector48);
		}
		else if (pLinearBones)
		{
			scale.Init(1.0f, 1.0f, 1.0f);
		}

		return pFrameData;
	}

	const uint8_t* const ExtractTwoFrames(const uint8_t flags, const float s, const uint8_t* pFrameData, const uint8_t*& pConstantData, const int framelength, Quaternion& q, Vector& pos, Vector& scale, bool bIsDelta, const mstudiolinearbone_t* const pLinearBones, int bone)
	{
		Quaternion q1, q2;
		Vector pos1, pos2;
		Vector scale1, scale2;

		if (flags & STUDIO_FRAME_FULLANIM)
		{
			// rotation
			if (flags & STUDIO_FRAME_ANIMROT)
			{
				q1 = *reinterpret_cast<const Quaternion48S* const>(pFrameData);
				q2 = *reinterpret_cast<const Quaternion48S* const>(pFrameData + framelength);

				QuaternionBlend(q1, q2, s, q);
				assert(q.IsValid());

				pFrameData += sizeof(Quaternion48S);
			}
			else if (flags & STUDIO_FRAME_RAWROT)
			{
				q = *reinterpret_cast<const Quaternion48S* const>(pConstantData);
				assert(q.IsValid());
				pConstantData += sizeof(Quaternion48S);
			}
			else if (pLinearBones)
			{
				if (bIsDelta)
				{
					q.Init(0.0f, 0.0f, 0.0f, 1.0f);
				}
				else
				{
					q = *pLinearBones->pQuat(bone);
				}
			}

			// position
			if (flags & STUDIO_FRAME_ANIMPOS)
			{
				memcpy_s(&pos1, sizeof(Vector), pFrameData, sizeof(Vector));
				memcpy_s(&pos2, sizeof(Vector), pFrameData + framelength, sizeof(Vector));

				//pos = (pos1 * (1.0f - s)) + (pos2 * s);
				pos = ((pos2 - pos1) * s) + pos1;
				assert(pos.IsValid());

				pFrameData += sizeof(Vector);
			}
			else if (flags & STUDIO_FRAME_RAWPOS)
			{
				memcpy_s(&pos, sizeof(Vector), pConstantData, sizeof(Vector));
				assert(pos.IsValid());
				pConstantData += sizeof(Vector);
			}
			else if (pLinearBones)
			{
				if (bIsDelta)
				{
					pos.Init(0.0f, 0.0f, 0.0f);
				}
				else
				{
					pos = *pLinearBones->pPos(bone);
				}
			}

			// scale
			if (flags & STUDIO_FRAME_ANIMSCALE)
			{
				memcpy_s(&scale1, sizeof(Vector), pFrameData, sizeof(Vector));
				memcpy_s(&scale2, sizeof(Vector), pFrameData + framelength, sizeof(Vector));

				//pos = (pos1 * (1.0f - s)) + (pos2 * s);
				scale = ((scale2 - scale1) * s) + scale1;
				assert(scale.IsValid());

				pFrameData += sizeof(Vector);
			}
			else if (flags & STUDIO_FRAME_RAWSCALE)
			{
				memcpy_s(&scale, sizeof(Vector), pConstantData, sizeof(Vector));
				assert(scale.IsValid());
				pConstantData += sizeof(Vector);
			}
			else if (pLinearBones)
			{
				scale.Init(1.0f, 1.0f, 1.0f);
			}

			return pFrameData;
		}

		// rotation
		if (flags & STUDIO_FRAME_ANIMROT)
		{
			q1 = *reinterpret_cast<const Quaternion48* const>(pFrameData);
			q2 = *reinterpret_cast<const Quaternion48* const>(pFrameData + framelength);

			QuaternionBlend(q1, q2, s, q);
			assert(q.IsValid());

			pFrameData += sizeof(Quaternion48);
		}
		else if (flags & STUDIO_FRAME_RAWROT)
		{
			q = *reinterpret_cast<const Quaternion48* const>(pConstantData);
			assert(q.IsValid());
			pConstantData += sizeof(Quaternion48);
		}
		else if (pLinearBones)
		{
			if (bIsDelta)
			{
				q.Init(0.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				q = *pLinearBones->pQuat(bone);
			}
		}

		// position
		if (flags & STUDIO_FRAME_ANIMPOS)
		{
			pos1 = *reinterpret_cast<const Vector48* const>(pFrameData);
			pos2 = *reinterpret_cast<const Vector48* const>(pFrameData + framelength);

			//pos = (pos1 * (1.0f - s)) + (pos2 * s);
			pos = ((pos2 - pos1) * s) + pos1;
			assert(pos.IsValid());

			pFrameData += sizeof(Vector48);
		}
		else if (flags & STUDIO_FRAME_RAWPOS)
		{
			pos = *reinterpret_cast<const Vector48* const>(pConstantData);
			assert(pos.IsValid());
			pConstantData += sizeof(Vector48);
		}
		else if (pLinearBones)
		{
			if (bIsDelta)
			{
				pos.Init(0.0f, 0.0f, 0.0f);
			}
			else
			{
				pos = *pLinearBones->pPos(bone);
			}
		}

		// scale
		if (flags & STUDIO_FRAME_ANIMSCALE)
		{
			scale1 = *reinterpret_cast<const Vector48* const>(pFrameData);
			scale2 = *reinterpret_cast<const Vector48* const>(pFrameData + framelength);

			//pos = (pos1 * (1.0f - s)) + (pos2 * s);
			scale = ((scale2 - scale1) * s) + scale1;
			assert(scale.IsValid());

			pFrameData += sizeof(Vector48);
		}
		else if (flags & STUDIO_FRAME_RAWSCALE)
		{
			scale = *reinterpret_cast<const Vector48* const>(pConstantData);
			assert(scale.IsValid());
			pConstantData += sizeof(Vector48);
		}
		else if (pLinearBones)
		{
			scale.Init(1.0f, 1.0f, 1.0f);
		}

		return pFrameData;
	}


	//-----------------------------------------------------------------------------
	// Purpose: calculate changes in position and angle relative to the start of an animations cycle
	// Output:	updated position and angle, relative to the origin
	//			returns false if animation is not a movement animation
	//-----------------------------------------------------------------------------
	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int frame, Vector& vecPos, float& yaw)
	{
		for (int i = 0; i < 3; i++)
		{
			ExtractAnimValue(frame, pFrameMovement->pAnimvalue(i), pFrameMovement->scale[i], vecPos[i]);
		}

		ExtractAnimValue(frame, pFrameMovement->pAnimvalue(3), pFrameMovement->scale[3], yaw);
	}

	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int iFrame, Vector& v1Pos, Vector& v2Pos, float& v1Yaw, float& v2Yaw)
	{
		for (int i = 0; i < 3; i++)
		{
			ExtractAnimValue(iFrame, pFrameMovement->pAnimvalue(i), pFrameMovement->scale[i], v1Pos[i], v2Pos[i]);
		}

		ExtractAnimValue(iFrame, pFrameMovement->pAnimvalue(3), pFrameMovement->scale[3], v1Yaw, v2Yaw);
	}

	bool Studio_AnimPosition(const mstudioanimdesc_t* const panim, float flCycle, Vector& vecPos, QAngle& vecAngle)
	{
		float	prevframe = 0;
		vecPos.Init();
		vecAngle.Init();

		int iLoops = 0;
		if (flCycle > 1.0)
		{
			iLoops = (int)flCycle;
		}
		else if (flCycle < 0.0)
		{
			iLoops = (int)flCycle - 1;
		}
		flCycle = flCycle - iLoops;

		float	flFrame = flCycle * (panim->numframes - 1);

		if (panim->flags & STUDIO_FRAMEMOVEMENT)
		{
			int iFrame = (int)flFrame;
			float s = (flFrame - iFrame);

			if (s == 0)
			{
				Studio_FrameMovement(panim->pFrameMovement(), iFrame, vecPos, vecAngle.y);
				return true;
			}
			else
			{
				Vector v1Pos, v2Pos;
				float v1Yaw, v2Yaw;

				Studio_FrameMovement(panim->pFrameMovement(), iFrame, v1Pos, v2Pos, v1Yaw, v2Yaw);

				vecPos = ((v2Pos - v1Pos) * s) + v1Pos; // this should work on paper?

				float yawDelta = AngleDiff(v2Yaw, v1Yaw);
				vecAngle.y = (yawDelta * s) + v1Yaw;

				return true;
			}
		}
		else
		{
			for (int i = 0; i < panim->nummovements; i++)
			{
				const mstudiomovement_t* pmove = panim->pMovement(i);

				if (pmove->endframe >= flFrame)
				{
					float f = (flFrame - prevframe) / (pmove->endframe - prevframe);

					float d = pmove->v0 * f + 0.5f * (pmove->v1 - pmove->v0) * f * f;

					vecPos = vecPos + (pmove->vector * d);
					vecAngle.y = vecAngle.y * (1 - f) + pmove->angle * f;
					if (iLoops != 0)
					{
						const mstudiomovement_t* pmove_1 = panim->pMovement(panim->nummovements - 1);
						vecPos = vecPos + (pmove_1->position * iLoops);
						vecAngle.y = vecAngle.y + iLoops * pmove_1->angle;
					}
					return true;
				}
				else
				{
					prevframe = static_cast<float>(pmove->endframe);
					vecPos = pmove->position;
					vecAngle.y = pmove->angle;
				}
			}
		}

		return false;
	}
}

namespace r2
{
	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame rotation for a single bone
	//-----------------------------------------------------------------------------
	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1, float& v2)
	{
		if (!panimvalue)
		{
			v1 = v2 = 0;
			return;
		}

		// Avoids a crash reading off the end of the data
		// There is probably a better long-term solution; Ken is going to look into it.
		if ((panimvalue->num.total == 1) && (panimvalue->num.valid == 1))
		{
			v1 = v2 = panimvalue[1].value * scale;
			return;
		}

		int k = frame;

		// find the data list that has the frame
		while (panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += panimvalue->num.valid + 1;
			if (panimvalue->num.total == 0)
			{
				//assert(0); // running off the end of the animation stream is bad
				v1 = v2 = 0;
				return;
			}
		}
		if (panimvalue->num.valid > k)
		{
			// has valid animation data
			v1 = panimvalue[k + 1].value * scale;

			if (panimvalue->num.valid > k + 1)
			{
				// has valid animation blend data
				v2 = panimvalue[k + 2].value * scale;
			}
			else
			{
				if (panimvalue->num.total > k + 1)
				{
					// data repeats, no blend
					v2 = v1;
				}
				else
				{
					// pull blend from first data block in next list
					v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
				}
			}
		}
		else
		{
			// get last valid data block
			v1 = panimvalue[panimvalue->num.valid].value * scale;
			if (panimvalue->num.total > k + 1)
			{
				// data repeats, no blend
				v2 = v1;
			}
			else
			{
				// pull blend from first data block in next list
				v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
			}
		}
	}

	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1)
	{
		if (!panimvalue)
		{
			v1 = 0;
			return;
		}

		int k = frame;

		while (panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += panimvalue->num.valid + 1;
			if (panimvalue->num.total == 0)
			{
				//assert(0); // running off the end of the animation stream is bad
				v1 = 0;
				return;
			}
		}
		if (panimvalue->num.valid > k)
		{
			v1 = panimvalue[k + 1].value * scale;
		}
		else
		{
			// get last valid data block
			v1 = panimvalue[panimvalue->num.valid].value * scale;
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame rotation for a single bone
	//-----------------------------------------------------------------------------
	void CalcBoneQuaternion(int frame, float s, const Quaternion& baseQuat, const RadianEuler& baseRot, const Vector& baseRotScale, int iBaseFlags, const Quaternion& baseAlignment, const mstudio_rle_anim_t* panim, Quaternion& q)
	{
		if (panim->flags & STUDIO_ANIM_RAWROT)
		{
			q = *(panim->pQuat64());
			assert(q.IsValid());
			return;
		}

		if (panim->flags & STUDIO_ANIM_NOROT)
		{
			if (panim->flags & STUDIO_ANIM_DELTA)
			{
				q.Init(0.0f, 0.0f, 0.0f, 1.0f);
			}
			else
			{
				q = baseQuat;
			}
			return;
		}

		mstudioanim_valueptr_t* pValuesPtr = panim->pRotV();

		if (s > 0.001f)
		{
			Quaternion q1, q2; // QuaternionAligned
			RadianEuler angle1, angle2;

			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(0), baseRotScale.x, angle1.x, angle2.x);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(1), baseRotScale.y, angle1.y, angle2.y);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(2), baseRotScale.z, angle1.z, angle2.z);

			if (!(panim->flags & STUDIO_ANIM_DELTA))
			{
				angle1.x = angle1.x + baseRot.x;
				angle1.y = angle1.y + baseRot.y;
				angle1.z = angle1.z + baseRot.z;
				angle2.x = angle2.x + baseRot.x;
				angle2.y = angle2.y + baseRot.y;
				angle2.z = angle2.z + baseRot.z;
			}

			assert(angle1.IsValid() && angle2.IsValid());
			if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
			{
				AngleQuaternion(angle1, q1);
				AngleQuaternion(angle2, q2);

				QuaternionBlend(q1, q2, s, q);
			}
			else
			{
				AngleQuaternion(angle1, q);
			}
		}
		else
		{
			RadianEuler angle;

			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(0), baseRotScale.x, angle.x);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(1), baseRotScale.y, angle.y);
			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(2), baseRotScale.z, angle.z);

			if (!(panim->flags & STUDIO_ANIM_DELTA))
			{
				angle.x = angle.x + baseRot.x;
				angle.y = angle.y + baseRot.y;
				angle.z = angle.z + baseRot.z;
			}

			assert(angle.IsValid());
			AngleQuaternion(angle, q);
		}

		assert(q.IsValid());

		// align to unified bone
		if (!(panim->flags & STUDIO_ANIM_DELTA) && (iBaseFlags & BONE_FIXED_ALIGNMENT))
		{
			QuaternionAlign(baseAlignment, q, q);
		}
	}

	void CalcBoneQuaternion(int frame, float s, const mstudiobone_t* pBone, const mstudiolinearbone_t* pLinearBones, const mstudio_rle_anim_t* panim, Quaternion& q)
	{
		if (pLinearBones)
		{
			CalcBoneQuaternion(frame, s, *pLinearBones->pQuat(panim->bone), *pLinearBones->pRot(panim->bone), *pLinearBones->pRotScale(panim->bone), pLinearBones->flags(panim->bone), *pLinearBones->pQAlignment(panim->bone), panim, q);
		}
		else
		{
			CalcBoneQuaternion(frame, s, pBone->quat, pBone->rot, pBone->rotscale, pBone->flags, pBone->qAlignment, panim, q);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame position for a single bone
	//-----------------------------------------------------------------------------
	void CalcBonePosition(int frame, float s, const Vector& basePos, const float &baseBoneScale, const mstudio_rle_anim_t* panim, Vector& pos)
	{
		if (panim->flags & STUDIO_ANIM_RAWPOS)
		{
			pos = *(panim->pPos());
			assert(pos.IsValid());

			return;
		}

		mstudioanim_valueptr_t* pPosV = panim->pPosV();
		int j;

		if (s > 0.001f)
		{
			float v1, v2;
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pPosV->pAnimvalue(j), baseBoneScale, v1, v2);
				pos[j] = v1 * (1.0f - s) + v2 * s;
			}
		}
		else
		{
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pPosV->pAnimvalue(j), baseBoneScale, pos[j]);
			}
		}

		if (!(panim->flags & STUDIO_ANIM_DELTA))
		{
			pos.x = pos.x + basePos.x;
			pos.y = pos.y + basePos.y;
			pos.z = pos.z + basePos.z;
		}

		assert(pos.IsValid());
	}

	void CalcBonePosition(int frame, float s, const mstudiobone_t* pBone, const mstudiolinearbone_t* pLinearBones, const mstudio_rle_anim_t* panim, Vector& pos)
	{
		if (pLinearBones)
		{
			CalcBonePosition(frame, s, *pLinearBones->pPos(panim->bone), panim->posscale, panim, pos);
		}
		else
		{
			CalcBonePosition(frame, s, pBone->pos, panim->posscale, panim, pos);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame scale for a single bone
	//-----------------------------------------------------------------------------
	void CalcBoneScale(int frame, float s, const Vector& baseScale, const Vector& baseScaleScale, const mstudio_rle_anim_t* panim, Vector& scale)
	{
		if (panim->flags & STUDIO_ANIM_RAWSCALE)
		{
			scale = *(panim->pScale());
			assert(scale.IsValid());

			return;
		}

		mstudioanim_valueptr_t* pScaleV = panim->pScaleV();
		int j;

		if (s > 0.001f)
		{
			float v1, v2;
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pScaleV->pAnimvalue(j), baseScaleScale[j], v1, v2);
				scale[j] = v1 * (1.0f - s) + v2 * s;
			}
		}
		else
		{
			for (j = 0; j < 3; j++)
			{
				ExtractAnimValue(frame, pScaleV->pAnimvalue(j), baseScaleScale[j], scale[j]);
			}
		}

		if (!(panim->flags & STUDIO_ANIM_DELTA))
		{
			scale.x = scale.x + baseScale.x;
			scale.y = scale.y + baseScale.y;
			scale.z = scale.z + baseScale.z;
		}

		assert(scale.IsValid());
	}

	//-----------------------------------------------------------------------------
	// Purpose: calculate changes in position and angle relative to the start of an animations cycle
	// Output:	updated position and angle, relative to the origin
	//			returns false if animation is not a movement animation
	//-----------------------------------------------------------------------------
	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int frame, Vector& vecPos, float& yaw)
	{
		for (int i = 0; i < 3; i++)
		{
			ExtractAnimValue(frame, pFrameMovement->pAnimvalue(i), pFrameMovement->scale[i], vecPos[i]);
		}

		ExtractAnimValue(frame, pFrameMovement->pAnimvalue(3), pFrameMovement->scale[3], yaw);
	}

	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int iFrame, Vector& v1Pos, Vector& v2Pos, float& v1Yaw, float& v2Yaw)
	{
		for (int i = 0; i < 3; i++)
		{
			ExtractAnimValue(iFrame, pFrameMovement->pAnimvalue(i), pFrameMovement->scale[i], v1Pos[i], v2Pos[i]);
		}

		ExtractAnimValue(iFrame, pFrameMovement->pAnimvalue(3), pFrameMovement->scale[3], v1Yaw, v2Yaw);
	}

	bool Studio_AnimPosition(const mstudioanimdesc_t* const panim, float flCycle, Vector& vecPos, QAngle& vecAngle)
	{
		float	prevframe = 0;
		vecPos.Init();
		vecAngle.Init();

		int iLoops = 0;
		if (flCycle > 1.0)
		{
			iLoops = (int)flCycle;
		}
		else if (flCycle < 0.0)
		{
			iLoops = (int)flCycle - 1;
		}
		flCycle = flCycle - iLoops;

		float	flFrame = flCycle * (panim->numframes - 1);

		if (panim->flags & STUDIO_FRAMEMOVEMENT)
		{
			int iFrame = (int)flFrame;
			float s = (flFrame - iFrame);

			if (s == 0)
			{
				Studio_FrameMovement(panim->pFrameMovement(), iFrame, vecPos, vecAngle.y);
				return true;
			}
			else
			{
				Vector v1Pos, v2Pos;
				float v1Yaw, v2Yaw;

				Studio_FrameMovement(panim->pFrameMovement(), iFrame, v1Pos, v2Pos, v1Yaw, v2Yaw);

				vecPos = ((v2Pos - v1Pos) * s) + v1Pos; // this should work on paper?

				float yawDelta = AngleDiff(v2Yaw, v1Yaw);
				vecAngle.y = (yawDelta * s) + v1Yaw;

				return true;
			}
		}
		else
		{
			for (int i = 0; i < panim->nummovements; i++)
			{
				const mstudiomovement_t* pmove = panim->pMovement(i);

				if (pmove->endframe >= flFrame)
				{
					float f = (flFrame - prevframe) / (pmove->endframe - prevframe);

					float d = pmove->v0 * f + 0.5f * (pmove->v1 - pmove->v0) * f * f;

					vecPos = vecPos + (pmove->vector * d);
					vecAngle.y = vecAngle.y * (1 - f) + pmove->angle * f;
					if (iLoops != 0)
					{
						const mstudiomovement_t* pmove_1 = panim->pMovement(panim->nummovements - 1);
						vecPos = vecPos + (pmove_1->position * iLoops);
						vecAngle.y = vecAngle.y + iLoops * pmove_1->angle;
					}
					return true;
				}
				else
				{
					prevframe = static_cast<float>(pmove->endframe);
					vecPos = pmove->position;
					vecAngle.y = pmove->angle;
				}
			}
		}

		return false;
	}

	bool Studio_AnimMovement(const mstudioanimdesc_t* const panim, float flCycleFrom, float flCycleTo, Vector& deltaPos, QAngle& deltaAngle)
	{
		if (panim->nummovements == 0 || (panim->flags & STUDIO_FRAMEMOVEMENT) == 0)
			return false;

		Vector startPos;
		QAngle startA;
		Studio_AnimPosition(panim, flCycleFrom, startPos, startA);

		Vector endPos;
		QAngle endA;
		Studio_AnimPosition(panim, flCycleTo, endPos, endA);

		Vector tmp = endPos - startPos;
		deltaAngle.y = AngleDiff(endA.y, startA.y);
		VectorYawRotate(tmp, -startA.y, deltaPos);

		return true;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Scale a set of bones.  Must be of type delta
	//-----------------------------------------------------------------------------
	//void ScaleBones(const CStudioHdr* pStudioHdr, Quaternion q1[MAXSTUDIOBONES], Vector pos1[MAXSTUDIOBONES], Vector scale1[MAXSTUDIOBONES], int sequence, float s, int boneMask)
	//{
	//	// I think this is mostly the same, aside from the new flag. IDA just completely destroys it

	//	int			i, j;
	//	Quaternion		q3;

	//	const mstudioseqdesc_t* seqdesc = pStudioHdr->pSeqdesc(sequence);

	//	virtualmodel_t* pVModel = pStudioHdr->GetVirtualModel();
	//	const virtualgroup_t* pSeqGroup = NULL;
	//	if (pVModel)
	//	{
	//		//pSeqGroup = pVModel->pSeqGroup(sequence);
	//	}

	//	float s2 = s;
	//	float s1 = 1.0 - s2;

	//	for (i = 0; i < pStudioHdr->numbones(); i++)
	//	{
	//		// skip unused bones
	//		if (!(pStudioHdr->boneFlags(i) & boneMask))
	//		{
	//			continue;
	//		}

	//		if (pSeqGroup)
	//		{
	//			//j = pSeqGroup->boneMap[i];
	//		}
	//		else
	//		{
	//			j = i;
	//		}

	//		if (j >= 0 && seqdesc->weight(j) > 0.0)
	//		{
	//			if (seqdesc->flags & STUDIO_SCALEUSED)
	//			{
	//				QuaternionIdentityBlend(q1[i], s1, q1[i]);
	//				VectorScale(pos1[i], s2, pos1[i]);

	//				for (int axis = 0; axis < 3; axis++)
	//				{
	//					// if it's not default scale, adjust
	//					if (scale1[i][axis] != 1.0f)
	//					{
	//						scale1[i][axis] = std::powf(scale1[i][axis], s);
	//					}
	//				}
	//			}
	//			else
	//			{
	//				QuaternionIdentityBlend(q1[i], s1, q1[i]);
	//				VectorScale(pos1[i], s2, pos1[i]);
	//			}
	//		}
	//	}
	//}

	// this hasn't changed as far as I can tell
	/*bool Studio_SeqMovement(const CStudioHdr* pStudioHdr, int iSequence, float flCycleFrom, float flCycleTo, const float poseParameter[], Vector& deltaPos, QAngle& deltaAngles)
	{
		mstudioanimdesc_t* panim[4];
		float	weight[4];

		mstudioseqdesc_t& seqdesc = ((CStudioHdr*)pStudioHdr)->pSeqdesc(iSequence);

		Studio_SeqAnims(pStudioHdr, seqdesc, iSequence, poseParameter, panim, weight);

		deltaPos.Init();
		deltaAngles.Init();

		bool found = false;

		for (int i = 0; i < 4; i++)
		{
			if (weight[i])
			{
				Vector localPos;
				QAngle localAngles;

				localPos.Init();
				localAngles.Init();

				if (Studio_AnimMovement(panim[i], flCycleFrom, flCycleTo, localPos, localAngles))
				{
					found = true;
					deltaPos = deltaPos + localPos * weight[i];
					// FIXME: this makes no sense
					deltaAngles = deltaAngles + localAngles * weight[i];
				}
				else if (!(panim[i]->flags & STUDIO_DELTA) && panim[i]->nummovements == 0 && seqdesc.weight(0) > 0.0)
				{
					found = true;
				}
			}
		}
		return found;
	}*/


}

namespace r5
{
	static char s_FrameBitCountLUT[4]{ 0, 2, 4, 0 }; // 0x1412FC154i64 (12FC154h)
	static float s_FrameValOffsetLUT[4]{ 0.0f, 3.0f, 15.0f, 0.0f }; // dword_1412FED60 (12FED60) (1 << s_FrameBitCountLUT[i] ==  s_FrameValOffsetLUT[i])
	static char s_AnimSeekLUT[60]
	{
		1,  15, 16, 2,  7,  8,  2,  15, 0,  3,  15, 0,  4,  15, 0,  5,
		15, 0,  6,  15, 0,  7,  15, 0,  2,  15, 2,  3,  15, 2,  4,  15,
		2,  5,  15, 2,  6,  15, 2,  7,  15, 2,  2,  15, 4,  3,  15, 4,
		4,  15, 4, 5, 15, 4, 6, 15, 4,  7,  15, 4
	};

	// this LUT will Just Work within reason, no real need to fashion values specially
	//#define GetAnimValueOffset(panimvalue) (s_AnimSeekLUT[3 * panimvalue->num.valid] + ((s_AnimSeekLUT[3 * panimvalue->num.valid + 1] + panimvalue->num.total * s_AnimSeekLUT[3 * panimvalue->num.valid + 2]) / 16)) // literal magic
	#define FIXED_ROT_SCALE 0.00019175345f // rotation range -360 (-2pi rad) to 360 (2pi rad)
	#define FIXED_SCALE_SCALE 0.0030518509f // scale range 0 to 100

	// just some tinkering, these give slightly different results but are within the margin of error (percision loss)
	#define ANIM_VALUE_MAX 0x7fff

	#define ANIM_SCALE_MIN 0.0f
	#define ANIM_SCALE_MAX 100.0f

	#define ANIM_RADIAN_MIN -(2 * M_PI)
	#define ANIM_RADIAN_MAX (2 * M_PI)

	#define ANIM_RADIAN_SCALE ((ANIM_RADIAN_MAX - (ANIM_RADIAN_MIN)) / (2 * ANIM_VALUE_MAX)); // neg/pos used so two of ANIM_VALUE_MAX
	#define ANIM_SCALE_SCALE ((ANIM_SCALE_MAX - (ANIM_SCALE_MIN)) / (ANIM_VALUE_MAX));

	int GetAnimValueOffset(const mstudioanimvalue_t* const panimvalue)
	{
		const int lutBaseIdx = panimvalue->num.valid * 3;

		return (s_AnimSeekLUT[lutBaseIdx] + ((s_AnimSeekLUT[lutBaseIdx + 1] + panimvalue->num.total * s_AnimSeekLUT[lutBaseIdx + 2]) >> 4));
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame rotation for a single bone
	//-----------------------------------------------------------------------------
	void ExtractAnimValue(mstudioanimvalue_t* panimvalue, const int frame, const float scale, float& v1)
	{
		// note: in R5 'valid' is not really used the same way as traditional source.
		switch (panimvalue->num.valid)
		{
		// '0' valid values is treated as the traditional source style track, having a 16bit signed integer per frame value. this will be used if the more compact tracks cannot be utilized
		case 0:
		{
			v1 = static_cast<float>(panimvalue[frame + 1].value) * scale; // + 1 to skip over the base animvalue.

			return;
		}
		// '1' valid values is a new type of compression, the track consists of one short, followed by a series of signed 8bit integers used to adjust the value after the first frame.
		case 1:
		{
			int16_t value = panimvalue[1].value;

			// there is a signed char for every frame after index 0 to adjust the basic value, this limits changes to be +-127 compressed
			if (frame > 0)
				value += reinterpret_cast<char*>(&panimvalue[2])[frame - 1]; // the char values start after the base

			v1 = static_cast<float>(value) * scale;

			return;
		}
		// 'other' valid values is a new type of compression, but can also be used to have a single value across many frames due to how it is setup
		default:
		{
			float v7 = 1.0f;

			// note: we are subtracting two, as if it isn't above two, we should only have the single base value
			const int bitLUTIndex = (panimvalue->num.valid - 2) / 6; // how many bits are we storing per frame, if any.
			const int numUnkValue = (panimvalue->num.valid - 2) % 6; // number of 16 bit values on top of the base value, unsure what these are

			float value = static_cast<float>(panimvalue[1].value); // the base value after our animvalue

			const float cycle = static_cast<float>(frame) / static_cast<float>(panimvalue->num.total - 1); // progress through this block

			// this is very cursed and I am unsure of why it works, the 16 bit values are all over the place, perhaps these are a curve of sorts?
			for (int i = 0; i < numUnkValue; i++)
			{
				v7 *= cycle;
				value += static_cast<float>(panimvalue[i + 2].value) * v7; // + 2, skipping the animvalue, and the first base value, trailing values are for modification
			}

			// this is pretty straight forward, like with '1', we store adjustments to the base value saving space by packing them into a smaller value. the adjustments seem to be micro tweaks on top of how whatever unknown 16 bit values do
			if (bitLUTIndex)
			{
				const int16_t* pbits = &panimvalue[numUnkValue + 2].value; // + 2, skipping the animvalue, the first base value, and the unknown values, placing us at the bits

				value -= s_FrameValOffsetLUT[bitLUTIndex]; // subtract the max value that can be contained within our bitfield

				// number of bits per value adjustment
				const uint8_t maskBitCount = s_FrameBitCountLUT[bitLUTIndex];

				// data mask for value bits
				const uint8_t mask = (1 << maskBitCount) - 1;

				// get the bit offset to the value for this frame
				const int frameValueBitOffset = frame * maskBitCount;

				// number of bits stored in a "mstudioanimvalue_t" value
				constexpr int animValueBitSize = (8 * sizeof(mstudioanimvalue_t));
				static_assert(animValueBitSize == 16);

				// offset must be contained within the 16 bits of mstudioanimvalue_t::value, so only the lower 4 bits of the value are kept (bitmask 0xF)
				const int bitOffset = frameValueBitOffset & (animValueBitSize - 1);

				value += 2.0f * (mask & (pbits[frameValueBitOffset >> 4] >> bitOffset)); // we shift frameValueBitOffset to index for an int16_t pointer, then we offset into that int16_t and mask off the bits we wish to keep. some accuracy loss here considering we mult by two
			}

			v1 = value * scale;

			return;
		}
		}
	}

	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1)
	{
		int k = frame;

		// [rika]: for obvious reasons this func gets inlined in CalcBone functions
		while (panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += GetAnimValueOffset(panimvalue);
		}

		ExtractAnimValue(panimvalue, k, scale, v1);
	}

	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1, float& v2)
	{
		int k = frame;

		// find the data list that has the frame
		while (panimvalue->num.total <= k)
		{
			k -= panimvalue->num.total;
			panimvalue += GetAnimValueOffset(panimvalue); // [rika]: this is a macro because I thought it was used more than once initally
		}

		// [rika]: check if our track index is the last value, if it is, our second value (v2) gets taken from the first frame of the next track
		if (k >= panimvalue->num.total - 1)
		{
			ExtractAnimValue(panimvalue, k, scale, v1);
			ExtractAnimValue(panimvalue + GetAnimValueOffset(panimvalue), 0, scale, v2); // index 0 for the first frame, seek to next track with macro
		}
		else
		{
			// inlined in apex but it's actually just two ExtractAnimValue functions
			ExtractAnimValue(panimvalue, k, scale, v1);
			ExtractAnimValue(panimvalue, k + 1, scale, v2);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame rotation for a single bone
	//-----------------------------------------------------------------------------
	void CalcBoneQuaternion(int frame, float s, const mstudio_rle_anim_t* panim, const RadianEuler& baseRot, Quaternion& q, const char boneFlags)
	{
		if (!(panim->flags & RleFlags_t::STUDIO_ANIM_ANIMROT))
		{
			q = *(panim->pQuat64(boneFlags));
			assert(q.IsValid());
			return;
		}

		mstudioanim_valueptr_t* pRotV = panim->pRotV(boneFlags);
		int j;

		mstudioanimvalue_t* values[3] = { pRotV->pAnimvalue(), pRotV->pAnimvalue() + pRotV->axisIdx1, pRotV->pAnimvalue() + pRotV->axisIdx2 };

		// don't interpolate if first frame
		if (s == 0.0f)
		{
			RadianEuler angle;

			for (j = 0; j < 3; j++)
			{
				if (pRotV->flags & (ValuePtrFlags_t::STUDIO_ANIMPTR_X >> j))
				{
					ExtractAnimValue(frame, values[j], FIXED_ROT_SCALE, angle[j]);
				}
				else
				{
					// check this
					angle[j] = baseRot[j];
				}
			}

			assert(angle.IsValid());
			AngleQuaternion(angle, q);
		}
		else
		{
			Quaternion	q1, q2; // QuaternionAligned
			RadianEuler angle1, angle2;

			for (j = 0; j < 3; j++)
			{
				// extract anim value doesn't catch nullptrs like normal source, so we have to be careful
				if (pRotV->flags & (ValuePtrFlags_t::STUDIO_ANIMPTR_X >> j))
				{
					ExtractAnimValue(frame, values[j], FIXED_ROT_SCALE, angle1[j], angle2[j]);
				}
				else
				{
					// unsure
					angle1[j] = baseRot[j];
					angle2[j] = baseRot[j];
				}
			}

			// disassembly is flipped
			assert(angle1.IsValid() && angle2.IsValid());
			if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
			{
				AngleQuaternion(angle1, q1);
				AngleQuaternion(angle2, q2);

				QuaternionBlend(q1, q2, s, q);
			}
			else
			{
				AngleQuaternion(angle1, q);
			}
		}

		assert(q.IsValid());
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame position for a single bone
	//-----------------------------------------------------------------------------
	void CalcBonePosition(int frame, float s, const mstudio_rle_anim_t* panim, Vector& pos)
	{
		if (!(panim->flags & RleFlags_t::STUDIO_ANIM_ANIMPOS))
		{
			pos = *(panim->pPos());
			assert(pos.IsValid());

			return;
		}

		mstudioanim_valueptr_t* pPosV = panim->pPosV();
		int j;

		mstudioanimvalue_t* values[3] = { pPosV->pAnimvalue(), pPosV->pAnimvalue() + pPosV->axisIdx1, pPosV->pAnimvalue() + pPosV->axisIdx2 };
		Vector tmp(0.0f); // temp to load our values into, pos is passed in with data in apex (delta is checked externally)

		// don't interpolate if first frame
		if (s == 0.0f)
		{
			for (j = 0; j < 3; j++)
			{
				if (pPosV->flags & (ValuePtrFlags_t::STUDIO_ANIMPTR_X >> j))
				{
					ExtractAnimValue(frame, values[j], panim->PosScale(), tmp[j]);
				}
			}
		}
		else
		{
			float v1, v2;

			for (j = 0; j < 3; j++)
			{
				// extract anim value doesn't catch nullptrs like normal source, so we have to be careful
				if (pPosV->flags & (ValuePtrFlags_t::STUDIO_ANIMPTR_X >> j))
				{
					ExtractAnimValue(frame, values[j], panim->PosScale(), v1, v2);
					tmp[j] = v1 * (1.0f - s) + v2 * s;
				}
			}
		}

		pos += tmp; // add our values onto the existing pos

		assert(pos.IsValid());
	}

	//-----------------------------------------------------------------------------
	// Purpose: return a sub frame scale for a single bone
	//-----------------------------------------------------------------------------
	void CalcBoneScale(int frame, float s, const mstudio_rle_anim_t* panim, Vector& scale, const char boneFlags)
	{
		// basically the same as pos with the exception of a 'boneFlags' var so we can see what data this rle header has
		// apex just passes a pointer to the data but I am using funcs like normal source
		if (!(panim->flags & RleFlags_t::STUDIO_ANIM_ANIMSCALE))
		{
			scale = *(panim->pScale(boneFlags));
			assert(scale.IsValid());

			return;
		}

		mstudioanim_valueptr_t* pScaleV = panim->pScaleV(boneFlags);
		int j;

		mstudioanimvalue_t* values[3] = { pScaleV->pAnimvalue(), pScaleV->pAnimvalue() + pScaleV->axisIdx1, pScaleV->pAnimvalue() + pScaleV->axisIdx2 };
		Vector tmp(0.0f); // temp to load our values into, scale is passed in with data in apex (delta is checked externally)

		// don't interpolate if first frame
		if (s == 0.0f)
		{
			for (j = 0; j < 3; j++)
			{
				if (pScaleV->flags & (ValuePtrFlags_t::STUDIO_ANIMPTR_X >> j))
				{
					ExtractAnimValue(frame, values[j], FIXED_SCALE_SCALE, tmp[j]);
				}
			}
		}
		else
		{
			float v1, v2;

			for (j = 0; j < 3; j++)
			{
				// extract anim value doesn't catch nullptrs like normal source, so we have to be careful
				if (pScaleV->flags & (ValuePtrFlags_t::STUDIO_ANIMPTR_X >> j))
				{
					ExtractAnimValue(frame, values[j], FIXED_SCALE_SCALE, v1, v2);
					tmp[j] = v1 * (1.0f - s) + v2 * s;
				}
			}
		}

		scale += tmp; // what?? are the scale values stored differently in apex? delta vs absolute

		assert(scale.IsValid());
	}

	//-----------------------------------------------------------------------------
		// Purpose: calculate changes in position and angle relative to the start of an animations cycle
		// Output:	updated position and angle, relative to the origin
		//			returns false if animation is not a movement animation
		//-----------------------------------------------------------------------------
	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int iFrame, Vector& vecPos, float& yaw)
	{
		uint16_t* section = pFrameMovement->pSection(iFrame);

		const int iLocalFrame = iFrame % pFrameMovement->sectionframes;

		for (int i = 0; i < 3; i++)
		{
			if (section[i])
				ExtractAnimValue(iLocalFrame, pFrameMovement->pAnimvalue(i, section), pFrameMovement->scale[i], vecPos[i]);
			else
				vecPos[i] = 0.0f;
		}

		if (section[3])
			ExtractAnimValue(iLocalFrame, pFrameMovement->pAnimvalue(3, section), pFrameMovement->scale[3], yaw);
		else
			yaw = 0.0f;
	}

	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int iFrame, Vector& v1Pos, Vector& v2Pos, float& v1Yaw, float& v2Yaw)
	{
		uint16_t* section = pFrameMovement->pSection(iFrame);

		const int iLocalFrame = iFrame % pFrameMovement->sectionframes;

		for (int i = 0; i < 3; i++)
		{
			if (section[i])
				ExtractAnimValue(iLocalFrame, pFrameMovement->pAnimvalue(i, section), pFrameMovement->scale[i], v1Pos[i], v2Pos[i]);
			else
			{
				v1Pos[i] = 0.0f;
				v2Pos[i] = 0.0f;
			}
		}

		if (section[3])
			ExtractAnimValue(iLocalFrame, pFrameMovement->pAnimvalue(3, section), pFrameMovement->scale[3], v1Yaw, v2Yaw);
		else
		{
			v1Yaw = 0.0f;
			v2Yaw = 0.0f;
		}
	}

	bool Studio_AnimPosition(const mstudioanimdesc_t* const panim, float flCycle, Vector& vecPos, QAngle& vecAngle)
	{
		float	prevframe = 0;
		vecPos.Init();
		vecAngle.Init();

		int iLoops = 0;
		if (flCycle > 1.0)
		{
			iLoops = static_cast<int>(flCycle);
		}
		else if (flCycle < 0.0)
		{
			iLoops = static_cast<int>(flCycle) - 1;
		}
		flCycle = flCycle - iLoops;

		float	flFrame = flCycle * (panim->numframes - 1);

		if (panim->flags & STUDIO_FRAMEMOVEMENT)
		{
			int iFrame = static_cast<int>(flFrame);
			float s = (flFrame - iFrame);

			if (s == 0.0f)
			{
				Studio_FrameMovement(panim->pFrameMovement(), iFrame, vecPos, vecAngle.y);
				return true;
			}
			else
			{
				Vector v1Pos, v2Pos;
				float v1Yaw, v2Yaw;

				Studio_FrameMovement(panim->pFrameMovement(), iFrame, v1Pos, v2Pos, v1Yaw, v2Yaw);

				vecPos = ((v2Pos - v1Pos) * s) + v1Pos; // this should work on paper?

				float yawDelta = AngleDiff(v2Yaw, v1Yaw);
				vecAngle.y = (yawDelta * s) + v1Yaw;

				return true;
			}
		}
		else
		{
			for (int i = 0; i < panim->nummovements; i++)
			{
				const mstudiomovement_t* const pmove = panim->pMovement(i);

				if (pmove->endframe >= flFrame)
				{
					float f = (flFrame - prevframe) / (pmove->endframe - prevframe);

					float d = pmove->v0 * f + 0.5f * (pmove->v1 - pmove->v0) * f * f;

					vecPos = vecPos + (pmove->vector * d);
					vecAngle.y = vecAngle.y * (1 - f) + pmove->angle * f;
					if (iLoops != 0)
					{
						const mstudiomovement_t* const pmove_1 = panim->pMovement(panim->nummovements - 1);
						vecPos = vecPos + (pmove_1->position * iLoops);
						vecAngle.y = vecAngle.y + iLoops * pmove_1->angle;
					}
					return true;
				}
				else
				{
					prevframe = static_cast<float>(pmove->endframe);
					vecPos = pmove->position;
					vecAngle.y = pmove->angle;
				}
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Find and decode a sub-frame of animation
	//-----------------------------------------------------------------------------
	/*void CalcAnimation(const CStudioHdr* pStudioHdr, Vector* pos, Quaternion* q, Vector* scale, mstudioseqdesc_t& seqdesc, int sequence, int animation, float cycle, int boneMask)
	{
		// not a full working function just me trying to visualize how things are parsed
		// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L1036
		RadianEuler baseRot[256]{};

		virtualmodel_t* pVModel = pStudioHdr->GetVirtualModel();

		if (pVModel)
		{
			CalcVirtualAnimation(pVModel, pStudioHdr, pos, q, seqdesc, sequence, animation, cycle, boneMask);
			return;
		}

		mstudioanimdesc_t& animdesc = ((CStudioHdr*)pStudioHdr)->pAnimdesc(animation);
		mstudiobone_t* pbone = pStudioHdr->pBone(0);
		const mstudiolinearbone_t* pLinearBones = pStudioHdr->pLinearBones();

		// setup default bone values, this is why we add to the value instead of setting it in apex.
		if (animdesc.flags & STUDIO_DELTA)
		{
			for (int i = 0; i < pStudioHdr->numbones(); i++)
			{
				pos[i].Init(0.0f, 0.0f, 0.0f);
				q[i].Init(0.0f, 0.0f, 0.0f, 1.0f);
				scale[i].Init(1.0f, 1.0f, 1.0f);
				baseRot[i].Init(0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			if (nullptr != pLinearBones)
			{
				for (int i = 0; i < pStudioHdr->numbones(); i++)
				{
					pos[i] = pLinearBones->pPos(0)[i];
					q[i] = pLinearBones->pQuat(0)[i];
					scale[i].Init(1.0f, 1.0f, 1.0f);
					baseRot[i] = pLinearBones->pRot(0)[i];
				}
			}
			else
			{
				for (int i = 0; i < pStudioHdr->numbones(); i++)
				{
					pos[i] = pbone[i].pos;
					q[i] = pbone[i].quat;
					scale[i] = pbone[i].scale;
					baseRot[i] = pbone[i].rot;
				}
			}
		}

		// don't bother with animation data, there is none
		if (animdesc.flags & STUDIO_NOANIM)
			return;

		//float fFrame = cycle * static_cast<float>(animdesc.numframes - 1);
		//int iFrame = static_cast<int>(fFrame);
		//float s = (fFrame - iFrame);

		int i;
		int iFrame;
		float s;

		float fFrame = cycle * (animdesc.numframes - 1);

		iFrame = (int)fFrame;
		s = (fFrame - iFrame);

		int iLocalFrame = iFrame;

		const uint8_t* boneFlagArray = reinterpret_cast<uint8_t*>(animdesc.pAnimdata(&iLocalFrame));
		const mstudio_rle_anim_t* panim = reinterpret_cast<const mstudio_rle_anim_t*>(&boneFlagArray[ANIM_BONEFLAG_SIZE(pStudioHdr->numbones();)]);

		float* pweight = seqdesc.pBoneweight(0);

		for (i = 0; i < pStudioHdr->numbones(); i++, pbone++, pweight++)
		{
			bool magic = true;
			// if (*pweight > 0 && (pStudioHdr->boneFlags(i) & boneMask))
			if (magic)
			{
				const uint8_t boneFlags = ANIM_BONEFLAGS_FLAG(boneFlagArray, i); // truncate byte offset then shift if needed

				if (boneFlags & (RleBoneFlags_t::STUDIO_ANIM_POS | RleBoneFlags_t::STUDIO_ANIM_ROT | RleBoneFlags_t::STUDIO_ANIM_SCALE)) // check if this bone has data
				{
					if (boneFlags & RleBoneFlags_t::STUDIO_ANIM_POS)
						CalcBonePosition(iLocalFrame, s, panim, pos[i]);
					if (boneFlags & RleBoneFlags_t::STUDIO_ANIM_ROT)
						CalcBoneQuaternion(iLocalFrame, s, panim, baseRot[i], q[i], boneFlags);
					if (boneFlags & RleBoneFlags_t::STUDIO_ANIM_SCALE)
						CalcBoneScale(iLocalFrame, s, panim, scale[i], boneFlags);


					panim = panim->pNext();
				}
			}
			// this is so we actually move on from the anim data if it's undesired, not useful for our purposes ?
			else
			{
				const uint8_t boneFlags = ANIM_BONEFLAGS_FLAG(boneFlagArray, i); // truncate byte offset then shift if needed

				if (boneFlags & (RleBoneFlags_t::STUDIO_ANIM_POS | RleBoneFlags_t::STUDIO_ANIM_ROT | RleBoneFlags_t::STUDIO_ANIM_SCALE)) // check if this bone has data
				{
					panim = panim->pNext();
				}
			}
		}
	}*/
}

namespace r5_180
{
	// pre season 25
	float Studio_InterpFrames_Single(const float fFrame, mstudioseqdesc_t* seqdesc)
	{
		const int iFrame = static_cast<int>(fFrame);

		const bool useNoInterpFrames = true; // default game value is true

		// check 'use_no_interp_frames' setting
		// true here means interp is disabled, so check if false, if false interp.
		if (!useNoInterpFrames)
			return (fFrame - static_cast<float>(iFrame));

		// if we don't have this data, so we interpolate.
		if (!seqdesc->noInterpFrameCount)
			return (fFrame - static_cast<float>(iFrame));

		for (int i = 0; i < seqdesc->noInterpFrameCount; i++)
		{
			if (iFrame != seqdesc->NoInterpFrameSingle(i))
				continue;

			return 0.0;
		}

		return (fFrame - static_cast<float>(iFrame));
	}

	// as of season 25
	float Studio_InterpFrames_Pair(const float fFrame, mstudioseqdesc_t* seqdesc)
	{
		const int iFrame = static_cast<int>(fFrame);

		const bool useNoInterpFrames = true; // default game value is true

		// check 'use_no_interp_frames' setting
		// true here means interp is disabled, so check if false, if false interp.
		if (!useNoInterpFrames)
			return (fFrame - static_cast<float>(iFrame));

		// if we don't have this data, so we interpolate.
		if (!seqdesc->noInterpFrameCount)
			return (fFrame - static_cast<float>(iFrame));

		for (int i = 0; i < seqdesc->noInterpFrameCount; i++)
		{
			const mstudio_nointerpframes_t* const framerange = seqdesc->NoInterpFramePair(i);
			if (iFrame < framerange->firstFrame || iFrame > framerange->lastFrame)
				continue;

			return 0.0;
		}

		return (fFrame - static_cast<float>(iFrame));
	}
}

namespace r5_190
{
	//-----------------------------------------------------------------------------
	// Purpose: Find and decode a sub-frame of animation
	//-----------------------------------------------------------------------------
	/*void CalcAnimation(const CStudioHdr* pStudioHdr, Vector* pos, Quaternion* q, Vector* scale, mstudioseqdesc_t& seqdesc, int sequence, int animation, float cycle, int boneMask)
	{
		// not a full working function just me trying to visualize how things are parsed
		// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L1036
		RadianEuler baseRot[256]{};

		virtualmodel_t* pVModel = pStudioHdr->GetVirtualModel();

		if (pVModel)
		{
			CalcVirtualAnimation(pVModel, pStudioHdr, pos, q, seqdesc, sequence, animation, cycle, boneMask);
			return;
		}

		mstudioanimdesc_t& animdesc = ((CStudioHdr*)pStudioHdr)->pAnimdesc(animation);
		mstudiobone_t* pbone = pStudioHdr->pBone(0);
		const mstudiolinearbone_t* pLinearBones = pStudioHdr->pLinearBones();

		// setup default bone values, this is why we add to the value instead of setting it in apex.
		if (animdesc.flags & STUDIO_DELTA)
		{
			for (int i = 0; i < pStudioHdr->numbones(); i++)
			{
				pos[i].Init(0.0f, 0.0f, 0.0f);
				q[i].Init(0.0f, 0.0f, 0.0f, 1.0f);
				scale[i].Init(1.0f, 1.0f, 1.0f);
				baseRot[i].Init(0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			if (nullptr != pLinearBones)
			{
				for (int i = 0; i < pStudioHdr->numbones(); i++)
				{
					pos[i] = pLinearBones->pPos(0)[i];
					q[i] = pLinearBones->pQuat(0)[i];
					scale[i].Init(1.0f, 1.0f, 1.0f);
					baseRot[i] = pLinearBones->pRot(0)[i];
				}
			}
			else
			{
				for (int i = 0; i < pStudioHdr->numbones(); i++)
				{
					pos[i] = pbone[i].pos;
					q[i] = pbone[i].quat;
					scale[i] = pbone[i].scale;
					baseRot[i] = pbone[i].rot;
				}
			}
		}

		// don't bother with animation data, there is none
		if (animdesc.flags & STUDIO_NOANIM)
			return;

		//float fFrame = cycle * static_cast<float>(animdesc.numframes - 1);
		//int iFrame = static_cast<int>(fFrame);
		//float s = (fFrame - iFrame);

		int i;
		int iFrame;
		float s;

		float fFrame = cycle * (animdesc.numframes - 1);

		iFrame = (int)fFrame;
		s = (fFrame - iFrame);

		int iLocalFrame = iFrame;

		const uint8_t* boneFlagArray = reinterpret_cast<uint8_t*>(animdesc.pAnimdata(&iLocalFrame));
		const mstudio_rle_anim_t* panim = reinterpret_cast<const mstudio_rle_anim_t*>(&boneFlagArray[ANIM_BONEFLAG_SIZE(pStudioHdr->numbones();)]);

		float* pweight = seqdesc.pBoneweight(0);

		for (i = 0; i < pStudioHdr->numbones(); i++, pbone++, pweight++)
		{
			bool magic = true;
			// if (*pweight > 0 && (pStudioHdr->boneFlags(i) & boneMask))
			if (magic)
			{
				const uint8_t boneFlags = ANIM_BONEFLAGS_FLAG(boneFlagArray, i); // truncate byte offset then shift if needed

				if (boneFlags & (RleBoneFlags_t::STUDIO_ANIM_POS | RleBoneFlags_t::STUDIO_ANIM_ROT | RleBoneFlags_t::STUDIO_ANIM_SCALE)) // check if this bone has data
				{
					if (boneFlags & RleBoneFlags_t::STUDIO_ANIM_POS)
						CalcBonePosition(iLocalFrame, s, panim, pos[i]);
					if (boneFlags & RleBoneFlags_t::STUDIO_ANIM_ROT)
						CalcBoneQuaternion(iLocalFrame, s, panim, baseRot[i], q[i], boneFlags);
					if (boneFlags & RleBoneFlags_t::STUDIO_ANIM_SCALE)
						CalcBoneScale(iLocalFrame, s, panim, scale[i], boneFlags);


					panim = panim->pNext();
				}
			}
			// this is so we actually move on from the anim data if it's undesired, not useful for our purposes ?
			else
			{
				const uint8_t boneFlags = ANIM_BONEFLAGS_FLAG(boneFlagArray, i); // truncate byte offset then shift if needed

				if (boneFlags & (RleBoneFlags_t::STUDIO_ANIM_POS | RleBoneFlags_t::STUDIO_ANIM_ROT | RleBoneFlags_t::STUDIO_ANIM_SCALE)) // check if this bone has data
				{
					panim = panim->pNext();
				}
			}
		}
	}*/
}

