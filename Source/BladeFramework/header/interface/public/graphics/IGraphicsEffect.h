/********************************************************************
	created:	2011/06/02
	filename: 	IGraphicsEffect.h
	author:		Crazii
	purpose:	simple effect that can be add, remove, modify by users
				effects can be attached to elements to keep the transform (position, scale, rotation) to host IGraphicsInterface
				or it can be static, when not attached. most effects need to be attached to host to be working.
*********************************************************************/
#ifndef __Blade_IGraphicsEffect_h__
#define __Blade_IGraphicsEffect_h__
#include <Handle.h>
#include <Factory.h>
#include <BladeFramework.h>

namespace Blade
{
	class IGraphicsInterface;
	class IGraphicsEffectManager;

	class IGraphicsEffect
	{
	public:
		inline IGraphicsEffect(const TString& type)
			:mType(type)
			,mHost(NULL)
			,mManager(NULL)
		{}
		virtual ~IGraphicsEffect()	{}

		/** @brief can be shown, used for timing */
		virtual bool	isReady() const = 0;

		/** @brief  */
		inline const TString&	getType() const		{return mType;}

		/** @brief  */
		inline IGraphicsInterface* getHost() const	{return mHost;}
		/** @brief  */
		inline IGraphicsEffectManager* getManager() const {return mManager;}

	private:
		/** @brief  */
		inline void	setManager(IGraphicsEffectManager* manager) { mManager = manager; }

		/** @brief return true if attach successfully, or effect will not be attached */
		virtual bool	onAttach() = 0;
		/** @brief  */
		virtual bool	onDetach() = 0;

		/** @brief  */
		inline bool		attach(IGraphicsInterface* elem)
		{
			if( mHost == NULL )
			{
				mHost = elem;
				return this->onAttach();
			}
			else
				return false;
		}
		/** @brief  */
		inline bool		detach(IGraphicsInterface* elem)
		{
			bool ret = false;
			if( mHost != NULL && mHost == elem)
			{
				ret = this->onDetach();
				mHost = NULL;
			}
			return ret;
		}

		/** @brief notification from attaching host */
		virtual bool	onPositionChange(IGraphicsInterface* itf)	{BLADE_UNREFERENCED(itf);return false;}
		virtual bool	onRotationChange(IGraphicsInterface* itf)	{BLADE_UNREFERENCED(itf);return false;}
		virtual bool	onScaleChange(IGraphicsInterface* itf)	{BLADE_UNREFERENCED(itf);return false;}
		virtual bool	onLocalBoundsChange(IGraphicsInterface* itf) { BLADE_UNREFERENCED(itf); return false; }

		TString					mType;
		IGraphicsInterface*		mHost;
		IGraphicsEffectManager* mManager;

		friend class IGraphicsInterface;
		friend class IGraphicsEffectManager;
	};//class IGraphicsEffect

	typedef Handle<IGraphicsEffect> HGRAPHICSEFFECT;

}//namespace Blade



#endif // __Blade_IGraphicsEffect_h__