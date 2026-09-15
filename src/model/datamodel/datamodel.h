#pragma once

// elements
// combine includes for dmx here?

#define DMX_LEGACY_HEADER_FORMAT "%s %s %s\n"
#define DMX_HEADER_FORMAT "%s encoding %s %d format %s %d %s\n"

#define DMX_LEGACY_VERSION_STARTING_TOKEN "<!-- DMXVersion"
#define DMX_LEGACY_VERSION_ENDING_TOKEN "-->"

#define DMX_VERSION_STARTING_TOKEN "<!-- dmx"
#define DMX_VERSION_ENDING_TOKEN "-->"

#define DMX_BINARY_VER_STRINGTABLE				2
#define DMX_BINARY_VER_TIME						3
#define DMX_BINARY_VER_GLOBAL_STRINGTABLE		4
#define DMX_BINARY_VER_STRINGTABLE_LARGESYMBOLS 5

#define CURRENT_BINARY_ENCODING 5

#define CURRENT_BINARY_ENCODING			5
#define CURRENT_BINARY_MODEL_VERSION	19 // normal source is version 18, but we have added stuff
#define CURRENT_BINARY_PARTICLE_VERSION	2

//#define DMX_MAX_HEADER_LENGTH	64

static constexpr uint32_t dmMaxHeaderLength = 64u;
static constexpr uint32_t dmMaxStringLength = 256u; // not including null terminator

// -1 if no element
// -2 if element exists, but not present
constexpr int dmElementIndexInvalid = -1;
constexpr int dmElementIndexExternal = -2;

typedef uint32_t DmElementHashU32;
typedef uint64_t DmElementHashU64;
typedef GUID DmElementHashU128;
//typedef GUID DataModelID;

class CDmeBase;
class DmElement;
union DmElementHash;
class DmAttribute;
class DmAttributeList;


//=============================
// DataModel Version
//=============================

class DmVersion_t
{
public:
	enum Encoding_t : uint16_t
	{
		DMX_ENCODE_BINARY,
		DMX_ENCODE_KEYVALUES2,
		DMX_ENCODE_KEYVALUES2_FLAT,

		DMX_ENCODE_COUNT,
	};

	// do not use for actual version
	enum EncodeVersionBinary_t : int
	{
		DMX_BINARY_V1,
		DMX_BINARY_V2,
		DMX_BINARY,

		DMX_BINARY_COUNT,
	};

	enum EncodeVersionKV_t : int
	{
		DMX_KEYVALUES2_V1,
		DMX_KEYVALUES2,

		DMX_KEYVALUES_COUNT,
	};

	enum Format_t : uint16_t
	{
		DMX_FORMAT_MODEL,
		DMX_FORMAT_PCF,

		DMX_FORMAT_LEGACY,

		DMX_FORMAT_COUNT,
	};

	DmVersion_t(const DmVersion_t* const pVersion) : encoding(pVersion->encoding), format(pVersion->format), encodingVersion(pVersion->encodingVersion), formatVersion(pVersion->formatVersion) {};
	DmVersion_t(const Encoding_t inputEncoding, const Format_t inputFormat, const int inputEncodingVersion, const int inputFormatVersion) : encoding(inputEncoding), format(inputFormat), encodingVersion(inputEncodingVersion), formatVersion(inputFormatVersion) {}
	DmVersion_t(const char* const str)
	{
		FromString(str);
	}

	Encoding_t encoding;
	Format_t format;

	int encodingVersion;
	int formatVersion;

	const int ToString(char* const buf, const size_t bufSize) const;
	const bool FromString(const char* const str);

	inline const bool IsValid() const
	{
		return (encoding >= DMX_ENCODE_COUNT || format >= DMX_FORMAT_COUNT || encodingVersion == -1 || formatVersion == -1) ? false : true;
	}

private:
	const bool EncodingFromString(const char* const str);
	const bool EncodingFromString_LEGACY(const char* const str);
	const bool FormatFromString(const char* const str);
};

constexpr const char* const s_DMXBinaryEncodings[DmVersion_t::DMX_BINARY_COUNT] =
{
	"binary_v1",
	"binary_v2",
	"binary",
};

constexpr const char* const s_DMXKeyvalueEncodings[DmVersion_t::DMX_KEYVALUES_COUNT] =
{
	"keyvalues2_v1",
	"keyvalues2",
};

