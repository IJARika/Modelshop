//#include <vector>
//#include <stdio.h>
//#include <iostream>
//#include <fstream>
//#include <filesystem>

#include "../shareddefs.h"
#include "../utils.h"
#include "dmx.h"


//=============================
// DataModel String Dictionary
//=============================
int CDataModelStringDict::AddToStringDict(const char* string)
{
	int entryIdx = 0;

	for (auto& entry : strings)
	{
		if (!strcmp(string, entry.string))
		{
			return entryIdx;
		}

		entryIdx++;
	}

	stringentry_t newEntry{ string, entryIdx };
	strings.push_back(newEntry);
	numStrings++;

	return entryIdx;
}

int CDataModelStringDict::GetStringIndex(const char* string)
{
	int entryIdx = 0;

	for (auto& entry : strings)
	{
		if (!strcmp(string, entry.string))
		{
			return entryIdx;
		}

		entryIdx++;
	}
}

const char* CDataModelStringDict::StringFromIndex(int* index)
{
	return strings.at(*index).string;
}

void CDataModelStringDict::WriteStringDict(char** pData)
{
	//int* stringCount = reinterpret_cast<int*>(*pData);
	//*stringCount = numStrings;
	*reinterpret_cast<int*>(*pData) = numStrings;
	*pData += sizeof(numStrings);

	for (auto& entry : strings)
	{
		int strLength = strnlen_s(entry.string, MAX_PATH_SOURCE) + 1;

		strcpy_s(*pData, strLength, entry.string);

		*pData += strLength;
	}
}


//==========================
// DataModel Attribute List
//==========================

void DmAttribute::WriteAttributeValue(char** pData)
{
	switch (attributeType)
	{
	case AT_UNKNOWN:
		break;
	case AT_ELEMENT:
		break;
	case AT_INT:
		*reinterpret_cast<int*>(*pData) = *static_cast<int*>(attributeValue);
		*pData += sizeof(int);
		break;
	case AT_FLOAT:
		break;
	case AT_BOOL:
		break;
	case AT_STRING:
		break;
	case AT_VOID:
		*reinterpret_cast<int*>(*pData) = 0;
		*pData += 4;
		break;
	case AT_TIME:
		break;
	case AT_COLOR:
		break;
	case AT_VECTOR2:
		break;
	case AT_VECTOR3:
		break;
	case AT_VECTOR4:
		break;
	case AT_QANGLE:
		break;
	case AT_QUATERNION:
		break;
	case AT_VMATRIX:
		break;
	case AT_ELEMENT_ARRAY:
		break;
	case AT_INT_ARRAY:
		break;
	case AT_FLOAT_ARRAY:
		break;
	case AT_BOOL_ARRAY:
		break;
	case AT_STRING_ARRAY:
		break;
	case AT_VOID_ARRAY:
		break;
	case AT_TIME_ARRAY:
		break;
	case AT_COLOR_ARRAY:
		break;
	case AT_VECTOR2_ARRAY:
		break;
	case AT_VECTOR3_ARRAY:
		break;
	case AT_VECTOR4_ARRAY:
		break;
	case AT_QANGLE_ARRAY:
		break;
	case AT_QUATERNION_ARRAY:
		break;
	case AT_VMATRIX_ARRAY:
		break;
	case AT_TYPE_COUNT:
		break;
	case AT_TYPE_INVALID:
		break;
	default:
		break;
	}
}

void CDataModelAttributeList::AddAttribute(int* name, char* nameStr, DmAttributeType_t* type, void* value)
{
	if (Attributes.count(nameStr))
	{
		Error("Tried to add same attribute more than once!");
	}

	DmAttribute* newAttribute{};

	newAttribute->attributeName = *name;
	newAttribute->attributeNameStr = nameStr;
	newAttribute->attributeType = *type;
	newAttribute->attributeValue = value;

	Attributes.emplace(nameStr, newAttribute);

	numAttributes++;
}

void CDataModelAttributeList::AddAttribute(DmAttribute* attribute)
{
	if (Attributes.count(attribute->attributeNameStr))
	{
		Error("Tried to add same attribute more than once!");
	}

	DmAttribute* newAttribute = new DmAttribute;

	*newAttribute = *attribute;

	Attributes.emplace(newAttribute->attributeNameStr, newAttribute);

	numAttributes++;
}

DmAttribute* CDataModelAttributeList::GetAttribute(char* nameStr)
{
	return Attributes.find(nameStr)->second;
}

