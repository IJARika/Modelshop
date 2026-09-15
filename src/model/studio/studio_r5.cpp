#include <pch.h>

#include <model/studio/studio_r5.h>
#include <model/studio/studio_r5_v12.h>
#include <model/studio/studio_r5_v16.h>

namespace r5
{
	char* mstudioanimdesc_t::pAnimdata(int* piFrame) const
	{
		int index = animindex;
		int section = 0;

		if (sectionframes != 0)
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

			index = pSection(section)->animindex;
		}

		return reinterpret_cast<char*>((char*)this + index);
	}

	// same as r2
	mstudio_rle_anim_t* mstudioanimdesc_t::pAnim(int* piFrame, const int boneCount) const
	{
		return reinterpret_cast<mstudio_rle_anim_t*>(&pAnimdata(piFrame)[IALIGN2(ANIM_BONEFLAG_SIZE_4(boneCount))]); // round up, then add for truncation. data block is aligned to two bytes.
	}
}

namespace r5_121
{
	char* mstudioanimdesc_t::pAnim(int* piFrame) const
	{
		int index = animindex;
		int section = 0;

		if (sectionframes != 0)
		{
			if (*piFrame >= sectionstallframes)
			{
				if (numframes > sectionframes && *piFrame == numframes - 1)
				{
					*piFrame = 0;
					section = (numframes - sectionstallframes - 1) / sectionframes + 2;
				}
				else
				{
					section = (*piFrame - sectionstallframes) / sectionframes + 1;
					*piFrame = *piFrame - sectionframes * ((*piFrame - sectionstallframes) / sectionframes) - sectionstallframes;
				}

				index = pSection(section)->animindex;
			}

			if (pSection(section)->isExternal) // checks if it is external
			{
				if (sectionDataExternal) // the rpak ptr is likely converted into a raw offset on load into memory and placed here.
				{
					char* panim = (sectionDataExternal + index);

					if (unk1)
					{
						/*AcquireSRWLockExclusive(&stru_14776DB20);
						v17 = &byte_1496DA0C0[16 * v16];
						if (*((_DWORD*)v17 + 2) != dword_1496DA0BC)
						{
							v18 = *v17 == 1;
							*((_DWORD*)v17 + 2) = dword_1496DA0BC;
							if (v18)
							{
								word_1498BA100[qword_1498C6100] = v16;
								*((_WORD*)v17 + 2) = qword_1498C6100++;
							}
						}
						ReleaseSRWLockExclusive(&stru_14776DB20);*/
					}

					return panim;
				}

				index = pSection(0)->animindex;
				*piFrame = sectionstallframes - 1; // gets set to last frame of 'static'/'stall' section if the external data offset has not been cached(?)
			}
		}

		return ((char*)this + index);
	}
}

namespace r5_160
{
	char* mstudioanimdesc_t::pAnim(int* piFrame) const
	{
		int index = animindex;
		int section = 0;

		if (sectionframes != 0)
		{
			if (*piFrame >= sectionstallframes)
			{
				if (numframes > sectionframes && *piFrame == numframes - 1)
				{
					*piFrame = 0;
					section = (numframes - sectionstallframes - 1) / sectionframes + 2;
				}
				else
				{
					section = (*piFrame - sectionstallframes) / sectionframes + 1;
					*piFrame = *piFrame - sectionframes * ((*piFrame - sectionstallframes) / sectionframes) - sectionstallframes;
				}

				index = pSection(section)->Index();
			}

			if (pSection(section)->isExternal()) // checks if it is external
			{
				if (sectionDataExternal) // the rpak ptr is likely converted into a raw offset on load into memory and placed here.
				{
					char* panim = (sectionDataExternal + index);

					if (unk1)
					{
						//int64_t v18 = unk1 >> 6;
						//_InterlockedOr64(&qword_14D507540[v18 + 276635], 1i64 << (v17 & 0x3F));
						//_InterlockedOr64(&qword_14D507540[(v18 >> 6) + 277019], 1i64 << (v18 & 0x3F));
					}

					return panim;
				}

				index = pSection(0)->Index(); // stall if external data is not loaded
				*piFrame = sectionstallframes - 1; // gets set to last frame of 'static'/'stall' section if the external data offset has not been cached(?)
			}
		}

		return ((char*)this + index);
	}
}

namespace r5_190
{
	void AnimQuat32::Unpack(Quaternion& quat, const AnimQuat32 packedQuat, const AxisFixup_t* const axisFixup)
	{
		const int scaleFac = packedQuat.scaleFactor;

		const float scaleComponent = scaleFac ? 0.011048543f : 0.0055242716f;
		const float scaleFixup = static_cast<float>(1 << scaleFac) * 0.000021924432f;

		const float axis0 = ((static_cast<float>(packedQuat.value0) + 0.5f) * scaleComponent) + (static_cast<float>(axisFixup->adjustment[0]) * scaleFixup);
		const float axis1 = ((static_cast<float>(packedQuat.value1) + 0.5f) * scaleComponent) + (static_cast<float>(axisFixup->adjustment[1]) * scaleFixup);
		const float axis2 = ((static_cast<float>(packedQuat.value2) + 0.5f) * scaleComponent) + (static_cast<float>(axisFixup->adjustment[2]) * scaleFixup);

		float droppedComponent = 0.0f;

		const float dotProduct = (axis0 * axis0) + (axis1 * axis1) + (axis2 * axis2);

		if (dotProduct < 1.0f)
		{
			const float dprem = 1.0f - dotProduct;
			if ((1.0f - dotProduct) < 0.0)
				droppedComponent = sqrtf(dprem);
			else
				droppedComponent = FastSqrtFast(dprem); // fsqrt
		}

		switch (packedQuat.droppedAxis)
		{
		case 0:
		{
			quat.x = droppedComponent;
			quat.y = axis0;
			quat.z = axis1;
			quat.w = axis2;

			break;
		}
		case 1:
		{
			quat.x = axis0;
			quat.y = droppedComponent;
			quat.z = axis1;
			quat.w = axis2;

			break;
		}
		case 2:
		{
			quat.x = axis0;
			quat.y = axis1;
			quat.z = droppedComponent;
			quat.w = axis2;

			break;
		}
		case 3:
		{
			quat.x = axis0;
			quat.y = axis1;
			quat.z = axis2;
			quat.w = droppedComponent;

			break;
		}
		}
	}

	void AnimPos64::Unpack(Vector& pos, const AnimPos64 packedPos, const AxisFixup_t* const axisFixup)
	{
		const float scaleFac = static_cast<float>(packedPos.scaleFactor + 1);

		pos.x = ((static_cast<float>(axisFixup->adjustment[0]) / 100.0f) + static_cast<float>(packedPos.values[0])) * scaleFac;
		pos.y = ((static_cast<float>(axisFixup->adjustment[1]) / 100.0f) + static_cast<float>(packedPos.values[1])) * scaleFac;
		pos.z = ((static_cast<float>(axisFixup->adjustment[2]) / 100.0f) + static_cast<float>(packedPos.values[2])) * scaleFac;
	}
}