#include <pch.h>

#include <model/datamodel/cdmetransform.h>

CDmeTransform::CDmeTransform(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_TRANSFORM, nameIn, setIn, 3ull, truncateName)
{
	// set up vectors, scale defaults to 1
	position.Init(0.0f, 0.0f, 0.0f);
	orientation.Init(0.0f, 0.0f, 0.0f, 1.0f);
	scale.Init(1.0f, 1.0f, 1.0f);

	attributes->AddAttribute(dataModel, "position", DmAttributeType_t::AT_VECTOR3, &position, false);
	attributes->AddAttribute(dataModel, "orientation", DmAttributeType_t::AT_QUATERNION, &orientation, false);
	attributes->AddAttribute(dataModel, "scale", DmAttributeType_t::AT_VECTOR3, &scale, false);
}

CDmeTransform::CDmeTransform(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const Vector& posIn, const Quaternion& rotIn, const Vector& scaleIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_TRANSFORM, nameIn, setIn, 3ull, truncateName),
	position(posIn), orientation(rotIn), scale(scaleIn)
{
	attributes->AddAttribute(dataModel, "position", DmAttributeType_t::AT_VECTOR3, &position, false);
	attributes->AddAttribute(dataModel, "orientation", DmAttributeType_t::AT_QUATERNION, &orientation, false);
	attributes->AddAttribute(dataModel, "scale", DmAttributeType_t::AT_VECTOR3, &scale, false);
}