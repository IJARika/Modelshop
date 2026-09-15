#include <pch.h>

#include <model/datamodel/datamodel.h>


//=============================
// DataModel Version
//=============================

// adds one on export for null terminator
const int DmVersion_t::ToString(char* const buf, const size_t bufSize) const
{
	assertm(bufSize >= dmMaxHeaderLength, "lacks required space for header");

	switch (encoding)
	{
	case DmVersion_t::DMX_ENCODE_BINARY:
	{
		// ignores binary_v1 (no samples, allegedly doesn't exist according to wiki, but is mentioned in code)

		switch (encodingVersion)
		{
		case 0:
		{
			return snprintf(buf, bufSize, DMX_LEGACY_HEADER_FORMAT, DMX_LEGACY_VERSION_STARTING_TOKEN, s_DMXBinaryEncodings[DMX_BINARY_V2], DMX_LEGACY_VERSION_ENDING_TOKEN) + 1;
		}
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		{
			return snprintf(buf, bufSize, DMX_HEADER_FORMAT, DMX_VERSION_STARTING_TOKEN, s_DMXBinaryEncodings[DMX_BINARY], encodingVersion, s_DMXFormats[format], formatVersion, DMX_VERSION_ENDING_TOKEN) + 1;
		}
		default:
		{
			assertm(false, "invalid encoding version");
			return 0;
		}
		}
	}
	case DmVersion_t::DMX_ENCODE_KEYVALUES2:
	{
		switch (encodingVersion)
		{
		case DMX_KEYVALUES2_V1:
		{
			return snprintf(buf, bufSize, DMX_LEGACY_HEADER_FORMAT, DMX_LEGACY_VERSION_STARTING_TOKEN, s_DMXKeyvalueEncodings[encodingVersion], DMX_LEGACY_VERSION_ENDING_TOKEN) + 1;
		}
		case DMX_KEYVALUES2:
		{
			return snprintf(buf, bufSize, DMX_HEADER_FORMAT, DMX_VERSION_STARTING_TOKEN, s_DMXKeyvalueEncodings[encodingVersion], encodingVersion, s_DMXFormats[format], formatVersion, DMX_VERSION_ENDING_TOKEN) + 1;
		}
		default:
		{
			assertm(false, "invalid encoding version");
			return 0;
		}
		}
	}
	case DmVersion_t::DMX_ENCODE_KEYVALUES2_FLAT:
	{
		switch (encodingVersion)
		{
		case DMX_KEYVALUES2_V1:
		{
			return snprintf(buf, bufSize, DMX_LEGACY_HEADER_FORMAT, DMX_LEGACY_VERSION_STARTING_TOKEN, s_DMXKeyvalueFlatEncodings[encodingVersion], DMX_LEGACY_VERSION_ENDING_TOKEN) + 1;
		}
		case DMX_KEYVALUES2:
		{
			return snprintf(buf, bufSize, DMX_HEADER_FORMAT, DMX_VERSION_STARTING_TOKEN, s_DMXKeyvalueFlatEncodings[encodingVersion], encodingVersion, s_DMXFormats[format], formatVersion, DMX_VERSION_ENDING_TOKEN) + 1;
		}
		default:
		{
			assertm(false, "invalid encoding version");
			return 0;
		}
		}
	}
	case DmVersion_t::DMX_ENCODE_COUNT:
	default:
	{
		assertm(false, "invalid encoding");
		return 0;
	}
	}
}

// supports shorter strings so the start and end tokens are not required
const bool DmVersion_t::FromString(const char* const str)
{
	char inputEncoding[32]{};
	char inputFormat[32]{};

	int inputEncodingVersion = -1;
	int inputFormatVersion = -1;

	const char* const versionStart = strstr(str, "encoding");
	if (versionStart && sscanf_s(versionStart, "encoding %s %d format %s %d", inputEncoding, sizeof_u32(inputEncoding), &inputEncodingVersion, inputFormat, sizeof_u32(inputFormat), &inputFormatVersion) == 4)
	{
		encodingVersion = inputEncodingVersion;
		formatVersion = inputFormatVersion;

		assertm(encodingVersion > 0, "bad version, should be in legacy format only");

		if (EncodingFromString(inputEncoding) == false)
		{
			assertm(false, "bad encoding");
			return false;
		}

		if (FormatFromString(inputFormat) == false)
		{
			assertm(false, "bad encoding");
			return false;
		}

		return true;
	}

	// requires 'DMXVersion', because otherwise how would we distinguish them?
	const char* const legacyVersionStar = strstr(str, "DMXVersion");
	if (legacyVersionStar && sscanf_s(legacyVersionStar, "DMXVersion %s", inputEncoding, sizeof_u32(inputEncoding)) == 1)
	{
		encodingVersion = 0;
		formatVersion = 0;

		format = DMX_FORMAT_LEGACY;

		if (EncodingFromString_LEGACY(inputEncoding) == false)
		{
			assertm(false, "bad encoding");
			return false;
		}

		return true;
	}

	encoding = DMX_ENCODE_COUNT;
	format = DMX_FORMAT_COUNT;

	encodingVersion = -1;
	formatVersion = -1;

	assertm(false, "bad version");
	return false;
}

