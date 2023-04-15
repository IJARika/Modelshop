
#include "../math/vector.h"
#include "../datamodel/datamodel.h"
#include"../shareddefs.h"

#include "cdmebase.h"
#include "cdmevertexdata.h"

#pragma once


// TODO: add comments for what each attribute does

//==========
// CDmeBase
//==========

//===============
// CDmeTransform
//===============

class CDmeTransform : public CDmeBase
{
public:
	CDmeTransform(CDataModel* pDataModel, const char* name, const int set);

	Vector position;
	Quaternion orientation;
	Vector scale; // this is for respawn specific models
};


//===================
// CDmeTransformList
//===================

class CDmeTransformList : public CDmeBase
{
public:
	CDmeTransformList(CDataModel* pDataModel, const char* name, const int set);
	~CDmeTransformList();

	std::vector<int*> transforms;

	void AddTransform(const char* name, const int set, Vector& position, Quaternion& rotation, Vector& scale);

private:
	std::vector<CDmeTransform*> transformElements;
};


//==============
// CDmeMaterial
//==============

class CDmeMaterial : public CDmeBase
{
public:
	CDmeMaterial(CDataModel* pDataModel, const char* materialPath, const int set);

	int mtlName; // truncation required sometimes
};


//=============
// CDmeFaceSet
//=============

struct DmFace_t
{
	int indice[4] = { -1, -1, -1, -1 };
};

class CDmeFaceSet : public CDmeBase
{
public:
	CDmeFaceSet(CDataModel* pDataModel, const char* materialPath, const int set);
	~CDmeFaceSet();

	CDmeMaterial* material;
	std::vector<int*> faces; // quads

	void AddFace(DmFace_t& face);

private:
	std::vector<int> faceIndices; // hate this

};


//================
// CDmeVertexData
//================




//==========
// CDmeMesh
//==========

class CDmeMesh : public CDmeBase
{
public:
	CDmeMesh(CDataModel* pDataModel, const char* name, const int set);
	~CDmeMesh();

	bool visible = 1;
	CDmeVertexData* bindState; // need null element
	CDmeVertexData* currentState;
	std::vector<int*> baseStates;
	std::vector<int*> deltaStates;
	std::vector<int*> faceSets;
	std::vector<Vector2D*> deltaStateWeights;
	std::vector<Vector2D*> deltaStateWeightsLagged;

	// built externally for ease
	void AddVertexData(CDmeVertexData* pVertexData); // this is not the best way of doing this, todo: understand these attributes better
	void AddFaceSet(CDmeFaceSet* pFaceSet);

	CDmeFaceSet* pFaceSet(const int index) { return faceSetElements.at(index); }

private:
	std::vector<CDmeVertexData*> baseStateElements;
	std::vector<CDmeVertexData*> deltaStateElements;
	std::vector<CDmeFaceSet*> faceSetElements;

};


//=========
// CDmeDag
//=========

class CDmeDag : public CDmeBase
{
public:
	CDmeDag(CDataModel* pDataModel, const char* name, const int set);
	~CDmeDag();

	CDmeTransform* transform = nullptr;
	CDmeMesh* shape = nullptr;
	bool visible = 1;
	std::vector<int*> children;

	void AddTransform(const char* name, const int set);
	void AddShape(const char* name, const int set);

private:

};


//===========
// CDmeJoint
//===========

class CDmeJoint : public CDmeBase
{
public:
	CDmeJoint(CDataModel* pDataModel, const char* name, const int set);
	~CDmeJoint();

	CDmeTransform* transform = nullptr;
	CDmeMesh* shape = nullptr;
	bool visible = 1;
	std::vector<int*> children;
	int _surfaceProp = -1;
	bool lockInfluenceWeights = 0; // unsure what this is for as well

	void AddTransform(const char* name, const int set);
	void AddShape(const char* name, const int set);
	void AddChildJoint(CDmeJoint* pChildJoint);
	void SetUpSurfaceProp(const char* surfaceProp = "default");

private:
	std::vector<CDmeJoint*> childrenElements;

	const char* surfacePropStr = "default";
};


//===========
// CDmeModel
//===========

enum ModelAxis_t : char
{
	AXIS_X = 0,
	AXIS_X_NEG,
	AXIS_Y,
	AXIS_Y_NEG,
	AXIS_Z,
	AXIS_Z_NEG,
	_COUNT
};

static const char* modelAxis[ModelAxis_t::_COUNT]
{
	"X",
	"-X",
	"Y",
	"-Y",
	"Z",
	"-Z"
};

class CDmeModel : public CDmeBase
{
public:
	CDmeModel(CDataModel* pDataModel);
	~CDmeModel();

	// add to base DataModel
	inline void AddAsModel() { pBaseDataModel->AddAttribute(pBaseDataModel->pRootAttributeList(), "model", DmAttributeType_t::AT_ELEMENT, &pElement->elementIndex); };
	inline void AddAsSkeleton() { pBaseDataModel->AddAttribute(pBaseDataModel->pRootAttributeList(), "skeleton", DmAttributeType_t::AT_ELEMENT, &pElement->elementIndex); };

	CDmeTransform* transform = nullptr;
	CDmeMesh* shape = nullptr;
	bool visible = 1;
	std::vector<int*> children;
	std::vector<int*> jointList;
	std::vector<int*> baseStates;
	int upAxis;

	void AddTransform(const char* name, const int set);
	void AddShape(const char* name, const int set);
	CDmeJoint* AddJoint(const char* name, const int set);
	void AddBaseState(const char* name, const int set);
	void SetUpAxis(ModelAxis_t axis = ModelAxis_t::AXIS_Z);

	void AddChildJoint(); // this grabs first joint in the list
	void AddChildDag(const char* name, const int set);

	CDmeJoint* const pJoint(const int index) { return jointListElements.at(index); }
	CDmeTransformList* const pBaseStates(const int index) { return baseStateElements.at(index); }

private:
	CDmeJoint* pChildJoint;
	CDmeDag* pChildDag;

	std::vector<CDmeJoint*> jointListElements;
	std::vector<CDmeTransformList*> baseStateElements;

	const char* upAxisStr = "";
};