// unfinished
//int CDataModelAttributeList::GetAttributeValueSize(DmAttribute* attribute)
//{
//	int valueSize = 0;
//
//	int arraySize = reinterpret_cast<int>(attribute->attributeValue); // do not read this unless array
//
//	switch (attribute->attributeType)
//	{
//	case AT_ELEMENT:
//	case AT_INT:
//	case AT_FLOAT:
//		valueSize = sizeof(int); // float should be 4 bytes
//		break;
//	case AT_BOOL:
//		valueSize = sizeof(bool);
//		break;
//	case AT_STRING:
//	case AT_VOID:
//		valueSize = sizeof(int);
//		break;
//	case AT_TIME:
//		valueSize = sizeof(DmeTime_t);
//		Error("error: attribute type 'AT_TIME' not currently supported!!!");
//		break;
//	case AT_COLOR:
//		valueSize = sizeof(VertexColor_t);
//		break;
//	case AT_VECTOR2:
//		valueSize = sizeof(Vector2D);
//		break;
//	case AT_VECTOR3:
//		valueSize = sizeof(Vector);
//		break;
//	case AT_VECTOR4:
//		valueSize = sizeof(Vector4D);
//		break;
//	case AT_QANGLE:
//		valueSize = sizeof(QAngle);
//		break;
//	case AT_QUATERNION:
//		valueSize = sizeof(Quaternion);
//		break;
//	case AT_VMATRIX:
//		//valueSize = 0;
//		Error("error: attribute type 'AT_VMATRIX' not currently supported!!!");
//		break;
//	case AT_ELEMENT_ARRAY:
//	case AT_INT_ARRAY:
//	case AT_FLOAT_ARRAY:
//	case AT_BOOL_ARRAY: // this one may not be right
//		valueSize = sizeof(int) * (arraySize + 1);
//		break;
//	case AT_STRING_ARRAY:
//		Error("error: attribute type 'AT_STRING_ARRAY' not currently supported!!!"); // unsure how to do this currently
//		break;
//	case AT_VOID_ARRAY:
//		valueSize = sizeof(int); // whar
//		break;
//	case AT_TIME_ARRAY:
//		valueSize = sizeof(int) + (sizeof(DmeTime_t) * arraySize);
//		break;
//	case AT_COLOR_ARRAY:
//		valueSize = sizeof(int) + (sizeof(VertexColor_t) * arraySize);
//		break;
//	case AT_VECTOR2_ARRAY:
//		valueSize = sizeof(int) + (sizeof(Vector2D) * arraySize);
//		break;
//	case AT_VECTOR3_ARRAY:
//		valueSize = sizeof(int) + (sizeof(Vector) * arraySize);
//		break;
//	case AT_VECTOR4_ARRAY:
//		valueSize = sizeof(int) + (sizeof(Vector4D) * arraySize);
//		break;
//	case AT_QANGLE_ARRAY:
//		valueSize = sizeof(int) + (sizeof(QAngle) * arraySize);
//		break;
//	case AT_QUATERNION_ARRAY:
//		valueSize = sizeof(int) + (sizeof(Quaternion) * arraySize);
//		break;
//	case AT_VMATRIX_ARRAY:
//		//valueSize = sizeof(int) + (sizeof(QAngle) * arraySize);
//		Error("error: attribute type 'AT_VMATRIX_ARRAY' not currently supported!!!");
//		break;
//	default:
//		Error("error: unknown attribute type used!!!");
//		break;
//	}
//
//	return valueSize;
//}

int CDataModelAttributeList::GetAttributeCount()
{
	return numAttributes;
}

// accessor funcs
int* CDataModelAttributeList::pNumAttributes()
{
	return &numAttributes;
}

std::map<const char*, DmAttribute*>* CDataModelAttributeList::pAttributes()
{
	return &Attributes;
}


//===========
// DataModel
//===========
CDataModel::CDataModel(DataModelType_t type)
{
	dataModelType = type;

	switch (dataModelType)
	{
	case DataModelType_t::DM_MODEL:
		dataModelHeader = "<!-- dmx encoding binary 5 format model 18 -->\n";
		break;
	case DataModelType_t::DM_ANIMATION:
		dataModelHeader = "<!-- dmx encoding binary 5 format model 18 -->\n";
		break;
	case DataModelType_t::DM_PARTICLE:
		dataModelHeader = "<!-- dmx encoding binary 5 format pcf 2 -->";
		break;
	default:
		break;
	}

	rootElement = new DmElement;
	rootAttributeList = new CDataModelAttributeList;

	rootElement->elementType = pStringDict()->AddToStringDict("DmElement");
	rootElement->elementName = pStringDict()->AddToStringDict("root");
	// funny id
	rootElement->elementIndex = elementList.size(); // this is the root element, it should always be 0.
	rootElement->elementTypeStr = "DmElement";
	rootElement->elementNameStr = "root";
	rootElement->elementSet = 0;
	rootElement->elementHash = GetElementHash(rootElement);

	elementList.emplace(rootElement->elementHash, rootElement);
	AddAttributeList(rootAttributeList);

	numElements = 1;
}

