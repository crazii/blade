/********************************************************************
	created:	2014/10/01
	filename: 	ISyncAnimationController.h
	author:		Crazii
	
	purpose:	sync interface of model element's animation controller
*********************************************************************/
#ifndef __Blade_ISyncAnimationController_h__
#define __Blade_ISyncAnimationController_h__
#include <BladeModel.h>
#include <BladeFramework.h>

namespace Blade
{
	class ISyncAnimation : public Interface
	{
	public:
		/** @brief  */
		inline const tchar*	getCurrentAnimation()
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->getAnimationSync();
		}

		/** @brief  */
		inline scalar	getAnimationLength() const
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->getAnimtionLengthSync();
		}

		/** @brief  */
		inline scalar	getAnimationPos() const
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->getAnimationPosSync();
		}

		/** @brief  */
		inline bool		setAnimationPos(scalar pos)
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->setAnimationPosSync(pos);
		}

		/** @brief  */
		inline scalar	getFrameTime() const
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->getFrameTimeSync();
		}

		/** @brief  */
		inline void		setAniamtedBoundingVisible(bool visible)
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->setAniamtedBoundingVisibleSync(visible);
		}

		/** @brief  */
		inline void		setBoneVisible(bool visible)
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->setBoneVisibleSync(visible);
		}

		/** @brief  */
		inline void			setIKChainPosition(const TString& type, index_t index, const Vector3& pos)
		{
			BLADE_TS_VERIFY( TS_MAIN_SYNC );
			return this->setIKChainPositionSync(type, index, pos);
		}

	protected:

		/** @brief  */
		virtual const tchar*	getAnimationSync() = 0;

		/** @brief  */
		virtual scalar			getAnimtionLengthSync() const = 0;

		/** @brief  */
		virtual scalar			getAnimationPosSync() const = 0;

		/** @brief  */
		virtual bool			setAnimationPosSync(scalar pos) = 0;

		/** @brief  */
		virtual scalar			getFrameTimeSync() const = 0;

		/** @brief  */
		virtual	void			setAniamtedBoundingVisibleSync(bool visible) = 0;

		/** @brief  */
		virtual void			setBoneVisibleSync(bool visible) = 0;

		/** @brief  */
		virtual void			setIKChainPositionSync(const TString& type, index_t index, const Vector3& pos) = 0;
	};

	typedef InterfaceID<ISyncAnimation> IID_SYNCANIM;

}//namespace Blade

#endif // __Blade_ISyncAnimationController_h__