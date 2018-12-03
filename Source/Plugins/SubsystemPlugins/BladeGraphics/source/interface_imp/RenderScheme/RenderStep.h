/********************************************************************
	created:	2011/12/18
	filename: 	RenderStep.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderStep_h__
#define __Blade_RenderStep_h__
#include <BladeGraphics.h>
#include <utility/BladeContainer.h>

#include <interface/public/graphics/RenderProperty.h>

#include "RenderSchemeDesc.h"

namespace Blade
{
	class Technique;
	class Pass;
	class IRenderSorter;

	class RenderStep : public Allocatable
	{
	public:
		RenderStep(const STEP_DESC* desc);
		~RenderStep();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		/** @brief  */
		void			setShaderGroups(const Technique* tech);

		/** @brief  */
		inline const STEP_DESC* getDesc() const
		{
			return mDesc;
		}

		/** @brief  */
		inline const TString&	getTypeName() const
		{
			return mDesc->mTypeDesc->mRenderTypeName;
		}

		/** @brief  */
		inline const RenderType* getType() const
		{
			return mType;
		}

		/** @brief  */
		inline bool				isEnabled() const
		{
			return mEnabled;
		}

		/** @brief  */
		inline void				setEnable(bool enable)
		{
			mEnabled = enable;
		}

		/** @brief  */
		inline Pass*			getMaterialPass() const
		{
			return mRenderPass;
		}

		/** @brief  */
		inline void				setMaterialPass(Pass* pass)
		{
			mRenderPass = pass;
		}

		/** @brief  */
		FILL_MODE				getFillMode() const
		{
			return mFillMode;
		}

		/** @brief  */
		void					setFillMode(FILL_MODE fm)
		{
			mFillMode = fm;
		}

		/** @brief  */
		IRenderSorter*			getSorter() const
		{
			return mSorter;
		}

		/** @brief  */
		size_t					getShaderGroups(const index_t*& indices) const
		{
			size_t count = mShaderGroups.size();
			if( count > 0 )
				indices = &mShaderGroups[0];
			else
				indices = NULL;
			return count;
		}

	protected:
		typedef Vector<index_t>	ShaderGroupIndices;

		const STEP_DESC*	mDesc;
		Pass*				mRenderPass;
		IRenderSorter*		mSorter;
		RenderType*			mType;
		ShaderGroupIndices	mShaderGroups;			
		FILL_MODE			mFillMode : 8;
		bool				mEnabled;
	};//class RenderStep
	

}//namespace Blade



#endif // __Blade_RenderStep_h__