const bool DmVersion_t::EncodingFromString(const char* const str)
{
	if (strncmp(str, s_DMXBinaryEncodings[DMX_BINARY], strlen_ct(s_DMXBinaryEncodings[DMX_BINARY])) == 0)
	{
		encoding = DMX_ENCODE_BINARY;
	}
	else if (strncmp(str, s_DMXKeyvalueEncodings[DMX_KEYVALUES2], strlen_ct(s_DMXKeyvalueEncodings[DMX_KEYVALUES2])) == 0)
	{
		encoding = DMX_ENCODE_KEYVALUES2;
	}
	else if (strncmp(str, s_DMXKeyvalueFlatEncodings[DMX_KEYVALUES2], strlen_ct(s_DMXKeyvalueFlatEncodings[DMX_KEYVALUES2])) == 0)
	{
		encoding = DMX_ENCODE_KEYVALUES2_FLAT;
	}
	else
	{
		assertm(false, "unsupported encoding");
		encoding = DMX_ENCODE_COUNT;

		return false;
	}

	return true;
}

const bool DmVersion_t::EncodingFromString_LEGACY(const char* const str)
{
	if (strncmp(str, s_DMXBinaryEncodings[DMX_BINARY_V1], strlen_ct(s_DMXBinaryEncodings[DMX_BINARY_V1])) == 0)
	{
		assertm(false, "unsupported encoding");
		encoding = DMX_ENCODE_BINARY;
	}
	else if (strncmp(str, s_DMXBinaryEncodings[DMX_BINARY_V2], strlen_ct(s_DMXBinaryEncodings[DMX_BINARY_V2])) == 0)
	{
		encoding = DMX_ENCODE_BINARY;
	}
	else if (strncmp(str, s_DMXKeyvalueEncodings[DMX_KEYVALUES2_V1], strlen_ct(s_DMXKeyvalueEncodings[DMX_KEYVALUES2_V1])) == 0)
	{
		encoding = DMX_ENCODE_KEYVALUES2;
	}
	else if (strncmp(str, s_DMXKeyvalueFlatEncodings[DMX_KEYVALUES2_V1], strlen_ct(s_DMXKeyvalueFlatEncodings[DMX_KEYVALUES2_V1])) == 0)
	{
		encoding = DMX_ENCODE_KEYVALUES2_FLAT;
	}
	else
	{
		assertm(false, "unsupported encoding");
		encoding = DMX_ENCODE_COUNT;

		return false;
	}

	return true;
}

const bool DmVersion_t::FormatFromString(const char* const str)
{
	if (strncmp(str, s_DMXFormats[DMX_FORMAT_MODEL], strlen_ct(s_DMXFormats[DMX_FORMAT_MODEL])) == 0)
	{
		format = DMX_FORMAT_MODEL;
	}
	else if (strncmp(str, s_DMXFormats[DMX_FORMAT_PCF], strlen_ct(s_DMXFormats[DMX_FORMAT_PCF])) == 0)
	{
		format = DMX_FORMAT_PCF;
	}
	else
	{
		assertm(false, "unsupported format");
		format = DMX_FORMAT_COUNT;

		return false;
	}

	return true;
}


//=============================
// DataModel String Dictionary
//=============================

const int CDataModelStringDict::AddToStringDict(const char* string)
{
	if (string == nullptr)
	{
		return -1;
	}

	const int existingIndex = GetStringIndex(string);
	if (existingIndex > -1)
	{
		return existingIndex;
	}

	const int currentIndex = GetStringCount();
	const CDataModelStringEntry entry(string, currentIndex);

	assertm(currentIndex < dmMaxNumString_Binary5, "exceeded max string count");

	byHash.emplace(entry.hash, entry);
	byIndex.push_back(entry.hash);
	numStrings++;

	return currentIndex;
}

// limited to 'dmMaxNumString_Binary2' number of strings (int16)
const int16_t CDataModelStringDict::AddToStringDict_Small(const char* string)
{
	const int currentIndex = AddToStringDict(string);
	assertm(currentIndex < dmMaxNumString_Binary2, "exceeded max string count");

	return static_cast<int16_t>(currentIndex);
}

const int CDataModelStringDict::GetStringIndex(const char* string)
{
	const uint32_t hash = HASH(string);
	if (byHash.contains(hash))
	{
		return byHash.at(hash).index;
	}

	return -1;
}

//===========
// DataModel
//===========

CDataModel::CDataModel(const std::filesystem::path& path, const char* const name, const DmVersion_t* const targetVersion, char* const internalBuffer, const size_t internalBufferSize) : buffer(internalBuffer, internalBufferSize), version(targetVersion),
	fileName(name), filePath(path), rootElement(nullptr), rootAttributes(nullptr), stringDict(nullptr)
{
	elements.reserve(256ull);
	attributes.reserve(256ull);
	elementIndiceMap.reserve(256ull);
}
CDataModel::~CDataModel()
{
	FreeAllocVector(elements);
	FreeAllocVector(attributes);
}

