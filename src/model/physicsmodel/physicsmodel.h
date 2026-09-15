#pragma once

#include <model/studio/phyfile.h>

#define HAS_PHYSICSMODEL_PARSER

namespace PhysicsModel
{
	class RagdollConstraint
	{
	public:
		RagdollConstraint() : parent(-1), child(0), constraints() {};
		RagdollConstraint(const kv_parser::Token_t* const ragdollParams);

		enum ConstraintAxis_t
		{
			CONAXIS_X,
			CONAXIS_Y,
			CONAXIS_Z,

			CONAXIS_COUNT
		};

		struct Constraint_t
		{
			float min;
			float max;
			float friction;
		};

		inline const int GetParent() const { return parent; }
		inline const int GetChild() const { return child; }
		inline const Constraint_t* const GetConstraint(const int axis) const
		{
			if (axis >= CONAXIS_COUNT || axis < CONAXIS_X)
			{
				return nullptr;
			}

			return &constraints[axis];
		}

	private:
		int parent;
		int child;

		Constraint_t constraints[ConstraintAxis_t::CONAXIS_COUNT];
	};

	// binary format solid properties section
	typedef uint32_t ParamFlags_t;
	enum eParamFlags : ParamFlags_t
	{
		PHYSPARAM_FLAG_HAS_NONE			= 0x0,

		PHYSPARAM_FLAG_HAS_NAME			= 0x1,
		PHYSPARAM_FLAG_HAS_PARENT		= 0x2,
		PHYSPARAM_FLAG_HAS_SURFACEPROP	= 0x4,
		PHYSPARAM_FLAG_HAS_INDEX		= 0x8,
		PHYSPARAM_FLAG_HAS_MASS			= 0x10,
		PHYSPARAM_FLAG_HAS_MASSBIAS		= 0x20,
		PHYSPARAM_FLAG_HAS_DAMPING		= 0x40,
		PHYSPARAM_FLAG_HAS_ROTDAMPING	= 0x80,
		PHYSPARAM_FLAG_HAS_INERTIA		= 0x100,
		PHYSPARAM_FLAG_HAS_VOLUME		= 0x200,
		PHYSPARAM_FLAG_HAS_DRAG			= 0x400,
		PHYSPARAM_FLAG_HAS_ROLLINGDRAG	= 0x800,

		PHYSOLID_FLAG_HAS_RAGDOLLCONSTRAINT = 0x1000,
	};

	constexpr float s_defaultMassBias = 1.0f;
	constexpr float s_defaultDamping = 0.0f;
	constexpr float s_defaultRotDamping = 0.0f;
	constexpr float s_defaultInertia = 1.0f;
	constexpr float s_defaultDrag = -1.0f;
	constexpr float s_defaultRollingDrag = -1.0f; // probably from HL2 beta, cannot find a source

	class Solid
	{
	public:
		Solid() = default;
		Solid(const kv_parser::Token_t* const solidParams);
		~Solid()
		{
			FreeAllocArray(name);
			FreeAllocArray(parent);
			FreeAllocArray(surfaceprop);
		}

		inline const char* const GetName() const { return name; }
		inline const char* const GetParent() const { return parent; }
		inline int GetIndex() const { return index; }

		const float GetMassBias() const { return massbias; }
		const float GetDamping() const { return damping; }
		const float GetRotDamping() const { return rotdamping; }
		const float GetInertia() const { return inertia; }
		const float GetDrag() const { return drag; }
		const float GetRollingDrag() const { return rollingDrag; }

		inline const bool HasRagdollConstraint() const { paramUsage& PHYSOLID_FLAG_HAS_RAGDOLLCONSTRAINT ? true : false; }
		inline const RagdollConstraint* const GetRagdollConstraint() const { return &ragdollConstraint; }
		inline void SetRagdollConstraint(const RagdollConstraint* const constraint)
		{
			paramUsage |= PHYSOLID_FLAG_HAS_RAGDOLLCONSTRAINT;
			memcpy_s(&ragdollConstraint, sizeof(RagdollConstraint), constraint, sizeof(RagdollConstraint));
		}

		const ParamFlags_t GetFlags() const { return paramUsage; }
		ParamFlags_t& Flags() { return paramUsage; }

		Solid& operator=(const Solid&) = delete;
		Solid& operator=(Solid& solid) noexcept
		{
			if (this != &solid)
			{
				memcpy_s(this, sizeof(Solid), &solid, sizeof(Solid));

				name = solid.name;
				parent = solid.parent;
				surfaceprop = solid.surfaceprop;

				solid.name = nullptr;
				solid.parent = nullptr;
				solid.surfaceprop = nullptr;
			}

			return *this;
		}

	private:
		char* name;
		char* parent;
		char* surfaceprop;

		int index;
		float mass;
		float massbias;
		float damping;
		float rotdamping;
		float inertia;
		float volume;
		float drag;
		float rollingDrag;

		RagdollConstraint ragdollConstraint;

		// if one of these is set, custom data is used
		ParamFlags_t paramUsage;
	};

	class CollisionRule
	{
	public:
		CollisionRule(const kv_parser::Token_t* const ruleParams);
		~CollisionRule()
		{
			FreeAllocArray(collisionPairs);
		}

		struct CollisionPair_t
		{
			int solid0;
			int solid1;
		};