constexpr const char* const s_DMXKeyvalueFlatEncodings[DmVersion_t::DMX_KEYVALUES_COUNT] =
{
	"keyvalues2_flat_v1",
	"keyvalues2_flat",
};

constexpr const char* const s_DMXFormats[DmVersion_t::DMX_FORMAT_COUNT] =
{
	"model",
	"pcf",

	"legacy", // old style
};


//=============================
// DataModel String Dictionary
//=============================

struct CDataModelStringEntry
{
	CDataModelStringEntry() = default;
	CDataModelStringEntry(const char* const str, const int strIndex) : string(str), index(strIndex)
	{
		length = strnlen_s(string, dmMaxStringLength);
		hash = HASH(string);
	}

	const char* string;
	size_t length;
	int index;
	uint32_t hash;
};

constexpr int dmMaxNumString_Binary2 = INT16_MAX;
constexpr int dmMaxNumString_Binary5 = INT_MAX;

class CDataModelStringDict
{
public:
	CDataModelStringDict(const size_t numStrings = 256ull) : numStrings(0)
	{
		byHash.reserve(numStrings);
		byIndex.reserve(numStrings);
	}

	const int AddToStringDict(const char* string);
	const int16_t AddToStringDict_Small(const char* string); // limited to 'dmMaxNumString_Binary2' number of strings (int16)

	const int GetStringIndex(const char* string);
	inline const int GetStringCount() const { return numStrings; }

	const CDataModelStringEntry* const GetEntry(const int index) const { return &byHash.at(byIndex.at(index)); }
	const char* const GetStringFromIndex(int index) const { return GetEntry(index)->string; }
	const uint32_t GetHashFromIndex(int index) const { return byIndex.at(index); }

private:
	std::unordered_map<uint32_t, CDataModelStringEntry> byHash;
	std::vector<uint32_t> byIndex;
	int numStrings;

	// originally went with an array of CDataModelStringEntry but it was slow and bogging down hard on anims, so swapped to the current system
};


//===========
// DataModel
//===========

enum eDmElementClass : uint32_t
{
	DME_BASE,

	// general
	DME_TRANSFORM,
	DME_TRANSFORMLIST,
	DME_JOINT,

	// animation specific
	DME_ANIMATIONLIST,
	DME_CHANNELSCLIP,
	DME_CHANNEL,
	DME_TIMEFRAME,
	DME_VECTOR3LOG,
	DME_VECTOR3LOGLAYER,
	DME_QUATERNIONLOG,
	DME_QUATERNIONLOGLAYER,

	// mesh specific
	DME_MODEL,
	DME_DAG,
	DME_MESH,
	DME_FACESET,
	DME_MATERIAL,
	DME_VERTEXDATA,

	_DME_CLASS_COUNT,
};

static const char* s_DmElementClassName[eDmElementClass::_DME_CLASS_COUNT]
{
	"DmeBase",

	// general
	"DmeTransform",
	"DmeTransformList",
	"DmeJoint",

	// animation specific
	"DmeAnimationList",
	"DmeChannelsClip",
	"DmeChannel",
	"DmeTimeFrame",
	"DmeVector3Log",
	"DmeVector3LogLayer",
	"DmeQuaternionLog",
	"DmeQuaternionLogLayer",

	// mesh specific
	"DmeModel",
	"DmeDag",
	"DmeMesh",
	"DmeFaceSet",
	"DmeMaterial",
	"DmeVertexData",
};

class CDataModel
{
public:
	CDataModel(const std::filesystem::path& path, const char* const name, const DmVersion_t* const targetVersion, char* const internalBuffer, const size_t internalBufferSize);
	~CDataModel();

	inline DmElement* const GetElement(const int index) const { return index >= 0 ? elements.at(index) : nullptr; }
	DmElement* const GetElement(const DmElementHash hash) const;
	const int GetElementIndex(const DmElementHash hash) const;
	inline DmAttributeList* const GetAttributes(const int index) const { return attributes.at(index); }
	inline DmElement* const GetRootElement() const { return rootElement; }
	inline DmAttributeList* const GetRootAttributes() const { return rootAttributes; }

	inline const int GetElementCount() const { return static_cast<int>(elements.size()); }
	inline const int GetAttributeCount() const { return static_cast<int>(attributes.size()); }

	DmElement* const AddElement(const char* const type, const char* const name, const int set, const size_t numAttributes = 0ull);
	DmElement* const AddElement(eDmElementClass type, const char* const name, const int set, const size_t numAttributes = 0ull);

