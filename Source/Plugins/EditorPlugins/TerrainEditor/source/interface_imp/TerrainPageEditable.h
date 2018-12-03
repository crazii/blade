/********************************************************************
	created:	2011/04/15
	filename: 	TerrainPageEditable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainPageEditable_h__
#define __Blade_TerrainPageEditable_h__
#include <interface/public/IEditable.h>
#include <interface/public/EditableBase.h>
#include <interface/public/logic/ILogicScene.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class IStage;
	class TerrainPageEditable : public EditableBase , public Allocatable
	{
	public:
		static const TString TERRAIN_PAGE_EDITABLE_NAME;
	public:
		TerrainPageEditable();
		~TerrainPageEditable();

		/************************************************************************/
		/* IEditable interface                                                                     */
		/************************************************************************/
		/*
		@describe run time creation
		@param 
		@return 
		*/
		virtual bool			initialize(const IConfig* creationConfig,const ParamList* additionalParams = NULL);

		/*
		@describe serialization: loading
		@param 
		@return 
		*/
		virtual bool			initialize(EditorElement* element);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			save()							{return true;}


		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		TString						generateTilePath(size_t x, size_t z) const;

	protected:
		/** @brief  */
		void				setupTerrainEditables(ILogicWorld* pWorld);

	protected:
		IEntity*			mPageEntity;
	};//class TerrainPageEditable


	

}//namespace Blade



#endif // __Blade_TerrainPageEditable_h__