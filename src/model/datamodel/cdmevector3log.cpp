#include <pch.h>

#include <model/datamodel/cdmevector3log.h>

CDmeVector3Log::CDmeVector3Log(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_VECTOR3LOG, nameIn, setIn, 7ull, truncateName),
	curveinfo(dmElementNull), curveInfoElement(nullptr), usedefaultvalue(false), defaultvalue(0.0f, 0.0f, 0.0f)
{
	attributes->AddAttribute(dataModel, "layers", DmAttributeType_t::AT_ELEMENT_ARRAY, &layers, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "curveinfo", DmAttributeType_t::AT_ELEMENT, curveinfo);
	attributes->AddAttribute(dataModel, "usedefaultvalue", DmAttributeType_t::AT_BOOL, usedefaultvalue);
	attributes->AddAttribute(dataModel, "defaultvalue", DmAttributeType_t::AT_VECTOR3, &defaultvalue, false);
	attributes->AddAttribute(dataModel, "bookmarksX", DmAttributeType_t::AT_TIME_ARRAY, &bookmarksX, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "bookmarksY", DmAttributeType_t::AT_TIME_ARRAY, &bookmarksY, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "bookmarksZ", DmAttributeType_t::AT_TIME_ARRAY, &bookmarksZ, DmAttribute::ATT_VAL_VEC);
}

CDmeVector3LogLayer* const CDmeVector3Log::AddLayer(const char* const nameIn, const size_t numFrames, const bool truncateName)
{
	CDmeVector3LogLayer* const layer = new CDmeVector3LogLayer(dataModel, nameIn, set, numFrames, truncateName);

	layerElements.push_back(layer);
	layers.push_back(layer->GetHash());

	return layer;
}