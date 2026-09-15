#pragma once

#include <model/datamodel/cdmetransform.h>
#include <model/datamodel/cdmevector3log.h>
#include <model/datamodel/cdmequaternionlog.h>

enum ChannelMode_t : int
{
	CM_OFF,
	CM_PASS,
	CM_RECORD,
	CM_PLAY,
};

// unsure if we will need these
//enum PlayMode_t
//{
//	PM_HOLD,
//	PM_LOOP,
//};
//
//enum RecordOperationFlags_t
//{
//	RECORD_OPERATION_FLAG_PRESET = (1 << 0),
//	RECORD_OPERATION_FLAG_REVERSE = (1 << 1),
//	RECORD_OPERATION_FLAG_REVEAL = (1 << 2),
//	RECORD_OPERATION_FLAG_REBASE_TIME = (1 << 3)
//};

enum eDmeChannelType : uint32_t
{
	DME_CH_TYPE_POS,
	DME_CH_TYPE_ROT,
	DME_CH_TYPE_SCL,
	_DME_CH_COUNT,
};

static const char* s_DmeChannelTypes[eDmeChannelType::_DME_CH_COUNT]
{
	"position",
	"orientation",
	"scale",
};

static const char* s_DmeChannelSuffix[eDmeChannelType::_DME_CH_COUNT]
{
	"_p",
	"_o",
	"_s",
};

class CDmeChannel : public CDmeBase
{
public:
	CDmeChannel(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const eDmeChannelType type, const char* const joint, const bool truncateName = false);
	virtual ~CDmeChannel()
	{

	}

	CDmeBase* const AddLog(const char* const nameIn, const int setIn, const bool truncateName = false); // one of the major cases where set will be important. use the bone index, may cause issues with scale later as they would both be Vector logs.

	CDmeVector3Log* const GetLogVector() const { return logElementVector; }
	CDmeQuaternionLog* const GetLogQuat() const { return logElementQuat; }

	static const char* const GetNameForChannel(CDataModel* const dataModelIn, const char* const nameIn, const eDmeChannelType channelIn);

private:
	const char* fromAttribute; // string, simply "" in what I've seen (so far)
	const char* toAttribute; // string of type; "position", "orientation", "scale"

	CDmeTransform* fromElementElement;
	CDmeTransform* toElementElement;
	DmElementHash fromElement; // element
	DmElementHash toElement; // the bones base transformation, from the already constructed skeleton

	union {
		CDmeVector3Log* logElementVector;
		CDmeQuaternionLog* logElementQuat;
		CDmeBase* logElement;
	};
	DmElementHash log; // element, this is where our animation data goes

	eDmeChannelType channelType; // what type is this channel; pos, rot, scale

	// no clue what these are honestly
	int fromIndex;	
	int toIndex;

	ChannelMode_t mode; // enum value stored as an int

	CDmeVector3Log* const AddLogVec(const char* const nameIn, const int setIn, const bool truncateName = false);
	CDmeQuaternionLog* const AddLogQuat(const char* const nameIn, const int setIn, const bool truncateName = false);
};