
#include "../math/vector.h"
#include "../datamodel/datamodel.h"
#include"../shareddefs.h"

#pragma once


//===============
// CDmeTransform
//===============

class CDmeTransform : CDataModel
{
public:
	CDmeTransform(CDataModel* datamodel, const char* name, int set);

	inline int* const pIndex() { return &elementIndex; };

	Vector position;
	Quaternion orientation;
	Vector scale = { 1.0f, 1.0f, 1.0f }; // this is for respawn specific models

private:
	CDataModel* pBaseDataModel;
	DmElement* pElement;
	CDataModelAttributeList* pAttributes;

	const char* elementName;
	int elementSet;
	int elementIndex;
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

class CDmeModel : private CDataModel
{
public:
	CDmeModel(CDataModel* datamodel);
	~CDmeModel();

	// add to base DataModel
	void AddAsModel();
	void AddAsSkeleton();

	// add attributes
	void AddAttrTransform();
	// shape
	void AddAttrVisible(bool isVisible = 1); // default to 1
	// children
	// jointList
	// baseStates
	void AddAttrUpAxis(ModelAxis_t axis = ModelAxis_t::AXIS_Z);

private:
	CDataModel* pBaseDataModel;
	DmElement* pElement;
	CDataModelAttributeList* pAttributes;
	//int elementIndex = pElement->elementIndex; // index of this element in elementList

	const char* upAxis = "";

	CDmeTransform* transform;
	//int modelTransform = -1;
	int modelShape = -1;

	bool modelVisible = 1;

	int modelNumChildren = 0;
	std::vector<int> modelChildren;

	int modelNumJoints = 0; // number of bones
	std::vector<int> modelJointList;

	int modelNumBaseStates = 0;
	std::vector<int> modelBaseStates;

	int modelUpAxis;
 
};
