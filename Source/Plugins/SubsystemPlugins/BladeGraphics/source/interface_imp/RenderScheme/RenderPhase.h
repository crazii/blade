/********************************************************************
	created:	2013/04/28
	filename: 	RenderPhase.h
	author:		Crazii
	purpose:	this class is for control of render outputs, objects of this type is binded to each view objects
				it acts as a instance of render output
*********************************************************************/
#ifndef __Blade_RenderPhase_h__
#define __Blade_RenderPhase_h__
#include <interface/IRenderPhase.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class RenderOutput;
	class RenderStepInstance;

	class RenderPhase : public IRenderPhase, public Allocatable
	{
	public:
		typedef Vector<RenderStepInstance*>		StepInstanceList;
	public:
		RenderPhase(RenderOutput* output);
		~RenderPhase();

		/************************************************************************/
		/* IGraphicsPhase interface                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			isEnabled() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			setEnable(bool enabled);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getCameraName() const;

		/**
		@describe
		@param
		@return
		*/
		virtual IGraphicsView*	getView() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IRenderTarget*	getTarget() const;

		/**
		@describe
		@param
		@return
		*/
		virtual size_t			getStepCount() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual const RenderType* getStepRenderType(index_t step) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		RenderOutput*			getRenderOutput() const { return mOutput; }
		/** @brief  */
		size_t					getStepInstanceCount() const			{return mStepInstances.size();}
		/** @brief  */
		RenderStepInstance*		getStepInstance(index_t index) const	{return mStepInstances[index];}
		/** @brief  */
		const StepInstanceList&	getStepInstances() const				{return mStepInstances;}

	protected:	
		RenderOutput*		mOutput;
		StepInstanceList	mStepInstances;
		bool				mEnabled;
	};//class RenderPhase
	
}//namespace Blade

#endif //  __Blade_RenderPhase_h__