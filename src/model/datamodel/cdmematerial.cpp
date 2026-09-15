#include <pch.h>

#include <model/datamodel/cdmematerial.h>

// take a look here later, wondering about different name vs attribute
CDmeMaterial::CDmeMaterial(CDataModel* const dataModelIn, const char* const materialPath, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_MATERIAL, materialPath, setIn, 1ull, truncateName), mtlName(element->GetName())
{
	attributes->AddAttribute(dataModel, "mtlName", DmAttributeType_t::AT_STRING, mtlName, false);
}