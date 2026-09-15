#include <pch.h>

#include <model/studio/studio_r2.h>

namespace r2
{
	const mstudio_rle_anim_t* const mstudioanimdesc_t::pAnim(int* piFrame) const
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

		return reinterpret_cast<mstudio_rle_anim_t*>((char*)this + index);
	}

	// custom func defs
}