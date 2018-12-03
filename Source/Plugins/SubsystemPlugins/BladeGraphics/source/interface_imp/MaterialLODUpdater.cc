/********************************************************************
	created:	2012/03/25
	filename: 	MaterialLODUpdater.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "MaterialLODUpdater.h"
#include <interface/IPlatformManager.h>

namespace Blade
{

	MaterialLODUpdater::MaterialLODUpdater(ICamera* camera)
		:mCamera(camera)
	{
		mUpdateStateMask.raiseBitAtIndex(SS_FINAL_STATE);
		mTimer = ITimeDevice::create();
		mUpdateCursor = 0;
	}

	MaterialLODUpdater::~MaterialLODUpdater()
	{
		BLADE_DELETE mTimer;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			MaterialLODUpdater::getUpdateInterval() const
	{
		return 0.05f;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MaterialLODUpdater::addForLODUpdate(IMaterialLODUpdatable* updatable)
	{
		if( updatable != NULL )
			return mUpdaterList.insert(updatable).second;
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MaterialLODUpdater::removeFromLODUpdate(IMaterialLODUpdatable* updatable)
	{
		return mUpdaterList.erase(updatable) == 1;
	}

	/************************************************************************/
	/* IGraphicsUpdatable interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void			MaterialLODUpdater::update(SSTATE)
	{
		//update material LOD
		if( mUpdateCursor >= mUpdaterList.size() )
			mUpdateCursor = 0;
	
		static const uint64 MATERIAL_LOD_UPDATE_TIME = uint64(16.6667 / 6.0);

		MaterialLODUpdateList::const_iterator beg = mUpdaterList.begin();
		std::advance(beg, mUpdateCursor);

		mTimer->reset();
		size_t count = 0;
		for(MaterialLODUpdateList::const_iterator iter = beg; iter != mUpdaterList.end(); ++iter )
		{
			if ((++count) % 12 == 0)	//don't call timer->update() frequenty
			{
				mTimer->update();
				if (mTimer->getMilliseconds() > MATERIAL_LOD_UPDATE_TIME)
					break;
			}

			IMaterialLODUpdatable* updatable = *iter;
			IPlatformManager::prefetch<PM_READ>(updatable);
			++mUpdateCursor;

			this->MaterialLODUpdater::updateLOD(updatable);
		}//for
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MaterialLODUpdater::updateLOD(IMaterialLODUpdatable* updatable)
	{
		MaterialInstance* matInst = updatable->getMatInstance();
		if( matInst == NULL || matInst->getLODSetting() == NULL || !matInst->isLoaded() )
			return false;

		//level 0 is highest
		MATERIALLOD::LOD_INDEX currentLOD = matInst->getLOD();
		MATERIALLOD::LOD_INDEX maxLOD = (MATERIALLOD::LOD_INDEX)updatable->getMaxLOD();
		if( maxLOD > currentLOD )//max is lower than current
		{
			updatable->setMaterialLOD((MATERIALLOD::LOD_INDEX)updatable->getMaxLOD());
			return true;
		}

		const LODSetting& LODSettings = *( matInst->getLODSetting() );
		if( LODSettings.size() == 0 )
			return false;
		
		if( (currentLOD != MATERIALLOD::INVALID_LOD && currentLOD >= LODSettings.size()) || maxLOD >= LODSettings.size() )
		{
			assert(false);
			return false;
		}
		else if( currentLOD == MATERIALLOD::INVALID_LOD )
		{
			updatable->setMaterialLOD(maxLOD);
			return true;;
		}

		if( maxLOD == LODSettings.size()-1 )//lowest LOD limit is set, not possible to update
			return true;

		const Vector3& position = updatable->getMatLODPosition();
		scalar radiusSQR = updatable->getRadiusSQR();
		scalar distanceSQR = mCamera->getEyePosition().getSquaredDistance(position);
		distanceSQR -= radiusSQR;

		if( distanceSQR < 0)
		{
			updatable->setMaterialLOD(0);
			return true;
		}
		

		const MATERIALLOD& LOD = LODSettings[currentLOD];
		if( currentLOD > 0 && distanceSQR < LOD.mSquaredDistanceStart && maxLOD < currentLOD)
		{
			const MATERIALLOD& prev = LODSettings[(size_t)currentLOD-1];
			updatable->setMaterialLOD(prev.mLODIndex);
		}
		else if( currentLOD < LODSettings.size()-1 && distanceSQR > LOD.mSquaredDistanceEnd )
		{
			const MATERIALLOD& next = LODSettings[(size_t)currentLOD+1];
			updatable->setMaterialLOD(next.mLODIndex);
		}
		return true;
	}
	

}//namespace Blade