/********************************************************************
	created:	2013/10/18
	filename: 	EditGizmoEffet.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "EditGizmoEffect.h"
#include <Element/GraphicsElement.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	EditGizmoEffect::EditGizmoEffect()
	{
		mOrthoCamera.setProjectionType(PT_ORTHOGRAPHIC);
	}

	//////////////////////////////////////////////////////////////////////////
	EditGizmoEffect::~EditGizmoEffect()
	{

	}

	/************************************************************************/
	/* IGraphicsEffect interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	EditGizmoEffect::onAttach()
	{
		GraphicsElement* element = static_cast<GraphicsElement*>( this->getHost() );
		if( element == NULL )
			return false;
		ISpaceContent* content = element->getContent();
		if( content == NULL || content->getSpace() == NULL)
			return false;
		if( mGizmo.getTarget() != NULL )
			return false;

		mGizmo.setTarget(content);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	EditGizmoEffect::onDetach()
	{
		if( mGizmo.getTarget() == NULL )
			return false;

		mGizmo.setTarget(NULL);
		return true;
	}

	/************************************************************************/
	/* IEditGizmoEffect interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	EGizmoType	EditGizmoEffect::getGizmoType() const
	{
		return mGizmo.getCurrentType();
	}

	//////////////////////////////////////////////////////////////////////////
	bool EditGizmoEffect::isVisible() const
	{
		return mGizmo.isVisible();
	}

	//////////////////////////////////////////////////////////////////////////
	bool EditGizmoEffect::isEnabled() const
	{
		return mGizmo.isEnabled();
	}
	
	//////////////////////////////////////////////////////////////////////////
	void EditGizmoEffect::show(bool visible, bool enable, EGizmoType type/* = GT_MOVE*/, GEOM_SPACE space/* = GS_WORLD*/)
	{
		mGizmo.show(visible, enable, type, space);
	}

	//////////////////////////////////////////////////////////////////////////
	EAxis	EditGizmoEffect::hitTest(const Ray& ray, scalar* dist/* = NULL*/) const
	{
		return mGizmo.hitTest(ray, dist);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EditGizmoEffect::highLightAxes(EAxis axis)
	{
		return mGizmo.highLightAxis(axis);
	}
	
}//namespace Blade
