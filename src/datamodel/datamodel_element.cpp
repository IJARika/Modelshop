#include "../datamodel/datamodel_element.h"
#include "../datamodel/datamodel.h"
#include "../utils.h"


//==========
// CDmeBase
//==========

inline void CDmeBase::SetupElementBase(CDataModel* pDataModel, const char* type, const char* name, const int set)
{
	pBaseDataModel = pDataModel;
	pElement = pBaseDataModel->AddElement(type, name, set);
	pAttributes = pBaseDataModel->GetAttributeList(pElement->elementIndex);

	elementName = name;
	elementSet = set;
	elementIndex = pElement->elementIndex;
}


//===============
// CDmeTransform
//===============

CDmeTransform::CDmeTransform(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeTransform", name, set);

	// initalize these two as we don't set in class definition
	position.Init();
	orientation.Init();
	scale.Init();

	orientation.w = 1.0f; // amogus
	scale += 1.0f;

	pBaseDataModel->AddAttribute(pAttributes, "position", DmAttributeType_t::AT_VECTOR3, &position);
	pBaseDataModel->AddAttribute(pAttributes, "orientation", DmAttributeType_t::AT_QUATERNION, &orientation);
	pBaseDataModel->AddAttribute(pAttributes, "scale", DmAttributeType_t::AT_VECTOR3, &scale);
}


//===================
// CDmeTransformList
//===================

CDmeTransformList::CDmeTransformList(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeTransformList", name, set);

	pBaseDataModel->AddAttribute(pAttributes, "transforms", DmAttributeType_t::AT_ELEMENT_ARRAY, &transforms);
}

CDmeTransformList::~CDmeTransformList()
{
	for (auto& transform : transformElements)
	{
		delete transform;
	}
}

void CDmeTransformList::AddTransform(const char* name, const int set, Vector& position, Quaternion& rotation, Vector& scale)
{
	CDmeTransform* newTransform = new CDmeTransform(pBaseDataModel, name, set); // set will be 1 here

	newTransform->position = position;
	newTransform->orientation = rotation;
	newTransform->scale = scale;

	transformElements.push_back(newTransform);
	transforms.push_back(newTransform->pIndex());
}


//===========
// CDmeJoint
//===========

CDmeJoint::CDmeJoint(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeJoint", name, set);

	transform = new CDmeTransform(pDataModel, name, 0);

	pBaseDataModel->AddAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, transform->pIndex());
	pBaseDataModel->AddAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, &shape);
	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &visible);
	pBaseDataModel->AddAttribute(pAttributes, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children);
	pBaseDataModel->AddAttribute(pAttributes, "_surfaceProp", DmAttributeType_t::AT_STRING, &_surfaceProp);
	pBaseDataModel->AddAttribute(pAttributes, "lockInfluenceWeights", DmAttributeType_t::AT_BOOL, &lockInfluenceWeights);
}

CDmeJoint::~CDmeJoint()
{
	delete transform;
}

void CDmeJoint::AddChildJoint(CDmeJoint* pChildJoint)
{
	childrenElements.push_back(pChildJoint);
	children.push_back(&pChildJoint->elementIndex);
}

void CDmeJoint::SetUpSurfaceProp(const char* surfaceProp)
{
	surfacePropStr = surfaceProp;
	_surfaceProp = pBaseDataModel->pStringDict()->AddToStringDict(surfacePropStr);
}


//===========
// CDmeModel
//===========

CDmeModel::CDmeModel(CDataModel* pDataModel)
{
	SetupElementBase(pDataModel, "DmeModel", "mshop datamodel export", 0);

	transform = new CDmeTransform(pDataModel, "unnamed", 0);

	pBaseDataModel->AddAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, transform->pIndex());
	pBaseDataModel->AddAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, &shape);
	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &visible);
	pBaseDataModel->AddAttribute(pAttributes, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children);
	pBaseDataModel->AddAttribute(pAttributes, "jointList", DmAttributeType_t::AT_ELEMENT_ARRAY, &jointList);
	pBaseDataModel->AddAttribute(pAttributes, "baseStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &baseStates);
	pBaseDataModel->AddAttribute(pAttributes, "upAxis", DmAttributeType_t::AT_STRING, &upAxis);
}

CDmeModel::~CDmeModel()
{
	delete transform;

	for (auto& joint : jointListElements)
	{
		delete joint;
	}
	
	for (auto& baseState : baseStateElements)
	{
		delete baseState;
	}
}

CDmeJoint* CDmeModel::AddJoint(const char* name, const int set)
{
	CDmeJoint* newJoint = new CDmeJoint(pBaseDataModel, name, set);

	jointListElements.push_back(newJoint);
	jointList.push_back(newJoint->pIndex());

	return newJoint;
}

void CDmeModel::AddBaseState(const char* name, const int set)
{
	CDmeTransformList* newBaseState = new CDmeTransformList(pBaseDataModel, name, set); // set will be 1 here

	baseStateElements.push_back(newBaseState);
	baseStates.push_back(newBaseState->pIndex());
}

void CDmeModel::SetUpAxis(ModelAxis_t axis)
{
	upAxisStr = modelAxis[axis];
	upAxis = pBaseDataModel->pStringDict()->AddToStringDict(upAxisStr);
}

void CDmeModel::AddChildBaseJoint()
{
	if (!jointList.size())
	{
		Error("DmeModel %s does not contain any joints!!!\n", elementName);
	}

	children.push_back(jointList.at(0));
	AddAsSkeleton();
}