#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "../utils.h"
#include "dmx.h"
#include "../studio/studio.h"
//#include "../math/vector2d.h"
//#include "../math/vertexcolor.h"


//=============================
// DataModel String Dictionary
//=============================
CDataModelStringDict::CDataModelStringDict()
{
	strings.push_back(stringentry_t{ "", 0 });
	numStrings = 1;
}

int CDataModelStringDict::AddToStringDict(const char* string)
{
	int entryIdx = 0;

	for (auto& entry : strings)
	{
		if (!strcmp(string, entry.string))
		{
			return entryIdx;
		}

		entryIdx++;
	}

	stringentry_t newEntry{ string, entryIdx };
	strings.push_back(newEntry);
	numStrings++;

	return entryIdx;
}

void CDataModelStringDict::WriteStringDict(char** pData)
{
	int* stringCount = reinterpret_cast<int*>(pData);
	*stringCount = numStrings;
	*pData += sizeof(numStrings);

	for (auto& entry : strings)
	{
		int strLength = strlen(entry.string) + 1;

		strcpy_s(*pData, strLength, entry.string);

		*pData += strLength;
	}
}

int CDataModelStringDict::GetStringIndex(const char* string)
{
	int entryIdx = 0;

	for (auto& entry : strings)
	{
		if (!strcmp(string, entry.string))
		{
			return entryIdx;
		}

		entryIdx++;
	}
}

const char* CDataModelStringDict::StringFromIndex(int* index)
{
	return strings.at(*index).string;
}


//==========================
// DataModel Attribute List
//==========================
void CDataModelAttributeList::AddAtribute(int* name, char* type, void* value)
{
	DmxAttribute_t newAttribute{ *name, *type, &value };
	attributes.push_back(newAttribute);

	numAttributes++;
}


//========================
// DataModel Element List
//========================
int CDataModelElementList::AddElement(int type, int name, UUID& uuid, CDataModelAttributeList& attributes)
{
	DmxElement_t newElement{ type, name, uuid };
	elements.push_back(newElement);
	attributeList.push_back(attributes);

	numElements++;

	return std::distance(elements.begin(), elements.end()); // this may not work as intended, if encountering a bug with indexes check here
}


//===========
// DataModel
//===========
CDataModel::CDataModel(const char* header)
{
	DMXHeader = header;
}

// accessor funcs
std::string* CDataModel::pHeader()
{
	return &DMXHeader;
}

CDataModelStringDict* CDataModel::pStringDict()
{
	return &StringDict;
}

CDataModelElementList* CDataModel::pElementList()
{
	return &ElementList;
}

void CDataModel::WriteDataModel(char** pData)
{
	strcpy_s(*pData, DMXHeader.length() + 1, DMXHeader.c_str());

	*pData += DMXHeader.length() + 1;

	StringDict.WriteStringDict(*pData);

	//return pData;
}


void GetVertexesFromVVD(vvd::vertexFileHeader_t* pVVD, vvc::vertexColorFileHeader_t* pVVC, int lod, std::vector<const vvd::mstudiovertex_t*> &vvdVerts, std::vector<const VertexColor_t*> &vvcColors, std::vector<const Vector2D*> &vvcUV2s)
{
	// rebuild vertex vector per lod just incase it has fixups
	if (pVVD->numFixups)
	{
		for (int j = 0; j < pVVD->numFixups; j++)
		{
			const vvd::vertexFileFixup_t* vertexFixup = pVVD->GetFixupData(j);

			if (vertexFixup->lod >= lod)
			{
				for (int k = 0; k < vertexFixup->numVertexes; k++)
				{
					const vvd::mstudiovertex_t* vvdVert = pVVD->GetVertexData(vertexFixup->sourceVertexID + k);

					vvdVerts.push_back(vvdVert);

					// vvc
					if (pVVC)
					{
						// doesn't matter which we pack as long as it has vvc, we will only used what's needed later
						const VertexColor_t* vvcColor = pVVC->GetColorData(vertexFixup->sourceVertexID + k);
						const Vector2D* vvcUV2 = pVVC->GetUVData(vertexFixup->sourceVertexID + k);

						vvcColors.push_back(vvcColor);
						vvcUV2s.push_back(vvcUV2);
					}
				}
			}
		}
	}
	else
	{
		// using per lod vertex count may have issues (tbd)
		for (int j = 0; j < pVVD->numLODVertexes[lod]; j++)
		{
			const vvd::mstudiovertex_t* vvdVert = pVVD->GetVertexData(j);

			vvdVerts.push_back(vvdVert);

			// vvc
			if (pVVC)
			{
				// doesn't matter which we pack as long as it has vvc, we will only used what's needed later
				const VertexColor_t* vvcColor = pVVC->GetColorData(j);
				const Vector2D* vvcUV2 = pVVC->GetUVData(j);

				vvcColors.push_back(vvcColor);
				vvcUV2s.push_back(vvcUV2);
			}
		}
	}
}

// bad name for what this does
void DMXBuildSkeletonR2(CDataModel* dmx, r2::studiohdr_t* pHdr)
{
	CDataModelAttributeList list;
	UUID uuid;
	dmx->pElementList()->AddElement(dmx->pStringDict()->AddToStringDict("DmElement"), dmx->pStringDict()->AddToStringDict(pHdr->pszName()), uuid, list);
}

void DMXFromMDL(char* pMdlBuf, const std::string fileDir)
{
	// setup required bufferes
	r2::studiohdr_t* pHdr = reinterpret_cast<r2::studiohdr_t*>(pMdlBuf);
	vtx::FileHeader_t* pVtx = pHdr->pVTX();
	vvd::vertexFileHeader_t* pVVD = pHdr->pVVD();
	vvc::vertexColorFileHeader_t* pVVC = pHdr->pVVC();

	for (int lodIdx = 0; lodIdx < pVtx->numLODs; lodIdx++)
	{
		std::vector<const vvd::mstudiovertex_t*> vvdVerts;
		std::vector<const VertexColor_t*> vvcColors;
		std::vector<const Vector2D*> vvcUV2s;

		GetVertexesFromVVD(pVVD, pVVC, lodIdx, vvdVerts, vvcColors, vvcUV2s);

		for (int bodypartIdx = 0; bodypartIdx < pHdr->numbodyparts; bodypartIdx++)
		{
			r2::mstudiobodyparts_t* pBodypart = pHdr->pBodypart(bodypartIdx);
			vtx::BodyPartHeader_t* pVtxBodyPart = pVtx->pBodyPart(bodypartIdx);

			for (int modelIdx = 0; modelIdx < pBodypart->nummodels; modelIdx++)
			{
				r2::mstudiomodel_t* pModel = pBodypart->pModel(modelIdx);
				vtx::ModelHeader_t* pVtxModel = pVtxBodyPart->pModel(modelIdx);

				vtx::ModelLODHeader_t* pVtxLOD = pVtxModel->pLOD(lodIdx);

				CDataModel dmxOut("<!-- dmx encoding binary 5 format model 18 -->\n");

				DMXBuildSkeletonR2(&dmxOut, pHdr);

				char* pBase = new char[FILEBUFSIZE];
				char* pData = pBase;

				dmxOut.WriteDataModel(&pData);

				std::string fileName = GET_FILE_NAME(pModel->name);
				std::string fileOutPath = std::filesystem::path(fileDir).append(fileName).u8string();

				std::ofstream dmxFile(fileOutPath, std::ios::out | std::ios::binary);				

				dmxFile.write(pBase, pData - pBase);


			}
		}
	}
}