/********************************************************************
	created:	2013/04/28
	filename: 	RenderStepInstance.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderStepInstance_h__
#define __Blade_RenderStepInstance_h__
#include "RenderStep.h"

namespace Blade
{
	class RenderStepInstance : public Allocatable
	{
	public:
		RenderStepInstance(RenderStep* step);
		~RenderStepInstance();

		/*
		@describe 
		@param 
		@return 
		*/
		RenderStep*			getRenderStep() const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool				isEnabled() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void				setEnable(bool enabled);

		inline void			enable()	{this->setEnable(true);}
		inline void			disable()	{this->setEnable(false);}

		/*
		@describe 
		@param 
		@return 
		*/
		 void				setFillMode(FILL_MODE mode);

		/*
		@describe 
		@param 
		@return 
		*/
		FILL_MODE			getFillMode() const;

		/**
		@describe 
		@param
		@return
		*/
		bool				isFixedFillMode() const;
		
	protected:
		RenderStep*		mStep;
		FILL_MODE		mFillMode;
		bool			mEnable;
		bool			mFixedFillMode;
	};//class RenderStepInstance
	
}//namespace Blade

#endif //  __Blade_RenderStepInstance_h__