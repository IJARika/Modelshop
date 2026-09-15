#pragma once

#include <model/datamodel/cdmemesh.h>
#include <model/datamodel/cdmetransform.h>

// 'directed acyclic graph'
class CDmeDag : public CDmeBase
{
public:
	CDmeDag(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false);
	virtual ~CDmeDag()
	{

	}

	CDmeTransform* const AddTransform(const char* const nameIn, const int setIn);
	CDmeMesh* const AddShape(const char* const nameIn, const int setIn, const bool truncateName = false); // name in will have "Shape" appended to the end

	FORCEINLINE CDmeMesh* const GetShape() const { return shape; }

private:
	std::vector<DmElementHash> children;
	CDmeTransform* transform;
	CDmeMesh* shape;
	bool visible;
};