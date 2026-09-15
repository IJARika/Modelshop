#pragma once

// don't really have anything here yet because stringdict is mostly static
class CDmeMaterial : public CDmeBase
{
public:
	CDmeMaterial(CDataModel* const dataModelIn, const char* const materialPath, const int setIn, const bool truncateName = false);

private:
	const char* mtlName; // truncation required sometimes
};