CDataModel::~CDataModel()
{
	for (auto& element : elementList)
	{
		delete element.second;
	}

	for (auto& list : attributeList)
	{
		for (auto& attribute : *list->pAttributes())
		{
			delete attribute.second;

		}

		delete list;
	}
}

// may need tweaking
// element
size_t CDataModel::GetElementHash(DmElement* element)
{
	char str[MAX_PATH_SOURCE];

	sprintf_s(str, MAX_PATH_SOURCE, "%s_%s_%i", element->elementTypeStr, element->elementNameStr, element->elementSet);

	return std::hash<char*>{}(str);
}

size_t CDataModel::GetElementHash(char* type, char* name, int* set)
{
	char str[MAX_PATH_SOURCE];

	sprintf_s(str, MAX_PATH_SOURCE, "%s_%s_%i", type, name, *set);

	return std::hash<char*>{}(str);
}

// list part may need to be removed as it should be a given, and we should just add attributes to it after
int CDataModel::AddElement(DmElement* element, CDataModelAttributeList* list)
{
	if (elementList.count(element->elementHash))
		return elementList.find(element->elementHash)->second->elementIndex;

	DmElement* newElement = new DmElement;
	CDataModelAttributeList* newList = new CDataModelAttributeList;

	*newElement = *element;

	newElement->elementIndex = elementList.size();
	elementList.emplace(newElement->elementHash, newElement);
	attributeList.push_back(newList);

	numElements++;

	return element->elementIndex;
}

DmElement* CDataModel::GetElement(size_t* hash)
{
	return elementList.find(*hash)->second;
}

// this will not work as expected
DmElement* CDataModel::GetElement(int* index)
{
	return elementList.at(*index);
}

int CDataModel::GetElementIndex(size_t* hash)
{
	return elementList.find(*hash)->second->elementIndex;
}


// attributes
// unsure why I added this
int CDataModel::AddAttributeList(CDataModelAttributeList* list)
{
	attributeList.push_back(list);
	return attributeList.size() - 1;
}

CDataModelAttributeList* CDataModel::GetAttributeList(int* index)
{
	return attributeList.at(*index);
}


// accessor funcs
DataModelType_t* CDataModel::pDataModelType()
{
	return &dataModelType;
}

DmElement* CDataModel::pRootElement()
{
	return rootElement;
}

CDataModelAttributeList* CDataModel::pRootAttributeList()
{
	return rootAttributeList;
}


std::string* CDataModel::pHeader()
{
	return &dataModelHeader;
}

CDataModelStringDict* CDataModel::pStringDict()
{
	return &stringDict;
}

std::map<size_t, DmElement*>* CDataModel::pElementList()
{
	return &elementList;
}

std::vector<CDataModelAttributeList*>* CDataModel::pAttributeList()
{
	return &attributeList;
}


// write / read
void CDataModel::WriteDataModel(char** pData)
{
	strcpy_s(*pData, dataModelHeader.length() + 1, dataModelHeader.c_str()); // redo this
	*pData += dataModelHeader.length() + 1;

	stringDict.WriteStringDict(pData);

	memcpy_s(*pData, sizeof(int), &numElements, sizeof(int));
	*pData += sizeof(int);

	for (auto& element : elementList)
	{
		DmxElement_t* newElement = reinterpret_cast<DmxElement_t*>(*pData);

		newElement->elementType = element.second->elementType;
		newElement->elementName = element.second->elementName;
		newElement->elementId = element.second->elementId;

		*pData += sizeof(DmxElement_t);
	}

	for (auto& list : attributeList)
	{
		*reinterpret_cast<int*>(*pData) = list->GetAttributeCount();
		*pData += sizeof(int);

		for (auto& attribute : *list->pAttributes())
		{
			DmxAttribute_t* newAttribute = reinterpret_cast<DmxAttribute_t*>(*pData);

			newAttribute->attributeName = attribute.second->attributeName;
			newAttribute->attributeType = attribute.second->attributeType;

			*pData += sizeof(DmxAttribute_t);

			attribute.second->WriteAttributeValue(pData);

		}
	}
}