/********************************************************************
	created:	2010/04/08
	filename: 	IScene.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IScene_h__
#define __Blade_IScene_h__
#include <BladeFramework.h>
#include <utility/String.h>
#include <interface/public/ISerializable.h>

namespace Blade
{
	class IElement;
	class IStage;
	typedef TStringParam ElementTypeList;

	class BLADE_FRAMEWORK_API IScene : public ISerializable
	{
	public:

		virtual ~IScene()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneType() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			getAvailableElementTypes(ElementTypeList& elemList) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IElement*		createElement(const TString& type) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			onAttachToStage(IStage* pstage) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			onDetachFromStage(IStage* pstage) = 0;

	};

	extern template class BLADE_FRAMEWORK_API Factory<IScene>;
	typedef Factory<IScene> SceneFactory;
	
}//namespace Blade


#endif //__Blade_IScene_h__