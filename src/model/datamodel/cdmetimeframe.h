#pragma once

class CDmeTimeFrame : public CDmeBase
{
public:
	CDmeTimeFrame(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const float startIn, const float durIn, const bool truncateName = false);
	virtual ~CDmeTimeFrame()
	{

	}

private:
	DmeTime_t start;
	DmeTime_t duration;
	DmeTime_t offset;
	float scale;
};