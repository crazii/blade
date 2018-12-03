/********************************************************************
	created:	2015/11/27
	filename: 	IK.h
	author:		Crazii
	
	purpose:	inverse kinematics
*********************************************************************/
#ifndef __Blade_IK_h__
#define __Blade_IK_h__
#include <memory/BladeMemory.h>
#include <BladeException.h>
#include <utility/Mask.h>
#include "Bone.h"

namespace Blade
{

	enum EIKFlags
	{
		IKF_NONE	= 0x00000000,
		IKF_ACTIVE	= 0x00000001,	///some branches don't participate IK calculation, i.e. effector's children, or IK chain sibling branches.
		IKF_EFFECTOR= 0x00000002,	///end effector
		IKF_SUBBASE = 0x00000004,	///shared base for IK chains/branches
	};

	class IK : public Allocatable
	{
	public:
		
		class Joint
		{
		public:
			const BONE_DATA*	bone;
			mutable uint32		typeIndex;	//index to IK::getEffectorType(), only valid if IKF_EFFECTOR is set.
			mutable Mask		flags;
			mutable uint16		jointIndex;
			mutable uint16		jointParent;

			/** @brief  */
			inline Joint() :bone(0)	{}
			inline Joint(const BONE_DATA* _bone, uint _flags, uint32 type = uint32(-1))
				:bone(_bone),typeIndex(type),flags(_flags),jointIndex(uint16(-1)),jointParent(uint16(-1))
			{}

			/** @brief  */
			inline bool operator<(const Joint& rhs) const	{return bone->mIndex > rhs.bone->mIndex;}	//leaf comes first that parent
			inline bool operator==(const Joint& rhs) const	{return bone->mIndex == rhs.bone->mIndex;}

			/** @brief  */
			inline bool isActive() const	{return flags.checkBits(IKF_ACTIVE);}
			/** @brief  */
			inline bool	isEffector() const	{return flags.checkBits(IKF_EFFECTOR);}
			/** @brief  */
			inline bool isSubBase() const	{return flags.checkBits(IKF_SUBBASE);}
			/** @brief get the bone index of this joint */
			inline uint32 getIndex() const	{return bone->mIndex;}
			/** @brief get the parent bone index of this joint */
			inline uint32 getParent() const {return bone->mParent;}
			/** @brief get joint index */
			inline uint16 getJointIndex() const {return jointIndex;}
			/** @brief get the parent joint index of this joint */
			inline uint16 getJointParent() const {return jointParent;}
			/** @brief  */
			const IK_CONSTRAINTS& getConstraints() const {return bone->mConstraints;}
		private:
			/** @brief  */
			inline void raiseFlags(uint bits) const			{flags.raiseBits(bits);}
			/** @brief  */
			inline void clearFlags(uint bits) const			{flags.clearBits(bits);}
			/** @brief  */
			inline void setType(uint32 type) const			{typeIndex = type;}
			/** @brief  */
			inline void setJointIndex(uint16 index) const	{jointIndex = index;}
			/** @brief  */
			inline void setJointParent(uint16 parent) const	{jointParent = parent;}
			friend class IKChainBuilder;
		};

		class Chain : public Vector<Joint>
		{
		public:
			Chain()	{mEffectorIndex = INVALID_INDEX;}
			//effector index for single effector IK
			//for multiple effector IK(i.e. full body), this member is not used.
			index_t mEffectorIndex;
		};

		typedef List<Chain>					ChainList;

	public:

		IK()	{mSquaredSize = 0;}
		~IK()	{}

		/** @brief  */
		inline size_t	getChainCount() const		{return mChains.size();}

		/** @brief  */
		inline const Chain&	getChain(index_t chainIndex) const
		{
			if( chainIndex < mChains.size() )
			{
				ChainList::const_iterator iter = mChains.begin();
				std::advance(iter, chainIndex);
				return *iter;
			}
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("subscript out of range."));
		}

		/** @brief get skeleton size, it should approximate the model size in model space (un-scaled) */
		inline scalar	getSquaredSize() const{return mSquaredSize;}

		/** @brief  */
		static size_t	getEffectorTypeCount();

		/** @brief  */
		static const TString&	getEffectorType(index_t index);

	protected:
		ChainList	mChains;
		scalar		mSquaredSize;
		friend class IKChainBuilder;
	private:
		/** @brief  */
		static void	initEffectorTypes();

		typedef StaticList<TString> TypeList;
		static TypeList msEffectorTypes;
	};

	//////////////////////////////////////////////////////////////////////////
	class IKChainBuilder : public NonAllocatable, public NonAssignable
	{
	public:
		IKChainBuilder();
		~IKChainBuilder();

		/** @brief auto detect IK chains. TODO: use per-project config file to config IK chains */
		bool	buildIKChians(const BoneTable& boneTable, IK& chainSet);

	protected:
		typedef TempSet<IK::Joint>					JointSet;
		typedef TempMap<const BONE_DATA*, JointSet>	EffectorMap;

		/** @brief  */
		void	 countChainReference(size_t* childCounting, const BONE_DATA* effector, const BONE_DATA* boneData, size_t count);

		/** @brief  */
		void	collectChildJoints(const BONE_DATA* bone, JointSet& set, const BONE_DATA* boneData, size_t count);

		/** @brief  */
		void	collectJoints(const BONE_DATA* effector, JointSet& set, const BONE_DATA* boneData, const size_t* childCounting, size_t count);

		/** @brief  */
		inline static bool isEffector(const BONE_DATA* bone, const BoneTable& boneTable, uint32* typeIndex = NULL)
		{
			TString name = TStringHelper::getLowerCase( TString(boneTable.getBoneName(bone->mIndex)));
			size_t count = IK::getEffectorTypeCount();
			for(size_t i = 0; i < count; ++i)
			{
				const TString& type = IK::getEffectorType(i);
				if( name.find(type) != TString::npos )
				{
					if( typeIndex != NULL )
						*typeIndex = (uint32)i;
					return true;
				}
			}
			return false;
		}

		bool mEnableHand;
		bool mFullBody;
	};
	
}//namespace Blade

#endif // __Blade_IK_h__