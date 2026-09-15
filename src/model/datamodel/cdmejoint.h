#pragma once

#include <model/datamodel/cdmetransform.h>
#include <model/datamodel/cdmemesh.h>

class CDmeJoint : public CDmeBase
{
public:
	CDmeJoint(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName);
	virtual ~CDmeJoint()
	{

	}

	void AddTransform(const char* const nameIn, const int setIn, const Vector& posIn, const Quaternion& rotIn, const Vector& scaleIn, const bool truncateName = false);
	void AddShape(const char* const nameIn, const int setIn, const bool truncateName = false); // nameIn will have "Shape" appended on the end of it.
	void AddChildJoint(CDmeJoint* const childJoinIn);
	void SetUpSurfaceProp(const char* const surfaceProp = "default");

private:
	std::vector<CDmeJoint*> childrenElements;
	
	std::vector<DmElementHash> children;
	CDmeTransform* transform;
	CDmeMesh* shape;
	const char* _surfaceProp;
	bool visible;
	bool lockInfluenceWeights; // unsure what this is for as well
};