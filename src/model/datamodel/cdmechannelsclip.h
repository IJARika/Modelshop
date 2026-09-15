#pragma once

#include <model/datamodel/cdmetimeframe.h>
#include <model/datamodel/cdmechannel.h>

class CDmeChannelsClip : public CDmeBase
{
public:
	CDmeChannelsClip(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const int frameRateIn, const bool truncateName = false);
	virtual ~CDmeChannelsClip()
	{

	}

	CDmeChannel* const AddChannel(const char* const nameIn, const eDmeChannelType typeIn, const bool truncateName = false);
	CDmeTimeFrame* const AddTimeFrame(const char* const nameIn, const int setIn, const float startIn, const float durIn, const bool truncateName = false);

	inline CDmeChannel* const GetChannel(const int channel) const { return channelElements.at(channel); }

private:
	std::vector<DmElementHash> trackGroups;
	std::vector<CDmeBase*> trackGroupElements;

	std::vector<DmElementHash> channels;
	std::vector<CDmeChannel*> channelElements;

	CDmeTimeFrame* timeFrameElement;
	DmElementHash timeFrame;

	const char* text;

	int frameRate;
	Color32 color;
	float displayScale;
	
	bool mute;
};