/********************************************************************
	created:	2011/05/03
	filename: 	GameWorldData.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GameWorldData_h__
#define __Blade_GameWorldData_h__
#include <math/Vector3.h>

#include <interface/IStage.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include <interface/public/logic/CameraActor.h>

namespace Blade
{
	class IGraphicsScene;

	class GameWorldData : public ISerializable, public IGraphicsSpaceCoordinator::IPositionReference, public Allocatable
	{
	public:
		static const TString GAME_WORLD_DATA_TYPE;
		static const TString WORLD_NAME;
		static const TString WORLD_SART_POS;
	public:
		GameWorldData();
		~GameWorldData();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called before saving data  */
		virtual void			prepareSave();

		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IGraphicsSpaceCoordinator::IPositionReference interface                                                                     */
		/************************************************************************/
		virtual const Vector3& getPosition() const
		{
			return mCamera != NULL ? mCamera->getGeometry()->getGlobalPosition() : mStartPostition;
		}

		/** @brief  */
		inline void	setCamera(const CameraActor* camera)					{mCamera = camera;}
		inline const CameraActor* getCamera() const							{return mCamera;}

		/************************************************************************/
		/* custom methods                                                                      */
		/************************************************************************/
		/** @brief  */
		inline const Vector3&getStartPosition() const			{return mStartPostition;}
		
		/** @brief  */
		inline void			setStartPosition(const Vector3& pos){mStartPostition = pos;}

		/** @brief  */
		void		setGraphicsScene(IGraphicsScene* scene);

	protected:
		//runtime data
		IGraphicsScene*		mScene;
		const CameraActor*	mCamera;

		//serialization data
		Vector3			mStartPostition;
		TString			mWorldName;//deprecated:

		//config callback through UI
		void		onConfigChange(const void* data);
		friend class BladeGamePlugin;
	};//class GameWorldData
	

}//namespace Blade



#endif // __Blade_GameWorldData_h__