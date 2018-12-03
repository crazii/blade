/********************************************************************
	created:	2010/05/23
	filename: 	GraphicsResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsResource_h__
#define __Blade_GraphicsResource_h__
#include <BladeGraphics.h>
#include <interface/public/IResource.h>

namespace Blade
{
	class BLADE_GRAPHICS_API GraphicsResource : public IResource
	{
	public:
		GraphicsResource(const TString& type);
		~GraphicsResource();

		/************************************************************************/
		/* IResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const;

		/**
		@describe get the subsystem specific processing task type where the serializer need to do its work
		because tasks of the same type are executed serially,  this avoids doing sync works or process in main sync state
		subsystem's tasks can process its resource in parallel
		i.e. a graphics resource need to build video buffer, in main sync state to avoid rendering at the same time, when set
		task type of the same type of graphics task, resource manager will do the reprocessResource()/postProcessResource() in serial with graphics task.
		return TString::EMPTY if it want to be postProcessResource() in main sync state
		@param
		@return
		*/
		virtual const ITask::Type&	getProcessTaskType() const
		{
			return GraphicsResource::getGraphicsTaskType();
		}

		/**
		@describe 
		@param
		@return
		*/
		static const ITask::Type&	getGraphicsTaskType();

	protected:
		TString		mType;
	};
	
}//namespace Blade


#endif //__Blade_GraphicsResource_h__