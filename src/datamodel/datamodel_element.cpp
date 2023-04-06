#include "../datamodel/datamodel_element.h"
#include "../datamodel/datamodel.h"


//===============
// CDmeTransform
//===============

CDmeTransform::CDmeTransform(CDataModel* datamodel, const char* name, int set)
{
	pBaseDataModel = datamodel;
	//pBaseDataModel->AddElement("DmeTransform", name, set, pElement, pAttributes);
	pElement = pBaseDataModel->AddElement("DmeTransform", name, set);
	pAttributes = pBaseDataModel->GetAttributeList(pElement->elementIndex);

	// initalize these two as we don't set in class definition
	position.Init();
	orientation.Init();

	orientation.w = 1.0f; // amogus

	pBaseDataModel->AddAttribute(pAttributes, "position", DmAttributeType_t::AT_VECTOR3, &position);
	pBaseDataModel->AddAttribute(pAttributes, "orientation", DmAttributeType_t::AT_QUATERNION, &orientation);
	pBaseDataModel->AddAttribute(pAttributes, "scale", DmAttributeType_t::AT_VECTOR3, &scale);

	elementName = name;
	elementSet = set;
	elementIndex = pElement->elementIndex;
}


//===========
// CDmeModel
//===========

CDmeModel::CDmeModel(CDataModel* datamodel)
{
	pBaseDataModel = datamodel;

	pElement = pBaseDataModel->AddElement("DmeModel", "mshop datamodel export", 0);
	pAttributes = pBaseDataModel->GetAttributeList(pElement->elementIndex);

	transform = new CDmeTransform(datamodel, "unnamed", 0);
}

CDmeModel::~CDmeModel()
{
	delete transform;
}

// add to base
void CDmeModel::AddAsModel()
{
	pBaseDataModel->AddAttribute(pBaseDataModel->pRootAttributeList(), "model", DmAttributeType_t::AT_ELEMENT, &pElement->elementIndex);
}

void CDmeModel::AddAsSkeleton()
{
	pBaseDataModel->AddAttribute(pBaseDataModel->pRootAttributeList(), "skeleton", DmAttributeType_t::AT_ELEMENT, &pElement->elementIndex);
}

// attributes
void CDmeModel::AddAttrTransform()
{
	pBaseDataModel->AddAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, transform->pIndex());
}

void CDmeModel::AddAttrVisible(bool isVisible)
{
	modelVisible = isVisible;
	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &modelVisible);
}

void CDmeModel::AddAttrUpAxis(ModelAxis_t axis)
{
	upAxis = modelAxis[axis];
	modelUpAxis = pBaseDataModel->pStringDict()->AddToStringDict(upAxis);
	pBaseDataModel->AddAttribute(pAttributes, "upAxis", DmAttributeType_t::AT_STRING, &modelUpAxis);
}