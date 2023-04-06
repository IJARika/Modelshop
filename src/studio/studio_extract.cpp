#include <stdlib.h>
#include <vector>
#include <string>
//#include <rpc.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "../utils.h"
//#include "../shareddefs.h"
#include "../datamodel/datamodel.h"
#include "../datamodel/datamodel_element.h"
#include "../studio/studio.h"
#include "../studio/studio_extract.h"

// adjusts dmx file name for export, this is really bad lol
void DMXRenameLODs(std::string& fileName, int lodIdx)
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

// bad name for what this does
//void DMXBuildSkeletonR2(CDataModel* dmx, r2::studiohdr_t* pHdr)
//{
//	DmAttribute newAttribute{};
//	int lefunnitrol = 0;
//
//	newAttribute.attributeName = dmx->pStringDict()->AddToStringDict("test");
//	newAttribute.attributeType = DmAttributeType_t::AT_STRING_ARRAY;
//	//newAttribute.attributeValue = &lefunnitrol;
//	newAttribute.attributeNameStr = "test";
//
//	std::vector<void*> alittlebitoftrolling;
//
//	/*alittlebitoftrolling.push_back(&pHdr->flags);
//	alittlebitoftrolling.push_back(&pHdr->checksum);
//	alittlebitoftrolling.push_back(&pHdr->version);*/
//
//	alittlebitoftrolling.push_back(pHdr->pszName());
//	alittlebitoftrolling.push_back(pHdr->pszSourceFiles());
//
//	newAttribute.attributeValues = alittlebitoftrolling;
//	newAttribute.numValues = 2;
//
//	dmx->pRootAttributeList()->AddAttribute(&newAttribute);
//
//	/*UUID rootUUID;
//	CDataModelAttributeList rootList;
//
//	dmx->pElementList()->AddElement(dmx->pStringDict()->AddToStringDict("CDmElement"), dmx->pStringDict()->AddToStringDict(pHdr->pszName()), rootUUID, rootList);
//
//	dmx->pElementList()->pAttributeList(0)->AddAttribute(dmx->pStringDict()->AddToStringDict("PISS"), 6, nullptr);*/
//}

// add vvw
void GetVertexesFromVVD(vvd::vertexFileHeader_t* pVVD, vvc::vertexColorFileHeader_t* pVVC, const int lod, std::vector<const vvd::mstudiovertex_t*>& vvdVerts, std::vector<const VertexColor_t*>& vvcColors, std::vector<const Vector2D*>& vvcUV2s)
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

void DMXFromMDL(char* pMdlBuf, const std::string fileDir)
{
	// setup required bufferes
	r2::studiohdr_t* const pHdr = reinterpret_cast<r2::studiohdr_t*>(pMdlBuf);
	vtx::FileHeader_t* const pVtx = pHdr->pVTX();
	vvd::vertexFileHeader_t* const pVVD = pHdr->pVVD();
	vvc::vertexColorFileHeader_t* const pVVC = pHdr->pVVC();

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

				CDataModel dmxOut(DataModelType_t::DM_MODEL);

				CDmeModel* dmxModel = new CDmeModel(&dmxOut);

				dmxModel->AddAsSkeleton();
				dmxModel->AddAsModel();

				// messing around
				dmxModel->AddAttrTransform();
				dmxModel->AddAttrVisible();
				dmxModel->AddAttrUpAxis();

				//DMXBuildSkeletonR2(&dmxOut, pHdr);

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

				delete dmxModel;
			}
		}
	}
}