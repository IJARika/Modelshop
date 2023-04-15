#ifndef CDMEVERTEXDATA_H
#define CDMEVERTEXDATA_H

#pragma once

#include "cdmebase.h"

class CDmeVertexData : public CDmeBase
{
public:
	CDmeVertexData(CDataModel* pDataModel, const char* name, const int set);
	~CDmeVertexData();

	enum Attributes_t
	{
		AT_POSITION,
		AT_NORMAL,
		AT_TANGENT,
		AT_TEXCOORD,
		AT_TEXCOORD2,
		AT_COLOR,
		AT_JOINT_WEIGHTS,
		AT_JOINT_INDICES,
		AT_COUNT
	};

	// what are indices here
	std::vector<const char*> vertexFormat;
	int jointCount;
	bool flipVCoordinates = 1; // ?
	std::vector<Vector*> positions;
	std::vector<int*> positionsIndices; // ???
	std::vector<Vector*> normals;
	std::vector<int*> normalsIndices; // ???
	std::vector<Vector2D*> textureCoordinates;
	std::vector<int*> textureCoordinatesIndices; // ???
	std::vector<float*> jointWeights; // needs to support potentially 16 weights per vert
	std::vector<int*> jointIndices; // bone idx??

	// optional
	std::vector<VertexColor_t*> colors;
	std::vector<int*> colorIndices; // ???
	std::vector<Vector2D*> textureCoordinates2;
	std::vector<int*> textureCoordinates2Indices; // ???

private:

};

#endif // !CDMEVERTEXDATA_H


#pragma once