	FORCEINLINE void AddDependent(CDataModel* const dependent) { dependencies.push_back(dependent); };

	void InitForModel();

	const bool WriteDataModel(const bool writeDeps = false);

	// storage buffer access
	FORCEINLINE const void* const WriteData(const void* data, const size_t dataSize) { return buffer.WriteBufferData(data, dataSize); };
	FORCEINLINE const char* const WriteString(const char* const str) { return buffer.WriteBufferString(str, dmMaxStringLength); }
	FORCEINLINE void* const ReserveData(const size_t dataSize) { return buffer.ReserveBufferData(dataSize); }

	FORCEINLINE const DmVersion_t::Format_t GetFormat() const { return version.format; }

private:
	CTextBuffer buffer;
	DmVersion_t version;

	const char* fileName;
	std::filesystem::path filePath;

	std::vector<DmElement*> elements;
	std::vector<DmAttributeList*> attributes;
	std::vector<CDataModel*> dependencies;

	// root element
	DmElement* rootElement;
	DmAttributeList* rootAttributes;

	// for writing
	std::unordered_map<DmElementHashU64, int> elementIndiceMap;
	CDataModelStringDict* stringDict;

	const size_t WriteBinaryModel(char* const buf, const size_t size, const bool writeDeps);
	const size_t WriteBinaryAttribute(char* const buf, const size_t size, const DmAttribute* const attribute) const;
	const size_t WriteBinaryAttribute_Value(char* const buf, const size_t size, const DmAttribute* const attribute) const;

	const size_t WriteKeyValuesModel(char* const buf, const size_t size);
};


//===================
// DataModel Element
//===================

struct DmxElement_v1_t
{
	char* type;	// string dict index of type string
	char* name;	// string dict index of name string

	GUID id;	// unique id for this element
};

struct DmxElement_v2_t
{
	int16_t type;	// string dict index of type string
	char* name;		// string dict index of name string

	GUID id;		// unique id for this element
};

struct DmxElement_v4_t
{
	DmxElement_v4_t(const int16_t typeIn, const int16_t nameIn, const GUID& idIn) : type(typeIn), name(nameIn), id(idIn) {};

	int16_t type;	// string dict index of type string
	int16_t name;	// string dict index of name string

	GUID id;		// unique id for this element
};

// current version binary 5
struct DmxElement_t
{
	DmxElement_t(const int typeIn, const int nameIn, const GUID& idIn) : type(typeIn), name(nameIn), id(idIn) {};

	int type;	// string dict index of type string
	int name;	// string dict index of name string

	GUID id;	// unique id for this element
};

// should take rig name into account, that way ids for similarly named elements are different accross models
union DmElementHash
{
	DmElementHash() = default;
	DmElementHash(const char* const type, const char* const name, const int set)
	{
		*this = HashElement(type, name, set);
	}
	DmElementHash(const eDmElementClass type, const char* const name, const int set)
	{
		*this = HashElement(s_DmElementClassName[type], name, set);
	}
	DmElementHash(const DmElementHash& hash)
	{
		hash_u64 = hash.hash_u64;
	}
	constexpr DmElementHash(const DmElementHashU64 hash) : hash_u64(hash)
	{

	}

	DmElementHashU32 hash_u32[2];
	DmElementHashU64 hash_u64;

	// todo: check for collision
	/*static const DmElementHash HashElement(const char* const type, const char* const name, const int set)
	{
		DmElementHash hash;

		hash.hash_u32[0] = HASH(type);
		hash.hash_u32[1] = HASH(name);

		hash.hash_u64 = RotateBits(hash.AsU64(), static_cast<uint32_t>(set));

		return hash;
	}*/

	// on paper slower but doesn't noticeably impact performance, and provides a better, more unique 'hash'
	static const DmElementHash HashElement(const char* const type, const char* const name, const int set)
	{
		DmElementHash hash;

		char buffer[dmMaxStringLength];

		const size_t typeLength = strnlen_s(type, dmMaxStringLength);
		const size_t nameLength = strnlen_s(name, dmMaxStringLength);

		size_t offset = 0ull;
		memcpy_s(buffer + offset, dmMaxStringLength - offset, type, typeLength);
		offset += typeLength;

		memcpy_s(buffer + offset, dmMaxStringLength - offset, name, nameLength);
		offset += nameLength;

		_itoa_s(set, buffer + offset, dmMaxStringLength - offset, 10); // it's faster than sprintf!
		itoa64(set, buffer + offset);

		hash.hash_u32[0] = HASH_INSENSITIVE(buffer);
		hash.hash_u32[1] = HASH(buffer);

		return hash;
	}

