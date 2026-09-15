#pragma once

enum VertexAttributes_t : int
{
	AT_VERTEX_POSITION,
	AT_VERTEX_NORMAL,
	AT_VERTEX_TANGENT,
	AT_VERTEX_TEXCOORD,
	AT_VERTEX_TEXCOORD2,
	AT_VERTEX_COLOR,
	AT_VERTEX_JOINT_WEIGHTS,
	AT_VERTEX_JOINT_INDICES,
	AT_VERTEX_COUNT,
	AT_VERTEX_INDICE_COUNT = AT_VERTEX_JOINT_WEIGHTS,
};

// arrays of goodies for vertex data
constexpr const char* const attributeNames[AT_VERTEX_COUNT] =
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

// this sux
constexpr const char* const attributeIndexNames[AT_VERTEX_COUNT] =
{
	"positionsIndices",
	"normalsIndices",
	"tangentsIndices",
	"textureCoordinatesIndices",
	"textureCoordinates2Indices",
	"colorsIndices",
	nullptr,
	nullptr,
};

static DmAttributeType_t attributeTypes[AT_VERTEX_COUNT] =
{
	DmAttributeType_t::AT_VECTOR3_ARRAY,
	DmAttributeType_t::AT_VECTOR3_ARRAY,
	DmAttributeType_t::AT_VECTOR4_ARRAY,
	DmAttributeType_t::AT_VECTOR2_ARRAY,
	DmAttributeType_t::AT_VECTOR2_ARRAY,
	DmAttributeType_t::AT_COLOR_ARRAY,
	DmAttributeType_t::AT_FLOAT_ARRAY,
	DmAttributeType_t::AT_INT_ARRAY
};

class CDmeVertexData : public CDmeBase
{
public:
	CDmeVertexData(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numMaxVerts = 0ull, int numJoints = -1);
	virtual ~CDmeVertexData()
	{

	}
	
	int JointCount() const { return jointCount; }
	//const size_t VertexCount() const { return vertexCount; } // this is cheaty and honestly not a good way to do this

	//const Vector& GetPosition(const int idx) const { return positions.at(idx); }
	//const int GetPositionIndex(const int idx) const { return positionsIndices.at(idx); } // maybe having these return the data it refers to could be better?
	void* const GetVertexData(const VertexAttributes_t vertexField);
	std::vector<int>* const GetVertexIndices(const VertexAttributes_t vertexField);
	//std::vector<const char*>& GetVertexFormat() { return vertexFormat; }

	// add a value for vertex data to its respective array
	template<class T>
	void AddVertexData(VertexAttributes_t vertexField, T vertexValue)
	{
		std::vector<T>* vec = reinterpret_cast<std::vector<T>*>(GetVertexData(vertexField));
		std::vector<int>* vecIdx = GetVertexIndices(vertexField);

		if (vec->empty())
		{
			vertexFormat.push_back(attributeNames[vertexField]);

			vec->reserve(maxVertexCount);
			vecIdx->reserve(maxVertexCount);

			attributes->AddAttribute(dataModel, attributeNames[vertexField], attributeTypes[vertexField], vec, DmAttribute::ATT_VAL_VEC);
			attributes->AddAttribute(dataModel, attributeIndexNames[vertexField], DmAttributeType_t::AT_INT_ARRAY, vecIdx, DmAttribute::ATT_VAL_VEC);
		}

		// slight hack to make it only compile on texcoords, as they are the only ones using Vector2D
		if constexpr (false)
		{
			int dataIndex = static_cast<int>(vec->size());

			for (size_t i = 0ull; i < vec->size(); i++)
			{
				// minor improvements over std::memcmp, however it does at up.
				const __m128 dBase = _mm_load_ps(reinterpret_cast<const float*>(&vertexValue));
				const __m128 d0 = _mm_load_ps(reinterpret_cast<const float*>(&vec->at(i)));

				const __m128 dCmp = _mm_cmpeq_ps(dBase, d0);

				const int fieldsMatchingBits = _mm_movemask_ps(dCmp);
				const int mask = ~(0xffffffff << (sizeof(T) / 4));

				if ((fieldsMatchingBits & mask) != mask) // get important bits, then check if it's a match
				{
					continue;
				}

				dataIndex = static_cast<int>(i);
				break;
			}

			vecIdx->push_back(dataIndex);

			if (vec->size() == dataIndex)
			{
				vec->push_back(vertexValue);
			}
		}
		else
		{
			const int dataIndex = static_cast<int>(vec->size());

			vec->push_back(vertexValue);
			vecIdx->push_back(dataIndex);
		}
	}

	void AddMappedIndice(int indice);
	void AddVertexWeight(float weight, int indice);

	void MapVertexIndices();

private:
	std::vector<const char*> vertexFormat; // format of vertices, array of attribute names

	// IMPORTANT: indices will always equal the number verts used by faces, that being the face
	std::vector<Vector> positions;
	std::vector<int> positionsIndices;

	std::vector<Vector> normals;
	std::vector<int> normalsIndices;
	std::vector<Vector4D> tangents;
	std::vector<int> tangentsIndices;

	std::vector<Vector2D> textureCoordinates;
	std::vector<int> textureCoordinatesIndices;
	std::vector<Vector2D> textureCoordinates2;
	std::vector<int> textureCoordinates2Indices;

	std::vector<Color32> colors;
	std::vector<int> colorIndices;

	std::vector<float> jointWeights; // needs to support potentially 16 weights per vert, jointCount * vertexCount number of fields
	std::vector<int> jointIndices; // joint idx for each weight, 0 if not

	std::vector<int> vertexIndiceMap;

	size_t maxVertexCount; // number of expected vertices

	int jointCount; // max number of bones per vert
	bool flipVCoordinates; // flip the vertical coords of uv vectors, should be set to true for export.
};