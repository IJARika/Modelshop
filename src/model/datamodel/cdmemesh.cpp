#include <pch.h>

#include <model/datamodel/cdmemesh.h>

CDmeMesh::CDmeMesh(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_MESH, nameIn, setIn, 8ull, truncateName),
visible(true), bindState(nullptr), currentState(nullptr)
{
	attributes->AddAttribute(dataModel, "visible", DmAttributeType_t::AT_BOOL, visible);
	attributes->AddAttribute(dataModel, "bindState", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "currentState", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "baseStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &baseStates, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "deltaStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &deltaStates, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "faceSets", DmAttributeType_t::AT_ELEMENT_ARRAY, &faceSets, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "deltaStateWeights", DmAttributeType_t::AT_VECTOR2_ARRAY, &deltaStateWeights, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "deltaStateWeightsLagged", DmAttributeType_t::AT_VECTOR2_ARRAY, &deltaStateWeightsLagged, DmAttribute::ATT_VAL_VEC);
}

CDmeVertexData* const CDmeMesh::AddBaseState(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t vertCount, const int numJoints)
{
	CDmeVertexData* const vertexData = new CDmeVertexData(dataModelIn, nameIn, setIn, vertCount, numJoints);

	baseStates.push_back(vertexData->GetHash());
	baseStateElements.push_back(vertexData);

	return vertexData;
}

CDmeVertexData* const CDmeMesh::AddCurrentState(const int idx)
{
	currentState = GetBaseState(idx);

	attributes->EditAttribute(dataModel, "currentState", DmAttributeType_t::AT_ELEMENT, currentState->GetHash());

	return currentState;
}

void CDmeMesh::AddCurrentState(CDmeVertexData* const vertexData)
{
	currentState = vertexData;

	attributes->EditAttribute(dataModel, "currentState", DmAttributeType_t::AT_ELEMENT, currentState->GetHash());
}

CDmeFaceSet* const CDmeMesh::AddFaceSet(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFaces, const bool truncateName)
{
	CDmeFaceSet* const faceSet = new CDmeFaceSet(dataModelIn, nameIn, setIn, numFaces, truncateName);

	faceSets.push_back(faceSet->GetHash());
	faceSetElements.push_back(faceSet);

	return faceSet;
}

const char* const CDmeMesh::GetNameForShape(CDataModel* const dataModelIn, const char* const nameIn)
{
	char tmp[dmMaxStringLength]{};
	snprintf(tmp, dmMaxStringLength, "%sShape", nameIn);
	const char* const str = dataModelIn->WriteString(tmp);

	return str;
}