	inline const DmElementHashU64 AsU64() const { return hash_u64; }
	inline void ToU128(DmElementHashU128* const guid) const
	{
		const DmElementHashU64 bottomHash = AsU64();
		const uint32_t topShift = (bottomHash & 0x3F);
		const DmElementHash topHash(RotateBits(bottomHash, topShift));

		guid->Data1 = hash_u32[0];
		guid->Data2 = (hash_u32[1] & 0xFFFF);
		guid->Data3 = ((hash_u32[1] >> 16) & 0xFFFF);
		memcpy_s(guid->Data4, sizeof(guid->Data4), &topHash, sizeof(DmElementHashU64));
	}

	const bool operator==(const DmElementHash hash) const { return hash_u64 == hash.hash_u64; }
	//DmElementHash operator=(const DmElementHashU64 hash) { hash_u64 = hash; }
};

constexpr DmElementHash dmElementNull(0ull);

class DmElement
{
public:
	DmElement(const char* const typeIn, const char* const nameIn, const int setIn);
	DmElement(const eDmElementClass classIn, const char* const nameIn, const int setIn);
	DmElement(const DmElement* elementIn);
	~DmElement();

	// get values
	inline const char* GetName() const { return name; }
	inline const char* GetType() const { return type; }
	inline const DmElementHashU128* const GetId() const { return &id; }

	inline const int GetSet() const { return set; }
	inline const DmElementHash GetHash() const { return hash; }
	inline CDmeBase* const GetElement() const { return element; }

	inline DmAttributeList* const GetAttributes() const { return attributes; }

	// set values after construction
	void SetElement(CDmeBase* elementIn, const eDmElementClass classIn);
	void SetAttributes(DmAttributeList* const attributeList) { attributes = attributeList; }

	DmElement& operator=(const DmElement&& other) noexcept
	{
		if (this != &other)
		{
			/*type = other.type;
			name = other.name;

			id = other.id;

			hash = other.hash;
			set = other.set;

			elementClass = other.elementClass;
			element = other.element;*/

			memcpy_s(this, sizeof(DmElement), &other, sizeof(DmElement));
		}

		return *this;
	}

private:
	const char* type;
	const char* name;

	DmElementHashU128 id; // unique id for this element

	DmElementHash hash;
	int set; // if there are duplicate sets of elements, which set is this one in?

	eDmElementClass elementClass;
	CDmeBase* element; // pointer to specific element class using this

	DmAttributeList* attributes;
};


//==========================
// DataModel Attribute List
//==========================

enum class DmAttributeType_t : char
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
	AT_TYPE_INVALID = AT_TYPE_COUNT,

	// legacy attribute types
	AT_OBJECTID = AT_TIME,
	AT_OBJECTID_ARRAY = AT_TIME_ARRAY,
};

// for writing to disk
#pragma pack(push, 1)
struct DmxAttribute_t
{
	DmxAttribute_t(const int nameIn, const DmAttributeType_t typeIn) : name(nameIn), type(typeIn) {};

	int name; // string dictionary index
	DmAttributeType_t type; // DmAttributeType_t

	// these two entries are directly followed by the attribute data
};
#pragma pack(pop)

class DmAttribute
{
public:
	DmAttribute(const char* const nameIn, const DmAttributeType_t typeIn, const uint64_t value, const int valueCount = 1) : name(nameIn), hash(HASH(name)), type(typeIn), raw(value), storage(ATT_VAL_RAW), count(valueCount) {}
	DmAttribute(const char* const nameIn, const DmAttributeType_t typeIn, const void* const value, const int valueCount = 1) : name(nameIn), hash(HASH(name)), type(typeIn), ptr(value), storage(ATT_VAL_PTR), count(valueCount) {}
	DmAttribute(const char* const nameIn, const DmAttributeType_t typeIn, const std::vector<const void*>* value) : name(nameIn), hash(HASH(name)), type(typeIn), ptr(value), storage(ATT_VAL_VEC), count(-1) {}
	~DmAttribute() {};

	enum Store_t : uint8_t
	{
		ATT_VAL_PTR,
		ATT_VAL_RAW,
		ATT_VAL_VEC, // container (std::vector)

		// todo more containers/custom container
	};

