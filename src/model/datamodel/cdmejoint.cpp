#include <pch.h>

#include <model/datamodel/cdmejoint.h>

CDmeJoint::CDmeJoint(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false) : CDmeBase(dataModelIn, eDmElementClass::DME_JOINT, nameIn, setIn, 6ull, truncateName),
	transform(nullptr), shape(nullptr), visible(true), _surfaceProp(nullptr), lockInfluenceWeights(false)
{
	attributes->AddAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "visible", DmAttributeType_t::AT_BOOL, visible);
	attributes->AddAttribute(dataModel, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "_surfaceProp", DmAttributeType_t::AT_STRING, _surfaceProp, false);
	attributes->AddAttribute(dataModel, "lockInfluenceWeights", DmAttributeType_t::AT_BOOL, lockInfluenceWeights);
}

void CDmeJoint::AddTransform(const char* const nameIn, const int setIn, const Vector& posIn, const Quaternion& rotIn, const Vector& scaleIn, const bool truncateName)
{
	transform = new CDmeTransform(dataModel, nameIn, setIn, posIn, rotIn, scaleIn, truncateName);

	attributes->EditAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, transform->GetHash());
}

void CDmeJoint::AddShape(const char* const nameIn, const int setIn, const bool truncateName)
{
	const char* const shapeName = CDmeMesh::GetNameForShape(dataModel, nameIn);

	shape = new CDmeMesh(dataModel, shapeName, setIn, truncateName);

	attributes->EditAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, shape->GetHash());
}

void CDmeJoint::AddChildJoint(CDmeJoint* const childJointIn)
{
	childrenElements.push_back(childJointIn);
	children.push_back(childJointIn->GetHash());
}

void CDmeJoint::SetUpSurfaceProp(const char* const surfaceProp)
{
	_surfaceProp = surfaceProp;

	attributes->EditAttribute(dataModel, "_surfaceProp", DmAttributeType_t::AT_STRING, _surfaceProp, false);
}