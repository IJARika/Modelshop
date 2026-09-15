#include <pch.h>

#include <model/datamodel/cdmebase.h>

CDmeBase::CDmeBase(CDataModel* const dataModelIn, const eDmElementClass classIn, const char* const nameIn, const int setIn, const size_t numBaseAttributes, const bool truncateName) : dataModel(dataModelIn), type(classIn), set(setIn)
{
	name = truncateName ? keepAfterLastSlashOrBackslash(nameIn) : nameIn;

	element = dataModel->AddElement(type, name, set, numBaseAttributes);
	attributes = element->GetAttributes();

	element->SetElement(this, type);
};