// access
DmElement* const CDataModel::GetElement(const DmElementHash hash) const
{
	/*if (elementIndiceMap.empty())
	{
		const DmElementHashU64 hash64 = hash.AsU64();
		auto it = std::find_if(elements.begin(), elements.end(), [hash64](DmElement* element) -> bool { return hash64 == element->GetHash().AsU64(); });

		return it == elements.end() ? nullptr : *it._Ptr;
	}*/
	if (elementIndiceMap.contains(hash.AsU64()) == false)
	{
		return nullptr;
	}

	const int index = GetElementIndex(hash);
	return GetElement(index);
}

const int CDataModel::GetElementIndex(const DmElementHash hash) const
{
	if (hash == dmElementNull)
	{
		return dmElementIndexInvalid;
	}

	const DmElementHashU64 hash64 = hash.AsU64();

	return (elementIndiceMap.contains(hash64)) ? elementIndiceMap.at(hash64) : dmElementIndexExternal;
}

DmElement* const CDataModel::AddElement(const char* const type, const char* const name, const int set, const size_t numAttributes)
{
	const DmElementHash hash(type, name, set);
	DmElement* element = GetElement(hash);

	if (element)
	{
		//assertm(element->GetElement() == nullptr, "invalid usage, could cause memory leak");
		printf("Element '0x%llx' already exists! Returning ptr to existing element.\n", hash.AsU64());
		return element;
	}

	element = new DmElement(type, name, set);
	DmAttributeList* const attributeList = new DmAttributeList(element, numAttributes);

	element->SetAttributes(attributeList);

	// temporary for quick access while building
	const int currentIndex = static_cast<int>(elements.size());
	elementIndiceMap.emplace(element->GetHash().AsU64(), currentIndex);

	elements.push_back(element);
	attributes.push_back(attributeList);

	return element;
}

DmElement* const CDataModel::AddElement(eDmElementClass type, const char* const name, const int set, const size_t numAttributes)
{
	return AddElement(s_DmElementClassName[type], name, set, numAttributes);
}

void CDataModel::InitForModel()
{
	rootElement = new DmElement("DmElement", "root", 0);
	rootAttributes = new DmAttributeList(rootElement, 2ull); // skeleton & model/anim

	rootElement->SetAttributes(rootAttributes);

	elements.push_back(rootElement);
	attributes.push_back(rootAttributes);

	elementIndiceMap.emplace(rootElement->GetHash().AsU64(), 0);
}

// read/write
#define DM_ADVANCE_BUF(buf, size, count)	\
	buf += count;							\
	size -= count;							\
	assertm(size > 0 && (size + count) > size, "buffer overflow");

const bool CDataModel::WriteDataModel(const bool writeDeps)
{
	assertm(elements.size() == attributes.size(), "something bad has happened");

	char* pCurrent = buffer.Writer();
	size_t remainingSize = buffer.Capacity();

	const int headerSize = version.ToString(pCurrent, remainingSize);
	assertm(dmMaxHeaderLength >= headerSize, "header was larger than expected!");

	DM_ADVANCE_BUF(pCurrent, remainingSize, headerSize);

	switch (version.encoding)
	{
	case DmVersion_t::DMX_ENCODE_BINARY:
	{
		const size_t dataSize = WriteBinaryModel(pCurrent, remainingSize, writeDeps);
		if (dataSize == 0ull)
		{
			return false;
		}

		DM_ADVANCE_BUF(pCurrent, remainingSize, dataSize);

		break;
	}
	case DmVersion_t::DMX_ENCODE_KEYVALUES2:
	case DmVersion_t::DMX_ENCODE_KEYVALUES2_FLAT:
	{
		const size_t dataSize = WriteKeyValuesModel(pCurrent, remainingSize);
		if (dataSize == 0ull)
		{
			return false;
		}

		DM_ADVANCE_BUF(pCurrent, remainingSize, dataSize);

		break;
	}
	default:
	{
		assertm(false, "invalid encoding for write");
		return false;;
	}
	}

	const size_t outSize = pCurrent - buffer.Writer();
	std::filesystem::path outPath(filePath);
	outPath.append(fileName);
	outPath.replace_extension(".dmx");
	if (!CreateDirectories(filePath))
	{
		assertm(false, "failed to create directory");
		return false;
	}

	std::ofstream file(outPath, std::ios::binary | std::ios::beg);
	file.write(buffer.Writer(), outSize);

	return true;
}

