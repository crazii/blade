/********************************************************************
	created:	2013/11/03
	filename: 	ModelViewEditable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelViewEditable_h__
#define __Blade_ModelViewEditable_h__
#include <interface/public/EditableBase.h>
#include <interface/public/geometry/IGeometry.h>

namespace Blade
{
	class IStage;
	class IGraphicsScene;
	class IGeometryScene;
	class ModelViewEditable : public EditableBase, public Allocatable
	{
	public:
		ModelViewEditable();
		~ModelViewEditable();

		/************************************************************************/
		/* IEditable interface                                                                     */
		/************************************************************************/

		/*
		@describe run time creation
		@param 
		@return 
		*/
		virtual bool			initialize(const IConfig* /*creationConfig*/,const ParamList* /*additionalParams = NULL*/)	{return false;}

		/*
		@describe serialization: loading
		@param 
		@return 
		*/
		virtual bool			initialize(EditorElement* /*element*/)	{return false;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			save()		{return false;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/** @brief  */
		bool			initialize(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene, const TString& modelFile);

		/** @brief  */
		const HELEMENT&	getModelElement() const		{return mModelElement;}
		
		/** @brief  */
		IGeometry* getGeometry() const		{return mGeometry;}

	protected:
		IEntity*		mEntity;
		HELEMENT		mModelElement;
		IGeometry*		mGeometry;
	};
	
}//namespace Blade

#endif //  __Blade_ModelViewEditable_h__