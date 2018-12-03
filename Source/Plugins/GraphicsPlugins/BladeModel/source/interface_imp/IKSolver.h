/********************************************************************
	created:	2015/11/27
	filename: 	IKSolver.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IKSolver_h__
#define __Blade_IKSolver_h__
#include "Bone.h"

namespace Blade
{
	class IK;
	class DualQuaternion;

	/** @brief default solver using CCD, for multiple separated chains with one effector in each */
	class IKSolver : public Allocatable
	{
	public:
		IKSolver();
		virtual ~IKSolver();

		/** @brief  */
		void	reset();
		
		/** @brief  */
		bool	setTarget(const TString& effectorType, index_t index, const Vector3& modelPosition);

		/** @brief  */
		bool	removeTarget(const TString& effectorType, index_t index);

		/** @brief  */
		size_t	getEffectorCount(const TString& effectorType) const;

		/** @brief keep an reference of chainSet */
		virtual bool	initialize(const IK* _IK);

		/** @brief bonePalette in model space (applied skeleton hierarchy but not apply inversed binding pose yet) */
		virtual bool	solve(DualQuaternion* bonePalette, const BoneActivity* boneAcitvity, const BONE_DATA* boneData, size_t count);

	protected:
		struct EffectorTarget
		{
			TString	type;
			index_t	index;	//chain index, or effector index in full body mode
			Vector3 pos;
			bool	enabled;

			inline bool operator==(const EffectorTarget& rhs) const	{return type == rhs.type && index == rhs.index;}
			inline bool operator<(const EffectorTarget& rhs) const	{return FnTStringFastLess::compare(type, rhs.type) || (type == rhs.type &&  index < rhs.index);}
		};
		typedef Vector<EffectorTarget>	TargetList;
		typedef TempSet<EffectorTarget>	TargetSet;

		TargetList			mTargets;
		const IK*			mIK;	//linkage to resource data
		size_t				mValidTargets;
	};

	/** @brief full body IK solver for unique chain with multiple end effector */
	class FullBodyIKSolver : public IKSolver
	{
	public:
		FullBodyIKSolver();
		virtual ~FullBodyIKSolver();

		/** @brief keep an reference of chainSet */
		virtual bool	initialize(const IK* _IK);

		/** @brief bonePalette in model space (applied skeleton hierarchy but not apply inversed binding pose yet) */
		virtual bool	solve(DualQuaternion* bonePalette, const BoneActivity* boneAcitvity, const BONE_DATA* boneData, size_t count);
	};

	extern template class Factory<IKSolver>;
	
}//namespace Blade


#endif // __Blade_IKSolver_h__