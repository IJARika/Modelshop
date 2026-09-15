#include <pch.h>

#include <model/datamodel/cdmechannel.h>

CDmeChannel::CDmeChannel(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const eDmeChannelType type, const char* const joint, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_CHANNEL, nameIn, setIn, 8ull, truncateName),
	fromElement(dmElementNull), toElement(dmElementNull), log(dmElementNull), fromElementElement(nullptr), toElementElement(nullptr), logElement(nullptr), fromAttribute(""), toAttribute(s_DmeChannelTypes[type]), toIndex(0), fromIndex(0),
	mode(ChannelMode_t::CM_PLAY), channelType(type)
{
	// our skeleton should already be set up
#if 1
	const DmElementHash toElementHash(eDmElementClass::DME_TRANSFORM, joint, 0);
	DmElement* const toElementBase = dataModel->GetElement(toElementHash);
	assertm(toElementBase, "skeleton has not been created");

	toElementElement = static_cast<CDmeTransform* const>(toElementBase->GetElement());
	assertm(toElementElement, "skeleton is invalid");

	toElement = toElementElement->GetHash();
#else
	const DmElementHash toElementHash(eDmElementClass::DME_TRANSFORM, joint, 0);
	DmElement* const toElementBase = dataModel->GetElement(toElementHash);

	toElementElement = toElementBase ? static_cast<CDmeTransform* const>(toElementBase->GetElement()) : nullptr;

	toElement = toElementHash;
#endif // 0

	attributes->AddAttribute(dataModel, "fromElement", DmAttributeType_t::AT_ELEMENT, fromElement);
	attributes->AddAttribute(dataModel, "fromAttribute", DmAttributeType_t::AT_STRING, fromAttribute, false);
	attributes->AddAttribute(dataModel, "fromIndex", DmAttributeType_t::AT_INT, fromIndex);
	attributes->AddAttribute(dataModel, "toElement", DmAttributeType_t::AT_ELEMENT, toElement);
	attributes->AddAttribute(dataModel, "toAttribute", DmAttributeType_t::AT_STRING, toAttribute, false);
	attributes->AddAttribute(dataModel, "toIndex", DmAttributeType_t::AT_INT, toIndex);
	attributes->AddAttribute(dataModel, "mode", DmAttributeType_t::AT_INT, mode);
	attributes->AddAttribute(dataModel, "log", DmAttributeType_t::AT_ELEMENT, log);
}

CDmeVector3Log* const CDmeChannel::AddLogVec(const char* const nameIn, const int setIn, const bool truncateName)
{
	logElementVector = new CDmeVector3Log(dataModel, nameIn, setIn, truncateName);
	log = logElementVector->GetHash();

	attributes->EditAttribute(dataModel, "log", DmAttributeType_t::AT_ELEMENT, log);

	return logElementVector;
}

CDmeQuaternionLog* const CDmeChannel::AddLogQuat(const char* const nameIn, const int setIn, const bool truncateName)
{
	logElementQuat = new CDmeQuaternionLog(dataModel, nameIn, setIn, truncateName);
	log = logElementQuat->GetHash();

	attributes->EditAttribute(dataModel, "log", DmAttributeType_t::AT_ELEMENT, log);

	return logElementQuat;
}

CDmeBase* const CDmeChannel::AddLog(const char* nameIn, const int setIn, const bool truncateName)
{
	switch (channelType)
	{
	case DME_CH_TYPE_POS:
	case DME_CH_TYPE_SCL:
	{
		return AddLogVec(nameIn, setIn, truncateName);
	}
	case DME_CH_TYPE_ROT:
	{
		return AddLogQuat(nameIn, setIn, truncateName);
	}
	default:
	{
		assertm(false, "invalid channel");
		return nullptr;
	}
	}
}

const char* const CDmeChannel::GetNameForChannel(CDataModel* const dataModelIn, const char* const nameIn, const eDmeChannelType channelIn)
{
	char tmp[dmMaxStringLength]{};
	const size_t length = strncpy_mem(tmp, dmMaxStringLength, nameIn, dmMaxStringLength);

	strncpy_mem(tmp + length, dmMaxStringLength - length, s_DmeChannelSuffix[channelIn], 3ull);

	const char* const str = dataModelIn->WriteString(tmp);

	return str;
}