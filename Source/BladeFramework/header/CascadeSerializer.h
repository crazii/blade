/********************************************************************
	created:	2013/04/13
	filename: 	CascadeSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CascadeSerializer_h__
#define __Blade_CascadeSerializer_h__
#include <interface/public/ISerializer.h>
#include <ResourceState.h>

namespace Blade
{

	class BLADE_FRAMEWORK_API CascadeSerializer : public ISerializer
	{
	public:
		class SubResourceGroup : public ResourceStateGroup, public TempAllocatable
		{
		public:
			using ResourceStateGroup::addSubState;
			using ResourceStateGroup::getSubStateCount;
			using ResourceStateGroup::getSubState;
			using ResourceStateGroup::clearSubStates;
			using ResourceStateGroup::clearAndDeleteSubStates;

			virtual ~SubResourceGroup()
			{
				this->clearAndDeleteSubStates();
			}
		};

		class SubResource : public SubResourceState, public TempAllocatable	{};
	public:
		CascadeSerializer();
		virtual ~CascadeSerializer();

		/**
		@describe this method is newly added to solve sub resource problem : whether load synchronously or not.
		commonly, you don't need to override this method, it's used by framework.
		@param 
		@return 
		*/
		virtual bool	loadResourceSync(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			mSyncLoading = true;
			return this->loadResource(resource, stream, params);
		}

		/**
		@describe this method is lately added to solve sub resource problem : whether load synchronously or not.
		commonly, you don't need to override this method, it's used by framework.
		@param 
		@return 
		*/
		virtual bool	reloadResourceSync(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			mSyncLoading = true;
			return this->reloadResource(resource, stream, params);
		}

		/**
		@describe check whether the serializer finished loading the resource (before post-process)
		commonly one serlializer is done loading after loadResource().
		but if some cascade resource contains linkage to other sub resources, then it is not done until the sub resources is loaded
		and this method is right for the cascade resource type.
		@param 
		@return 
		*/
		virtual bool	isLoaded(IResource* resource)
		{
			bool result = mGroup.isLoaded();
			if (result)
				this->postLoad(resource);
			return result;
		}

	protected:
		/** @brief called before postProcess, after all sub resources loaded  */
		virtual void postLoad(IResource* resource) { BLADE_UNREFERENCED(resource); }

		/** @brief  */
		inline bool	isSyncLoading() const	{return mSyncLoading;}

		/** @brief  */
		inline void		clearSubResources() { mGroup.unload(); }

		/* note: sub state is auto-deleted on destruction */
		inline bool		addSubResource(ResourceState* state)	{return mGroup.addSubState(state);}

		/** @brief wrapper for addSubState, you can use addSubState for more flexibility */
		SubResource*		addSubResource(const TString& resourcePath,
			const TString& resourceType = TString::EMPTY, const TString& serializerType = TString::EMPTY);

		/** @brief wrapper method, you can use addSubState for more flexibility */
		SubResourceGroup*	addSubGroup();

		/** @brief !important: start sub resource loading */
		void		loadSubResources();

	private:
		SubResourceGroup	mGroup;
		bool				mSyncLoading;
	};

	
}//namespace Blade


#endif //  __Blade_CascadeSerializer_h__