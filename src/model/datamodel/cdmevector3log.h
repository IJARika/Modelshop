#pragma once

#include <model/datamodel/cdmevector3loglayer.h>

class CDmeVector3Log : public CDmeBase
{
public:
	CDmeVector3Log(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false);
	virtual ~CDmeVector3Log()
	{

	}

	CDmeVector3LogLayer* const AddLayer(const char* const nameIn, const size_t numFrames = 0ull, const bool truncateName = false); // set will be taken from this class

private:
	std::vector<DmeTime_t> bookmarksX;
	std::vector<DmeTime_t> bookmarksY;
	std::vector<DmeTime_t> bookmarksZ;

	std::vector<DmElementHash> layers;
	std::vector<CDmeVector3LogLayer*> layerElements;

	void* curveInfoElement;
	DmElementHash curveinfo;

	// if this bone doesn't need data, can we use this?
	Vector defaultvalue;
	bool usedefaultvalue;
};