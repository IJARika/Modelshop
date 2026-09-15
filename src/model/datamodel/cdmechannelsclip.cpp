#include <pch.h>

#include <model/datamodel/cdmechannelsclip.h>

CDmeChannelsClip::CDmeChannelsClip(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const int frameRateIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_CHANNELSCLIP, nameIn, setIn, 8ull, truncateName),
	timeFrame(dmElementNull), color(0, 0), text(""), mute(false), displayScale(1.0f), frameRate(frameRateIn)
{
	attributes->AddAttribute(dataModel, "timeFrame", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "color", DmAttributeType_t::AT_COLOR, color.AsUint32());
	attributes->AddAttribute(dataModel, "text", DmAttributeType_t::AT_STRING, text, false);
	attributes->AddAttribute(dataModel, "mute", DmAttributeType_t::AT_BOOL, mute);
	attributes->AddAttribute(dataModel, "trackGroups", DmAttributeType_t::AT_ELEMENT_ARRAY, &trackGroups, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "displayScale", DmAttributeType_t::AT_FLOAT, displayScale);
	attributes->AddAttribute(dataModel, "channels", DmAttributeType_t::AT_ELEMENT_ARRAY, &channels, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "frameRate", DmAttributeType_t::AT_INT, frameRate);
}

CDmeChannel* const CDmeChannelsClip::AddChannel(const char* const nameIn, const eDmeChannelType typeIn, const bool truncateName)
{
	const char* const channelName = CDmeChannel::GetNameForChannel(dataModel, nameIn, typeIn);

	CDmeChannel* const channel = new CDmeChannel(dataModel, channelName, 0, typeIn, nameIn, truncateName);

	channelElements.push_back(channel);
	channels.push_back(channel->GetHash());

	return channel;
}

CDmeTimeFrame* const CDmeChannelsClip::AddTimeFrame(const char* const nameIn, const int setIn, const float startIn, const float durIn, const bool truncateName)
{
	timeFrameElement = new CDmeTimeFrame(dataModel, nameIn, setIn, startIn, durIn, truncateName);
	timeFrame = timeFrameElement->GetHash();

	attributes->EditAttribute(dataModel, "timeFrame", DmAttributeType_t::AT_ELEMENT, timeFrame);

	return timeFrameElement;
}