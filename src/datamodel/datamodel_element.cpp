#include "../datamodel/datamodel_element.h"


CDmeModel::CDmeModel(CDataModel* datamodel)
{
	pBaseDataModel = datamodel;

	pElement = pBaseDataModel->AddElement("DmeModel", "mshop datamodel export", 0);
	pAttributes = pBaseDataModel->GetAttributeList(&pElement->elementIndex);


	//upAxis = upAxis;
}

void CDmeModel::AddAsModel()
{
	pBaseDataModel->AddAttribute(pBaseDataModel->pRootAttributeList(), "model", DmAttributeType_t::AT_ELEMENT, &pElement->elementIndex);
}

void CDmeModel::AddAsSkeleton()
{
	pBaseDataModel->AddAttribute(pBaseDataModel->pRootAttributeList(), "skeleton", DmAttributeType_t::AT_ELEMENT, &pElement->elementIndex);
}