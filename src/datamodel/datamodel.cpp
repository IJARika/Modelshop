//#include <vector>
//#include <stdio.h>
//#include <iostream>
//#include <fstream>
//#include <filesystem>
#include <algorithm>

#include "../utils.h"
#include "../datamodel/datamodel.h"


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

// thank you amogus
template<class T>
__forceinline void DmAttribute::WriteAttributeValue(char** pData, void* attrVal, int attrOptSize)
{
	*reinterpret_cast<T*>(*pData) = *reinterpret_cast<T*>(attrVal);

	if (attrOptSize > -1)
		*pData += attrOptSize;
	else
		*pData += sizeof(T);
}

template<class T>
void DmAttribute::WriteAttributeArray(char** pData, int* numVals, std::vector<void*> attrVals, int attrOptSize)
{
	*reinterpret_cast<int*>(*pData) = *numVals;
	*pData += sizeof(int);

	for (auto& value : attrVals)
	{
		WriteAttributeValue<T>(pData, value);
	}
}

void DmAttribute::WriteAttributeValue(char** pData)
{
	switch (attributeType)
	{
	case AT_UNKNOWN:
		Error("Attribute used unknown type!\n");
		break;
	case AT_ELEMENT:
	case AT_INT:
		WriteAttributeValue<int>(pData, attributeValue);
		break;
	case AT_FLOAT:
		WriteAttributeValue<float>(pData, attributeValue);
		break;
	case AT_BOOL:
		WriteAttributeValue<bool>(pData, attributeValue);
		break;
	case AT_STRING:
	case AT_VOID:
		WriteAttributeValue<int>(pData, attributeValue);
		break;
	case AT_TIME:
		Error("Attribute 'AT_TIME' not fully implemented\n");
		WriteAttributeValue<DmeTime_t>(pData, attributeValue);
		break;
	case AT_COLOR:
		WriteAttributeValue<VertexColor_t>(pData, attributeValue);
		break;
	case AT_VECTOR2:
		WriteAttributeValue<Vector2D>(pData, attributeValue);
		break;
	case AT_VECTOR3:
		WriteAttributeValue<Vector>(pData, attributeValue);
		break;
	case AT_VECTOR4:
		WriteAttributeValue<Vector4D>(pData, attributeValue);
		break;
	case AT_QANGLE:
		WriteAttributeValue<QAngle>(pData, attributeValue);
		break;
	case AT_QUATERNION:
		WriteAttributeValue<Quaternion>(pData, attributeValue);
		break;
	case AT_VMATRIX:
		Error("Attribute 'AT_VMATRIX' not fully implemented\n");
		break;
	case AT_ELEMENT_ARRAY:
	case AT_INT_ARRAY:
		WriteAttributeArray<int>(pData, &numValues, attributeValues);
		break;
	case AT_FLOAT_ARRAY:
		WriteAttributeArray<float>(pData, &numValues, attributeValues);
		break;
	case AT_BOOL_ARRAY:
		WriteAttributeArray<bool>(pData, &numValues, attributeValues);
		break;
	case AT_STRING_ARRAY: // custom case for strings
		*reinterpret_cast<int*>(*pData) = numValues;
		*pData += sizeof(int);

		for (auto& value : attributeValues)
		{
			strcpy_s(*pData, MAX_PATH_SOURCE, reinterpret_cast<char*>(value));
			*pData += strnlen_s(reinterpret_cast<char*>(value), MAX_PATH_SOURCE) + 1;
		}
		break;
	case AT_VOID_ARRAY:
		WriteAttributeArray<int>(pData, &numValues, attributeValues);
		break;
	case AT_TIME_ARRAY:
		Error("Attribute 'AT_TIME_ARRAY' not fully implemented\n");
		WriteAttributeArray<DmeTime_t>(pData, &numValues, attributeValues);
		break;
	case AT_COLOR_ARRAY:
		WriteAttributeArray<VertexColor_t>(pData, &numValues, attributeValues);
		break;
	case AT_VECTOR2_ARRAY:
		WriteAttributeArray<Vector2D>(pData, &numValues, attributeValues);
		break;
	case AT_VECTOR3_ARRAY:
		WriteAttributeArray<Vector>(pData, &numValues, attributeValues);
		break;
	case AT_VECTOR4_ARRAY:
		WriteAttributeArray<Vector4D>(pData, &numValues, attributeValues);
		break;
	case AT_QANGLE_ARRAY:
		WriteAttributeArray<QAngle>(pData, &numValues, attributeValues);
		break;
	case AT_QUATERNION_ARRAY:
		WriteAttributeArray<Quaternion>(pData, &numValues, attributeValues);
		break;
	case AT_VMATRIX_ARRAY:
		Error("Attribute 'AT_VMATRIX_ARRAY' not fully implemented\n");
		break;
	case AT_TYPE_COUNT:
	case AT_TYPE_INVALID:
	default:
		Error("Attribute used invalid type!\n");
		break;
	}
}

