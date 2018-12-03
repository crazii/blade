/********************************************************************
	created:	2012/03/25
	filename: 	IMaterialLODUpdater.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IMaterialLODUpdater_h__
#define __Blade_IMaterialLODUpdater_h__
#include <math/Vector3.h>
#include <interface/IGraphicsUpdater.h>
#include <MaterialInstance.h>

namespace Blade
{
	class IMaterialLODUpdatable
	{
	public:
		virtual index_t				getMaxLOD() const = 0;
		virtual MaterialInstance*	getMatInstance() = 0;
		virtual Vector3				getMatLODPosition() const = 0;
		virtual scalar				getRadiusSQR() const = 0;
		virtual void				setMaterialLOD(MATERIALLOD::LOD_INDEX LOD) { this->getMatInstance()->setLOD(LOD); }
	};


	class BLADE_GRAPHICS_API IMaterialLODUpdater : public IGraphicsUpdatable
	{
	public:
		virtual ~IMaterialLODUpdater()	{}

		/*
		@describe
		@param
		@return
		*/
		virtual scalar			getUpdateInterval() const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			addForLODUpdate(IMaterialLODUpdatable* updatable) = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			removeFromLODUpdate(IMaterialLODUpdatable* updatable) = 0;

		/*
		@describe udpate right now
		@param 
		@return 
		*/
		virtual bool			updateLOD(IMaterialLODUpdatable* updatable) = 0;

	};
	

}//namespace Blade


#endif //__Blade_IMaterialLODUpdater_h__