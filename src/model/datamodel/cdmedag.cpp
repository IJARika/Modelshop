#include <pch.h>

#include <model/datamodel/cdmedag.h>

CDmeDag::CDmeDag(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_DAG, nameIn, setIn, 4ull, truncateName), transform(nullptr), shape(nullptr), visible(true)
{
	attributes->AddAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "visible", DmAttributeType_t::AT_BOOL, visible);
	attributes->AddAttribute(dataModel, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, children.data(), static_cast<int>(children.size()), false);
}

CDmeTransform* const CDmeDag::AddTransform(const char* const nameIn, const int setIn)
{
	transform = new CDmeTransform(dataModel, nameIn, setIn);

	attributes->EditAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, transform->GetHash());

	return transform;
}

CDmeMesh* const CDmeDag::AddShape(const char* nameIn, const int setIn, const bool truncateName)
{
	const char* const shapeName = CDmeMesh::GetNameForShape(dataModel, nameIn);

	shape = new CDmeMesh(dataModel, shapeName, setIn, truncateName);

	attributes->EditAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, shape->GetHash());

	return shape;
}
