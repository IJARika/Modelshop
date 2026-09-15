#include <pch.h>

#include <model/physicsmodel/physicsmodel.h>

namespace PhysicsModel
{
	RagdollConstraint::RagdollConstraint(const kv_parser::Token_t* const ragdollParams)
	{
		// this param should always have the same five params
		assertm(ragdollParams->GetNumChildren() == 11, "bad formatting");

		parent = ragdollParams->GetChild("parent")->GetIntValue();
		child = ragdollParams->GetChild("child")->GetIntValue();

		constraints[CONAXIS_X].min = ragdollParams->GetChild("xmin")->GetFloatValue();
		constraints[CONAXIS_X].max = ragdollParams->GetChild("xmax")->GetFloatValue();
		constraints[CONAXIS_X].friction = ragdollParams->GetChild("xfriction")->GetFloatValue();

		constraints[CONAXIS_Y].min = ragdollParams->GetChild("ymin")->GetFloatValue();
		constraints[CONAXIS_Y].max = ragdollParams->GetChild("ymax")->GetFloatValue();
		constraints[CONAXIS_Y].friction = ragdollParams->GetChild("yfriction")->GetFloatValue();

		constraints[CONAXIS_Z].min = ragdollParams->GetChild("zmin")->GetFloatValue();
		constraints[CONAXIS_Z].max = ragdollParams->GetChild("zmax")->GetFloatValue();
		constraints[CONAXIS_Z].friction = ragdollParams->GetChild("zfriction")->GetFloatValue();
	}

	Solid::Solid(const kv_parser::Token_t* const solidParams) : name(nullptr), parent(nullptr), surfaceprop(nullptr), index(-1), mass(0.0f), massbias(s_defaultMassBias),
		damping(s_defaultDamping), rotdamping(s_defaultRotDamping), inertia(s_defaultInertia), volume(0.0f), drag(s_defaultDrag), rollingDrag(s_defaultRollingDrag), ragdollConstraint(), paramUsage(PHYSPARAM_FLAG_HAS_NONE)
	{
		const kv_parser::Token_t* currentKey = nullptr;
		int remainingParams = solidParams->GetNumChildren();

		if (solidParams->HasChild("name", &currentKey))
		{
			name = AllocString(currentKey->GetStringValue());
			paramUsage |= PHYSPARAM_FLAG_HAS_NAME;

			remainingParams--;
		}

		if (solidParams->HasChild("parent", &currentKey))
		{
			parent = AllocString(currentKey->GetStringValue());
			paramUsage |= PHYSPARAM_FLAG_HAS_PARENT;

			remainingParams--;
		}

		if (solidParams->HasChild("surfaceprop", &currentKey))
		{
			surfaceprop = AllocString(currentKey->GetStringValue());
			paramUsage |= PHYSPARAM_FLAG_HAS_SURFACEPROP;

			remainingParams--;
		}

		if (solidParams->HasChild("index", &currentKey))
		{
			index = currentKey->GetIntValue();
			paramUsage |= PHYSPARAM_FLAG_HAS_INDEX;

			remainingParams--;
		}

		if (solidParams->HasChild("mass", &currentKey))
		{
			mass = currentKey->GetFloatValue();
			paramUsage |= PHYSPARAM_FLAG_HAS_MASS;

			remainingParams--;
		}

		if (solidParams->HasChild("massbias", &currentKey))
		{
			massbias = currentKey->GetFloatValue();
			paramUsage |= PHYSPARAM_FLAG_HAS_MASSBIAS;

			remainingParams--;
		}

		if (solidParams->HasChild("damping", &currentKey))
		{
			damping = currentKey->GetFloatValue();

			if (damping != 0.0f)
			{
				paramUsage |= PHYSPARAM_FLAG_HAS_DAMPING;
			}

			remainingParams--;
		}

		if (solidParams->HasChild("rotdamping", &currentKey))
		{
			rotdamping = currentKey->GetFloatValue();
			
			if (rotdamping != 0.0f)
			{
				paramUsage |= PHYSPARAM_FLAG_HAS_ROTDAMPING;
			}

			remainingParams--;
		}

		if (solidParams->HasChild("inertia", &currentKey))
		{
			inertia = currentKey->GetFloatValue();
			
			if (inertia != 1.0f)
			{
				paramUsage |= PHYSPARAM_FLAG_HAS_INERTIA;
			}

			remainingParams--;
		}

		if (solidParams->HasChild("volume", &currentKey))
		{
			volume = currentKey->GetFloatValue();
			paramUsage |= PHYSPARAM_FLAG_HAS_VOLUME;

			remainingParams--;
		}

		if (solidParams->HasChild("drag", &currentKey))
		{
			drag = currentKey->GetFloatValue();
			paramUsage |= PHYSPARAM_FLAG_HAS_DRAG;

			remainingParams--;
		}

		if (solidParams->HasChild("rollingDrag", &currentKey))
		{
			rollingDrag = currentKey->GetFloatValue();
			paramUsage |= PHYSPARAM_FLAG_HAS_ROLLINGDRAG;

			remainingParams--;
		}

		constexpr ParamFlags_t requiredFlags = (PHYSPARAM_FLAG_HAS_NAME | PHYSPARAM_FLAG_HAS_INDEX);
		assertm(paramUsage & requiredFlags, "solid was missing required data, should not be possible");
		assertm(remainingParams == 0, "solid had params that were skipped");
	}