const size_t CDataModel::WriteBinaryModel(char* const buf, const size_t size, const bool writeDeps)
{
	if (version.encodingVersion >= DMX_BINARY_VER_STRINGTABLE)
	{
		stringDict = new CDataModelStringDict(GetElementCount() * 2);
	}

	assertm(GetElementCount() == GetAttributeCount(), "should be one list per element");
	int numElements = GetElementCount();

	// store the current map so we can restore it later
	std::unordered_map<DmElementHashU64, int> currentIndiceMap;
	currentIndiceMap.swap(elementIndiceMap);

	// set up the list we'll pull elements from, always contains the elements from this model
	std::vector<DmElement*> elementsForWrite(numElements);

	// copy our local elements into the list we're cooking
	const size_t elementSize = sizeof(DmElement*) * numElements;
	memcpy_s(elementsForWrite.data(), elementSize, elements.data(), elementSize);

	// add elements from dependencies
	if (writeDeps && dependencies.size() > 0ull)
	{
		std::unordered_set<DmElementHashU64> addedElements;

		for (size_t i = 0; i < dependencies.size(); i++)
		{
			CDataModel* const dependent = dependencies.at(i);
			elementsForWrite.reserve(elementsForWrite.size() + dependent->GetElementCount());
			addedElements.reserve(addedElements.size() + dependent->GetElementCount());

			for (int j = 0; j < dependent->GetElementCount(); j++)
			{
				DmElement* const element = dependent->GetElement(j);
				const DmElementHashU64 hash = element->GetHash().AsU64();

				// skip existing elements, and don't bother to merge them
				if (currentIndiceMap.contains(hash) || addedElements.contains(hash))
				{
					continue;
				}

				elementsForWrite.push_back(element);
				addedElements.emplace(hash);
			}
		}

		numElements = static_cast<int>(elementsForWrite.size());
	}

	// assure the root element is at index 0, and reserve space to create our writing lookup map
	assertm(rootElement == elementsForWrite.at(0), "root element was not first element!");
	elementIndiceMap.reserve(numElements);

	// set buffers for writing data
	char* pCurrent = buf;
	size_t remainingSize = size;

	// write number of elements
	*reinterpret_cast<int*>(pCurrent) = numElements;
	DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(int));

	// write elements based on version
	switch (version.encodingVersion)
	{
	case 0:
	case 1:
	{
		for (int i = 0; i < numElements; i++)
		{
			const DmElement* const pElement = elementsForWrite.at(i);

			// type is written as text
			const size_t typeLength = strncpy_mem(pCurrent, remainingSize, pElement->GetType(), dmMaxStringLength);
			DM_ADVANCE_BUF(pCurrent, remainingSize, (typeLength + 1ull));

			// name is written as text
			const size_t nameLength = strncpy_mem(pCurrent, remainingSize, pElement->GetName(), dmMaxStringLength);
			DM_ADVANCE_BUF(pCurrent, remainingSize, (nameLength + 1ull));

			memcpy_s(pCurrent, remainingSize, pElement->GetId(), sizeof(DmElementHashU128));
			DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(DmElementHashU128));

			const DmElementHashU64 hash = pElement->GetHash().AsU64();
			assertm(elementIndiceMap.contains(hash) == false, "element hash collision");
			elementIndiceMap.emplace(hash, i);
		}

		break;
	}
	case DMX_BINARY_VER_STRINGTABLE:
	case DMX_BINARY_VER_TIME:
	{
		for (int i = 0; i < numElements; i++)
		{
			const DmElement* const pElement = elementsForWrite.at(i);

			// type is stored in string table
			*reinterpret_cast<int16_t*>(pCurrent) = stringDict->AddToStringDict_Small(pElement->GetType());
			DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(int16_t));

			// name is written as text
			const size_t nameLength = strncpy_mem(pCurrent, remainingSize, pElement->GetName(), dmMaxStringLength);
			DM_ADVANCE_BUF(pCurrent, remainingSize, (nameLength + 1ull));

			memcpy_s(pCurrent, remainingSize, pElement->GetId(), sizeof(DmElementHashU128));
			DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(DmElementHashU128));

			const DmElementHashU64 hash = pElement->GetHash().AsU64();
			assertm(elementIndiceMap.contains(hash) == false, "element hash collision");
			elementIndiceMap.emplace(hash, i);
		}

		break;
	}
	case DMX_BINARY_VER_GLOBAL_STRINGTABLE:
	{
		DmxElement_v4_t* const pElements = reinterpret_cast<DmxElement_v4_t* const>(pCurrent);
		DM_ADVANCE_BUF(pCurrent, remainingSize, (sizeof(DmxElement_v4_t) * numElements));

		for (int i = 0; i < numElements; i++)
		{
			const DmElement* const pElement = elementsForWrite.at(i);
			pElements[i] = DmxElement_v4_t(stringDict->AddToStringDict_Small(pElement->GetType()), stringDict->AddToStringDict_Small(pElement->GetName()), *pElement->GetId());

			const DmElementHashU64 hash = pElement->GetHash().AsU64();
			assertm(elementIndiceMap.contains(hash) == false, "element hash collision");
			elementIndiceMap.emplace(hash, i);
		}

		break;
	}
	case DMX_BINARY_VER_STRINGTABLE_LARGESYMBOLS:
	{
		DmxElement_t* const pElements = reinterpret_cast<DmxElement_t* const>(pCurrent);
		DM_ADVANCE_BUF(pCurrent, remainingSize, (sizeof(DmxElement_t) * numElements));

		for (int i = 0; i < numElements; i++)
		{
			const DmElement* const pElement = elementsForWrite.at(i);
			pElements[i] = DmxElement_t(stringDict->AddToStringDict(pElement->GetType()), stringDict->AddToStringDict(pElement->GetName()), *pElement->GetId());

			const DmElementHashU64 hash = pElement->GetHash().AsU64();
			assertm(elementIndiceMap.contains(hash) == false, "element hash collision");
			elementIndiceMap.emplace(hash, i);
		}

		break;
	}
	default:
	{
		assertm(false, "invalid encoding version");
		FreeAllocVar(stringDict);
		return 0ull;
	}
	}

	// write attribute list
	for (int i = 0; i < numElements; i++)
	{
		const DmElement* const pElement = elementsForWrite.at(i);
		const DmAttributeList* const pAttributeList = pElement->GetAttributes();

		*reinterpret_cast<int*>(pCurrent) = pAttributeList->GetAttributeCount();
		DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(int));

		for (int attributeIdx = 0; attributeIdx < pAttributeList->GetAttributeCount(); attributeIdx++)
		{
			const DmAttribute* const pAttribute = pAttributeList->GetAttribute(attributeIdx);
			const size_t dataSize = WriteBinaryAttribute(pCurrent, remainingSize, pAttribute);

			DM_ADVANCE_BUF(pCurrent, remainingSize, dataSize);
		}
	}

	size_t dataSize = size - remainingSize;

	// write string table
	if (version.encodingVersion >= DMX_BINARY_VER_STRINGTABLE)
	{
		assertm(numElements, "should not have strings without elements");

		// store written data
		char* const tmp = new char[dataSize];
		memcpy_s(tmp, dataSize, buf, dataSize);

		remainingSize = size;
		pCurrent = buf;

		switch (version.encodingVersion)
		{
		case DMX_BINARY_VER_STRINGTABLE:
		case DMX_BINARY_VER_TIME:
		{
			*reinterpret_cast<int16_t*>(pCurrent) = static_cast<int16_t>(stringDict->GetStringCount());
			DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(int16_t));

			break;
		}
		case DMX_BINARY_VER_GLOBAL_STRINGTABLE:
		case DMX_BINARY_VER_STRINGTABLE_LARGESYMBOLS:
		{
			*reinterpret_cast<int*>(pCurrent) = stringDict->GetStringCount();
			DM_ADVANCE_BUF(pCurrent, remainingSize, sizeof(int));

			break;
		}
		default:
		{
			assertm(false, "invalid encoding version");
			break;
		}
		}

		for (int i = 0; i < stringDict->GetStringCount(); i++)
		{
			const CDataModelStringEntry* const entry = stringDict->GetEntry(i);

			const size_t stringSize = (entry->length + 1);
			memcpy_s(pCurrent, remainingSize, entry->string, stringSize);
			DM_ADVANCE_BUF(pCurrent, remainingSize, stringSize);
		}

		memcpy_s(pCurrent, remainingSize, tmp, dataSize);
		DM_ADVANCE_BUF(pCurrent, remainingSize, dataSize);

		dataSize = pCurrent - buf;

		FreeAllocArray(tmp);
	}

	FreeAllocVar(stringDict);

	// restore old element look up just in case
	elementIndiceMap.swap(currentIndiceMap);

	return dataSize;
}

