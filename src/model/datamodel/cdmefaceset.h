#pragma once

#include <model/datamodel/cdmematerial.h>

class CDmeFaceSet : public CDmeBase
{
public:
	CDmeFaceSet(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFaces = 0ull, const bool truncateName = false); // name here should be similar to the material.
	virtual ~CDmeFaceSet()
	{

	}

	void AddMaterial(const char* const materialPath, const int materialId, bool truncate = false); // truncate will decide if we want to shorten it or not

	void AddFace(const int indiceCount, ...);
	void AddFace(const int indiceCount, const int* const indices);
	void AddFace(const int indiceCount, const uint32_t* const indices);

private:
	CDmeMaterial* material;
	std::vector<int> faces; // each face is terminated by a '-1' value, means any face could be a tri, quad, etc.
};