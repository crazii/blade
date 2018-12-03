/********************************************************************
	created:	2011/04/09
	filename: 	ElementBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ElementBase.h>
#include <interface/IResourceManager.h>
#include "interface_imp/EntityPostLoader.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma  warning(disable:4355) //'this' : used in base member initializer list
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ElementBase::ElementBase(const TString& systemType, const TString& type, EParaPriority priority)
		:mParallelStates(this, (int16)priority)
	{
		mDesc = BLADE_NEW ELEMENT_DESC();
		mDesc->mEntity = NULL;
		mDesc->mScene = NULL;
		mDesc->mType = type;
		mDesc->mSytemType = systemType;
		mDesc->mLoadedCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	ElementBase::ElementBase(const ElementBase& src)
		:mParallelStates(this, src.mParallelStates.getPriority() )
	{
		mDesc = BLADE_NEW ELEMENT_DESC();
		*mDesc = *(src.mDesc);
	}

	//////////////////////////////////////////////////////////////////////////
	ElementBase& ElementBase::operator=(const ElementBase& rhs)
	{
		*mDesc = *(rhs.mDesc);
		return *this;
	}
	
	//////////////////////////////////////////////////////////////////////////
	ElementBase::~ElementBase()
	{
		//assert(mDesc->mLoadedCount <= 2);
		if (mDesc->mResource != NULL)
		{
			this->unloadResource(true);
		}
		BLADE_DELETE mDesc;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ElementBase::loadResource(bool forceAsync)
	{
		{
			ScopedLock lck(mDesc->mSyncLock);

			if (mDesc->mLoadedCount != 0)
			{
				++mDesc->mLoadedCount;
				return true;
			}
			//loading state
			mDesc->mLoadedCount = 1;
		}

		if (mDesc->mResourcePath.empty())
		{
			this->ISerializable::postProcess();
			return false;
		}

		return ElementPostLoader::loadElement(this, forceAsync);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ElementBase::unloadResource(bool forceUnload/* = false*/)
	{
		ScopedLock lck(mDesc->mSyncLock);

		if (mDesc->mLoadedCount > 0 || forceUnload)
		{
			if (--mDesc->mLoadedCount == 1 || forceUnload)
			{
				mDesc->mLoadedCount = 0;
				this->onResourceUnload();
				if (mDesc->mResource != NULL)
					return IResourceManager::getSingleton().unloadResource(mDesc->mResource);
			}
		}
		return false;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

}//namespace Blade