	CollisionRule::CollisionRule(const kv_parser::Token_t* const ruleParams) : collisionPairs(nullptr), collisionPairCount(0), selfcollisions(true)
	{
		const bool hasSelfCollideParam = ruleParams->HasChild("selfcollisions");
		const int numCollisionPairs = ruleParams->GetNumChildren() - hasSelfCollideParam;

		if (numCollisionPairs > 0)
		{
			collisionPairs = new CollisionPair_t[numCollisionPairs]{};
		}

		for (int i = 0; i < ruleParams->GetNumChildren(); i++)
		{
			const kv_parser::Token_t* const currentKey = ruleParams->GetChild(i);

			if (currentKey->CheckKey("selfcollisions", sizeof("selfcollisions")))
			{
				selfcollisions = false;
				continue;
			}

			if (currentKey->CheckKey("collisionpair", sizeof("collisionpair")))
			{
				AddCollisionPair(currentKey);

				continue;
			}

			assertm(false, "collisionrules had params that were skipped");
		}

		assertm(numCollisionPairs == collisionPairCount, "something bad happened");
	}

	AnimatedFriction::AnimatedFriction(const kv_parser::Token_t* const animFricParams) : animFrictionMin(1.0f), animFrictionMax(1.0f), animFrictionTimeIn(0.0f), animFrictionTimeOut(0.0f), animFrictionTimeHold(0.0f)
	{
		// this param should always have the same five params
		assertm(animFricParams->GetNumChildren() == 5, "bad formatting");

		animFrictionMin = animFricParams->GetChild("animfrictionmin")->GetFloatValue();
		animFrictionMax = animFricParams->GetChild("animfrictionmax")->GetFloatValue();
		animFrictionTimeIn = animFricParams->GetChild("animfrictiontimein")->GetFloatValue();
		animFrictionTimeOut = animFricParams->GetChild("animfrictiontimeout")->GetFloatValue();
		animFrictionTimeHold = animFricParams->GetChild("animfrictiontimehold")->GetFloatValue();
	}

	EditParam::EditParam(const kv_parser::Token_t* const editParams) : rootname(nullptr), totalmass(0.0f), concave(false), jointMergeCount(0), jointMerges(nullptr)
	{
		const bool hasRootName = editParams->HasChild("rootname");
		const bool hasTotalMass = editParams->HasChild("totalmass");
		const bool hasConcave = editParams->HasChild("concave");
		const int numJointMerges = editParams->GetNumChildren() - hasRootName - hasTotalMass - hasConcave;

		if (numJointMerges > 0)
		{
			jointMerges = new JointMerge_t[numJointMerges]{};
		}

		for (int i = 0; i < editParams->GetNumChildren(); i++)
		{
			const kv_parser::Token_t* const currentKey = editParams->GetChild(i);

			if (currentKey->CheckKey("rootname", sizeof("rootname")))
			{
				rootname = AllocString(currentKey->GetStringValue());
				continue;
			}

			if (currentKey->CheckKey("totalmass", sizeof("totalmass")))
			{
				totalmass = currentKey->GetFloatValue();
				continue;
			}

			if (currentKey->CheckKey("concave", sizeof("concave")))
			{
				concave = currentKey->GetIntValue();
				continue;
			}

			if (currentKey->CheckKey("jointmerge", sizeof("jointmerge")))
			{
				AddJointMerge(currentKey);

				continue;
			}

			assertm(false, "editparams had unparsed param");
		}
	}

	inline void IsParamUnique(const float value, int& numValues, float* const values, int* const valueUses)
	{
		bool exists = false;

		for (int i = 0; i < numValues; i++)
		{
			if (values[i] != value)
			{
				continue;
			}

			valueUses[i]++;
			exists = true;

			break;
		}

		if (exists == false)
		{
			values[numValues] = value;
			valueUses[numValues]++;
			numValues++;
		}
	}

	inline const float MostUsedParam(const int numValues, const float* const values, const int* const valueUses)
	{
		int maxUses = 0;
		int maxUseIndex = 0;

		for (int i = 0; i < numValues; i++)
		{
			if (maxUses >= valueUses[i])
			{
				continue;
			}
			
			maxUses = valueUses[i];
			maxUseIndex = i;
		}

		return values[maxUseIndex];
	}

