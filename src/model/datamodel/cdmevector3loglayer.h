#pragma once

class CDmeVector3LogLayer : public CDmeBase
{
public:
	CDmeVector3LogLayer(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFrames = 0ull, const bool truncateName = false);
	virtual ~CDmeVector3LogLayer()
	{

	}

	void AddFrame(const Vector& valueIn, const float timeIn);

private:
	std::vector<DmeTime_t> times;
	std::vector<int> curvetypes;
	std::vector<Vector> values;
	int compressed;
};