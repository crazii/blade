/********************************************************************
	created:	2013/11/03
	filename: 	ModelViewEditable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelViewEditable.h"
#include "BladeModel.h"
#include <interface/IStage.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/geometry/IGeometryScene.h>
#include <EntityResourceDesc.h>
#include "ModelViewElement.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ModelViewEditable::ModelViewEditable()
		:EditableBase(BTString("ModelViewEditable"), BTString("ModelViewEditable"))
	{
		mEntity = NULL;
		mFlag = EDF_EDF_TRASFORMABLE | EDF_SELECTABLE;
	}

	//////////////////////////////////////////////////////////////////////////
	ModelViewEditable::~ModelViewEditable()
	{

	}

	/************************************************************************/
	/* IEditable interface                                                                     */
	/************************************************************************/


	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ModelViewEditable::initialize(IStage* stage, IGraphicsScene* graphicsScene, IGeometryScene* geomScene, const TString& modelFile)
	{
		if( mEntity != NULL || mModelElement != NULL )
		{
			assert(false);
			return false;
		}

		assert( geomScene != NULL && graphicsScene != NULL && stage != NULL );

		//create entity & graphics model element
		mEntity = stage->createEntity(modelFile);
		mModelElement.bind( graphicsScene->createElement(ModelConsts::MODEL_ELEMENT_TYPE) );
		const TString ELEMENT_NAME = BTString("Model");
		mEntity->addElement(ELEMENT_NAME,  mModelElement);

		//create editor element
		if( !this->createEditorElement(ModelViewElement::MODELVIEW_ELEMENT_TYPE, mEntity) )
			return false;

		//create geometry
		IElement* geomElem = geomScene->createElement(GeomConsts::GEOMETRY_ELEMENT);
		mGeometry = geomElem->getInterface(IID_GEOMETRY());
		const TString GEOM_ELEMENT_NAME = BTString("Geom");
		mEntity->addElement(GEOM_ELEMENT_NAME, HELEMENT(geomElem) );

		//load model file
		EntityResourceDesc desc;
		desc.addElementResource( ELEMENT_NAME, modelFile );
		//TODO: error handling - such as model not found or model's texture not found.
		return stage->loadEntitySync(mEntity, &desc);
	}
	
}//namespace Blade