const size_t CDataModel::WriteBinaryAttribute(char* const buf, const size_t size, const DmAttribute* const attribute) const
{
	size_t offset = 0ull;

	switch (version.encodingVersion)
	{
	case 0:
	case 1:
	{
		const size_t length = strncpy_mem(buf, size, attribute->GetName(), dmMaxStringLength);
		offset += (length + 1ull);

		break;
	}
	case DMX_BINARY_VER_STRINGTABLE:
	case DMX_BINARY_VER_TIME:
	case DMX_BINARY_VER_GLOBAL_STRINGTABLE:
	{
		*reinterpret_cast<int16_t*>(buf) = static_cast<int16_t>(stringDict->AddToStringDict(attribute->GetName()));
		offset += sizeof(int16_t);
		break;
	}
	case DMX_BINARY_VER_STRINGTABLE_LARGESYMBOLS:
	{
		*reinterpret_cast<int*>(buf) = stringDict->AddToStringDict(attribute->GetName());
		offset += sizeof(int);

		break;
	}
	default:
	{
		assertm(false, "invalid version");
		break;
	}
	}

	*reinterpret_cast<DmAttributeType_t*>(buf + offset) = attribute->GetType();
	offset += sizeof(DmAttributeType_t);

	return WriteBinaryAttribute_Value(buf + offset, size - offset, attribute) + offset;
}

