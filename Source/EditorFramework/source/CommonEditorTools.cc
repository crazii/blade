/********************************************************************
	created:	2011/05/17
	filename: 	CommonEditorTools.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "CommonEditorTools.h"
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/geometry/IGeometryScene.h>
#include <interface/IPage.h>
#include <interface/IEventManager.h>
#include <interface/EditorElement.h>
#include <interface/IEditorFramework.h>
#include <interface/IEditorConfigManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/EditorEvents.h>
#include <interface/public/EditableBase.h>
#include <EditorHelper.h>
#include "interface_imp/LightEditable.h"

namespace Blade
{
	static const Color COLOR_HIGHLIGHT = Color::WHITE;
	static const Color COLOR_SELECTED =  Color::RED;
	static const Color COLOR_SELECTED_HIGHLIGHT = Color::GREEN;

	//////////////////////////////////////////////////////////////////////////
	SelectTool::SelectTool(IconIndex icon,HOTKEY hotkey/* = KC_UNDEFINED*/,const TString name/* = BTString("Select Tool") */)
		:EditorCommonTool(name,icon,hotkey)
	{
		mHighLight = NULL;
		mEnableHighLight = true;
	}

	//////////////////////////////////////////////////////////////////////////
	SelectTool::~SelectTool()
	{

	}

	/************************************************************************/
	/* IEditorTool interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				SelectTool::onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		IEditable* editable = this->pickEditable(x, y, file);
		if(editable == NULL )
			this->markAllSelected(false);

		if( (keyModifier&KM_SHIFT) && editable != NULL )
		{
			if( editable->isSelected() )
			{
				file->removeSelectedEditable(editable);
				if( mEnableHighLight )
					editable->getEditorElement()->showBounding(false);
			}
			else
			{
				file->addSelectedEditable(editable);
				if( mEnableHighLight  )
					editable->getEditorElement()->showBounding(true, COLOR_SELECTED);
			}
		}
		else
		{
			this->markAllSelected(false);
			file->setSelectedEditable(editable);
		}
		mHighLight = editable;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SelectTool::onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		BLADE_UNREFERENCED(x);
		BLADE_UNREFERENCED(y);
		BLADE_UNREFERENCED(keyModifier);
		BLADE_UNREFERENCED(file);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SelectTool::onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		BLADE_UNREFERENCED(x);
		BLADE_UNREFERENCED(y);
		BLADE_UNREFERENCED(keyModifier);
		BLADE_UNREFERENCED(file);

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SelectTool::update(scalar x,scalar y , const POINT3& /*deltaMove*/, uint32 /*keyModifier*/,scalar /*deltaTime*/,IEditorFile* file)
	{
		//the effect attach/detach is thread safe now,
		if (file == NULL)
			return false;

		if (!mEnableHighLight)
		{
			if (mHighLight != NULL)
			{
				mHighLight->getEditorElement()->showBounding(false);
				mHighLight = NULL;
			}
			return false;
		}

		IEditable* editable = this->pickEditable(x, y, file);
		if (mHighLight != NULL && mHighLight != editable)
		{
			if (mHighLight->isSelected())
				mHighLight->getEditorElement()->showBounding(true, COLOR_SELECTED);
			else
				mHighLight->getEditorElement()->showBounding(false);
		}

		mHighLight = editable;

		if (mHighLight != NULL)
		{
			if (mHighLight->isSelected())
				mHighLight->getEditorElement()->showBounding(true, COLOR_SELECTED_HIGHLIGHT);
			else
				mHighLight->getEditorElement()->showBounding(true, COLOR_HIGHLIGHT);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				SelectTool::mainUpdate(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* /*file*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	static TString COMMON_CATEGORY = BXLang(BLANG_COMMON);
	const TString&		SelectTool::getMannaulCategory() const
	{
		return COMMON_CATEGORY;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		SelectTool::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	IconIndex			SelectTool::getToolIcon() const
	{
		return mIconIndex;
	}

	//////////////////////////////////////////////////////////////////////////
	HOTKEY				SelectTool::getHotkey() const
	{
		return mHotkey;
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&		SelectTool::getToolConfig() const
	{
		return HCONFIG::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				SelectTool::getSupportedFiles(TStringParam& outFiles)
	{
		BLADE_UNREFERENCED(outFiles);

		//default: support all file types
		return size_t(-1);
	}

	/************************************************************************/
	/* EditorCommonTool                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				SelectTool::onSelection()
	{
		IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
		if( file == NULL )
			return;
		if( mHighLight != NULL )
			mHighLight->getEditorElement()->showBounding(false);
		mHighLight = NULL;
		this->markAllSelected(mSelected);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	SelectTool::markAllSelected(bool showMark)
	{
		IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
		if( file == NULL )
			return 0;

		size_t count = 0;
		IEditable*const* editables = file->getSelectedEditables(count);
		for(size_t i = 0; i < count; ++i)
		{
			IEditable* editable = editables[i];
			editable->getEditorElement()->showBounding(showMark, COLOR_SELECTED);
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		class EditableGizmoSetImpl : public Set<IEditable*>, public  Allocatable	{};
	}//namespace Impl
	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	TransformTool::TransformTool(IconIndex icon,HOTKEY hotkey, const TString& name, EGizmoType type)
		:SelectTool(icon, hotkey, name)
		,mGizmoSet(BLADE_NEW EditableGizmoSetImpl() )
		,mType(type)
	{
		mCustomSpace = false;
		mUpdateTarget = NULL;
		mUpdateAxis = GA_NONE;
		mTranslationSpace = GS_WORLD;
		mEnableHighLight = false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TransformTool::onSelection()
	{
		IEditorFile* file = IEditorFramework::getSingleton().getCurrentFile();
		if( file == NULL )
			return;
		if( mSelected )
		{
			mGizmoSet->clear();
			GEOM_SPACE space = mCustomSpace ? mTranslationSpace : IEditorConfigManager::getSingleton().getTransformSpace();

			size_t count = 0;
			IEditable*const* editables = file->getSelectedEditables(count);
			if( space == GS_LOCAL || space == GS_OBJECT )
			{
				//GS_LOCAL / GS_OBJECT: multiple separated helpers
				for(size_t i = 0; i < count; ++i)
				{
					IEditable* editable = editables[i];
					this->addEditableGizmo(editable);
				}
			}
			else
			{
				//TODO: GS_WORLD: one helper for single grouped editable, calculate pivot
				for(size_t i = 0; i < count; ++i)
				{
					IEditable* editable = editables[i];
					this->addEditableGizmo(editable);
				}
			}
			IEventManager::getSingleton().addEventHandler(TransformSpaceEvent::NAME, EventDelegate(this, &TransformTool::onTransformSpaceChange));
		}
		else
		{
			//clear all gizmo helpers
			size_t count = 0;
			IEditable*const* editables = file->getSelectedEditables(count);
			for(size_t i = 0; i < count; ++i)
			{
				IEditable* editable = editables[i];
				IEditGizmoEffect* effect = static_cast<IEditGizmoEffect*>(editable->getEditorElement()->getGizmoEffect());
				if( effect != NULL && effect->getGizmoType() == mType )
					editable->getEditorElement()->showGizmoHelper(false);
			}
			mGizmoSet->clear();
			IEventManager::getSingleton().removeEventHandlers(TransformSpaceEvent::NAME, this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TransformTool::onFileSwitch(const IEditorFile* prevFile,const IEditorFile* currentFile)
	{
		SelectTool::onFileSwitch(prevFile, currentFile);
		this->clearTarget();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TransformTool::onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		//do hit test, if hit on current editing one, don't switch target
		this->onMouseLeftDown(x, y, keyModifier, file);
		if (mUpdating)
		{
			mUpdating = false;
			return false;
		}

		bool ret = this->SelectTool::onMouseLeftClick(x, y, keyModifier, file);
		if( !ret )
			return ret;

		//add new gizmo if selected any
		size_t count = 0;
		IEditable*const* editables = file->getSelectedEditables(count);
		for(size_t i = 0; i < count; ++i)
			this->addEditableGizmo(editables[i]);

		GEOM_SPACE space = mCustomSpace ? mTranslationSpace : IEditorConfigManager::getSingleton().getTransformSpace();

		//remove existing gizmo if un-selected
		if( space == GS_LOCAL || space == GS_OBJECT )
		{
			//GS_LOCAL / GS_OBJECT: multiple separated helpers
			for(EditableGizmoSetImpl::iterator i = mGizmoSet->begin(); i != mGizmoSet->end();)
			{
				IEditable* editable = *i;
				if(!editable->isSelected())
				{
					mGizmoSet->erase(i++);
					editable->getEditorElement()->showGizmoHelper(false);
				}
				else
					++i;
			}
		}
		else
		{
			//TODO: add to grouped editable
			for(EditableGizmoSetImpl::iterator i = mGizmoSet->begin(); i != mGizmoSet->end(); )
			{
				IEditable* editable = *i;
				if(!editable->isSelected())
				{
					mGizmoSet->erase(i++);
					editable->getEditorElement()->showGizmoHelper(false);
				}
				else
					++i;
			}
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TransformTool::onMouseLeftDown(scalar x,scalar y,uint32 /*keyModifier*/,IEditorFile* file)
	{
		mUpdating = false;
		if( file == NULL )
			return false;
		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		if( view == NULL || scene == NULL || file->getDefaultCamera()->getCamera() == NULL)
			return false;

		if( x < 0 || x > 1 || y < 0 || y > 1)
			return false;

		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		Ray ray = camera->getSpaceRayfromViewPoint(x,y);
		scalar fDist = FLT_MAX;
		IEditGizmoEffect* nearestEffect = NULL;
		EAxis nearestEffectAxis = GA_NONE;
		mUpdateTarget = NULL;
		for(EditableGizmoSetImpl::const_iterator i = mGizmoSet->begin(); i != mGizmoSet->end(); ++i)
		{
			const HGRAPHICSEFFECT& hEffect = (*i)->getEditorElement()->getGizmoEffect();
			IEditGizmoEffect* effect = static_cast<IEditGizmoEffect*>(hEffect);
			scalar dist = fDist;
			//disable high lights
			effect->highLightAxes(GA_NONE);
			EAxis axis = effect->hitTest(ray, &dist);
			if( axis != GA_NONE && dist < fDist)
			{
				nearestEffect = effect;
				nearestEffectAxis = axis;
				mUpdateTarget = *i;
				fDist = dist;
			}
		}
		mUpdateAxis = GA_NONE;
		if( nearestEffect != NULL && nearestEffect->isEnabled() )
		{
			mUpdateAxis = nearestEffectAxis;
			nearestEffect->highLightAxes(nearestEffectAxis);
			mUpdating =  true;
		}
		else
		{
			mUpdateTarget = NULL;
			mUpdating = false;
			mUpdateAxis = GA_NONE;
		}

		if( !mUpdating )
		{
			assert(mUpdateTarget == NULL);
			assert(mUpdateAxis == GA_NONE);
		}
		else
		{
			assert(mUpdateTarget != NULL);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TransformTool::onMouseLeftUp(scalar /*x*/,scalar /*y*/,uint32 /*keyModifier*/,IEditorFile* file)
	{
		if (mUpdating && (file->getTypeInfo()->mOperations&EFO_SAVE))
		{
			assert(mUpdateTarget != NULL);
			mUpdateTarget->onTransformed(file);
			file->setModified();
		}

		mUpdating = false;
		return mUpdating;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TransformTool::mainUpdate(scalar x, scalar y, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* file)
	{
		if( mUpdating )
			return false;

		if( file == NULL )
			return false;
		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		if( view == NULL || scene == NULL || file->getDefaultCamera()->getCamera() == NULL)
			return false;

		if( x < 0 || x > 1 || y < 0 || y > 1)
			return false;

		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		Ray ray = camera->getSpaceRayfromViewPoint(x,y);
		scalar fDist = FLT_MAX;
		IEditGizmoEffect* nearestEffect = NULL;
		EAxis nearestEffectAxis = GA_NONE;
		for(EditableGizmoSetImpl::const_iterator i = mGizmoSet->begin(); i != mGizmoSet->end(); ++i)
		{
			const HGRAPHICSEFFECT& hEffect = (*i)->getEditorElement()->getGizmoEffect();
			IEditGizmoEffect* effect =static_cast<IEditGizmoEffect*>(hEffect);
			scalar dist = fDist;
			//disable high lights
			effect->highLightAxes(GA_NONE);
			EAxis axis = effect->hitTest(ray, &dist);
			if( axis != GA_NONE && dist < fDist)
			{
				nearestEffect = effect;
				nearestEffectAxis = axis;
				fDist = dist;
			}
		}
		if( nearestEffect != NULL )
		{
			if( (nearestEffectAxis&GA_VIEWXY) == 0 )
				nearestEffect->highLightAxes(nearestEffectAxis);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TransformTool::addEditableGizmo(IEditable* editable)
	{
		if( editable == NULL || editable->getEditorElement() == NULL )
			return false;
		GEOM_SPACE space = mCustomSpace ? mTranslationSpace : IEditorConfigManager::getSingleton().getTransformSpace();
		editable->getEditorElement()->showGizmoHelper(true, mType, space);
		editable->getEditorElement()->showBounding(true, COLOR_HIGHLIGHT);

		const HGRAPHICSEFFECT& gizmo = editable->getEditorElement()->getGizmoEffect();
		if( gizmo == NULL )
			return false;
		return mGizmoSet->insert( editable ).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TransformTool::removeEditableGizmo(IEditable* editable)
	{
		if( editable != NULL )
		{
			editable->getEditorElement()->showGizmoHelper(false);
			editable->getEditorElement()->showBounding(false);
			return mGizmoSet->erase(editable) == 1;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TransformTool::clearTarget()
	{
		mUpdateTarget = NULL;
		mUpdating = false;
		mGizmoSet->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	Vector3		TransformTool::getAxis(EAxis axis, const Quaternion& rotation)
	{
		if( axis == GA_X || axis == GA_Y || axis == GA_Z )
		{
			const EAxis AXES[3] = {GA_X, GA_Y, GA_Z};
			const Vector3* AXIS_DIR[3] = { &Vector3::UNIT_X, &Vector3::UNIT_Y, &Vector3::UNIT_Z,  };
			for(int i = 0; i < 3; ++i)
			{
				if( axis == AXES[i] )
					return (*AXIS_DIR[i]) * rotation;
			}
			assert(false);
			return Vector3::ZERO;
		}
		else
		{
			assert(false);
			return Vector3::ZERO;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const Quaternion&	TransformTool::getAxisRotation() const
	{
		if( mUpdateTarget == NULL || mUpdateTarget->getEditorElement()->getGeomtry() == NULL )
		{
			assert(false);
			return Quaternion::IDENTITY;
		}

		GEOM_SPACE space = mCustomSpace ? mTranslationSpace : IEditorConfigManager::getSingleton().getTransformSpace();
		IGeometry* geom = mUpdateTarget->getEditorElement()->getGeomtry();
		const Quaternion& rotation = space == GS_WORLD ? Quaternion::IDENTITY : geom->getGlobalRotation();
		return rotation;
	}

	//////////////////////////////////////////////////////////////////////////
	static const Plane INVALID_PLANE = Plane(Vector3::ZERO, 0);
	Plane				TransformTool::getGizmoPlane(const Ray& ray, IGraphicsCamera* camera)
	{
		if( camera == NULL || mUpdateTarget == NULL || mUpdateTarget->getEditorElement()->getGeomtry() == NULL )
		{
			assert(false);
			return INVALID_PLANE;
		}
		IGeometry* geom = mUpdateTarget->getEditorElement()->getGeomtry();
		const POINT3& pos = geom->getGlobalPosition();
		const Quaternion& rotation = this->getAxisRotation();
		if( mUpdateAxis == GA_XYZ )//and: use view plane
			return Plane(-camera->getLookAtDirection(), pos);

		//plane decided by intersection angle between ray & plane (MAX)
		//plane with largest intersection angle will be selected
		Plane planeX(rotation.getXAxis(), pos);
		Plane planeY(rotation.getYAxis(), pos);
		Plane planeZ(rotation.getZAxis(), pos);

		scalar cosXR = std::fabs( planeX.getNormal().dotProduct(ray.getDirection()) );
		scalar cosYR = std::fabs( planeY.getNormal().dotProduct(ray.getDirection()) );
		scalar cosZR = std::fabs( planeZ.getNormal().dotProduct(ray.getDirection()) );
		if( (mUpdateAxis&GA_X) )
			cosXR = 0;
		if( (mUpdateAxis&GA_Y) )
			cosYR = 0;
		if( (mUpdateAxis&GA_Z) )
			cosZR = 0;
		if( cosXR > cosYR && cosXR > cosZR )
			return planeX;
		else if( cosYR > cosXR && cosYR > cosZR )
			return planeY;
		else
			return planeZ;
	}

	//////////////////////////////////////////////////////////////////////////
	void				TransformTool::onTransformSpaceChange(const Event& data)
	{
		assert(data.mName == TransformSpaceEvent::NAME );
		if( mCustomSpace )
			return;
		const TransformSpaceEvent& state = static_cast<const TransformSpaceEvent&>(data);

		for(EditableGizmoSetImpl::const_iterator i = mGizmoSet->begin(); i != mGizmoSet->end(); ++i)
		{
			IEditable* editable = *i;
			editable->getEditorElement()->showGizmoHelper(true, mType, state.mSpace);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	TranslateTool::TranslateTool(IconIndex icon,HOTKEY hotkey/* = KC_UNDEFINED*/)
		:TransformTool(icon,hotkey, BXLang(BLANG_TOOL_TRANSLATE), GT_MOVE )
	{

	}

	TranslateTool::~TranslateTool()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool		TranslateTool::onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		bool ret = TransformTool::onMouseLeftDown(x, y, keyModifier, file);
		if( ret && mUpdating )
		{
			IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
			Ray curRay = camera->getSpaceRayfromViewPoint(x, y);
			Plane axisPlane = this->getGizmoPlane(curRay, camera);
			scalar t = 0;
			curRay.intersect(axisPlane, t);
			mUpdatePos = curRay.getRayPoint(t);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TranslateTool::update(scalar x, scalar y, const POINT3& deltaMove, uint32 /*keyModifier*/,scalar /*deltaTime*/,IEditorFile* file)
	{
		if( !mUpdating )
			return false;
		if(file == NULL )
			return false;
		IGraphicsView* view = file->getView();
		if(view == NULL)
			return false;
		if( file->getDefaultCamera() == NULL )
			return false;
		if( Math::Equal(deltaMove.x, scalar(0.0), Math::HIGH_EPSILON) &&
			Math::Equal(deltaMove.y, scalar(0.0), Math::HIGH_EPSILON)
			)
			return true;
		assert( mUpdateAxis != GA_NONE );
		assert( mUpdateTarget != NULL);

		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		Ray curRay = camera->getSpaceRayfromViewPoint(x, y);
		Plane axisPlane = this->getGizmoPlane(curRay, camera);
		scalar t;
		if( !curRay.intersect(axisPlane, t) )
		{
			//because we use a MAX intersection angle plane for the ray
			//the ray should be hit the plane always
			//assert(false);
			return true;
		}

		POINT3 curPoint = curRay.getRayPoint(t);
		IGeometry* geomtry = mUpdateTarget->getEditorElement()->getGeomtry();
		assert(geomtry != NULL);
		const POINT3& pos = geomtry->getGlobalPosition();
		Vector3 moveVector = curPoint - mUpdatePos;
		const scalar MAX_MOVEMENT = 100.0f;
		if( moveVector.isZero() || moveVector.getLength() > MAX_MOVEMENT)
			return true;

		EAxis AXES[3] = {GA_X, GA_Y, GA_Z};
		Vector3 totalMoveDelta = Vector3::ZERO;
		for(int i = 0; i < 3; ++i)
		{
			if( (mUpdateAxis&AXES[i]) == 0 )
				continue;
			const Vector3& axis = TransformTool::getAxis(AXES[i], this->getAxisRotation() );
			//projected move delta
			scalar dist = moveVector.dotProduct(axis);
			totalMoveDelta += axis*dist;
		}
		geomtry->setGlobalPosition( pos + totalMoveDelta );
		mUpdatePos = curPoint;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	RotateTool::RotateTool(IconIndex icon,HOTKEY hotkey/* = KC_UNDEFINED*/)
		:TransformTool(icon,hotkey, BXLang(BLANG_TOOL_ROATE), GT_ROTATE )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	RotateTool::~RotateTool()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool		RotateTool::onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		bool ret = TransformTool::onMouseLeftDown(x, y, keyModifier, file);
		if( ret && mUpdating )
		{
			IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
			Ray ray = camera->getSpaceRayfromViewPoint(x, y);
			const HGRAPHICSEFFECT& hEffect = mUpdateTarget->getEditorElement()->getGizmoEffect();
			IEditGizmoEffect* effect = static_cast<IEditGizmoEffect*>(hEffect);
			scalar dist = FLT_MAX;
			EAxis axis = effect->hitTest(ray, &dist);

			POINT3 hitPoint = ray.getRayPoint(dist);
			POINT3 center = mUpdateTarget->getEditorElement()->getGeomtry()->getGlobalPosition();
			POINT3 vRadius = hitPoint - center;
			Vector3 vAxis;
			if( (axis&GA_VIEWMASK) == 0 )
				vAxis = this->getAxis(axis, this->getAxisRotation());
			else if( axis == GA_VIEWZ )
				vAxis = -camera->getLookAtDirection();
			else
				return ret;

			Vector3 tangent = vAxis.crossProduct(vRadius);
			tangent *= (Matrix33)camera->getViewMatrix();	//to view space
			mProjectedTangent = Plane(Vector3::NEGATIVE_UNIT_Z, 0).getProjectedVector(tangent);	//project to a view plane (projection space vector)
			mProjectedTangent.normalize();
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		RotateTool::update(scalar /*x*/, scalar /*y*/, const POINT3& deltaMove, uint32 /*keyModifier*/,scalar /*deltaTime*/,IEditorFile* file)
	{
		if( !mUpdating )
			return false;
		if(file == NULL )
			return false;
		IGraphicsView* view = file->getView();
		if(view == NULL)
			return false;
		if( file->getDefaultCamera() == NULL )
			return false;
		if( Math::Equal(deltaMove.x, scalar(0.0), Math::HIGH_EPSILON) &&
			Math::Equal(deltaMove.y, scalar(0.0), Math::HIGH_EPSILON)
			)
			return true;
		assert( mUpdateAxis != GA_NONE );
		assert( mUpdateTarget != NULL);
		//normalized movement to view/screen movement
		Vector3 movement = deltaMove;
		movement.x *= (view->getPixelRight()-view->getPixelLeft());
		movement.y *= -(view->getPixelBottom()-view->getPixelTop());	//screen space dir => projection space dir

		IGeometry* geomtry = mUpdateTarget->getEditorElement()->getGeomtry();
		assert(geomtry != NULL);
		Quaternion rotation = geomtry->getGlobalRotation();

		
		if( (mUpdateAxis&GA_VIEWMASK) != 0 )
		{
			//view space rotation
			IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
			const Vector3 VIEWAXES_DIR[3] = {Vector3::UNIT_X*camera->getOrientation(), Vector3::UNIT_Y*camera->getOrientation(), Vector3::UNIT_Z*camera->getOrientation()};
			const EAxis AXES[3] = {GA_VIEWX, GA_VIEWY, GA_VIEWZ};

			/** @brief screen space dirs Y+, X+, P => projection space : -Y, X+, P) */
			const Vector3 REFERENCE_DIR[3] = {Vector3::NEGATIVE_UNIT_Y, Vector3::UNIT_X, mProjectedTangent };

			for(int i = 0; i < 3; ++i)
			{
				if( (mUpdateAxis&AXES[i]) == 0 )
					continue;
				const Vector3& axis = VIEWAXES_DIR[i]; 
				scalar degree = movement.dotProduct(REFERENCE_DIR[i]);
				rotation = rotation*Quaternion(axis, Math::Degree2Radian(degree) );
				geomtry->setGlobalRotation(rotation);
			}
		}
		else
		{
			const Vector3& axis = this->getAxis(mUpdateAxis, this->getAxisRotation());
			scalar degree = movement.dotProduct(mProjectedTangent);
			rotation = rotation*Quaternion(axis, Math::Degree2Radian(degree) );
			geomtry->setGlobalRotation(rotation);
		}
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ScaleTool::ScaleTool(IconIndex icon,HOTKEY hotkey/* = KC_UNDEFINED*/)
		:TransformTool(icon,hotkey, BXLang(BLANG_TOOL_SCALE), GT_SCALE )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ScaleTool::~ScaleTool()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool		ScaleTool::onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
	{
		bool ret = TransformTool::onMouseLeftDown(x, y, keyModifier, file);
		if( ret && mUpdating )
		{
			EAxis constraint = mUpdateTarget->getScaleConstraint();
			if( (mUpdateAxis&constraint) != 0 )
				(int&)mUpdateAxis |= constraint;

			IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
			Ray curRay = camera->getSpaceRayfromViewPoint(x, y);
			Plane axisPlane = this->getGizmoPlane(curRay, camera);
			scalar t = 0;
			curRay.intersect(axisPlane, t);

			POINT3 curPoint = camera->getProjectedPosition( curRay.getRayPoint(t) );
			POINT3 projectedPos = POINT3::ZERO;
			IGeometry* geomtry = mUpdateTarget->getEditorElement()->getGeomtry();

			ret = geomtry != NULL && curPoint.z >= 0;
			if(ret)
			{
				projectedPos =  camera->getProjectedPosition(geomtry->getGlobalPosition());
				ret = projectedPos.z >= 0;
			}
			if(ret)
			{
				curPoint.z = 0;
				projectedPos.z = 0;
				mUpdateLength = curPoint.getDistance(projectedPos);
				ret = mUpdateLength > FLT_MIN;
			}
		}
		if(!ret)
		{
			mUpdating = false;
			mUpdateTarget = NULL;
		}
		return ret;
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool		ScaleTool::update(scalar x, scalar y, const POINT3& deltaMove, uint32 /*keyModifier*/,scalar /*deltaTime*/,IEditorFile* file)
	{
		if( !mUpdating )
			return false;
		if(file == NULL )
			return false;
		IGraphicsView* view = file->getView();
		if(view == NULL)
			return false;
		if( file->getDefaultCamera() == NULL )
			return false;
		if( Math::Equal(deltaMove.x, scalar(0.0), Math::HIGH_EPSILON) &&
			Math::Equal(deltaMove.y, scalar(0.0), Math::HIGH_EPSILON)
			)
			return true;
		assert( mUpdateAxis != GA_NONE );
		assert( mUpdateTarget != NULL);
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();

		Ray curRay = camera->getSpaceRayfromViewPoint(x, y);
		Plane axisPlane = this->getGizmoPlane(curRay, camera);
		scalar t;
		if( !curRay.intersect(axisPlane, t) )
		{
			//because we use a MAX intersection angle plane for the ray
			//the ray should be hit the plane always
			assert(false);
			return true;
		}
		IGeometry* geomtry = mUpdateTarget->getEditorElement()->getGeomtry();
		assert(geomtry != NULL);

		//check ray point or object visible
		POINT3 curPoint = camera->getProjectedPosition( curRay.getRayPoint(t) );
		if(curPoint.z < 0)
			return true;
		POINT3 projectedPos =  camera->getProjectedPosition(geomtry->getGlobalPosition());
		if( projectedPos.z < 0 )
			return true;
		curPoint.z = 0;
		projectedPos.z = 0;
		scalar curLength = curPoint.getDistance(projectedPos);
		if( curLength < FLT_MIN )
			return true;

		const Vector3& scale = geomtry->getGlobalScale();

		EAxis AXES[3] = {GA_X, GA_Y, GA_Z};
		Vector3 scaleDelta = Vector3::UNIT_ALL;
		for(size_t i = 0; i < 3; ++i)
		{
			if( (mUpdateAxis&AXES[i]) == 0 )
				continue;
			scaleDelta[i] *= curLength / mUpdateLength;
		}
		geomtry->setGlobalScale(scale*scaleDelta);
		mUpdateLength = curLength;
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	LightCreationTool::LightCreationTool(IconIndex icon, HOTKEY hotKey/* = HOTKEY()*/)
		:EditorCommonTool(BTString("Create Lights"), icon, hotKey)
	{
		mCreationDistance = 10;
		mLightEditable = NULL;
		mCreating = false;
	}

	//////////////////////////////////////////////////////////////////////////
	LightCreationTool::~LightCreationTool()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightCreationTool::onMouseLeftDown(scalar x, scalar y, uint32 keyModifier, IEditorFile* file)
	{
		BLADE_UNREFERENCED(keyModifier);
		if( !(x >= 0 && x <= 1 && y >= 0 && y <= 1) )
			return false;

		if(file->getGraphicsScene() == NULL || file->getStage() == NULL || file->getDefaultCamera() == NULL 
			|| file->getEditableManager() == NULL )
			return false;

		if (mCreating)
			return true;

		//create light entity
		//IGraphicsCamera* cam = file->getDefaultCamera()->getCamera();
		//Ray ray = cam->getSpaceRayfromViewPoint(x, y);
		POINT3 pos = this->pickPosition(x, y, FULL_APPFLAG, mCreationDistance, file);

		IPage* page = file->getStage()->getPagingManager()->getPageByPosition(pos.x, pos.z);
		assert(page != NULL);

		TString lightName = file->getValidEntityName(LightEditable::LIGHT_EDITABLE_NAME, EditableBase::generatePageSuffix(page));
		HEDITABLE lightEditable = HEDITABLE(EditorHelper::createLightEditable(file, lightName));
		file->getEditableManager()->addEditable(lightEditable);
		IEntity* lightEntity = lightEditable->getTargetEntity();

		size_t count = 0;
		IEditable*const* editables = file->getSelectedEditables(count);
		for (size_t i = 0; i < count; ++i)
		{
			IEditable* editable = editables[i];
			editable->getEditorElement()->showBounding(false);
		}

		file->setSelectedEditable(lightEditable);
		lightEditable->getEditorElement()->showBounding(true, COLOR_HIGHLIGHT);
		mLightEditable = lightEditable;

		mCreating = true;

		IGeometry* geom = mLightEditable->getEditorElement()->getGeomtry();
		geom->setGlobalPosition(pos);
		file->getStage()->getPagingManager()->notifyEntity(lightEntity, true, true);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightCreationTool::onMouseLeftUp(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, IEditorFile* file)
	{
		//done creating
		if (mCreating && (file->getTypeInfo()->mOperations&EFO_SAVE))
		{
			assert(mLightEditable != NULL);
			file->setModified();
		}
		if (mLightEditable != NULL)
			mLightEditable->onTransformed(file);	//notify paging change

		mCreating = false;
		mLightEditable = NULL;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightCreationTool::update(scalar /*x*/, scalar /*y*/, const POINT3& /*deltaMove*/, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* /*file*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				LightCreationTool::mainUpdate(scalar x, scalar y, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* file)
	{
		if( mCreating )
		{
			if(mLightEditable == NULL)
			{
				assert(false);
				return false;
			}

			//move creating light entity
			//TODO: or adjust light radius?
			POINT3 newPos = this->pickPosition(x, y, FULL_APPFLAG, mCreationDistance, file);
			IGeometry* geom = mLightEditable->getEditorElement()->getGeomtry();
			geom->setGlobalPosition(newPos);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		LightCreationTool::getMannaulCategory() const
	{
		return COMMON_CATEGORY;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				LightCreationTool::getSupportedFiles(TStringParam& outFiles)
	{
		BLADE_UNREFERENCED(outFiles);
		//default: support all file types
		return size_t(-1);
	}

}//namespace Blade
