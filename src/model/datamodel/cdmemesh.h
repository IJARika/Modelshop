#pragma once

#include <model/datamodel/cdmevertexdata.h>
#include <model/datamodel/cdmefaceset.h>

class CDmeMesh : public CDmeBase
{
public:
	CDmeMesh(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false);
	virtual ~CDmeMesh()
	{

	}

	// built externally for ease
	CDmeVertexData* const AddBaseState(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t vertCount, const int numJoints = -1);
	CDmeVertexData* const AddCurrentState(const int idx); // pulled from base states.
	void AddCurrentState(CDmeVertexData* const vertexData); // pulled from base states.
	CDmeFaceSet* const AddFaceSet(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFaces = 0ull, const bool truncateName = false);

	FORCEINLINE CDmeVertexData* const GetBaseState(const int idx) { return baseStateElements.at(idx); }
	FORCEINLINE CDmeFaceSet* const GetFaceSet(const int idx) { return faceSetElements.at(idx); }

	static const char* const GetNameForShape(CDataModel* const dataModelIn, const char* const nameIn);

private:
	std::vector<CDmeVertexData*> baseStateElements;
	std::vector<CDmeVertexData*> deltaStateElements;
	std::vector<CDmeFaceSet*> faceSetElements;

	std::vector<DmElementHash> baseStates;
	std::vector<DmElementHash> deltaStates;
	std::vector<DmElementHash> faceSets;
	std::vector<Vector2D> deltaStateWeights;
	std::vector<Vector2D> deltaStateWeightsLagged;

	CDmeVertexData* bindState; // need null element
	CDmeVertexData* currentState;

	bool visible;
};