const size_t CDataModel::WriteBinaryAttribute_Value(char* const buf, const size_t size, const DmAttribute* const attribute) const
{
	switch (attribute->GetType())
	{
	case DmAttributeType_t::AT_UNKNOWN:
	{
		Error("Attribute used unknown type!\n");
		return 0ull;
	}
	case DmAttributeType_t::AT_ELEMENT:
	{
		return attribute->WriteBinaryElement(buf, this);
	}
	case DmAttributeType_t::AT_INT:
	{
		return attribute->WriteBinarySingle<int>(buf);
	}
	case DmAttributeType_t::AT_FLOAT:
	{
		return attribute->WriteBinarySingle<float>(buf);
	}
	case DmAttributeType_t::AT_BOOL:
	{
		return attribute->WriteBinarySingle<bool>(buf);
	}
	case DmAttributeType_t::AT_STRING:
	{
		switch (version.encodingVersion)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			return attribute->WriteBinaryString(buf, size);
		}
		case DMX_BINARY_VER_GLOBAL_STRINGTABLE:
		{
			*reinterpret_cast<int16_t*>(buf) = static_cast<int16_t>(stringDict->AddToStringDict(reinterpret_cast<const char* const>(attribute->GetDataPtr())));
			return sizeof(int16_t);
		}
		case DMX_BINARY_VER_STRINGTABLE_LARGESYMBOLS:
		{
			*reinterpret_cast<int*>(buf) = stringDict->AddToStringDict(reinterpret_cast<const char* const>(attribute->GetDataPtr()));
			return sizeof(int);
		}
		default:
		{
			assertm(false, "invalid version");
			return 0ull;
		}
		}
	}
	case DmAttributeType_t::AT_VOID:
	{
		return attribute->WriteBinarySingle<int>(buf);
	}
	case DmAttributeType_t::AT_TIME:
	{
		assertm(version.encodingVersion >= DMX_BINARY_VER_TIME, "lower versions use AT_OBJECTID in this slot");
		return attribute->WriteBinarySingle<DmeTime_t>(buf);
	}
	case DmAttributeType_t::AT_COLOR:
	{
		return attribute->WriteBinarySingle<Color32>(buf);
	}
	case DmAttributeType_t::AT_VECTOR2:
	{
		return attribute->WriteBinarySingle<Vector2D>(buf);
	}
	case DmAttributeType_t::AT_VECTOR3:
	{
		return attribute->WriteBinarySingle<Vector>(buf);
	}
	case DmAttributeType_t::AT_VECTOR4:
	{
		return attribute->WriteBinarySingle<Vector4D>(buf);
	}
	case DmAttributeType_t::AT_QANGLE:
	{
		return attribute->WriteBinarySingle<QAngle>(buf);
	}
	case DmAttributeType_t::AT_QUATERNION:
	{
		return attribute->WriteBinarySingle<Quaternion>(buf);
	}
	case DmAttributeType_t::AT_VMATRIX:
	{
		return attribute->WriteBinarySingle<VMatrix>(buf);
	}
	case DmAttributeType_t::AT_ELEMENT_ARRAY:
	{
		return attribute->WriteBinaryElementArray(buf, this);
	}
	case DmAttributeType_t::AT_INT_ARRAY:
	{
		return attribute->WriteBinaryArray<int>(buf);
	}
	case DmAttributeType_t::AT_FLOAT_ARRAY:
	{
		return attribute->WriteBinaryArray<float>(buf);
	}
	case DmAttributeType_t::AT_BOOL_ARRAY:
	{
		return attribute->WriteBinaryArray<bool>(buf);
	}
	case DmAttributeType_t::AT_STRING_ARRAY:
	{
		return attribute->WriteBinaryStringArray(buf, size);
	}
	case DmAttributeType_t::AT_VOID_ARRAY:
	{
		return attribute->WriteBinaryArray<int>(buf);
	}
	case DmAttributeType_t::AT_TIME_ARRAY:
	{
		return attribute->WriteBinaryArray<DmeTime_t>(buf);
	}
	case DmAttributeType_t::AT_COLOR_ARRAY:
	{
		return attribute->WriteBinaryArray<Color32>(buf);
	}
	case DmAttributeType_t::AT_VECTOR2_ARRAY:
	{
		return attribute->WriteBinaryArray<Vector2D>(buf);
	}
	case DmAttributeType_t::AT_VECTOR3_ARRAY:
	{
		return attribute->WriteBinaryArray<Vector>(buf);
	}
	case DmAttributeType_t::AT_VECTOR4_ARRAY:
	{
		return attribute->WriteBinaryArray<Vector4D>(buf);
	}
	case DmAttributeType_t::AT_QANGLE_ARRAY:
	{
		return attribute->WriteBinaryArray<QAngle>(buf);
	}
	case DmAttributeType_t::AT_QUATERNION_ARRAY:
	{
		return attribute->WriteBinaryArray<Quaternion>(buf);
	}
	case DmAttributeType_t::AT_VMATRIX_ARRAY:
	{
		return attribute->WriteBinaryArray<VMatrix>(buf);
	}
	case DmAttributeType_t::AT_TYPE_INVALID:
	default:
	{
		Error("Attribute used invalid type!\n");
		return 0ull;
	}
	}
}

const size_t CDataModel::WriteKeyValuesModel(char* buf, size_t size)
{
	UNUSED(buf);
	UNUSED(size);

	return false;
}


//==========================
// DataModel Element
//==========================

DmElement::DmElement(const char* const typeIn, const char* const nameIn, const int setIn) : type(typeIn), name(nameIn), set(setIn), hash(typeIn, nameIn, setIn), element(nullptr), elementClass(eDmElementClass::DME_BASE), attributes(nullptr)
{
	hash.ToU128(&id);
};

DmElement::DmElement(const eDmElementClass classIn, const char* const nameIn, const int setIn) : type(s_DmElementClassName[classIn]), name(nameIn), set(setIn), hash(s_DmElementClassName[classIn], nameIn, setIn), element(nullptr), elementClass(classIn), attributes(nullptr)
{
	hash.ToU128(&id);
};

DmElement::DmElement(const DmElement* elementIn) : type(elementIn->type), name(elementIn->name), id(elementIn->id), set(elementIn->set), hash(elementIn->hash), element(elementIn->element), elementClass(eDmElementClass::DME_BASE), attributes(nullptr)
{
	hash.ToU128(&id);
};

