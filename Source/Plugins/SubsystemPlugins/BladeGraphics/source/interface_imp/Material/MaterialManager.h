/********************************************************************
	created:	2012/01/19
	filename: 	MaterialManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialManager_h__
#define __Blade_MaterialManager_h__
#include <interface/IMaterialManager.h>
#include <utility/BladeContainer.h>
#include <Material.h>

namespace Blade
{

	class MaterialManager : public IMaterialManager, public Singleton<MaterialManager>
	{
	public:
		using Singleton<MaterialManager>::getSingleton;
		using Singleton<MaterialManager>::getSingletonPtr;
	public:
		MaterialManager();
		~MaterialManager();

		/************************************************************************/
		/* IMaterialManager interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			initialize();

		/*
		@describe
		@param
		@return
		*/
		virtual void			shutdown();

		/*
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			destroyMaterial(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Material*		createMaterial(const TString& name);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool					addMaterial(const HMATERIAL& hMaterial);

		/*
		@describe
		@param
		@return
		*/
		HMATERIAL&				getOrAddMaterialRef(const TString& name);

	protected:
		typedef TStringMap<HMATERIAL>	MaterialList;
		MaterialList			mMaterialList;
	};//class MaterialManager
	

}//namespace Blade



#endif // __Blade_MaterialManager_h__