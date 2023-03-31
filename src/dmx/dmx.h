#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
//#include <rpc.h>
#include <guiddef.h>

#include "../math/vector.h"
#include "../math/vector2d.h"
#include "../math/vector4d.h"
#include "../math/vertexcolor.h"

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
	int AddToStringDict(const char* string); // change type to int?
	int GetStringIndex(const char* string);
	const char* StringFromIndex(int* index);

	void WriteStringDict(char** pData); // write stringdict into dmx file buffer

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
	AT_STRING = 5, // int index for string dictionary
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

	AT_TYPE_INVALID // anything else should be this
};

// need this
struct DmeTime_t
{

};

struct DmxAttributeArray_t
{
	int numAttributeValues;
	void* attributeValues;
};

#pragma pack(push, 1)
struct DmxAttribute_t
{
	int attributeName; // string dictionary index
	DmAttributeType_t attributeType; // DmAttributeType_t
};
#pragma pack(pop)

struct DmAttribute
{
	void* attributeValue;
	int attributeName; // string dictionary index
	DmAttributeType_t attributeType; // DmAttributeType_t

	const char* attributeNameStr;

	void WriteAttributeValue(char** pData);
};

class CDataModelAttributeList
{
public:
	void AddAttribute(int* name, char* nameStr, DmAttributeType_t* type, void* value);
	void AddAttribute(DmAttribute* attribute);
	DmAttribute* GetAttribute(char* nameStr);

	//int GetAttributeValueSize(DmAttribute* attribute);
	int GetAttributeCount();

	// accessor funcs
	int* pNumAttributes();
	std::map<const char*, DmAttribute*>* pAttributes();

	// reading/writing
	void WriteAttributeList(char** pData);

private:
	int numAttributes;
	std::map<const char*, DmAttribute*> Attributes;
};


//===================
// DataModel Element
//===================
struct DmxElement_t
{
	int elementType; // string dict index of type string
	int elementName; // string dict index of name string
	GUID elementId; // unique id for this element
};

struct DmElement
{
	int elementType; // string dict index of type string
	int elementName; // string dict index of name string
	GUID elementId; // unique id for this element

	int elementIndex; // index in the ElementList array
	const char* elementTypeStr;
	const char* elementNameStr;
	int elementSet; // if there are duplicate sets of elements, which is this one in?
	size_t elementHash;
};


//===========
// DataModel
//===========
enum class DataModelType_t
{
	DM_MODEL,
	DM_ANIMATION,
	DM_PARTICLE // unused for now likely
};

class CDataModel
{
public:
	CDataModel(DataModelType_t datamodelType); // will set header on creation
	~CDataModel();

	// element
	size_t GetElementHash(DmElement* element);
	size_t GetElementHash(char* type, char* name, int* set);
	int AddElement(DmElement* element, CDataModelAttributeList* list);
	DmElement* GetElement(size_t* hash);
	DmElement* GetElement(int* index);
	int GetElementIndex(size_t* hash);

	// attributes
	int AddAttributeList(CDataModelAttributeList* list);
	CDataModelAttributeList* GetAttributeList(int* index);

	// accessor funcs
	DataModelType_t* pDataModelType();
	DmElement* pRootElement();
	CDataModelAttributeList* pRootAttributeList();

	std::string* pHeader();
	CDataModelStringDict* pStringDict();
	std::map<size_t, DmElement*>* pElementList();
	std::vector<CDataModelAttributeList*>* pAttributeList();

	// writing/reading
	void WriteDataModel(char** pData);

private: // these need better packing (?)
	// interal
	DataModelType_t dataModelType;
	DmElement* rootElement;
	CDataModelAttributeList* rootAttributeList;

	// for writing
	std::string dataModelHeader;
	CDataModelStringDict stringDict;

	int numElements;
	std::map<size_t, DmElement*> elementList;
	std::vector<CDataModelAttributeList*> attributeList; // should match the total number of elements
};