		const CollisionPair_t* const GetCollisionPair(const int index) const { return collisionPairs + index; }
		const int GetCollisionPairCount() const { return collisionPairCount; }

		inline const bool NoSelfCollisions() const { return selfcollisions == false ? true : false; }

	private:
		CollisionPair_t* collisionPairs;
		int collisionPairCount;

		bool selfcollisions;

		inline void AddCollisionPair(const kv_parser::Token_t* const collisionPair)
		{
			assertm(collisionPair->GetChild(0)->GetTokenCount() == 2, "collisionpair should have two tokens");

			collisionPairs[collisionPairCount].solid0 = collisionPair->GetIntValue(0);
			collisionPairs[collisionPairCount].solid1 = collisionPair->GetIntValue(1);

			collisionPairCount++;
		}
	};

	class AnimatedFriction
	{
	public:
		AnimatedFriction(const kv_parser::Token_t* const animFricParams);

		inline const float GetFrictionMin() const { return animFrictionMin; }
		inline const float GetFrictionMax() const { return animFrictionMax; }
		inline const float GetFrictionTimeIn() const { return animFrictionTimeIn; }
		inline const float GetFrictionTimeOut() const { return animFrictionTimeOut; }
		inline const float GetFrictionTimeHold() const { return animFrictionTimeHold; }

	private:
		float animFrictionMin;
		float animFrictionMax;
		float animFrictionTimeIn;
		float animFrictionTimeOut;
		float animFrictionTimeHold;
	};

	class EditParam
	{
	public:
		EditParam(const kv_parser::Token_t* const editParams);
		~EditParam()
		{
			FreeAllocArray(rootname);
			FreeAllocArray(jointMerges);
		}

		struct JointMerge_t
		{
			~JointMerge_t()
			{
				FreeAllocArray(origin);
				FreeAllocArray(destination);
			}

			char* origin;
			char* destination;

			JointMerge_t& operator=(const JointMerge_t&) = delete;
			JointMerge_t& operator=(JointMerge_t& merge) noexcept
			{
				if (this != &merge)
				{
					origin = merge.origin;
					destination = merge.destination;

					merge.origin = nullptr;
					merge.destination = nullptr;
				}

				return *this;
			}
		};

		const char* const GetRootName() const { return rootname; }
		const float GetTotalMass() const { return totalmass; }
		const bool IsConcave() const { return concave; }

		const int GetJointMergeCount() const { return jointMergeCount; }
		const JointMerge_t* const GetJointMerge(const int index) const { return jointMerges + index; }

	private:
		char* rootname;
		float totalmass;
		bool concave;

		JointMerge_t* jointMerges;
		int jointMergeCount;

		inline void AddJointMerge(const kv_parser::Token_t* const jointMerge)
		{
			assertm(jointMerge->GetChild(0)->GetTokenCount() == 2, "jointmerge should have two tokens");

			jointMerges[jointMergeCount].origin = AllocString(jointMerge->GetStringValue(0));
			jointMerges[jointMergeCount].destination = AllocString(jointMerge->GetStringValue(1));

			jointMergeCount++;
		}
	};

	constexpr int16_t defaultAllowedConvexPieces = 40;
	class CParsedPhys
	{
	public:
		CParsedPhys(const ivps::phyheader_t* const pPHYS);
		~CParsedPhys()
		{
			FreeAllocArray(solids);

			FreeAllocVar(collisionRule);
			FreeAllocVar(animatedFricton);
			FreeAllocVar(editParams);

			FreeAllocArray(collisionText);
		}

		inline const Solid* const GetSolid(const int index) const { return solids + index; }
		inline const uint32_t GetSolidCount() const { return solidCount; }

		inline void SetMaxConvexPieces(const int16_t newCount) { maxConvexPieces = newCount > maxConvexPieces ? newCount : maxConvexPieces; }
		inline const int16_t MaxConvexPieces() const { return maxConvexPieces; }
		inline void SetConcavePerJoint() { concavePerJoint = (isJointedCollision && maxConvexPieces > 1) ? true : false; }
		inline const bool ConcavePerJoint() const { return concavePerJoint; }
		inline void SetJointed(const bool hasJoints) { isJointedCollision = hasJoints; }
		inline const bool IsJointed() const { return isJointedCollision; }

		const CollisionRule* const GetCollisionRule() const { return collisionRule; }
		const AnimatedFriction* const GetAnimatedFriction() const { return animatedFricton; }
		const EditParam* const GetEditParam() const { return editParams; }

		const char* const GetCollisionText() const { return collisionText; }

		const float GetDamping() const { return defaultDamping; }
		const float GetRotDamping() const { return defaultRotDamping; }
		const float GetInertia() const { return defaultInertia; }
		const ParamFlags_t GetFlags() const { return defaultParamUsage; }

	private:
		Solid* solids;
		int solidCount;

		int16_t maxConvexPieces;
		bool concavePerJoint;
		bool isJointedCollision; // was this model compiled using '$collisionjoints'?

		CollisionRule* collisionRule;
		AnimatedFriction* animatedFricton;
		EditParam* editParams;

		char* collisionText;

		float defaultInertia;
		float defaultDamping;
		float defaultRotDamping;
		ParamFlags_t defaultParamUsage; // if one of these is set, custom data is used
	};
}