void CDataModelAttributeList::AddAttribute(int* name, char* nameStr, DmAttributeType_t* type, void* value)
{
	if (attributes.count(nameStr))
	{
		Error("Tried to add same attribute more than once!");
	}

	DmAttribute* newAttribute = new DmAttribute;

	newAttribute->attributeName = *name;
	newAttribute->attributeNameStr = nameStr;
	newAttribute->attributeType = *type;
	newAttribute->attributeValue = value;

	attributes.emplace(nameStr, newAttribute);
}

void CDataModelAttributeList::AddAttribute(DmAttribute* attribute)
{
	if (attributes.count(attribute->attributeNameStr))
	{
		Error("Tried to add same attribute more than once!");
	}

	DmAttribute* newAttribute = new DmAttribute;

	*newAttribute = *attribute;

	attributes.emplace(newAttribute->attributeNameStr, newAttribute);
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
		//dataModelHeader = "<!-- dmx encoding binary 5 format model 18 -->\n";
		//break;
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

	rootElement->elementTypeStr = "DmElement";
	rootElement->elementNameStr = "root";
	rootElement->elementSet = 0;

	rootElement->elementHash = GetElementHash(rootElement); // after because the above fields need to be filled

	rootElement->elementType = pStringDict()->AddToStringDict("DmElement");
	rootElement->elementName = pStringDict()->AddToStringDict("root");

	// funny id

	rootElement->elementIndex = elementList.size(); // this is the root element, it should always be 0.
	rootAttributeList->attributeListIndex = rootElement->elementIndex;
	

	elementList.push_back(rootElement);
	attributeList.push_back(rootAttributeList);
}

