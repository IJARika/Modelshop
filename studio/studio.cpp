#include "studio.h"

namespace r2
{
	mstudio_rle_anim_t* mstudioanimdesc_t::pAnim(int* piFrame) const
	{
		mstudio_rle_anim_t* panim = nullptr;

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

		return panim = reinterpret_cast<mstudio_rle_anim_t*>((char*)this + index);
	}
}

namespace r5
{
	namespace v8
	{
		// same as r2
		mstudio_rle_anim_t* mstudioanimdesc_t::pAnim(int* piFrame) const
		{
			mstudio_rle_anim_t* panim = nullptr;

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

			return panim = reinterpret_cast<mstudio_rle_anim_t*>((char*)this + index);
		}
	}

	namespace v121
	{
		char* mstudioanimdesc_t::pAnim(int* piFrame) const
		{
			char* panim = nullptr;

			int index = animindex;
			int section = 0;

			// may change this later
			if (!sectionframes)
				return ((char*)this + index);

			if (*piFrame >= sectionstaticframes)
			{
				if (numframes > sectionframes && *piFrame == numframes - 1)
				{
					*piFrame = 0;
					section = (numframes - sectionstaticframes - 1) / sectionframes + 2;
				}
				else
				{
					section = (*piFrame - sectionstaticframes) / sectionframes + 1;
					*piFrame -= *piFrame - sectionframes * ((*piFrame - sectionstaticframes) / sectionframes) - sectionstaticframes;
				}

				index = pSection(section)->animindex;
			}

			if (pSection(section)->isExternal) // checks if it is external
			{
				if (unk2) // the rpak ptr is likely converted into a raw offset on load into memory and placed here.
				{
					panim = ((char*)unk2 + index);

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
				*piFrame = sectionstaticframes - 1; // gets set to last frame of 'static'/'stall' section if the external data offset has not been cached(?)
			}

			return panim = ((char*)this + index);
		}
	}
}