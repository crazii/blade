/********************************************************************
	created:	2013/10/18
	filename: 	EditGizmoEffect.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_EditGizmoEffet_h__
#define __Blade_EditGizmoEffet_h__
#include <interface/public/graphics/GraphicsEffect.h>
#include "../RenderHelper/AxisGizmo.h"
#include "../RenderHelper/HelperRenderType.h"
#include "../../Camera.h"


namespace Blade
{
	class EditGizmoEffect : public IEditGizmoEffect, public Allocatable
	{
	public:
		EditGizmoEffect();
		~EditGizmoEffect();

		/************************************************************************/
		/* IGraphicsEffect interface                                                                     */
		/************************************************************************/

		/** @brief can be shown */
		virtual bool	isReady() const {return true;}

		/** @brief  */
		virtual bool	onAttach();
		/** @brief  */
		virtual bool	onDetach();

		/************************************************************************/
		/* IEditGizmoEffect interface                                                                     */
		/************************************************************************/

		/** @brief  */
		virtual EGizmoType	getGizmoType() const;

		/** @brief  */
		virtual bool isVisible() const;

		/** @brief  */
		virtual bool isEnabled() const;

		/** @brief  */
		virtual void show(bool visible, bool enable, EGizmoType type = GT_MOVE, GEOM_SPACE space = GS_WORLD);

		/** @brief  */
		/* @note x,y are normalized screen space coordinates */
		virtual EAxis	hitTest(const Ray& ray, scalar* dist = NULL) const;

		/** @brief  */
		virtual bool		highLightAxes(EAxis axis);

	protected:
		AxisGizmo			mGizmo;
		mutable Camera		mOrthoCamera;
	};
}//namespace Blade

#endif // __EditGizmoEffet_h__