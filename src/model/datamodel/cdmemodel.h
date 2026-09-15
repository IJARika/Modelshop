#pragma once

#include <model/datamodel/cdmemesh.h>
#include <model/datamodel/cdmedag.h>
#include <model/datamodel/cdmejoint.h>
#include <model/datamodel/cdmetransform.h>
#include <model/datamodel/cdmetransformlist.h>

// todo: figure out how to make this enum class
enum ModelAxis_t : char
{
	MODEL_AXIS_X = 0,
	MODEL_AXIS_X_NEG,
	MODEL_AXIS_Y,
	MODEL_AXIS_Y_NEG,
	MODEL_AXIS_Z,
	MODEL_AXIS_Z_NEG,
	MODEL_AXIS_COUNT
};

constexpr const char* const modelAxis[MODEL_AXIS_COUNT]
{
	"X",
	"-X",
	"Y",
	"-Y",
	"Z",
	"-Z"
};

class CDmeModel : public CDmeBase
{
public:
	CDmeModel(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const size_t numJoints = 0ull, const bool truncateName = false);
	CDmeModel(CDataModel* const dataModelIn, const char* const nameIn, const int setIn, const CDmeModel* const source, const bool truncateName = false);
	virtual ~CDmeModel()
	{

	}

	// add to parent datamodel's root element as a 'model'
	inline void AddAsModel()
	{
		assertm(dataModel->GetFormat() == DmVersion_t::DMX_FORMAT_MODEL, "dmx was not model format");
		DmAttributeList* const pRootAttributes = dataModel->GetRootAttributes();
		pRootAttributes->AddAttribute(dataModel, "model", DmAttributeType_t::AT_ELEMENT, element->GetHash());
	}

	// add to parent datamodel's root element as a 'skeleton'
	inline void AddAsSkeleton()
	{
		assertm(dataModel->GetFormat() == DmVersion_t::DMX_FORMAT_MODEL, "dmx was not model format");
		DmAttributeList* const pRootAttributes = dataModel->GetRootAttributes();
		pRootAttributes->AddAttribute(dataModel, "skeleton", DmAttributeType_t::AT_ELEMENT, element->GetHash());
	}

	// add to this element
	void AddTransform(const char* const nameIn, const int setIn, const bool truncateName = false);
	void AddShape(const char* const nameIn, const int setIn, const bool truncateName = false); // nameIn will have "Shape" appended to the end of it
	CDmeJoint* const AddJoint(const char* const nameIn, const int setIn, const bool truncateName = false);
	CDmeTransformList* const AddBaseState(const char* const nameIn, const int setIn, const size_t numJoints = 0ull , const bool truncateName = false);
	void SetUpAxis(ModelAxis_t axis = ModelAxis_t::MODEL_AXIS_Z);

	CDmeJoint* const AddChildJoint(); // this grabs first joint in the list
	CDmeDag* const AddChildDag(const char* const nameIn, const int setIn, const bool truncateName = false); // calls AddAsModel

	FORCEINLINE CDmeJoint* const GetJoint(const int idx) const { return jointListElements.at(idx); }
	FORCEINLINE CDmeDag* const GetDag() const { return childDag; }
	FORCEINLINE CDmeTransformList* const GetBaseStates(const int idx) const { return baseStateElements.at(idx); }
	FORCEINLINE const size_t GetJointCount() const { return jointList.size(); }

private:
	std::vector<CDmeJoint*> jointListElements;
	std::vector<CDmeTransformList*> baseStateElements;

	std::vector<DmElementHash> children;
	std::vector<DmElementHash> jointList;
	std::vector<DmElementHash> baseStates;

	CDmeJoint* childJoint;
	CDmeDag* childDag;
	const char* upAxis;

	CDmeTransform* transform;
	CDmeMesh* shape;

	bool visible;
};