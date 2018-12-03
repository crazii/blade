/********************************************************************
	created:	2010/04/10
	filename: 	IResource.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IResource_h__
#define __Blade_IResource_h__
#include <BladeFramework.h>
#include <Handle.h>
#include <Factory.h>
#include <utility/BiRef.h>
#include "ITask.h"

namespace Blade
{
	class IResource;
	typedef Handle<IResource>	HRESOURCE;

	class BLADE_FRAMEWORK_API IResource : public NonAssignable, public NonCopyable
	{
	public:
		enum EUserFlag
		{
			UF_UNDEFINED = 0,
			UF_PHONY,		//a phony resource doesn't require actual data in disk/media
		};

		/************************************************************************/
		/* Listener class                                                                     */
		/************************************************************************/
		class BLADE_FRAMEWORK_API IListener : public BiRef
		{
		public:
			virtual ~IListener()		{}

			/**
			@describe 
			@param 
			@return 
			*/
			virtual void	preLoad()	{}

			/**
			@describe 
			@param resource maybe NULL when loading failed
			@return 
			*/
			virtual void	postLoad(const HRESOURCE& resource) { BLADE_UNREFERENCED(resource); }

			/**
			@describe when loading succeed
			@param
			@return return value is implementation defined, not used by resource manager
			*/
			virtual bool	onReady() { return true; }

			/**
			@describe when loading failed
			@param
			@return return value is implementation defined, not used by resource manager
			*/
			virtual bool	onFailed() { return true; }

			/**
			@describe 
			@param 
			@return 
			*/
			virtual void	onUnload()		{}

		protected:
			/** @brief  */
			inline bool		isListening() const { return this->getRef() != NULL; }
			/** @brief cancel listening resource, usually for unload a loading resource */
			inline void		cancel() { this->detach(); }

		};//class Listener

	public:
		IResource() { mFlag = 0; mUserFlag = UF_UNDEFINED; }
		virtual ~IResource()	{}

		/** @brief  */
		inline const TString&	getSource() const	{return mSource;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const = 0;

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
		virtual const ITask::Type&	getProcessTaskType() const { return ITask::NO_TYPE; }

		/** @brief  */
		inline void				setUserFlag(uint16 flag) { mUserFlag = flag; }
		/** @brief  */
		inline uint16			getUserFlag() const { return mUserFlag; }

	private:
		/** @brief  */
		inline void				setSource(const TString& source) { mSource = source; }
		/** @brief  */
		inline void				setFlag(uint16 flag)				{ mFlag = flag;}
		/** @brief  */
		inline uint16			getFlag() const { return mFlag; }

		TString	mSource;
		uint16 	mFlag;	//used internally
		uint16	mUserFlag;
		friend class IResourceManager;
	};//class IResource

	extern template class BLADE_FRAMEWORK_API Factory<IResource>;
	typedef Factory<IResource>	ResourceFactory;
	
}//namespace Blade


#endif //__Blade_IResource_h__