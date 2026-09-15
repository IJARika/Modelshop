#include <pch.h>

#include <model/datamodel/cdmequaternionlog.h>

CDmeQuaternionLog::CDmeQuaternionLog(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_QUATERNIONLOG, nameIn, setIn, 7ull, truncateName),
	curveinfo(dmElementNull), curveInfoElement(nullptr), usedefaultvalue(false), defaultvalue(0.0f, 0.0f, 0.0f, 1.0f)
{
	attributes->AddAttribute(dataModel, "layers", DmAttributeType_t::AT_ELEMENT_ARRAY, &layers, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "curveinfo", DmAttributeType_t::AT_ELEMENT, curveinfo);
	attributes->AddAttribute(dataModel, "usedefaultvalue", DmAttributeType_t::AT_BOOL, usedefaultvalue);
	attributes->AddAttribute(dataModel, "defaultvalue", DmAttributeType_t::AT_QUATERNION, &defaultvalue, false);
	attributes->AddAttribute(dataModel, "bookmarksX", DmAttributeType_t::AT_TIME_ARRAY, &bookmarksX, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "bookmarksY", DmAttributeType_t::AT_TIME_ARRAY, &bookmarksY, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "bookmarksZ", DmAttributeType_t::AT_TIME_ARRAY, &bookmarksZ, DmAttribute::ATT_VAL_VEC);
}

CDmeQuaternionLogLayer* const CDmeQuaternionLog::AddLayer(const char* const nameIn, const size_t numFrames, const bool truncateName)
{
	CDmeQuaternionLogLayer* const layer = new CDmeQuaternionLogLayer(dataModel, nameIn, set, numFrames, truncateName);

	layerElements.push_back(layer);
	layers.push_back(layer->GetHash());

	return layer;
}