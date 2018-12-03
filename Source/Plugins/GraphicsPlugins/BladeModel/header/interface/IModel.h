/********************************************************************
	created:	2017/2/27
	filename: 	IModel.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IModel_h__
#define __Blade_IModel_h__
#include <interface/Interface.h>
#include <utility/StringList.h>

namespace Blade
{
	class Vector3;
	class Quaternion;

	class IModel : public Interface
	{
	public:
		/** @brief  */
		virtual void			getSubMeshNameList(TStringParam& names) const = 0;

		/** @brief  */
		virtual void			setSubMeshVisible(const TString& name, bool visible) = 0;

		/** @brief  */
		virtual void			getAnimationList(TStringParam& animations) const = 0;

		/** @brief  */
		virtual void			getBoneList(TStringParam& bones) const = 0;

		/** @brief  */
		virtual bool			setAnimation(const TString& animName) = 0;

		/** @brief editor helper */
		virtual void			setSelectedBone(const TString& boneName) = 0;
	};

	typedef InterfaceID<IModel> IID_MODEL;
	
}//namespace Blade

#endif//__Blade_IModel_h__