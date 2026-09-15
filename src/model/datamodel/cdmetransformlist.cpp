#include <pch.h>

#include <model/datamodel/cdmetransformlist.h>

CDmeTransformList::CDmeTransformList(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numJoints, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_TRANSFORMLIST, nameIn, setIn, 1ull, truncateName)
{
	transformElements.reserve(numJoints);
	transforms.reserve(numJoints);

	attributes->AddAttribute(dataModel, "transforms", DmAttributeType_t::AT_ELEMENT_ARRAY, &transforms, DmAttribute::ATT_VAL_VEC);
}

void CDmeTransformList::AddTransform(const char* const nameIn, const int setIn, const Vector& position, const Quaternion& rotation, const Vector& scale, const bool truncateName)
{
	CDmeTransform* const transform = new CDmeTransform(dataModel, nameIn, setIn, position, rotation, scale, truncateName); // set will be 1 here

	transformElements.push_back(transform);
	transforms.push_back(transform->GetHash());
}
