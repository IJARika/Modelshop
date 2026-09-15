#include <pch.h>

#include <model/datamodel/cdmequaternionloglayer.h>

CDmeQuaternionLogLayer::CDmeQuaternionLogLayer(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFrames, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_QUATERNIONLOGLAYER, nameIn, setIn, 4ull, truncateName),
	compressed(0)
{
	times.reserve(numFrames);
	values.reserve(numFrames);

	attributes->AddAttribute(dataModel, "times", DmAttributeType_t::AT_TIME_ARRAY, &times, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "curvetypes", DmAttributeType_t::AT_INT_ARRAY, &curvetypes, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "values", DmAttributeType_t::AT_QUATERNION_ARRAY, &values, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "compressed", DmAttributeType_t::AT_VOID, compressed);
}

void CDmeQuaternionLogLayer::AddFrame(const Quaternion& valueIn, const float timeIn)
{
	values.push_back(valueIn);
	times.push_back(DmeTime_t(timeIn));
}