#include <pch.h>

#include <model/datamodel/cdmetimeframe.h>

CDmeTimeFrame::CDmeTimeFrame(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const float startIn, const float durIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_TIMEFRAME, nameIn, setIn, truncateName),
	start(startIn), duration(durIn), offset(0), scale(1.0f)
{
	// just get the int from these
	// todo: custom write func for conversion on write?
	attributes->AddAttribute(dataModel, "start", DmAttributeType_t::AT_TIME, start.GetTenthsOfMS());
	attributes->AddAttribute(dataModel, "duration", DmAttributeType_t::AT_TIME, duration.GetTenthsOfMS());
	attributes->AddAttribute(dataModel, "offset", DmAttributeType_t::AT_TIME, offset.GetTenthsOfMS());
	attributes->AddAttribute(dataModel, "scale", DmAttributeType_t::AT_FLOAT, scale);
}