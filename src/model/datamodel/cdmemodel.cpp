#include <pch.h>

#include <model/datamodel/cdmemodel.h>

CDmeModel::CDmeModel(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numJoints, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_MODEL, nameIn, setIn, 7ull, truncateName), transform(nullptr), shape(nullptr), visible(true),
	childJoint(nullptr), childDag(nullptr), upAxis("")
{
	jointList.reserve(numJoints);
	jointListElements.reserve(numJoints);

	attributes->AddAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "visible", DmAttributeType_t::AT_BOOL, visible);
	attributes->AddAttribute(dataModel, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "jointList", DmAttributeType_t::AT_ELEMENT_ARRAY, &jointList, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "baseStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &baseStates, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "upAxis", DmAttributeType_t::AT_STRING, upAxis, false);
}

CDmeModel::CDmeModel(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const CDmeModel* const source, const bool truncateName) : CDmeBase(dataModelIn, eDmElementClass::DME_MODEL, nameIn, setIn, 7ull, truncateName), transform(nullptr), shape(nullptr), visible(source->visible),
	childJoint(nullptr), childDag(nullptr), upAxis(source->upAxis)
{
	attributes->AddAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, dmElementNull);
	attributes->AddAttribute(dataModel, "visible", DmAttributeType_t::AT_BOOL, visible);
	attributes->AddAttribute(dataModel, "children", DmAttributeType_t::AT_ELEMENT_ARRAY, &children, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "jointList", DmAttributeType_t::AT_ELEMENT_ARRAY, &jointList, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "baseStates", DmAttributeType_t::AT_ELEMENT_ARRAY, &baseStates, DmAttribute::ATT_VAL_VEC);
	attributes->AddAttribute(dataModel, "upAxis", DmAttributeType_t::AT_STRING, upAxis, false);

	// base states (transformlist)
	const size_t baseStateCount = source->baseStates.size();
	assertm(source->baseStates.size() == source->baseStateElements.size(), "this should not ever happen, wicked bad!");

	baseStateElements.resize(baseStateCount);
	baseStates.resize(baseStateCount);

	static_assert(sizeof(DmElementHash) == sizeof(CDmeBase*), "cannot cheat if sizes mismatch!");
	const size_t baseStateSize = sizeof(DmElementHash) * baseStateCount;
	memcpy_s(baseStateElements.data(), baseStateSize, source->baseStateElements.data(), baseStateSize);
	memcpy_s(baseStates.data(), baseStateSize, source->baseStates.data(), baseStateSize);

	// joints
	const size_t jointCount = source->jointList.size();
	assertm(source->jointList.size() == source->jointListElements.size(), "this should not ever happen, wicked bad!");

	jointListElements.resize(jointCount);
	jointList.resize(jointCount);

	static_assert(sizeof(DmElementHash) == sizeof(CDmeBase*), "cannot cheat if sizes mismatch!");
	const size_t jointSize = sizeof(DmElementHash) * jointCount;
	memcpy_s(jointListElements.data(), jointSize, source->jointListElements.data(), jointSize);
	memcpy_s(jointList.data(), jointSize, source->jointList.data(), jointSize);
}

void CDmeModel::AddTransform(const char* const nameIn, const int setIn, const bool truncateName)
{
	transform = new CDmeTransform(dataModel, nameIn, setIn, truncateName);

	attributes->EditAttribute(dataModel, "transform", DmAttributeType_t::AT_ELEMENT, transform->GetHash());
}

void CDmeModel::AddShape(const char* const nameIn, const int setIn, const bool truncateName)
{
	const char* const shapeName = CDmeMesh::GetNameForShape(dataModel, nameIn);

	shape = new CDmeMesh(dataModel, shapeName, setIn, truncateName);

	attributes->EditAttribute(dataModel, "shape", DmAttributeType_t::AT_ELEMENT, shape->GetHash());
}

CDmeJoint* const CDmeModel::AddJoint(const char* const nameIn, const int setIn, const bool truncateName)
{
	CDmeJoint* const joint = new CDmeJoint(dataModel, nameIn, setIn, truncateName);

	jointListElements.push_back(joint);
	jointList.push_back(joint->GetHash());

	return joint;
}

CDmeTransformList* const CDmeModel::AddBaseState(const char* const nameIn, const int setIn, const size_t numJoints, const bool truncateName)
{
	CDmeTransformList* const baseState = new CDmeTransformList(dataModel, nameIn, setIn, numJoints, truncateName); // set will be 1 here

	baseStateElements.push_back(baseState);
	baseStates.push_back(baseState->GetHash());

	return baseState;
}

void CDmeModel::SetUpAxis(ModelAxis_t axis)
{
	upAxis = modelAxis[axis];

	attributes->EditAttribute(dataModel, "upAxis", DmAttributeType_t::AT_STRING, upAxis, false);
}

// adds our first joint as the root of the skeleton
// todo: this probably has issues on bones with multiple root bones!
CDmeJoint* const CDmeModel::AddChildJoint()
{
	if (!jointList.size())
	{
		Error("DmeModel %s does not contain any joints!!!\n", name);
	}

	children.push_back(jointList.at(0));
	childJoint = jointListElements.at(0);

	AddAsSkeleton();

	return childJoint;
}

// calls AddAsModel
CDmeDag* const CDmeModel::AddChildDag(const char* const nameIn, const int setIn, const bool truncateName)
{
	childDag = new CDmeDag(dataModel, nameIn, setIn, truncateName);

	children.push_back(childDag->GetHash());

	AddAsModel();

	return childDag;
}