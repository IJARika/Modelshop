#pragma once

#include <model/datamodel/cdmequaternionloglayer.h>

class CDmeQuaternionLog : public CDmeBase
{
public:
	CDmeQuaternionLog(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false);
	virtual ~CDmeQuaternionLog()
	{

	}

	CDmeQuaternionLogLayer* const AddLayer(const char* const nameIn, const size_t numFrames = 0ull, const bool truncateName = false); // set will be taken from this class

private:
	std::vector<DmeTime_t> bookmarksX;
	std::vector<DmeTime_t> bookmarksY;
	std::vector<DmeTime_t> bookmarksZ;

	std::vector<DmElementHash> layers;
	std::vector<CDmeQuaternionLogLayer*> layerElements;

	void* curveInfoElement;
	DmElementHash curveinfo;

	// if this bone doesn't need data, can we use this?
	Quaternion defaultvalue;
	bool usedefaultvalue;
};