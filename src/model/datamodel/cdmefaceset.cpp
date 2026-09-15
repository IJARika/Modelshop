#include <pch.h>

#include <model/datamodel/cdmefaceset.h>

CDmeFaceSet::CDmeFaceSet(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFaces, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_FACESET, nameIn, setIn, 2ull, truncateName), material(nullptr)
{
	// this assumes tris, but if not, it'll just allocate more memory which is fine. we just want to reduce that as much as possible!
	faces.reserve(numFaces * 4ull);

	attributes->AddAttribute(dataModel, "material", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "faces", DmAttributeType_t::AT_INT_ARRAY, &faces, DmAttribute::ATT_VAL_VEC);
}

void CDmeFaceSet::AddMaterial(const char* const materialPath, const int materialId, bool truncate)
{
	material = new CDmeMaterial(dataModel, materialPath, materialId, truncate);

	attributes->EditAttribute(dataModel, "material", DmAttributeType_t::AT_ELEMENT, material->GetHash());
}

void CDmeFaceSet::AddFace(const int indiceCount, ...)
{
	va_list inIndices;
	va_start(inIndices, indiceCount);

	for (int idx = 0; idx < indiceCount; idx++)
	{
		faces.push_back(va_arg(inIndices, int));
	}

	faces.push_back(-1);
}

void CDmeFaceSet::AddFace(const int indiceCount, const int* const indices)
{
	for (int idx = 0; idx < indiceCount; idx++)
	{
		faces.push_back(indices[idx]);
	}

	faces.push_back(-1);
}

void CDmeFaceSet::AddFace(const int indiceCount, const uint32_t* const indices)
{
	for (int idx = 0; idx < indiceCount; idx++)
	{
		faces.push_back(static_cast<int>(indices[idx]));
	}

	faces.push_back(-1);
}