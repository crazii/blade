/********************************************************************
	created:	2011/08/30
	filename: 	LightManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/ISpace.h>
#include <GraphicsShaderShared.inl>
#include "LightManager.h"
#include "../Camera.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	LightManager::LightManager()
		:mGlobalAmbient(Color::WHITE*0.6f)
		,mSpace(NULL)
		,mMainLight(NULL)
	{
		mGlobalLightList.reserve(MAX_GLOBAL_LIGHT_COUNT);

		//extra ambient light for deferred shading
		//it doesn't take effect on forward shading
		mAmbient = BLADE_NEW AmbientLight(this);	//this will call notifyLightCreated()
	}

	//////////////////////////////////////////////////////////////////////////
	LightManager::~LightManager()
	{
		//all light should be destroyed before manager:
		//this should be better
		BLADE_DELETE mAmbient;
		assert(mLocalLightList.size() == 0);
		assert(mGlobalLightList.size() == 0);
	}

	//////////////////////////////////////////////////////////////////////////
	const Color&	LightManager::getGlobalAmbient() const
	{
		return mGlobalAmbient;
	}

	//////////////////////////////////////////////////////////////////////////
	void			LightManager::setGlobalAmbient(const Color& color)
	{
		mGlobalAmbient = color;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			LightManager::getGlobalLightCount() const
	{
		return mGlobalLightList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	ILight**		LightManager::getGlobalLightList()
	{
		if (mGlobalLightList.size() == 0)
			return NULL;
		else
			return &mGlobalLightList[0];
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			LightManager::getLightList(ILight** outList, size_t count)
	{
		if (outList == NULL)
			return mLocalLightList.size();

		count = std::min(count, mLocalLightList.size());
		LightList::iterator iter = mLocalLightList.begin();
		for (size_t i = 0; i < count; ++i)
			outList[i] = *(++iter);

		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	ILight*			LightManager::getMainLight() const
	{
		return mMainLight;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LightManager::setMainLight(ILight* light)
	{
		if( light != NULL && light->getType() != LT_DIRECTIONAL )
		{
			assert(false);
			return false;
		}
		mMainLight = light;

		if( light != NULL )
		{
			//set up main light camera
			Matrix44 viewMatrix;
			Matrix44::generateViewMatrix(viewMatrix, light->getPosition(), light->getPosition()+light->getDirection() );
		}
		return true;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	LightManager::notifyLightCreated(ILight* light)
	{
		assert( light != NULL && light->getType() <= LT_SPOT );
		if (light->getType() == LT_DIRECTIONAL)
		{
			//disable ambient for deferred shading since directional lights already take ambient in account
			mAmbient->setLocalAABB(AABB(BT_NULL));
			mGlobalLightList.push_back(light);
			if (mMainLight == NULL)
				mMainLight = light;
		}
		else
			mLocalLightList.push_back(light);
	}

	//////////////////////////////////////////////////////////////////////////
	void	LightManager::notifyLightDeleted(ILight* light)
	{
		assert( light != NULL && light->getType() <= LT_SPOT );
		LightList::iterator i = std::find(mLocalLightList.begin(), mLocalLightList.end(),light);
		if( i != mLocalLightList.end() )
			mLocalLightList.erase(i);
		else
		{
			LightArray::iterator iter = std::find(mGlobalLightList.begin(), mGlobalLightList.end(), light);
			if (iter != mGlobalLightList.end())
				mGlobalLightList.erase(iter);
			if (mMainLight == light)
				mMainLight = NULL;
		}

		if (light == mAmbient)
			return;
		if (mGlobalLightList.size() == 0)
			mAmbient->setLocalAABB(AABB(BT_INFINITE));
		else
			mAmbient->setLocalAABB(AABB(BT_NULL));
	}

	//////////////////////////////////////////////////////////////////////////
	void	LightManager::notifyLightTypeChanged(ILight* light, ELightType oldType, ELightType newType)
	{
		if (mAmbient == light)
			return;

		if (oldType == LT_DIRECTIONAL && newType != LT_DIRECTIONAL)
		{
			LightArray::iterator iter = std::find(mGlobalLightList.begin(), mGlobalLightList.end(), light);
			if (iter != mGlobalLightList.end())
				mGlobalLightList.erase(iter);
			mLocalLightList.push_back(light);

			if (mMainLight == light)
				mMainLight = NULL;
		}
		else if (oldType != LT_DIRECTIONAL && newType == LT_DIRECTIONAL)
		{
			LightList::iterator i = std::find(mLocalLightList.begin(), mLocalLightList.end(), light);
			if (i != mLocalLightList.end())
				mLocalLightList.erase(i);
			mGlobalLightList.push_back(light);

			assert(mMainLight != light);
			if (mMainLight == NULL)
				mMainLight = light;
		}

		if (mGlobalLightList.size() == 0)
			mAmbient->setLocalAABB(AABB(BT_INFINITE));
		else
			mAmbient->setLocalAABB(AABB(BT_NULL));
	}

	//////////////////////////////////////////////////////////////////////////
	void	LightManager::setSpace(ISpace* space)
	{
		assert(mSpace == NULL);
		mSpace = space;
		mSpace->addContent(mAmbient);
	}

}//namespace Blade