CDataModel::~CDataModel()
{
	for (auto& element : elementList)
	{
		delete element;
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
// cut this?
size_t CDataModel::GetElementHash(const DmElement* element)
{
	char str[MAX_PATH_SOURCE];

	sprintf_s(str, MAX_PATH_SOURCE, "%s_%s_%i", element->elementTypeStr, element->elementNameStr, element->elementSet);

	return std::hash<char*>{}(str);
} 

// strip this?
size_t CDataModel::GetElementHash(char* type, char* name, int set)
{
	char str[MAX_PATH_SOURCE];

	sprintf_s(str, MAX_PATH_SOURCE, "%s_%s_%i", type, name, set);

	return std::hash<char*>{}(str);
}

size_t CDataModel::GetElementHash(const char* type, const char* name, const int set)
{
	char str[MAX_PATH_SOURCE];

	sprintf_s(str, MAX_PATH_SOURCE, "%s_%s_%i", type, name, set);

	return std::hash<char*>{}(str);
}

DmElement* const CDataModel::FindElement(const size_t hash)
{
	auto it = std::find_if(elementList.begin(), elementList.end(), [hash](DmElement* element) -> bool { return element->elementHash == hash; });

	if (it != elementList.end())
	{
		return *it._Ptr;
	}

	return nullptr;
}

// list part may need to be removed as it should be a given, and we should just add attributes to it after
int CDataModel::AddElement(DmElement* element, CDataModelAttributeList* list) // list doesn't even get used lol
{
	DmElement* newElement = FindElement(element->elementHash);

	if (newElement)
	{
		printf("Element '0x%llx' already exists! Returning ptr to existing element.\n", element->elementHash);
		return newElement->elementIndex;
	}

	newElement = new DmElement;
	CDataModelAttributeList* newList = new CDataModelAttributeList;

	*newElement = *element;

	newElement->elementIndex = elementList.size();
	newList->attributeListIndex = newElement->elementIndex;
	elementList.push_back(newElement);
	attributeList.push_back(newList);

	return newElement->elementIndex;
}

int CDataModel::AddElement(const size_t hash)
{
	DmElement* newElement = FindElement(hash);

	if (newElement)
	{
		printf("Element '0x%llx' already exists! Returning ptr to existing element.\n", hash);
		return newElement->elementIndex;
	}

	newElement = new DmElement;
	CDataModelAttributeList* newList = new CDataModelAttributeList;

	newElement->elementIndex = elementList.size();
	newList->attributeListIndex = newElement->elementIndex;
	newElement->elementHash = hash;

	elementList.push_back(newElement);
	attributeList.push_back(newList);

	return newElement->elementIndex;
}

DmElement* CDataModel::AddElement(const char* type, const char* name, int set, DmElement* pElement, CDataModelAttributeList* pAttributes)
{
	size_t hash = GetElementHash(type, name, set);

	DmElement* newElement = FindElement(hash);

	if (newElement)
	{
		printf("Element '0x%llx' already exists! Returning ptr to existing element.\n", hash);
		return newElement;
	}

	newElement = new DmElement;
	CDataModelAttributeList* newList = new CDataModelAttributeList;

	newElement->elementHash = hash;

	newElement->elementTypeStr = type;
	newElement->elementNameStr = name;
	newElement->elementSet = set;

	newElement->elementType = stringDict.AddToStringDict(type);
	newElement->elementName = stringDict.AddToStringDict(name);

	//newElement->elementId = something;

	newElement->elementIndex = elementList.size();
	newList->attributeListIndex = newElement->elementIndex;

	elementList.push_back(newElement);
	attributeList.push_back(newList);

	if (pElement)
	{
		pElement = newElement;
	}

	if (pAttributes)
	{
		pAttributes = newList;
	}

	return newElement;
}

DmElement* const CDataModel::GetElement(const size_t hash)
{
	DmElement* element = FindElement(hash);

	if (element)
	{
		return element;
	}

	Error("Could not find element '0x%llx', exiting...\n", hash);
}

inline const int CDataModel::GetElementIndex(const size_t hash)
{
	DmElement* element = FindElement(hash);

	return element->elementIndex;
}


// attributes
// unsure why I added this
//int CDataModel::AddAttributeList(CDataModelAttributeList* list)
//{
//	attributeList.push_back(list);
//	return attributeList.size() - 1;
//}

void CDataModel::AddAttribute(CDataModelAttributeList* list, const char* name, DmAttributeType_t type, void* value)
{
	DmAttribute* newAttribute = new DmAttribute;

	newAttribute->attributeNameStr = name;
	newAttribute->attributeName = pStringDict()->AddToStringDict(name);

	newAttribute->attributeType = type;

	newAttribute->attributeValue = value;

	list->pAttributes()->emplace(name, newAttribute);
}

void CDataModel::AddAttributeArray(CDataModelAttributeList* list, const char* name, DmAttributeType_t type, std::vector<void*> values, int numValues)
{
	DmAttribute* newAttribute = new DmAttribute;

	newAttribute->attributeNameStr = name;
	newAttribute->attributeName = stringDict.AddToStringDict(name);

	newAttribute->attributeType = type;

	newAttribute->attributeValues = values;
	newAttribute->numValues = numValues;

	list->pAttributes()->emplace(name, newAttribute);
}

// write / read
void CDataModel::WriteDataModel(char** pData)
{
	strcpy_s(*pData, MAX_PATH_SOURCE, dataModelHeader.c_str());
	*pData += strnlen_s(dataModelHeader.c_str(), MAX_PATH_SOURCE) + 1;

	stringDict.WriteStringDict(pData);

	*reinterpret_cast<int*>(*pData) = GetElementCount();
	*pData += sizeof(int);

	int idx = 0;

	for (auto& element : elementList)
	{
		if (idx != element->elementIndex)
		{
			Error("Element index did not match position in 'elementList'; expected: %i, got: %i\n", idx, element->elementIndex);
		}

		DmxElement_t* newElement = reinterpret_cast<DmxElement_t*>(*pData);

		newElement->elementType = element->elementType;
		newElement->elementName = element->elementName;
		newElement->elementId = element->elementId;

		*pData += sizeof(DmxElement_t);

		idx++;
	}

	idx = 0;

	for (auto& list : attributeList)
	{
		if (idx != list->attributeListIndex)
		{
			Error("Attribute list index did not match position in 'attributeList'; expected: %i, got: %i\n", idx, list->attributeListIndex);
		}

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

		idx++;
	}

}