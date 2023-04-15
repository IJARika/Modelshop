#include "../datamodel/datamodel_element.h"
#include "../datamodel/datamodel.h"
#include "../utils.h"


//==========
// CDmeBase
//==========


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


//==============
// CDmeMaterial
//==============

CDmeMaterial::CDmeMaterial(CDataModel* pDataModel, const char* materialPath, const int set)
{
	SetupElementBase(pDataModel, "DmeMaterial", GET_FILE_STEM(materialPath).c_str(), set);

	mtlName = pBaseDataModel->pStringDict()->AddToStringDict(materialPath);

	pBaseDataModel->AddAttribute(pAttributes, "mtlName", DmAttributeType_t::AT_STRING, &mtlName);
}


//=============
// CDmeFaceSet
//=============

CDmeFaceSet::CDmeFaceSet(CDataModel* pDataModel, const char* materialPath, const int set)
{
	SetupElementBase(pDataModel, "DmeFaceSet", GET_FILE_STEM(materialPath).c_str(), set);

	material = new CDmeMaterial(pDataModel, materialPath, set);

	pBaseDataModel->AddAttribute(pAttributes, "material", DmAttributeType_t::AT_ELEMENT, material->pIndex());
	pBaseDataModel->AddAttribute(pAttributes, "faces", DmAttributeType_t::AT_INT_ARRAY, &faces);
}

CDmeFaceSet::~CDmeFaceSet()
{
	delete material;
}

void CDmeFaceSet::AddFace(DmFace_t& face)
{
	for (int idx = 0; idx < 4; idx++)
	{
		faceIndices.push_back(face.indice[idx]);
		faces.push_back(&faceIndices.at(faces.size() - 1));
	}
}


//================
// CDmeVertexData
//================


//==========
// CDmeMesh
//==========

CDmeMesh::CDmeMesh(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeMesh", name, set);

	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &visible);
	pBaseDataModel->AddAttribute(pAttributes, "bindState", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "currentState", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "baseStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &baseStates);
	pBaseDataModel->AddAttribute(pAttributes, "deltaStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &deltaStates);
	pBaseDataModel->AddAttribute(pAttributes, "deltaStateWeights", DmAttributeType_t::AT_VECTOR2_ARRAY, &deltaStateWeights);
	pBaseDataModel->AddAttribute(pAttributes, "deltaStateWeightsLagged", DmAttributeType_t::AT_VECTOR2_ARRAY, &deltaStateWeightsLagged);
}

CDmeMesh::~CDmeMesh()
{
	if (currentState != nullptr)
	{
		delete currentState;
	}

	for (auto& baseState : baseStateElements)
	{
		delete baseState;
	}
	
	for (auto& deltaState : deltaStateElements)
	{
		delete deltaState;
	}

	for (auto& faceSet : faceSetElements)
	{
		delete faceSet;
	}
}

void CDmeMesh::AddVertexData(CDmeVertexData* pVertexData)
{
	currentState = pVertexData;

	baseStates.push_back(pVertexData->pIndex());
	baseStateElements.push_back(pVertexData);
}

void CDmeMesh::AddFaceSet(CDmeFaceSet* pFaceSet)
{
	//CDmeFaceSet* newFaceSet = new CDmeFaceSet(pBaseDataModel, name, set);

	faceSets.push_back(pFaceSet->pIndex());
	faceSetElements.push_back(pFaceSet);
}


//=========
// CDmeDag
//=========

CDmeDag::CDmeDag(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeDag", name, set);

	pBaseDataModel->AddAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &visible);
	pBaseDataModel->AddAttribute(pAttributes, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children);
}

CDmeDag::~CDmeDag()
{
	if (transform != nullptr)
	{
		delete transform;
	}
	
	if (shape != nullptr)
	{
		delete shape;
	}
}

void CDmeDag::AddTransform(const char* name, const int set)
{
	transform = new CDmeTransform(pBaseDataModel, name, set);

	pBaseDataModel->EditAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, transform->pIndex());
}

void CDmeDag::AddShape(const char* name, const int set)
{
	shape = new CDmeMesh(pBaseDataModel, name, set);

	pBaseDataModel->EditAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, shape->pIndex());
}


//===========
// CDmeJoint
//===========

CDmeJoint::CDmeJoint(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeJoint", name, set);

	pBaseDataModel->AddAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &visible);
	pBaseDataModel->AddAttribute(pAttributes, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children);
	pBaseDataModel->AddAttribute(pAttributes, "_surfaceProp", DmAttributeType_t::AT_STRING, &_surfaceProp);
	pBaseDataModel->AddAttribute(pAttributes, "lockInfluenceWeights", DmAttributeType_t::AT_BOOL, &lockInfluenceWeights);
}

CDmeJoint::~CDmeJoint()
{
	if (transform != nullptr)
	{
		delete transform;
	}

	if (shape != nullptr)
	{
		delete shape;
	}
}

void CDmeJoint::AddTransform(const char* name, const int set)
{
	transform = new CDmeTransform(pBaseDataModel, name, set);

	pBaseDataModel->EditAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, transform->pIndex());
}

void CDmeJoint::AddShape(const char* name, const int set)
{
	shape = new CDmeMesh(pBaseDataModel, name, set);

	pBaseDataModel->EditAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, shape->pIndex());
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
	SetupElementBase(pDataModel, "DmeModel", "mshop dmxport", 0);

	pBaseDataModel->AddAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, &stubIndex);
	pBaseDataModel->AddAttribute(pAttributes, "visible", DmAttributeType_t::AT_BOOL, &visible);
	pBaseDataModel->AddAttribute(pAttributes, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children);
	pBaseDataModel->AddAttribute(pAttributes, "jointList", DmAttributeType_t::AT_ELEMENT_ARRAY, &jointList);
	pBaseDataModel->AddAttribute(pAttributes, "baseStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &baseStates);
	pBaseDataModel->AddAttribute(pAttributes, "upAxis", DmAttributeType_t::AT_STRING, &upAxis);
}

CDmeModel::~CDmeModel()
{
	// surely this can be done better???
	if (transform != nullptr)
	{
		delete transform;
	}

	if (shape != nullptr)
	{
		delete shape;
	}

	for (auto& joint : jointListElements)
	{
		delete joint;
	}
	
	for (auto& baseState : baseStateElements)
	{
		delete baseState;
	}

	if (pChildDag)
	{
		delete pChildDag;
	}
}

void CDmeModel::AddTransform(const char* name, const int set)
{
	transform = new CDmeTransform(pBaseDataModel, name, set);

	pBaseDataModel->EditAttribute(pAttributes, "transform", DmAttributeType_t::AT_ELEMENT, transform->pIndex());
}

void CDmeModel::AddShape(const char* name, const int set)
{
	shape = new CDmeMesh(pBaseDataModel, name, set);

	pBaseDataModel->EditAttribute(pAttributes, "shape", DmAttributeType_t::AT_ELEMENT, shape->pIndex());
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

void CDmeModel::AddChildJoint()
{
	if (!jointList.size())
	{
		Error("DmeModel %s does not contain any joints!!!\n", elementName);
	}

	children.push_back(jointList.at(0));
	pChildJoint = jointListElements.at(0);
	AddAsSkeleton();
}

void CDmeModel::AddChildDag(const char* name, const int set)
{
	CDmeDag* newDag = new CDmeDag(pBaseDataModel, name, set);

	children.push_back(newDag->pIndex());
	pChildDag = newDag;
	AddAsModel();
}