	// access data
	inline const char* const GetName() const { return name; }
	inline const uint32_t GetHash() const { return hash; }
	inline const DmAttributeType_t GetType() const { return type; }
	inline const void* const GetDataPtr() const { return ptr; }
	inline const uint64_t GetDataRaw() const { return raw; }
	inline const int GetDataCount() const { return count; }
	inline const Store_t GetDataStorage() const { return storage; }

	// parse type for writing to disk
	template<typename T> FORCEINLINE const size_t WriteBinarySingle(char* const buf) const;
	template<typename T> FORCEINLINE const size_t WriteBinaryArray(char* const buf) const;
	FORCEINLINE const size_t WriteBinaryString(char* const buf, const size_t size) const;
	FORCEINLINE const size_t WriteBinaryStringArray(char* const buf, const size_t size) const;
	FORCEINLINE const size_t WriteBinaryElement(char* const buf, const CDataModel* const datamodel) const;
	FORCEINLINE const size_t WriteBinaryElementArray(char* const buf, const CDataModel* const datamodel) const;

	inline void InitCount(const int countIn) { count = countIn; }
	void InitData(const DmAttributeType_t typeIn, const uint64_t rawIn, const int countIn = 1);
	void InitData(const DmAttributeType_t typeIn, const void* const ptrIn, const int countIn = 1);

private:

	const char* name;
	uint32_t hash;
	DmAttributeType_t type;

	union {
		const void* ptr; // pointer to raw data
		uint64_t raw; // raw data stored here, type varies; maybe just expand this with all types, cuts out typing the casts?
	};
	int count;
	Store_t storage;
};

class DmAttributeList
{
public:
	DmAttributeList(DmElement* const element, const size_t numAttributes = 0ull) : parent(element)
	{
		attributes.reserve(numAttributes);
	}
	~DmAttributeList()
	{
		
	};

	// accessor funcs
	DmAttribute* const GetAttribute(const char* const name) const;
	DmAttribute* const GetAttribute(const int index) const { return attributes.at(index); };
	DmAttribute* const GetAttribute(const char* const name, int& index) const;
	inline const int GetAttributeCount() const { return static_cast<int>(attributes.size()); };
	inline const DmElement* const GetParent() const { return parent; }