DmElement::~DmElement()
{
	// calls deconstructor for the specific dmx element class
	FreeAllocVar(element);
}

void DmElement::SetElement(CDmeBase* elementIn, const eDmElementClass classIn)
{
	if (element)
	{
		printf("element '0x%llx' had an existing base class, this might cause issues! freeing to prevent memory leak...\n", hash.AsU64());
		FreeAllocVar(element);
	}

	element = elementIn;
	elementClass = classIn;
}


//==========================
// DataModel Attribute List
//==========================

// attribute
template<typename T> FORCEINLINE const size_t DmAttribute::WriteBinarySingle(char* const buf) const
{
	assertm(count == 1, "did not have a single value");
	assertm(storage != DmAttribute::ATT_VAL_VEC, "cannot use container outside of arrays");

	constexpr size_t size = sizeof(T);

	switch (storage)
	{
	case DmAttribute::ATT_VAL_PTR:
	{
		memcpy_s(buf, size, ptr, size);
		break;
	}
	case DmAttribute::ATT_VAL_RAW:
	{
		*reinterpret_cast<T*>(buf) = *reinterpret_cast<const T*>(&raw);
		break;
	}
	default:
	{
		assertm(false, "bad storage");
		break;
	}
	}

	return size;
}

template<typename T> FORCEINLINE const size_t DmAttribute::WriteBinaryArray(char* const buf) const
{
	const size_t offset = sizeof(int);
	*reinterpret_cast<int*>(buf) = count;

	if (count == 0 && storage != DmAttribute::ATT_VAL_VEC)
	{
		return offset;
	}

	switch (storage)
	{
	case DmAttribute::ATT_VAL_PTR:
	{
		const size_t size = (sizeof(T) * count);
		memcpy_s(buf + offset, size, ptr, size);

		return offset + size;
	}
	case DmAttribute::ATT_VAL_RAW:
	{
		const size_t size = (sizeof(T) * count);
		memcpy_s(buf + offset, size, &raw, size);

		return offset + size;
	}
	case DmAttribute::ATT_VAL_VEC:
	{
		constexpr bool isBoolean = std::is_same<T, bool>::value;
		if constexpr (isBoolean)
		{
			assertm(false, "cannot have boolean in vector");
			return offset;
		}
		else
		{
			const std::vector<T>* const vec = reinterpret_cast<const std::vector<T>*const>(ptr);

			const int localCount = static_cast<int>(vec->size());
			*reinterpret_cast<int*>(buf) = localCount;

			if (localCount == 0)
			{
				return offset;
			}

			const size_t size = (sizeof(T) * localCount);
			memcpy_s(buf + offset, size, vec->data(), size);

			return offset + size;
		}
	}
	default:
	{
		assertm(false, "bad storage");
		return offset;
	}
	}
}

FORCEINLINE const size_t DmAttribute::WriteBinaryString(char* const buf, const size_t size) const
{
	assertm(count == 1, "did not have a single value");
	assertm(storage == DmAttribute::ATT_VAL_PTR, "string should be stored in pointer");

	const char* const string = reinterpret_cast<const char* const>(ptr);

	const size_t length = strncpy_mem(buf, size, string, dmMaxStringLength);

	return (length + 1ull);
}

FORCEINLINE const size_t DmAttribute::WriteBinaryStringArray(char* const buf, const size_t size) const
{
	switch (storage)
	{
	case DmAttribute::ATT_VAL_PTR:
	{
		const char* const* const strings = count > 1 ? reinterpret_cast<const char* const* const>(ptr) : reinterpret_cast<const char* const* const>(&ptr);

		size_t offset = sizeof(int);
		*reinterpret_cast<int*>(buf) = count;

		for (int i = 0; i < count; i++)
		{
			const size_t length = strncpy_mem(buf + offset, size, strings[i], dmMaxStringLength);
			offset += (length + 1ull);
		}

		return offset;
	}
	case DmAttribute::ATT_VAL_VEC:
	{
		const std::vector<const char*>* const vec = reinterpret_cast<const std::vector<const char*>*const>(ptr);

		const char* const* const strings = vec->data();

		const int localCount = static_cast<int>(vec->size());

		size_t offset = sizeof(int);
		*reinterpret_cast<int*>(buf) = localCount;

		for (int i = 0; i < localCount; i++)
		{
			const size_t length = strnlen_s(strings[i], dmMaxStringLength) + 1ull;
			memcpy_s(buf + offset, size, strings[i], length);

			offset += length;
		}

		return offset;
	}
	default:
	{
		assertm(false, "bad storage");

		size_t offset = sizeof(int);
		*reinterpret_cast<int*>(buf) = 0;

		return offset;
	}
	}
}

