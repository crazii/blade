/********************************************************************
	created:	2012/03/28
	filename: 	RenderChannel.h
	author:		Crazii
	purpose:	this class is for controll of render steps, objects of this type is binded to each render scheme
*********************************************************************/
#ifndef __Blade_RenderChannel_h__
#define __Blade_RenderChannel_h__
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IGraphicsChannel.h>

#include "RenderOutput.h"
#include "RenderStepInstance.h"

namespace Blade
{
	class RenderPhase;

	class RenderChannel : public IGraphicsChannel, public Allocatable
	{
	public:
		RenderChannel(const TString& renderTypeName);
		~RenderChannel();

		/************************************************************************/
		/* IGraphicsChannel interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&		getRenderType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				isEnabled() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setEnable(bool enabled);

		inline void					enable()	{this->setEnable(true);}
		inline void					disable()	{this->setEnable(false);}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setFillMode(FILL_MODE mode);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual FILL_MODE			getFillMode() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void				gatherRenderSteps(RenderPhase* phase);
		

	protected:

		typedef List<RenderStepInstance*>	StepInstanceList;
		StepInstanceList	mRenderSteps;

		TString		mRenderType;
		FILL_MODE	mFillMode;
		bool		mEnable;
	};
	

}//namespace Blade


#endif //__Blade_RenderChannel_h__