	// add new attributes
	const int AddAttribute(CDataModel* const dataModel, const DmAttribute& attribute);
	template<typename T> const int AddAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const T& value) // raw value, pointer to local data if too large
	{
		static_assert(std::is_pointer<T>() == false, "can't use pointers here");

		constexpr size_t size = sizeof(T);

		if constexpr (size > sizeof(uint64_t))
		{
			const void* const data = dataModel->WriteData(&value, size);
			const DmAttribute attribute(name, type, data);

			return AddAttribute(dataModel, attribute);
		}
		else
		{
			uint64_t raw = 0ull;
			memcpy_s(&raw, sizeof(uint64_t), &value, size);
			const DmAttribute attribute(name, type, raw);

			return AddAttribute(dataModel, attribute);
		}
	}
	template<typename T> const int AddAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const T* const value, const int count, const bool store) // pointer to value(s)
	{
		if (store == false)
		{
			const DmAttribute attribute(name, type, value, count);

			return AddAttribute(dataModel, attribute);
		}

		const size_t size = sizeof(T) * count;
		if (size > sizeof(uint64_t))
		{
			const void* const data = dataModel->WriteData(value, size);
			const DmAttribute attribute(name, type, data, count);

			return AddAttribute(dataModel, attribute);
		}
		else
		{
			uint64_t raw = 0ull;
			memcpy_s(&raw, sizeof(uint64_t), value, size);
			const DmAttribute attribute(name, type, raw, count);

			return AddAttribute(dataModel, attribute);
		}
	}
	template<typename T> const int AddAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const T* const value, const bool store) // pointer to value
	{
		return AddAttribute(dataModel, name, type, value, 1, store);
	}
	template<typename T> const int AddAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const std::vector<T>* const value, const DmAttribute::Store_t container) // pointer to std::vector
	{
		switch (container)
		{
		case DmAttribute::ATT_VAL_VEC:
		{
			const DmAttribute attribute(name, type, reinterpret_cast<const std::vector<const void*>*const>(value));

			return AddAttribute(dataModel, attribute);
		}
		default:
		{
			assertm(false, "invalid storage type");
			return -1;
		}
		}		
	}

	// edit existing attributes
	template<typename T> void EditAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const T& value) // raw value, pointer to local data if too large
	{
		static_assert(std::is_pointer<T>() == false, "can't use pointers here");

		DmAttribute* const pAttribute = GetAttribute(name);

		if (pAttribute == nullptr)
		{
			assertm(false, "attribute did not exist");
			return;
		}

		constexpr size_t size = sizeof(T);

		if constexpr (size > sizeof(uint64_t))
		{
			const void* const data = dataModel->WriteData(&value, size);
			pAttribute->InitData(type, data);
		}
		else
		{
			uint64_t raw = 0ull;
			memcpy_s(&raw, sizeof(uint64_t), &value, size);
			pAttribute->InitData(type, raw);
		}
	}
	template<typename T> void EditAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const T* const value, const int count, const bool store) // pointer to value(s)
	{
		DmAttribute* const pAttribute = GetAttribute(name);

		if (pAttribute == nullptr)
		{
			assertm(false, "attribute did not exist");
			return;
		}

		if (EditAttribute(pAttribute, type, reinterpret_cast<const void* const>(value), count))
		{
			return;
		}

		if (store == false)
		{
			pAttribute->InitData(type, value, count);

			return;
		}

		const size_t size = sizeof(T) * count;
		if (size > sizeof(uint64_t))
		{
			const void* const data = dataModel->WriteData(value, size);
			pAttribute->InitData(type, data, count);
		}
		else
		{
			uint64_t raw = 0ull;
			memcpy_s(&raw, sizeof(uint64_t), value, size);
			pAttribute->InitData(type, raw, count);
		}
	}
	template<typename T> void EditAttribute(CDataModel* const dataModel, const char* const name, const DmAttributeType_t type, const T* const value, const bool store) // pointer to value
	{
		EditAttribute(dataModel, name, type, value, 1, store);
	}
	template<typename T> const int EditAttribute(CDataModel* const dataModel, const char* const name, int index, const DmAttributeType_t type, const T& value) // raw value, pointer to local data if too large
	{
		static_assert(std::is_pointer<T>() == false, "can't use pointers here");

		DmAttribute* const pAttribute = GetAttribute(name, index);

		if (pAttribute == nullptr)
		{
			assertm(false, "attribute did not exist");
			return -1;
		}

		constexpr size_t size = sizeof(T);

		if constexpr (size > sizeof(uint64_t))
		{
			const void* const data = dataModel->WriteData(&value, size);
			pAttribute->InitData(type, data);
		}
		else
		{
			uint64_t raw = 0ull;
			memcpy_s(&raw, sizeof(uint64_t), &value, size);
			pAttribute->InitData(type, raw);
		}

		return index;
	}
	template<typename T> const int EditAttribute(CDataModel* const dataModel, const char* const name, int index, const DmAttributeType_t type, const T* const value, const int count, const bool store) // pointer to value(s)
	{
		DmAttribute* const pAttribute = GetAttribute(name, index);

		if (pAttribute == nullptr)
		{
			assertm(false, "attribute did not exist");
			return -1;
		}

		if (EditAttribute(pAttribute, type, reinterpret_cast<const void* const>(value), count))
		{
			return index;
		}

		if (store == false)
		{
			pAttribute->InitData(type, value, count);

			return index;
		}

		const size_t size = sizeof(T) * count;
		if (size > sizeof(uint64_t))
		{
			const void* const data = dataModel->WriteData(value, size);
			pAttribute->InitData(type, data, count);
		}
		else
		{
			uint64_t raw = 0ull;
			memcpy_s(&raw, sizeof(uint64_t), value, size);
			pAttribute->InitData(type, raw, count);
		}

		return index;
	}
	template<typename T> const int EditAttribute(CDataModel* const dataModel, const char* const name, int index, const DmAttributeType_t type, const T* const value, const bool store) // pointer to value
	{
		return EditAttribute(dataModel, name, index, type, value, 1, store);
	}
	const bool EditAttribute(DmAttribute* const attribute, const DmAttributeType_t type, const void* const value, const int count = 1);

	DmAttributeList& operator=(const DmAttributeList&& list) = delete;
	DmAttributeList& operator=(DmAttributeList&& list) noexcept
	{
		if (this != &list)
		{
			attributes.swap(list.attributes);
			parent = list.parent;

			list.parent = nullptr;
		}

		return *this;
	}

private:
	std::vector<DmAttribute*> attributes;
	DmElement* parent; // this should match its element
};