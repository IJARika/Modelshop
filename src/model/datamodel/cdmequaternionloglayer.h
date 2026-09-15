#pragma once

class CDmeQuaternionLogLayer : public CDmeBase
{
public:
	CDmeQuaternionLogLayer(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numFrames = 0ull, const bool truncateName = false);
	virtual ~CDmeQuaternionLogLayer()
	{

	}

	void AddFrame(const Quaternion& valueIn, const float timeIn);

private:
	std::vector<DmeTime_t> times;
	std::vector<int> curvetypes;
	std::vector<Quaternion> values;
	int compressed;
};