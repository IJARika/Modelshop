#pragma once

class CDmeBase
{
public:
	// truncateName always set to false for standardized usage
	CDmeBase() = default;
	CDmeBase(CDataModel* const dataModelIn, const eDmElementClass classIn, const char* const nameIn, const int setIn, const size_t numBaseAttributes = 0ull, const bool truncateName = false);
	virtual ~CDmeBase()
	{

	};

	inline const char* GetName() const { return name; }
	inline const eDmElementClass GetType() const { return type; }
	inline const DmElementHash GetHash() const { return element->GetHash(); }

protected:
	CDataModel* dataModel;
	DmElement* element;
	DmAttributeList* attributes;

	const char* name;
	eDmElementClass type;
	int set;
};