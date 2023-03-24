#include <cassert>
#include <math.h>

#include "studio.h"
#include "bone_setup.h"
#include "../math/mathlib.h"

//namespace r2
//{
//	//-----------------------------------------------------------------------------
//	// Purpose: return a sub frame rotation for a single bone
//	//-----------------------------------------------------------------------------
//	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1, float& v2)
//	{
//		if (!panimvalue)
//		{
//			v1 = v2 = 0;
//			return;
//		}
//
//		// Avoids a crash reading off the end of the data
//		// There is probably a better long-term solution; Ken is going to look into it.
//		if ((panimvalue->num.total == 1) && (panimvalue->num.valid == 1))
//		{
//			v1 = v2 = panimvalue[1].value * scale;
//			return;
//		}
//
//		int k = frame;
//
//		// find the data list that has the frame
//		while (panimvalue->num.total <= k)
//		{
//			k -= panimvalue->num.total;
//			panimvalue += panimvalue->num.valid + 1;
//			if (panimvalue->num.total == 0)
//			{
//				assert(0); // running off the end of the animation stream is bad
//				v1 = v2 = 0;
//				return;
//			}
//		}
//		if (panimvalue->num.valid > k)
//		{
//			// has valid animation data
//			v1 = panimvalue[k + 1].value * scale;
//
//			if (panimvalue->num.valid > k + 1)
//			{
//				// has valid animation blend data
//				v2 = panimvalue[k + 2].value * scale;
//			}
//			else
//			{
//				if (panimvalue->num.total > k + 1)
//				{
//					// data repeats, no blend
//					v2 = v1;
//				}
//				else
//				{
//					// pull blend from first data block in next list
//					v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
//				}
//			}
//		}
//		else
//		{
//			// get last valid data block
//			v1 = panimvalue[panimvalue->num.valid].value * scale;
//			if (panimvalue->num.total > k + 1)
//			{
//				// data repeats, no blend
//				v2 = v1;
//			}
//			else
//			{
//				// pull blend from first data block in next list
//				v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
//			}
//		}
//	}
//
//	void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1)
//	{
//		if (!panimvalue)
//		{
//			v1 = 0;
//			return;
//		}
//
//		int k = frame;
//
//		while (panimvalue->num.total <= k)
//		{
//			k -= panimvalue->num.total;
//			panimvalue += panimvalue->num.valid + 1;
//			if (panimvalue->num.total == 0)
//			{
//				assert(0); // running off the end of the animation stream is bad
//				v1 = 0;
//				return;
//			}
//		}
//		if (panimvalue->num.valid > k)
//		{
//			v1 = panimvalue[k + 1].value * scale;
//		}
//		else
//		{
//			// get last valid data block
//			v1 = panimvalue[panimvalue->num.valid].value * scale;
//		}
//	}
//
//	//-----------------------------------------------------------------------------
//	// Purpose: return a sub frame rotation for a single bone
//	//-----------------------------------------------------------------------------
//	void CalcBoneQuaternion(int frame, float s, const Quaternion& baseQuat, const RadianEuler& baseRot, const Vector& baseRotScale, int iBaseFlags, const Quaternion& baseAlignment, const mstudio_rle_anim_t* panim, Quaternion& q)
//	{
//		if (panim->flags & STUDIO_ANIM_RAWROT)
//		{
//			q = *(panim->pQuat64());
//			assert(q.IsValid());
//			return;
//		}
//
//		if (!(panim->flags & STUDIO_ANIM_NOROT))
//		{
//			if (panim->flags & STUDIO_ANIM_DELTA)
//			{
//				q.Init(0.0f, 0.0f, 0.0f, 1.0f);
//			}
//			else
//			{
//				q = baseQuat;
//			}
//			return;
//		}
//
//		mstudioanim_valueptr_t* pValuesPtr = panim->pRotV();
//
//		if (s > 0.001f)
//		{
//			Quaternion	q1, q2; // QuaternionAligned
//			RadianEuler			angle1, angle2;
//
//			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(0), baseRotScale.x, angle1.x, angle2.x);
//			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(1), baseRotScale.y, angle1.y, angle2.y);
//			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(2), baseRotScale.z, angle1.z, angle2.z);
//
//			if (!(panim->flags & STUDIO_ANIM_DELTA))
//			{
//				angle1.x = angle1.x + baseRot.x;
//				angle1.y = angle1.y + baseRot.y;
//				angle1.z = angle1.z + baseRot.z;
//				angle2.x = angle2.x + baseRot.x;
//				angle2.y = angle2.y + baseRot.y;
//				angle2.z = angle2.z + baseRot.z;
//			}
//
//			assert(angle1.IsValid() && angle2.IsValid());
//			if (angle1.x != angle2.x || angle1.y != angle2.y || angle1.z != angle2.z)
//			{
//				AngleQuaternion(angle1, q1);
//				AngleQuaternion(angle2, q2);
//
//				//QuaternionBlend(q1, q2, s, q);
//			}
//			else
//			{
//				AngleQuaternion(angle1, q);
//			}
//		}
//		else
//		{
//			RadianEuler			angle;
//
//			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(0), baseRotScale.x, angle.x);
//			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(1), baseRotScale.y, angle.y);
//			ExtractAnimValue(frame, pValuesPtr->pAnimvalue(2), baseRotScale.z, angle.z);
//
//			if (!(panim->flags & STUDIO_ANIM_DELTA))
//			{
//				angle.x = angle.x + baseRot.x;
//				angle.y = angle.y + baseRot.y;
//				angle.z = angle.z + baseRot.z;
//			}
//
//			assert(angle.IsValid());
//			AngleQuaternion(angle, q);
//		}
//
//		assert(q.IsValid());
//
//		// align to unified bone
//		if (!(panim->flags & STUDIO_ANIM_DELTA) && (iBaseFlags & BONE_FIXED_ALIGNMENT))
//		{
//			//QuaternionAlign(baseAlignment, q, q);
//		}
//	}
//
//	inline void CalcBoneQuaternion(int frame, float s, const mstudiobone_t* pBone, const mstudiolinearbone_t* pLinearBones, const mstudio_rle_anim_t* panim, Quaternion& q)
//	{
//		if (pLinearBones)
//		{
//			CalcBoneQuaternion(frame, s, *pLinearBones->pQuat(panim->bone), *pLinearBones->pRot(panim->bone), *pLinearBones->pRotScale(panim->bone), pLinearBones->flags(panim->bone), *pLinearBones->pQAlignment(panim->bone), panim, q);
//		}
//		else
//		{
//			CalcBoneQuaternion(frame, s, pBone->quat, pBone->rot, pBone->rotscale, pBone->flags, pBone->qAlignment, panim, q);
//		}
//	}
//
//	//-----------------------------------------------------------------------------
//	// Purpose: return a sub frame position for a single bone
//	//-----------------------------------------------------------------------------
//	void CalcBonePosition(int frame, float s, const Vector& basePos, const float &baseBoneScale, const mstudio_rle_anim_t* panim, Vector& pos)
//	{
//		if (panim->flags & STUDIO_ANIM_RAWPOS)
//		{
//			pos = *(panim->pPos());
//			assert(pos.IsValid());
//
//			return;
//		}
//
//		mstudioanim_valueptr_t* pPosV = panim->pPosV();
//		int					j;
//
//		if (s > 0.001f)
//		{
//			float v1, v2;
//			for (j = 0; j < 3; j++)
//			{
//				ExtractAnimValue(frame, pPosV->pAnimvalue(j), baseBoneScale, v1, v2);
//				pos[j] = v1 * (1.0 - s) + v2 * s;
//			}
//		}
//		else
//		{
//			for (j = 0; j < 3; j++)
//			{
//				ExtractAnimValue(frame, pPosV->pAnimvalue(j), baseBoneScale, pos[j]);
//			}
//		}
//
//		if (!(panim->flags & STUDIO_ANIM_DELTA))
//		{
//			pos.x = pos.x + basePos.x;
//			pos.y = pos.y + basePos.y;
//			pos.z = pos.z + basePos.z;
//		}
//
//		assert(pos.IsValid());
//	}
//
//	inline void CalcBonePosition(int frame, float s, const mstudiobone_t* pBone, const mstudiolinearbone_t* pLinearBones, const mstudio_rle_anim_t* panim, Vector& pos)
//	{
//		if (pLinearBones)
//		{
//			CalcBonePosition(frame, s, *pLinearBones->pPos(panim->bone), panim->posscale, panim, pos);
//		}
//		else
//		{
//			CalcBonePosition(frame, s, pBone->pos, panim->posscale, panim, pos);
//		}
//	}
//
//	//-----------------------------------------------------------------------------
//	// Purpose: return a sub frame scale for a single bone
//	//-----------------------------------------------------------------------------
//	void CalcBoneScale(int frame, float s, const Vector& baseScale, const Vector& baseScaleScale, const mstudio_rle_anim_t* panim, Vector& scale)
//	{
//		if (panim->flags & STUDIO_ANIM_RAWSCALE)
//		{
//			scale = *(panim->pScale());
//			assert(scale.IsValid());
//
//			return;
//		}
//
//		mstudioanim_valueptr_t* pScaleV = panim->pScaleV();
//		int					j;
//
//		if (s > 0.001f)
//		{
//			float v1, v2;
//			for (j = 0; j < 3; j++)
//			{
//				ExtractAnimValue(frame, pScaleV->pAnimvalue(j), baseScaleScale[j], v1, v2);
//				scale[j] = v1 * (1.0 - s) + v2 * s;
//			}
//		}
//		else
//		{
//			for (j = 0; j < 3; j++)
//			{
//				ExtractAnimValue(frame, pScaleV->pAnimvalue(j), baseScaleScale[j], scale[j]);
//			}
//		}
//
//		if (!(panim->flags & STUDIO_ANIM_DELTA))
//		{
//			scale.x = scale.x + baseScale.x;
//			scale.y = scale.y + baseScale.y;
//			scale.z = scale.z + baseScale.z;
//		}
//
//		assert(scale.IsValid());
//	}
//
//	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int frame, Vector& vecPos, float& yaw)
//	{
//		for (int i = 0; i < 3; i++)
//		{
//			ExtractAnimValue(frame, pFrameMovement->pAnimvalue(i), pFrameMovement->scale[i], vecPos[i]);
//		}
//
//		ExtractAnimValue(frame, pFrameMovement->pAnimvalue(3), pFrameMovement->scale[3], yaw);
//	}
//
//	void Studio_FrameMovement(const mstudioframemovement_t* pFrameMovement, int iFrame, Vector& v1Pos, Vector& v2Pos, float& v1Yaw, float& v2Yaw)
//	{
//		for (int i = 0; i < 3; i++)
//		{
//			ExtractAnimValue(iFrame, pFrameMovement->pAnimvalue(i), pFrameMovement->scale[i], v1Pos[i], v2Pos[i]);
//		}
//
//		ExtractAnimValue(iFrame, pFrameMovement->pAnimvalue(3), pFrameMovement->scale[3], v1Yaw, v2Yaw);
//	}
//
//	//-----------------------------------------------------------------------------
//	// Purpose: calculate changes in position and angle relative to the start of an animations cycle
//	// Output:	updated position and angle, relative to the origin
//	//			returns false if animation is not a movement animation
//	//-----------------------------------------------------------------------------
//	bool Studio_AnimPosition(mstudioanimdesc_t* panim, float flCycle, Vector& vecPos, QAngle& vecAngle)
//	{
//		float	prevframe = 0;
//		vecPos.Init();
//		vecAngle.Init();
//
//		int iLoops = 0;
//		if (flCycle > 1.0)
//		{
//			iLoops = (int)flCycle;
//		}
//		else if (flCycle < 0.0)
//		{
//			iLoops = (int)flCycle - 1;
//		}
//		flCycle = flCycle - iLoops;
//
//		float	flFrame = flCycle * (panim->numframes - 1);
//
//		if (panim->flags & STUDIO_FRAMEMOVEMENT)
//		{
//			int iFrame = (int)flFrame;
//			float s = (flFrame - iFrame);
//
//			if (s == 0)
//			{
//				Studio_FrameMovement(panim->pFrameMovement(), iFrame, vecPos, vecAngle.y);
//				return true;
//			}
//			else
//			{
//				Vector v1Pos, v2Pos;
//				float v1Yaw, v2Yaw;
//
//				Studio_FrameMovement(panim->pFrameMovement(), iFrame, v1Pos, v2Pos, v1Yaw, v2Yaw);
//
//				/*vecPos.x = ((v2Pos.x - v1Pos.x) * s) + v1Pos.x;
//				vecPos.y = ((v2Pos.y - v1Pos.y) * s) + v1Pos.y;
//				vecPos.z = ((v2Pos.z - v1Pos.z) * s) + v1Pos.z;*/
//
//				vecPos = ((v2Pos - v1Pos) * s) + v1Pos; // this should work on paper?
//
//				float yawNormalized = NormalizeAngle(v2Yaw, v1Yaw);
//				vecAngle.y = (yawNormalized * s) + v1Yaw;	
//
//				return true;
//			}
//		}
//		else
//		{
//			for (int i = 0; i < panim->nummovements; i++)
//			{
//				mstudiomovement_t* pmove = panim->pMovement(i);
//
//				if (pmove->endframe >= flFrame)
//				{
//					float f = (flFrame - prevframe) / (pmove->endframe - prevframe);
//
//					float d = pmove->v0 * f + 0.5 * (pmove->v1 - pmove->v0) * f * f;
//
//					vecPos = vecPos + d * pmove->vector;
//					vecAngle.y = vecAngle.y * (1 - f) + pmove->angle * f;
//					if (iLoops != 0)
//					{
//						mstudiomovement_t* pmove = panim->pMovement(panim->nummovements - 1);
//						vecPos = vecPos + iLoops * pmove->position;
//						vecAngle.y = vecAngle.y + iLoops * pmove->angle;
//					}
//					return true;
//				}
//				else
//				{
//					prevframe = pmove->endframe;
//					vecPos = pmove->position;
//					vecAngle.y = pmove->angle;
//				}
//			}
//		}
//
//		return false;
//	}
//
//	bool Studio_AnimMovement(mstudioanimdesc_t* panim, float flCycleFrom, float flCycleTo, Vector& deltaPos, QAngle& deltaAngle)
//	{
//		if (panim->nummovements == 0 || (panim->flags & STUDIO_FRAMEMOVEMENT) == 0)
//			return false;
//
//		Vector startPos;
//		QAngle startA;
//		Studio_AnimPosition(panim, flCycleFrom, startPos, startA);
//
//		Vector endPos;
//		QAngle endA;
//		Studio_AnimPosition(panim, flCycleTo, endPos, endA);
//
//		Vector tmp = endPos - startPos;
//		deltaAngle.y = NormalizeAngle(endA.y, startA.y); // unsure why this was added
//		VectorYawRotate(tmp, -startA.y, deltaPos);
//
//		return true;
//	}
//
//	// this hasn't changed as far as I can tell
//	//bool Studio_SeqMovement(const CStudioHdr* pStudioHdr, int iSequence, float flCycleFrom, float flCycleTo, const float poseParameter[], Vector& deltaPos, QAngle& deltaAngles)
//	//{
//	//	mstudioanimdesc_t* panim[4];
//	//	float	weight[4];
//
//	//	mstudioseqdesc_t& seqdesc = ((CStudioHdr*)pStudioHdr)->pSeqdesc(iSequence);
//
//	//	Studio_SeqAnims(pStudioHdr, seqdesc, iSequence, poseParameter, panim, weight);
//
//	//	deltaPos.Init();
//	//	deltaAngles.Init();
//
//	//	bool found = false;
//
//	//	for (int i = 0; i < 4; i++)
//	//	{
//	//		if (weight[i])
//	//		{
//	//			Vector localPos;
//	//			QAngle localAngles;
//
//	//			localPos.Init();
//	//			localAngles.Init();
//
//	//			if (Studio_AnimMovement(panim[i], flCycleFrom, flCycleTo, localPos, localAngles))
//	//			{
//	//				found = true;
//	//				deltaPos = deltaPos + localPos * weight[i];
//	//				// FIXME: this makes no sense
//	//				deltaAngles = deltaAngles + localAngles * weight[i];
//	//			}
//	//			else if (!(panim[i]->flags & STUDIO_DELTA) && panim[i]->nummovements == 0 && seqdesc.weight(0) > 0.0)
//	//			{
//	//				found = true;
//	//			}
//	//		}
//	//	}
//	//	return found;
//	//}
//
//
//}
//
//namespace r5
//{
//	namespace v8
//	{
//		// unsure how this was determined
//		char s_AnimSeekLUT[60]{ 1, 15, 16, 2, 7, 8, 2, 15, 0, 3, 15, 0, 4, 15, 0, 5,
//								15, 0, 6, 15, 0, 07, 15, 0, 2, 15, 3, 15, 2, 4, 15,
//								2, 5, 15, 2, 6, 15, 2, 7, 15, 2, 2, 15, 4, 3, 15, 4,
//								4, 15, 04, 5, 15, 4, 6, 15, 4, 7, 15, 4 };
//
//		#define GetAnimValueOffset(panimvalue) s_AnimSeekLUT[3 * panimvalue->num.valid] + ((s_AnimSeekLUT[3 * panimvalue->num.valid + 1] + panimvalue->num.total * s_AnimSeekLUT[3 * panimvalue->num.valid + 2]) / 16) // this works I guess? super duper weird
//
//		float ExtractValue(mstudioanimvalue_t* panimvalue, int frame, float scale)
//		{
//			int k = frame;
//			float out;
//
//			if (!panimvalue->num.valid)
//				return panimvalue[k + 1].value * scale; // v1 = panimvalue[k + 1].value * scale;
//
//			if (panimvalue->num.valid == 1)
//			{
//				out = panimvalue[1].value;
//
//				if (k > 0)
//				{
//					out += (panimvalue[1].num.total + k) * scale;
//				}
//			}
//			else
//			{
//				int v9 = (panimvalue->num.valid - 2) / 6;
//				int v11 = (panimvalue->num.valid - 2) % 6;
//
//				if (v11)
//				{
//					if (v11 >= 4)
//					{
//
//					}
//				}
//				if (v9)
//				{
//					
//				}
//			}
//
//			return out;
//		}
//
//		//-----------------------------------------------------------------------------
//		// Purpose: return a sub frame rotation for a single bone
//		//-----------------------------------------------------------------------------
//		void ExtractAnimValue(int frame, mstudioanimvalue_t* panimvalue, float scale, float& v1, float& v2)
//		{
//			/*if (!panimvalue)
//			{
//				v1 = v2 = 0;
//				return;
//			}*/
//
//			// Avoids a crash reading off the end of the data
//			// There is probably a better long-term solution; Ken is going to look into it.
//			/*if ((panimvalue->num.total == 1) && (panimvalue->num.valid == 1))
//			{
//				v1 = v2 = panimvalue[1].value * scale;
//				return;
//			}*/
//
//			int k = frame;
//			
//			// find the data list that has the frame
//			while (panimvalue->num.total <= k)
//			{
//				k -= panimvalue->num.total;
//				//panimvalue += panimvalue->num.valid + 1;
//				panimvalue += GetAnimValueOffset(panimvalue); // this works I guess? super duper weird
//
//				//if (panimvalue->num.total == 0)
//				//{
//				//	assert(0); // running off the end of the animation stream is bad
//				//	v1 = v2 = 0;
//				//	return;
//				//}
//			}
//
//			// v11 = animvalue->num.total - 1;
//
//			if (k >= panimvalue->num.total - 1)
//			{
//				v1 = ExtractValue(panimvalue, k, scale);
//				v2 = ExtractValue(&panimvalue[GetAnimValueOffset(panimvalue)], 0, scale);
//			}
//			else
//			{
//
//			}
//
//
//
//
//			if (panimvalue->num.valid > k)
//			{
//				// has valid animation data
//				v1 = panimvalue[k + 1].value * scale;
//
//				if (panimvalue->num.valid > k + 1)
//				{
//					// has valid animation blend data
//					v2 = panimvalue[k + 2].value * scale;
//				}
//				else
//				{
//					if (panimvalue->num.total > k + 1)
//					{
//						// data repeats, no blend
//						v2 = v1;
//					}
//					else
//					{
//						// pull blend from first data block in next list
//						v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
//					}
//				}
//			}
//			else
//			{
//				// get last valid data block
//				v1 = panimvalue[panimvalue->num.valid].value * scale;
//				if (panimvalue->num.total > k + 1)
//				{
//					// data repeats, no blend
//					v2 = v1;
//				}
//				else
//				{
//					// pull blend from first data block in next list
//					v2 = panimvalue[panimvalue->num.valid + 2].value * scale;
//				}
//			}
//		}
//	}
//}
//
