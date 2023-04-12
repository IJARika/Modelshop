
#include "../math/vector.h"
#include "../datamodel/datamodel.h"
#include"../shareddefs.h"

#pragma once


//==========
// CDmeBase
//==========

class CDmeBase
{
public:
	CDmeBase() = default;

	inline int* const pIndex() { return &elementIndex; };

protected:
	CDataModel* pBaseDataModel;
	DmElement* pElement;
	CDataModelAttributeList* pAttributes;

	const char* elementName;
	int elementSet;
	int elementIndex;

	inline void SetupElementBase(CDataModel* pDataModel, const char* type, const char* name, const int set);
};


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


//===========
// CDmeJoint
//===========

class CDmeJoint : public CDmeBase
{
public:
	CDmeJoint(CDataModel* pDataModel, const char* name, const int set);
	~CDmeJoint();

	CDmeTransform* transform;
	//CDmeTransform* shape;
	int shape = -1; // hardcode as I'm not really sure what this attribute is for
	bool visible = 1;
	std::vector<int*> children;
	int _surfaceProp = -1;
	bool lockInfluenceWeights = 0; // unsure what this is for as well

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

	CDmeTransform* transform;
	//CDmeTransform* shape; // need a way to do 'null' elements i.e. index of -1
	int shape = -1; // hardcode as I'm not really sure what this attribute is for
	bool visible = 1;
	std::vector<int*> children;
	std::vector<int*> jointList;
	std::vector<int*> baseStates;
	int upAxis;

	CDmeJoint* AddJoint(const char* name, const int set);
	void AddBaseState(const char* name, const int set);
	void SetUpAxis(ModelAxis_t axis = ModelAxis_t::AXIS_Z);

	void AddChildBaseJoint();
	void AddChildModel();

	CDmeJoint* const pJoint(const int index) { return jointListElements.at(index); }
	CDmeTransformList* const pBaseStates(const int index) { return baseStateElements.at(index); }

private:
	std::vector<CDmeJoint*> jointListElements;
	std::vector<CDmeTransformList*> baseStateElements;

	const char* upAxisStr = "";
};
