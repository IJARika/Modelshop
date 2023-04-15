#include "cdmebase.h"

//template<class T*>
//__forceinline void DeallocateElement(void* pElement)
//{
//	
//}

void CDmeBase::SetupElementBase(CDataModel* pDataModel, const char* type, const char* name, const int set)
{
	pBaseDataModel = pDataModel;
	pElement = pBaseDataModel->AddElement(type, name, set);
	pAttributes = pBaseDataModel->GetAttributeList(pElement->elementIndex);

	elementName = name;
	elementSet = set;
	elementIndex = pElement->elementIndex;
}