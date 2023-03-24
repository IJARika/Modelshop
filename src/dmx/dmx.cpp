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
	// this can be removed because it's an extra byte if never used
	/*strings.push_back(stringentry_t{ "", 0 });
	numStrings = 1;*/
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
	//int* stringCount = reinterpret_cast<int*>(*pData);
	//*stringCount = numStrings;
	*reinterpret_cast<int*>(*pData) = numStrings;
	*pData += sizeof(numStrings);

	for (auto& entry : strings)
	{
		int strLength = strnlen_s(entry.string, MAX_PATH_SOURCE) + 1;

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
CDataModelAttribute::CDataModelAttribute(int name, char type, void* value)
{
	attributeName = name;
	attributeType = type;
	attributeValue = &value;
}

// unfinished
int CDataModelAttribute::ValueSizeFromType()
{
	int valueSize = 0;

	switch (attributeType)
	{
	case AT_VOID:
		valueSize = 0; // not correct
		break;
	case AT_BOOL:
		valueSize = 1;
		break;
	case AT_ELEMENT:
	case AT_INT:
	case AT_FLOAT:
	case AT_STRING:
		valueSize = 4;
		break;
	default:
		Error("error: unknown attribute type used!!!");
		break;
	}

	return valueSize;
}

void CDataModelAttribute::WriteAttribute(char** pData)
{
	memcpy(*pData, &attributeName, sizeof(attributeName) + sizeof(attributeType));
	*pData += sizeof(attributeName) + sizeof(attributeType);

	// too be tested
	int valueSize = ValueSizeFromType();
	memcpy(*pData, &attributeValue, valueSize);
	*pData += valueSize;
}

void CDataModelAttributeList::AddAttribute(int name, char type, void* value)
{
	CDataModelAttribute newAttribute{ name, type, value };
	Attributes.push_back(newAttribute);

	numAttributes++;
}

void CDataModelAttributeList::WriteAttributeList(char** pData)
{
	memcpy(*pData, &numAttributes, sizeof(numAttributes));
	*pData += sizeof(numAttributes);

	for (auto& attribute : Attributes)
	{
		attribute.WriteAttribute(pData);
	}
}


//========================
// DataModel Element List
//========================
DmxElement_t* CDataModelElementList::pElement(int index)
{
	return &Elements.at(index);
}

CDataModelAttributeList* CDataModelElementList::pAttributeList(int index)
{
	return &AttributeList.at(index);
}

int CDataModelElementList::AddElement(int type, int name, UUID& uuid, CDataModelAttributeList& attributes)
{
	DmxElement_t newElement{ type, name, uuid };
	Elements.push_back(newElement);
	AttributeList.push_back(attributes);

	numElements++;

	return std::distance(Elements.begin(), Elements.end()); // this may not work as intended, if encountering a bug with indexes check here
}

void CDataModelElementList::WriteElementList(char** pData)
{
	memcpy(*pData, &numElements, sizeof(numElements));
	*pData += sizeof(numElements);
	
	/*memcpy(*pData, &elements, sizeof(DmxElement_t) * elements.size());
	*pData += sizeof(DmxElement_t) * elements.size();*/

	for (auto& element : Elements)
	{
		memcpy(*pData, &element, sizeof(DmxElement_t));
		*pData += sizeof(DmxElement_t);
	}

	for (auto& attributelist : AttributeList)
	{
		attributelist.WriteAttributeList(pData);
	}
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

	StringDict.WriteStringDict(pData);

	ElementList.WriteElementList(pData);
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
	UUID rootUUID;
	CDataModelAttributeList rootList;

	dmx->pElementList()->AddElement(dmx->pStringDict()->AddToStringDict("DmElement"), dmx->pStringDict()->AddToStringDict(pHdr->pszName()), rootUUID, rootList);

	dmx->pElementList()->pAttributeList(0)->AddAttribute(dmx->pStringDict()->AddToStringDict("PISS"), 6, nullptr);
}

// adjusts dmx file name for export, this is really bad lol
void DMXRenameLODs(std::string &fileName, int lodIdx)
{
	char lodName[8] = "";
	snprintf(lodName, 8, "_lod%i", lodIdx);

	if (fileName.rfind("_lod0") != std::string::npos)
	{
		fileName.replace(fileName.length() - 5, 8, lodName);
	}
	else
	{
		fileName.append(lodName);
	}
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

				if (!pModel->nummeshes)
				{
					continue;
				}

				vtx::ModelLODHeader_t* pVtxLOD = pVtxModel->pLOD(lodIdx);

				CDataModel dmxOut("<!-- dmx encoding binary 5 format model 18 -->\n");

				DMXBuildSkeletonR2(&dmxOut, pHdr);

				char* pBase = new char[FILEBUFSIZE];
				char* pData = pBase;

				dmxOut.WriteDataModel(&pData);

				std::string fileName = GET_FILE_STEM(pModel->name);
				
				if (lodIdx > 0)
				{
					DMXRenameLODs(fileName, lodIdx);
				}

				fileName.append(".dmx");
				
				std::string fileOutPath = std::filesystem::path(fileDir).append(fileName).u8string();		

				std::ofstream dmxFile(fileOutPath, std::ios::out | std::ios::binary);				
				dmxFile.write(pBase, pData - pBase);
			}
		}
	}
}