#include "cdmevertexdata.h"

CDmeVertexData::CDmeVertexData(CDataModel* pDataModel, const char* name, const int set)
{
	SetupElementBase(pDataModel, "DmeVertexData", name, set);
}

CDmeVertexData::~CDmeVertexData()
{

}

static const char* attributeNames[CDmeVertexData::AT_COUNT] =
{
	"positions",
	"normals",
	"tangents",
	"textureCoordinates",
	"textureCoordinates2",
	"colors",
	"jointWeights",
	"jointIndices"
};

static DmAttributeType_t attributeTypes[CDmeVertexData::AT_COUNT] =
{
	AT_VECTOR3_ARRAY,
	AT_VECTOR3_ARRAY,
	AT_VECTOR4_ARRAY,
	AT_VECTOR2_ARRAY,
	AT_VECTOR2_ARRAY,
	AT_COLOR_ARRAY,
	AT_FLOAT_ARRAY,
	AT_INT_ARRAY
};
