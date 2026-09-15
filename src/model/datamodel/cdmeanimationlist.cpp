#include <pch.h>

#include <model/datamodel/cdmeanimationlist.h>

CDmeAnimationList::CDmeAnimationList(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_ANIMATIONLIST, nameIn, setIn, 2ull, truncateName)
{
	// add this to our dmx as an animation list on construction
	DmAttributeList* const rootAttributes = dataModel->GetRootAttributes();
	assertm(rootAttributes, "root attributes did not exist, empty model?");

	rootAttributes->AddAttribute(dataModel, "animationList", DmAttributeType_t::AT_ELEMENT, GetHash());

	attributes->AddAttribute(dataModel, "animations", DmAttributeType_t::AT_ELEMENT_ARRAY, &animations, DmAttribute::ATT_VAL_VEC);
}

CDmeChannelsClip* const CDmeAnimationList::AddAnimation(const char* nameIn, const int setIn, const int frameRateIn, const bool truncateName)
{
	CDmeChannelsClip* const channelclip = new CDmeChannelsClip(dataModel, nameIn, setIn, frameRateIn, truncateName);

	animations.push_back(channelclip->GetHash());
	animationElements.push_back(channelclip);

	return channelclip;
}