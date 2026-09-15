#pragma once

#include <model/studio/studio.h>
#include <model/studio/studio_p2.h>
#include <model/studio/studio_r1.h>
#include <model/studio/studio_r2.h>
#include <model/studio/studio_r5.h>
#include <model/studio/studio_r5_v12.h>
#include <model/studio/studio_r5_v16.h>

namespace r1
{
	void CalcBoneQuaternion(int frame, float s,
		const mstudiobone_t* pBone,
		const mstudiolinearbone_t* pLinearBones,
		const mstudio_rle_anim_t* panim, Quaternion& q);

	void CalcBonePosition(int frame, float s,
		const mstudiobone_t* pBone,
		const mstudiolinearbone_t* pLinearBones,
		const mstudio_rle_anim_t* panim, Vector& pos);

	void CalcBoneScale(int frame, float s,
		const Vector& baseScale, const Vector& baseScaleScale,
		const mstudio_rle_anim_t* panim, Vector& scale);

	void SkipBoneFrame(const uint8_t flags, const uint8_t*& pConstData, const uint8_t*& pFrameData);
	const uint8_t* const ExtractSingleFrame(const uint8_t flags, const uint8_t* pFrameData, const uint8_t*& pConstantData, Quaternion& q, Vector& pos, Vector& scale, bool bIsDelta = false, const mstudiolinearbone_t* const pLinearBones = nullptr, int bone = 0);
	const uint8_t* const ExtractTwoFrames(const uint8_t flags, const float s, const uint8_t* pFrameData, const uint8_t*& pConstantData, const int framelength, Quaternion& q, Vector& pos, Vector& scale, bool bIsDelta = false, const mstudiolinearbone_t* const pLinearBones = nullptr, int bone = 0);

	bool Studio_AnimPosition(const mstudioanimdesc_t* const panim, float flCycle, Vector& vecPos, QAngle& vecAngle); // might use 'Studio_AnimMovement' in the future
}

namespace r2
{
	void CalcBoneQuaternion(int frame, float s,
		const mstudiobone_t* pBone,
		const mstudiolinearbone_t* pLinearBones,
		const mstudio_rle_anim_t* panim, Quaternion& q);

	void CalcBonePosition(int frame, float s,
		const mstudiobone_t* pBone,
		const mstudiolinearbone_t* pLinearBones,
		const mstudio_rle_anim_t* panim, Vector& pos);

	void CalcBoneScale(int frame, float s,
		const Vector& baseScale, const Vector& baseScaleScale,
		const mstudio_rle_anim_t* panim, Vector& scale);

	bool Studio_AnimPosition(const mstudioanimdesc_t* const panim, float flCycle, Vector& vecPos, QAngle& vecAngle); // might use 'Studio_AnimMovement' in the future
}

namespace r5
{
	void CalcBoneQuaternion(int frame, float s, const mstudio_rle_anim_t* panim, const RadianEuler& baseRot, Quaternion& q, const char boneFlags);
	void CalcBonePosition(int frame, float s, const mstudio_rle_anim_t* panim, Vector& pos);
	void CalcBoneScale(int frame, float s, const mstudio_rle_anim_t* panim, Vector& scale, const char boneFlags);

//#pragma pack(push, 2)
//	namespace v8
//	{
//		struct ikcontextikrule_t
//		{
//			int			index;
//
//			int			type;
//			int			chain;
//
//			int			bone;
//
//			int			slot;	// iktarget slot.  Usually same as chain.
//			float		height;
//			float		endHeight;
//			float		radius;
//			float		floor;
//			Vector		pos;
//			Quaternion	q;
//
//			float		start;	// beginning of influence
//			float		peak;	// start of full influence
//			float		tail;	// end of full influence
//			float		end;	// end of all influence
//
//			float		top;
//			float		drop;
//
//			float		commit;		// frame footstep target should be committed
//			float		release;	// frame ankle should end rotation from latched orientation
//
//			float		flWeight;		// processed version of start-end cycle
//			float		flRuleWeight;	// blending weight
//			float		latched;		// does the IK rule use a latched value?
//			char* szLabel;
//
//			Vector		kneeDir;
//			Vector		kneePos;
//
//			ikcontextikrule_t() {};
//
//		private:
//			// No copy constructors allowed
//			//ikcontextikrule_t(const ikcontextikrule_t& vOther);
//		};
//		static_assert(sizeof(ikcontextikrule_t) == 0x90);
//	}
//
//	namespace v16
//	{
//		struct ikcontextikrule_t
//		{
//			// possibly 16bit
//			uint8_t type;
//			uint8_t unk_1;
//			short chain;
//
//			// possibly 32bit
//			short bone;
//			short unk_6;
//
//			int slot;
//			float height;
//			float endHeight;
//			float radius;
//			float floor;
//			Vector pos;
//			Quaternion q;
//
//			float start;
//			float peak;
//			float tail;
//			float end;
//
//			float top;
//			float drop;
//
//			float commit;
//			float release;
//
//			float flWeight;
//			float flRuleWeight;
//			float latched;
//			char* szLabel;
//
//			Vector kneeDir;
//			Vector kneePos;
//		};
//		static_assert(sizeof(ikcontextikrule_t) == 0x88);
//	}
//#pragma pack(pop)
}