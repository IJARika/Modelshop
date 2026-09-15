#pragma once

#include <model/datamodel/cdmechannelsclip.h>

class CDmeAnimationList : public CDmeBase
{
public:
	CDmeAnimationList(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false);
	virtual ~CDmeAnimationList()
	{

	}

	CDmeChannelsClip* const AddAnimation(const char* const nameIn, const int setIn, const int frameRateIn, const bool truncateName = false);
	inline CDmeChannelsClip* const GetAnimation(const int animation) const { return animationElements.at(animation); }

private:
	std::vector<DmElementHash> animations;
	std::vector<CDmeChannelsClip*> animationElements;
};