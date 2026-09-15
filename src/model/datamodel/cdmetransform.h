#pragma once

class CDmeTransform : public CDmeBase
{
public:
	CDmeTransform(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const bool truncateName = false);
	CDmeTransform(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const Vector& posIn, const Quaternion& rotIn, const Vector& scaleIn, const bool truncateName = false);
	virtual ~CDmeTransform()
	{

	}

private:
	Vector position;
	Quaternion orientation;
	Vector scale; // this is for respawn specific models
};