FORCEINLINE const size_t DmAttribute::WriteBinaryElement(char* const buf, const CDataModel* const datamodel) const
{
	assertm(storage != DmAttribute::ATT_VAL_VEC, "cannot use container outside of arrays");

	DmElementHash element(dmElementNull);

	switch (storage)
	{
	case DmAttribute::ATT_VAL_PTR:
	{
		element = *reinterpret_cast<const DmElementHash* const>(ptr);

		break;
	}
	case DmAttribute::ATT_VAL_RAW:
	{
		element = DmElementHash(raw);

		break;
	}
	default:
	{
		assertm(false, "bad storage");
		break;
	}
	}

	const int index = datamodel->GetElementIndex(element.AsU64());
	*reinterpret_cast<int*>(buf) = index;

	if (index == dmElementIndexExternal)
	{
		assertm(false, "needs to be ascii"); // verify this is correct

		/*DmElementHashU128 guid;
		element.ToU128(&guid);

		RPC_CSTR guidASCII = reinterpret_cast<uint8_t* const>(buf + sizeof(int));
		RPC_STATUS status = UuidToStringA(&guid, &guidASCII);
		assertm(status == RPC_S_OK, "failed to convert to string");

		const size_t length = strnlen_s(buf + sizeof(int), dmMaxStringLength) + 1ull;

		return length + sizeof(int);*/
	}

	return sizeof(int);
}

FORCEINLINE const size_t DmAttribute::WriteBinaryElementArray(char* const buf, const CDataModel* const datamodel) const
{
	size_t offset = sizeof(int);
	*reinterpret_cast<int*>(buf) = count;

	if (count == 0 && storage != DmAttribute::ATT_VAL_VEC)
	{
		return offset;
	}

	const DmElementHash* hashes = nullptr;
	int localCount = count;
	switch (storage)
	{
	case DmAttribute::ATT_VAL_PTR:
	{
		hashes = reinterpret_cast<const DmElementHash* const>(ptr);

		break;
	}
	case DmAttribute::ATT_VAL_RAW:
	{
		hashes = reinterpret_cast<const DmElementHash* const>(&ptr);

		break;
	}
	case DmAttribute::ATT_VAL_VEC:
	{
		const std::vector<DmElementHash>* const vec = reinterpret_cast<const std::vector<DmElementHash>*const>(ptr);

		localCount = static_cast<int>(vec->size());
		*reinterpret_cast<int*>(buf) = localCount;

		if (vec->size() == 0ull)
		{
			return offset;
		}

		hashes = vec->data();

		break;
	}
	default:
	{
		assertm(false, "bad storage");
		break;
	}
	}

	for (int i = 0; i < localCount; i++)
	{
		const DmElementHash element = hashes[i];
		const int index = datamodel->GetElementIndex(element.AsU64());

		*reinterpret_cast<int*>(buf + offset) = index;
		offset += sizeof(int);

		if (index == dmElementIndexExternal)
		{
			assertm(false, "needs to be ascii"); // verify this is correct

			/*DmElementHashU128 guid;
			element.ToU128(&guid);

			RPC_CSTR guidASCII = reinterpret_cast<uint8_t* const>(buf + sizeof(int));
			RPC_STATUS status = UuidToStringA(&guid, &guidASCII);
			assertm(status == RPC_S_OK, "failed to convert to string");

			const size_t length = strnlen_s(buf + sizeof(int), dmMaxStringLength) + 1ull;

			offset += length;*/
		}
	}

	return offset;
}

void DmAttribute::InitData(const DmAttributeType_t typeIn, const uint64_t rawIn, const int countIn)
{
	count = countIn;
	type = typeIn;
	raw = rawIn;

	storage = ATT_VAL_RAW;
}

void DmAttribute::InitData(const DmAttributeType_t typeIn, const void* const ptrIn, const int countIn)
{
	count = countIn;
	type = typeIn;
	ptr = ptrIn;

	storage = ATT_VAL_PTR;
}

// list
DmAttribute* const DmAttributeList::GetAttribute(const char* const name) const
{
	const uint32_t hash = HASH(name);

	auto it = std::find_if(attributes.begin(), attributes.end(), [hash](DmAttribute* att) -> bool { return hash == att->GetHash(); });

	return it == attributes.end() ? nullptr : *it._Ptr;
}

// save the index for quick lookup later
DmAttribute* const DmAttributeList::GetAttribute(const char* const name, int& index) const
{
	if (index > -1 && index < GetAttributeCount())
	{
		DmAttribute* const pAttribute = GetAttribute(index);
		const uint32_t hash = HASH(name);

		assertm(pAttribute->GetHash() == hash, "index did not match name");

		return pAttribute;
	}

	const uint32_t hash = HASH(name);

	for (int i = 0; i < GetAttributeCount(); i++)
	{
		DmAttribute* const pAttribute = GetAttribute(i);

		if (pAttribute->GetHash() == hash)
		{
			index = i;
			return pAttribute;
		}
	}

	return nullptr;
}

const int DmAttributeList::AddAttribute(CDataModel* const dataModel, const DmAttribute& attribute)
{
	constexpr size_t sizeAtt = sizeof(DmAttribute);

	DmAttribute* const pAttribute = reinterpret_cast<DmAttribute* const>(dataModel->ReserveData(sizeAtt));
	memcpy_s(pAttribute, sizeAtt, &attribute, sizeAtt);

	const int index = static_cast<int>(attributes.size());
	attributes.push_back(pAttribute);

	return index;
}

const bool DmAttributeList::EditAttribute(DmAttribute* const attribute, const DmAttributeType_t type, const void* const value, const int count)
{
	UNUSED(type);

	// same data
	if (attribute->GetDataPtr() == value)
	{
		attribute->InitCount(count);
		return true;
	}

	return false;
}