#include <stdlib.h>
#include <vector>
#include <string>
#include <rpc.h>

#pragma once

#define DMX_VERSION_STARTING_TOKEN "<!-- dmx"
#define DMX_VERSION_ENDING_TOKEN "-->"

#define CURRENT_BINARY_ENCODING		5
#define CURRENT_BINARY_VERSION		18


//=============================
// DataModel String Dictionary
//=============================
struct stringentry_t
{
	// may need more vars here if it is needed.
	// if not swap to std::map??
	const char* string;
	int index;
};

class CDataModelStringDict
{
public:
	CDataModelStringDict();
	int AddToStringDict(const char* string); // change type to int?
	void WriteStringDict(char** pData); // write stringdict into dmx file buffer

	int GetStringIndex(const char* string);
	const char* StringFromIndex(int* index);

private:
	int numStrings;
	std::vector<stringentry_t> strings;
};


//==========================
// DataModel Attribute List
//==========================
enum DmAttributeType_t : char
{
	AT_UNKNOWN = 0,

	AT_FIRST_VALUE_TYPE = 1,

	AT_ELEMENT = AT_FIRST_VALUE_TYPE,
	AT_INT = 2,
	AT_FLOAT = 3,
	AT_BOOL = 4,
	AT_STRING = 5,
	AT_VOID = 6,
	AT_TIME = 7,
	AT_COLOR = 8, //rgba
	AT_VECTOR2 = 9,
	AT_VECTOR3 = 10,
	AT_VECTOR4 = 11,
	AT_QANGLE = 12,
	AT_QUATERNION = 13,
	AT_VMATRIX = 14,

	AT_FIRST_ARRAY_TYPE = 15,

	AT_ELEMENT_ARRAY = AT_FIRST_ARRAY_TYPE,
	AT_INT_ARRAY = 16,
	AT_FLOAT_ARRAY = 17,
	AT_BOOL_ARRAY = 18,
	AT_STRING_ARRAY = 19,
	AT_VOID_ARRAY = 20,
	AT_TIME_ARRAY = 21,
	AT_COLOR_ARRAY = 22,
	AT_VECTOR2_ARRAY = 23,
	AT_VECTOR3_ARRAY = 24,
	AT_VECTOR4_ARRAY = 25,
	AT_QANGLE_ARRAY = 26,
	AT_QUATERNION_ARRAY = 27,
	AT_VMATRIX_ARRAY = 28,
	AT_TYPE_COUNT = 29,

	AT_TYPE_INVALID = 0xff // unsure the actual value for this
};

struct DmxAttribute_t
{
	int name; // string dictionary index
	char type; // DmAttributeType_t
	
	void* value;
};

class CDataModelAttributeList
{
public:
	void AddAtribute(int* name, char* type, void* value);

private:
	int numAttributes;
	std::vector<DmxAttribute_t> attributes;
};


//========================
// DataModel Element List
//========================
struct DmxElement_t
{
	int type; // string dictionary index
	int name; // string dictionary index
	UUID uuid; // little-endian, this is GUID/UUID type I think
};

class CDataModelElementList
{
public:
	int AddElement(int type, int name, UUID& uuid, CDataModelAttributeList& attributes);

private:
	int numElements;
	std::vector<DmxElement_t> elements;
	std::vector< CDataModelAttributeList> attributeList; // should match the total number of elements
};


//===========
// DataModel
//===========
class CDataModel
{
public:
	CDataModel(const char* header); // will set header on creation

	// accessor funcs
	std::string* pHeader();
	CDataModelStringDict* pStringDict();
	CDataModelElementList* pElementList();

	void WriteDataModel(char** pData);

private:
	std::string DMXHeader;
	CDataModelStringDict StringDict;
	CDataModelElementList ElementList;
};

void DMXFromMDL(char* pMdlBuf, const std::string fileDir);