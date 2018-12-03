/********************************************************************
	created:	2012/03/25
	filename: 	MaterialLODUpdater.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialLODUpdater_h__
#define __Blade_MaterialLODUpdater_h__
#include <utility/BladeContainer.h>

#include <interface/IMaterialLODUpdater.h>
#include <interface/ICamera.h>


namespace Blade
{

	class MaterialLODUpdater : public IMaterialLODUpdater, public Allocatable
	{
	public:
		MaterialLODUpdater(ICamera* camera);
		~MaterialLODUpdater();

		/************************************************************************/
		/* IGraphicsUpdatable interface                                                                     */
		/************************************************************************/

		virtual void			update(SSTATE state);

		/************************************************************************/
		/* IMaterialLODUpdatable interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getUpdateInterval() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			addForLODUpdate(IMaterialLODUpdatable* updatable);

		/*
		@describe
		@param
		@return
		*/
		virtual bool			removeFromLODUpdate(IMaterialLODUpdatable* updatable);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			updateLOD(IMaterialLODUpdatable* updatable);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline void			setCamera(ICamera* camera)		{mCamera = camera;}

	protected:
		typedef Set<IMaterialLODUpdatable*>	MaterialLODUpdateList;

		MaterialLODUpdateList	mUpdaterList;
		ICamera*				mCamera;
		ITimeDevice*			mTimer;
		index_t					mUpdateCursor;
	};
	

}//namespace Blade


#endif //__Blade_MaterialLODUpdater_h__