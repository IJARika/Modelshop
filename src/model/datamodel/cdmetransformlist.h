#pragma once

#include <model/datamodel/cdmetransform.h>

class CDmeTransformList : public CDmeBase
{
public:
	CDmeTransformList(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numJoints = 0ull, const bool truncateName = false);
	virtual ~CDmeTransformList()
	{

	}

	inline CDmeTransform* const GetTransform(const int transformIdx) const { return transformElements.at(static_cast<size_t>(transformIdx)); }

	void AddTransform(const char* const nameIn, const int setIn, const Vector& position, const Quaternion& rotation, const Vector& scale, const bool truncateName = false);

private:
	std::vector<CDmeTransform*> transformElements;
	std::vector<DmElementHash> transforms; // element indices
};