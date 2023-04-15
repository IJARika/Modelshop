#ifndef CDMEBASE_H
#define CDMEBASE_H

#pragma once

#include "../datamodel/datamodel.h"

class CDmeBase
{
public:
	CDmeBase() = default;

	inline int* const pIndex() { return &elementIndex; };

	/*template<class T*> __forceinline void DeallocateElement(void* pElement);*/

protected:
	CDataModel* pBaseDataModel;
	DmElement* pElement;
	CDataModelAttributeList* pAttributes;

	const char* elementName;
	int elementSet;
	int elementIndex;

	int stubIndex = -1;

	void SetupElementBase(CDataModel* pDataModel, const char* type, const char* name, const int set);
};


#endif // !CDMEBASE_H
