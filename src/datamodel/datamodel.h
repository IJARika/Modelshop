#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
//#include <rpc.h>
#include <guiddef.h>

#include "../shareddefs.h"
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

	AT_FIRST_VALUE_TYPE,

	AT_ELEMENT = AT_FIRST_VALUE_TYPE,
	AT_INT,
	AT_FLOAT,
	AT_BOOL,
	AT_STRING, // int index for string dictionary
	AT_VOID,
	AT_TIME,
	AT_COLOR, //rgba
	AT_VECTOR2,
	AT_VECTOR3,
	AT_VECTOR4,
	AT_QANGLE,
	AT_QUATERNION,
	AT_VMATRIX,

	AT_FIRST_ARRAY_TYPE,

	AT_ELEMENT_ARRAY = AT_FIRST_ARRAY_TYPE,
	AT_INT_ARRAY,
	AT_FLOAT_ARRAY,
	AT_BOOL_ARRAY,
	AT_STRING_ARRAY,
	AT_VOID_ARRAY,
	AT_TIME_ARRAY,
	AT_COLOR_ARRAY,
	AT_VECTOR2_ARRAY,
	AT_VECTOR3_ARRAY,
	AT_VECTOR4_ARRAY,
	AT_QANGLE_ARRAY,
	AT_QUATERNION_ARRAY,
	AT_VMATRIX_ARRAY,
	AT_TYPE_COUNT,

	AT_TYPE_INVALID // anything else should be this
};

// need this
struct DmeTime_t
{

};

//struct DmxAttributeArray_t
//{
//	int numAttributeValues;
//	void* attributeValues;
//
//	void WriteAttributeValue(char** pData);
//};

#pragma pack(push, 1)
struct DmxAttribute_t
{
	int attributeName; // string dictionary index
	DmAttributeType_t attributeType; // DmAttributeType_t
};
#pragma pack(pop)

struct DmAttribute
{
	const char* attributeNameStr;
	int attributeName; // string dictionary index

	DmAttributeType_t attributeType; // DmAttributeType_t

	//int numValues; // cut this
	void* attributeValue;
	//std::vector<void*>* attributeValues;	

	template<class T> __forceinline void WriteSingle(char** pData, void* attrVal, int attrOptSize = -1);
	//template<class T> void WriteAttributeArray(char** pData, int* numVals, std::vector<void*>* attrVals, int attrOptSize = -1);
	template<class T> void WriteArray(char** pData, void* attrVals, int attrOptSize = -1);
	__forceinline void WriteStrings(char** pData, void* attrStrs);
	void WriteValue(char** pData);
};

class CDataModelAttributeList
{
public:
	void AddAttribute(int* name, char* nameStr, DmAttributeType_t* type, void* value);
	void AddAttribute(DmAttribute* attribute);
	inline DmAttribute* const GetAttribute(const char* nameStr) { return attributes.find(nameStr)->second; };

	//int GetAttributeValueSize(DmAttribute* attribute);
	inline const int GetAttributeCount() const { return attributes.size(); };

	// accessor funcs
	//inline int* pNumAttributes() { return &numAttributes; };
	inline std::map<const char*, DmAttribute*>* pAttributes() { return &attributes; };
	//inline int* pIndex() { return &attributeListIndex; }

	// reading/writing
	void WriteAttributeList(char** pData);

	int attributeListIndex;

private:
	std::map<const char*, DmAttribute*> attributes;
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
	const char* elementTypeStr;
	const char* elementNameStr;

	GUID elementId; // unique id for this element
	int elementType; // string dict index of type string
	int elementName; // string dict index of name string

	int elementIndex = -1; // index in the ElementList array
	
	int elementSet; // if there are duplicate sets of elements, which is this one in?
	size_t elementHash;

	void* pDme; // pointer to specific element class using this
	// ptr to attr list?
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
	CDataModel() = default;
	CDataModel(DataModelType_t datamodelType); // will set header on creation
	~CDataModel();

	// element
	size_t GetElementHash(const DmElement* element);
	size_t GetElementHash(char* type, char* name, int set);
	size_t GetElementHash(const char* type, const char* name, const int set);

	DmElement* const FindElement(const size_t hash);

	int AddElement(DmElement* element, CDataModelAttributeList* list);
	int AddElement(const size_t hash); // adds a new element without need for input
	DmElement* AddElement(const char* type, const char* name, int set, DmElement* pElement = nullptr, CDataModelAttributeList* pAttributes = nullptr);

	DmElement* const GetElement(const size_t hash);
	inline DmElement* const GetElement(const int index) { return elementList.at(index); };
	inline const int GetElementIndex(const size_t hash);

	// attributes
	//int AddAttributeList(CDataModelAttributeList* list);
	inline CDataModelAttributeList* const GetAttributeList(const int index) { return attributeList.at(index); };
	void AddAttribute(CDataModelAttributeList* pAttributes, const char* attributeName, DmAttributeType_t type, void* value);
	void EditAttribute(CDataModelAttributeList* pAttributes, const char* attributeName, DmAttributeType_t type, void* newValue);
	//void AddAttributeArray(CDataModelAttributeList* list, const char* name, DmAttributeType_t type, std::vector<void*>* values, int numValues = -1);

	// general
	inline const int GetElementCount() const { return elementList.size(); };

	// accessor funcs
	inline DataModelType_t* const pDataModelType() { return &dataModelType; }
	inline DmElement* const pRootElement() { return rootElement; }
	inline CDataModelAttributeList* const pRootAttributeList() { return rootAttributeList; }

	inline std::string* const pHeader() { return &dataModelHeader; }
	inline CDataModelStringDict* const pStringDict() { return &stringDict; }
	inline std::vector<DmElement*>* const pElementList() { return &elementList; }
	inline std::vector<CDataModelAttributeList*>* const pAttributeList() { return &attributeList; }

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

	std::vector<DmElement*> elementList;
	std::vector<CDataModelAttributeList*> attributeList; // should match the total number of elements
};