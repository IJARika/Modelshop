#include <pch.h>

#include <model/datamodel/cdmevertexdata.h>

CDmeVertexData::CDmeVertexData(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numMaxVerts, int numJoints) : CDmeBase(dataModelIn, eDmElementClass::DME_VERTEXDATA, nameIn, setIn, 17ull, false), jointCount(numJoints), flipVCoordinates(true),
	maxVertexCount(numMaxVerts)
{
	attributes->AddAttribute(dataModel, "vertexFormat", DmAttributeType_t::AT_STRING_ARRAY, &vertexFormat, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "jointCount", DmAttributeType_t::AT_INT, jointCount);
	attributes->AddAttribute(dataModel, "flipVCoordinates", DmAttributeType_t::AT_BOOL, flipVCoordinates);
}

void* const CDmeVertexData::GetVertexData(const VertexAttributes_t vertexField)
{
	switch (vertexField)
	{
	case AT_VERTEX_POSITION:
	{
		return &positions;
	}
	case AT_VERTEX_NORMAL:
	{
		return &normals;
	}
	case AT_VERTEX_TANGENT:
	{
		return &tangents;;
	}
	case AT_VERTEX_TEXCOORD:
	{
		return &textureCoordinates;
	}
	case AT_VERTEX_TEXCOORD2:
	{
		return &textureCoordinates2;
	}
	case AT_VERTEX_COLOR:
	{
		return &colors;
	}
	case AT_VERTEX_JOINT_WEIGHTS:
	case AT_VERTEX_JOINT_INDICES:
	case AT_VERTEX_COUNT:
	default:
	{
		Error("Invaild vertex field field!");
		return nullptr;
	}
	}
}

std::vector<int>* const CDmeVertexData::GetVertexIndices(const VertexAttributes_t vertexField)
{
	switch (vertexField)
	{
	case AT_VERTEX_POSITION:
	{
		return &positionsIndices;
	}
	case AT_VERTEX_NORMAL:
	{
		return &normalsIndices;
	}
	case AT_VERTEX_TANGENT:
	{
		return &tangentsIndices;
	}
	case AT_VERTEX_TEXCOORD:
	{
		return &textureCoordinatesIndices;
	}
	case AT_VERTEX_TEXCOORD2:
	{
		return &textureCoordinates2Indices;
	}
	case AT_VERTEX_COLOR:
	{
		return &colorIndices;
	}
	case AT_VERTEX_JOINT_WEIGHTS:
	case AT_VERTEX_JOINT_INDICES:
	case AT_VERTEX_COUNT:
	default:
	{
		Error("Invaild vertex field field!");
		return nullptr;
	}
	}
}

void CDmeVertexData::AddMappedIndice(int indice)
{
	vertexIndiceMap.push_back(indice);
}

void CDmeVertexData::AddVertexWeight(float weight, int indice)
{
	// added dynamically and always last in vertex format, so check here if we still need to add them
	if (jointWeights.empty())
	{
		jointWeights.reserve(maxVertexCount * jointCount);
		jointIndices.reserve(maxVertexCount * jointCount);

		attributes->AddAttribute(dataModel, attributeNames[AT_VERTEX_JOINT_WEIGHTS], attributeTypes[AT_VERTEX_JOINT_WEIGHTS], &jointWeights, DmAttribute::ATT_VAL_VEC);
		attributes->AddAttribute(dataModel, attributeNames[AT_VERTEX_JOINT_INDICES], attributeTypes[AT_VERTEX_JOINT_INDICES], &jointIndices, DmAttribute::ATT_VAL_VEC);

		vertexFormat.push_back(attributeNames[AT_VERTEX_JOINT_WEIGHTS]);
		vertexFormat.push_back(attributeNames[AT_VERTEX_JOINT_INDICES]);

		attributes->EditAttribute(dataModel, "vertexFormat", DmAttributeType_t::AT_STRING_ARRAY, vertexFormat.data(), static_cast<int>(vertexFormat.size()), false);
	}

	jointWeights.push_back(weight);
	jointIndices.push_back(indice);
}

// proof of concept, needs to be better
void CDmeVertexData::MapVertexIndices()
{
	for (int typeIdx = 0; typeIdx < VertexAttributes_t::AT_VERTEX_INDICE_COUNT; typeIdx++)
	{
		std::vector<int>* vec = GetVertexIndices(static_cast<VertexAttributes_t>(typeIdx));

		if (!vec->size())
		{
			continue;
		}

		std::vector<int> map;
		map.swap(*vec);

		vec->resize(vertexIndiceMap.size());

		for (size_t i = 0; i < vertexIndiceMap.size(); i++)
		{
			const int curIdx = vertexIndiceMap.at(i);

			vec->data()[i] = map.data()[curIdx];
		}
	}
}