	CParsedPhys::CParsedPhys(const ivps::phyheader_t* const pPHYS) : solids(nullptr), solidCount(pPHYS->solidCount), maxConvexPieces(0), concavePerJoint(false), isJointedCollision(true), collisionRule(nullptr), animatedFricton(nullptr), editParams(nullptr), collisionText(nullptr),
		defaultInertia(s_defaultInertia), defaultDamping(s_defaultDamping), defaultRotDamping(s_defaultRotDamping), defaultParamUsage(PHYSPARAM_FLAG_HAS_NONE)
	{
		assertm(solidCount, "physics with no solid");

		solids = new Solid[solidCount]{};

		const char* properties = pPHYS->pszProperties();
		while (properties[0])
		{
			kv_parser::Token_t parsedParam(&properties, kv_parser::TOKEN_KEY);

			if (parsedParam.CheckKey("solid", sizeof("solid")))
			{
				Solid parsedSolid(&parsedParam);
				const int index = parsedSolid.GetIndex();

				assertm(index < static_cast<int>(solidCount), "solid index was invalid");

				solids[index] = parsedSolid;
				continue;
			}

			if (parsedParam.CheckKey("ragdollconstraint", sizeof("ragdollconstraint")))
			{
				const RagdollConstraint parsedConstraint(&parsedParam);

				// discard ragdoll constraints without a parent (-1)
				const int child = parsedConstraint.GetChild();
				if (child < 0 || child >= pPHYS->solidCount)
				{
					continue;
				}

				solids[child].SetRagdollConstraint(&parsedConstraint);

				continue;
			}

			// for collision rules: collisionpair count = numChildren - 1 (if it has 'selfcollisions'($noselfcollisions))
			if (parsedParam.CheckKey("collisionrules", sizeof("collisionrules")))
			{
				collisionRule = new CollisionRule(&parsedParam);
				continue;
			}

			if (parsedParam.CheckKey("animatedfriction", sizeof("animatedfriction")))
			{
				animatedFricton = new AnimatedFriction(&parsedParam);
				continue;
			}

			if (parsedParam.CheckKey("editparams", sizeof("editparams")))
			{
				editParams = new EditParam(&parsedParam);
				continue;
			}

			const size_t remainingLength = strnlen_s(properties, 0xffff); // no real good way to get this
			assertm(remainingLength < 0xffff, "collision text requires bigger buffer");
			collisionText = AllocString(properties, remainingLength);

			break;
		}

		assertm(editParams, "physics should have editparams");

		// default params
		if (solidCount == 1)
		{
			defaultInertia = solids[0].GetInertia();
			defaultDamping = solids[0].GetDamping();
			defaultRotDamping = solids[0].GetRotDamping();
		}
		// deduplicate and try to find the most used (default value) for inertia, damping, and rotdamping
		// primarily for qc
		else
		{
			constexpr size_t sizePerOneData = (sizeof(float) * 3) + (sizeof(int) * 3);
			char* buf = new char[sizePerOneData * solidCount] {};

			float* inertias = reinterpret_cast<float*>(buf);
			float* dampings = inertias + solidCount;
			float* rotdampings = dampings + solidCount;

			int* inertiaUses = reinterpret_cast<int*>(rotdampings + solidCount);
			int* dampingUses = inertiaUses + solidCount;
			int* rotdampingUses = dampingUses + solidCount;

			int numInterias = 0;
			int numDampings = 0;
			int numRotDampings = 0;

			for (int i = 0; i < solidCount; i++)
			{
				const Solid* const solid = GetSolid(i);

				IsParamUnique(solid->GetInertia(), numInterias, inertias, inertiaUses);
				IsParamUnique(solid->GetDamping(), numDampings, dampings, dampingUses);
				IsParamUnique(solid->GetRotDamping(), numRotDampings, rotdampings, rotdampingUses);
			}

			defaultInertia = MostUsedParam(numInterias, inertias, inertiaUses);
			defaultDamping = MostUsedParam(numDampings, dampings, dampingUses);
			defaultRotDamping = MostUsedParam(numRotDampings, rotdampings, rotdampingUses);

			FreeAllocArray(buf);
		}

		// cycle through solids and remove flags for ones matching the default values
		for (int i = 0; i < solidCount; i++)
		{
			Solid* const solid = solids + i;
			ParamFlags_t& solidFlags = solid->Flags();

			const float inertia = solid->GetInertia();
			const float damping = solid->GetDamping();
			const float rotdamping = solid->GetRotDamping();

			if ((solidFlags & PHYSPARAM_FLAG_HAS_INERTIA) && (inertia == defaultInertia))
			{
				solidFlags &= ~PHYSPARAM_FLAG_HAS_INERTIA;
			}

			if ((solidFlags & PHYSPARAM_FLAG_HAS_DAMPING) && (damping == defaultDamping))
			{
				solidFlags &= ~PHYSPARAM_FLAG_HAS_DAMPING;
			}

			if ((solidFlags & PHYSPARAM_FLAG_HAS_ROTDAMPING) && (rotdamping == defaultRotDamping))
			{
				solidFlags &= ~PHYSPARAM_FLAG_HAS_ROTDAMPING;
			}
		}

		if (defaultInertia != s_defaultInertia)
		{
			defaultParamUsage |= PHYSPARAM_FLAG_HAS_INERTIA;
		}

		if (defaultDamping != s_defaultDamping)
		{
			defaultParamUsage |= PHYSPARAM_FLAG_HAS_DAMPING;
		}

		if (defaultRotDamping != s_defaultRotDamping)
		{
			defaultParamUsage |= PHYSPARAM_FLAG_HAS_ROTDAMPING;
		}

		// todo: surface parsing if needed?
	}
}