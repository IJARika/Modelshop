
#include "../datamodel/datamodel.h"

#pragma once

class CDmeModel : public CDataModel
{
public:
	CDmeModel(CDataModel* datamodel);

	// add to base DataModel
	void AddAsModel();
	void AddAsSkeleton();

private:
	CDataModel* pBaseDataModel;
	DmElement* pElement;
	CDataModelAttributeList* pAttributes;
	//int elementIndex = pElement->elementIndex; // index of this element in elementList

	const char* upAxis;

	int modelTransform = -1;
	int modelShape = -1;

	bool modelVisible = 1;

	int modelNumChildren = 0;
	std::vector<int> modelChildren;

	int modelNumJoints = 0; // number of bones
	std::vector<int> modelJointList;

	int modelNumBaseStates = 0;
	std::vector<int> modelBaseStates;

	int modelUpAxis = stringDict.AddToStringDict(upAxis);
 
};
