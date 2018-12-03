/********************************************************************
	created:	2013/05/17
	filename: 	AxisGizmo.h
	author:		Crazii
	
	purpose:	renderable instance for Gizmo helper
				TODO: move AxisGizmo to editor framework,
				otherwise we need extra data/interface for transform operations (hit test/ray cast to determine axis)
*********************************************************************/
#ifndef __Blade_AxisGizmo_h__
#define __Blade_AxisGizmo_h__
#include <SpaceContent.h>
#include <interface/public/IRenderable.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/GraphicsEffect.h>
#include <math/Ray.h>
#include <utility/Mask.h>
#include "HelperRenderType.h"
#include <interface/ISpace.h>
#include <MaterialInstance.h>

namespace Blade
{
	class AxisGizmo;

	class AxisRenderable : public IRenderable, public Allocatable
	{
	public:
		AxisRenderable()
			:mContent(NULL)
			,mWorldTransform(NULL)
			,mMaterial(NULL)
		{

		}

		~AxisRenderable()
		{

		}

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual RenderType&				getRenderType() const
		{
			return HelperRenderType::getSingleton();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const	{return mGeometry;}

		/*
		@describe
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const	{return mMaterial[mMaterialIndex];}

		/*
		@describe
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const		{return *mWorldTransform;}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const;

	protected:
		AxisGizmo*				mContent;
		Matrix44*				mWorldTransform;
		const HMATERIALINSTANCE*mMaterial;
		GraphicsGeometry		mGeometry;
		index_t					mMaterialIndex;
		friend class AxisGizmoState;
	};//class AxisRenderable

	//internal gizmo state
	class AxisGizmoState
	{
	public:
		//instances count
		//0 - X, 1 - Y, 2 - Z
		static const size_t COLOR_COUNT = 3;
		static const Color	DISABLED_COLOR;
		static const Color	NORMAL_COLORS[COLOR_COUNT];
		static const Color	HILIGHT_COLORS[COLOR_COUNT];
		static const EAxis	AXIS[COLOR_COUNT];

		static const EAxis	AXIS_PLANE[COLOR_COUNT];
		static const Color	PLANE_NORMAL_COLORS[COLOR_COUNT];
		static const Color	PLANE_HILIGHT_COLORS[COLOR_COUNT];

		inline AxisGizmoState(EGizmoType type) :mType(type),mSpaceMode(GS_WORLD)
		{
			mTransform = Matrix44::IDENTITY;
			mInited = false;
			mAxisMaterial = NULL;
			mAxisHighLightMaterial = NULL;
		}
		inline virtual ~AxisGizmoState()	{}

		/** @brief  */
		inline EGizmoType	getType() const					{return mType;}
		/** @brief  */
		inline GEOM_SPACE	getSpaceMode() const			{return mSpaceMode;}
		/** @brief  */
		inline const Matrix44&	getWorldTransform() const	{return mTransform;}
		inline virtual void	setWorldTransform(const Vector3& pos,  const Vector3& scale, const Quaternion& rotation, const Quaternion& cameraRotation)
		{
			BLADE_UNREFERENCED(cameraRotation);
			Matrix44::generateTransform(mTransform, pos, scale, mSpaceMode == GS_WORLD ? Quaternion::IDENTITY : rotation);
		}
		/** @brief  */
		inline void			setWorldTransform(const Matrix44& xform)	{mTransform = xform;}
		/** @brief  */
		inline bool			isInited() const	{return mInited;}
		/** @brief  */
		inline bool			isEnabled() const	{return mEnabled;}
		/** @brief  */
		inline void			setEnable(bool enable)	{mEnabled = enable; this->rebindMaterial();}
		/** @brief  */
		inline void			initialize()
		{
			if( !mInited )
			{
				mInited = true;
				this->initializeImpl();
			}
		}
		/** @brief  */
		inline void			setSpaceMode(GEOM_SPACE space)
		{
			if(mSpaceMode != space )
			{
				//this->setSpaceModeImpl();
				mSpaceMode = space;
			}
		}

		/** @brief  */
		inline bool		isLoaded() const
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( mAxisMaterial[i] == NULL )
					return false;
				if( !mAxisMaterial[i]->isLoaded() )
					return false;
			}
			return true;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	rebindMaterial() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	highLightAxis(EAxis axis) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	enqueueRenderable(IRenderQueue* queue) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	initializeImpl() = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual EAxis hitTest(const Ray& ray, scalar* dist = NULL) const = 0;

	protected:
		/** @brief  */
		static inline void		initGizmoContent(AxisRenderable& renderable, AxisGizmo* gizmo)
		{
			renderable.mContent = gizmo;
		}

		/** @brief */
		static inline void		bindTransform(AxisRenderable& renderable, Matrix44* transform)
		{
			renderable.mWorldTransform = transform;
		}

		/** @brief  */
		static inline void		bindMaterial(AxisRenderable& renderable, const HMATERIALINSTANCE* material, index_t index)
		{
			renderable.mMaterial = material;
			renderable.mMaterialIndex = index;
		}

		/** @brief  */
		static inline void		bindGeometry(AxisRenderable& renderable, const GraphicsGeometry& geom)
		{
			renderable.mGeometry = geom;
		}

		//world transform
		Matrix44					mTransform;
		//material
		const HMATERIALINSTANCE*	mAxisMaterial;
		const HMATERIALINSTANCE*	mAxisHighLightMaterial;

		EGizmoType	mType;
		GEOM_SPACE	mSpaceMode;
		bool		mInited;
		bool		mEnabled;
	};


	class AxisGizmo : public SpaceContent, public Allocatable
	{
	public:
		AxisGizmo();
		~AxisGizmo();

		/************************************************************************/
		/* SpaceContent override                                                                     */
		/************************************************************************/
		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* queue)			{return mState->enqueueRenderable(queue);}

		/*
		@describe this function will be called if content is visible and CUF_VISIBLE_UPDATE is set
		@param
		@return
		*/
		virtual void			visibleUpdate(const ICamera* current, const ICamera* main, const ICamera::VISIBILITY /*visibility*/);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline const Matrix44&	getWorldTransform() const					{return mState->getWorldTransform();}

		/** @brief  */
		inline virtual void	setWorldTransform(const Vector3& pos,  const Vector3& scale, const Quaternion& rotation, const Quaternion& camearRotation)
		{
			mState->setWorldTransform(pos, scale, rotation, camearRotation);
		}

		/** @brief  */
		inline void				setTarget(ISpaceContent* target)
		{
			mTarget = target;
			if( mTarget != NULL )
				mTarget->getSpace()->addContent(this);
			else if(this->getSpace() != NULL)
				this->getSpace()->removeContent(this);
		}

		/** @brief  */
		inline ISpaceContent*	getTarget() const							{return mTarget;}

		/** @brief  */
		inline EGizmoType	getCurrentType() const {return mState->getType();}

		/** @brief  */
		inline bool		isLoaded() const	{return mState->isLoaded();}

		/** @brief  */
		inline bool		isEnabled() const	{return mState->isEnabled();}

		/** @brief  */
		inline EAxis hitTest(const Ray& ray, scalar* dist = NULL)	const 
		{
			return mState->hitTest(ray, dist);
		}

		/** @brief  */
		inline bool		highLightAxis(EAxis axis)	{return mState->highLightAxis(axis);}

		/** @brief show gizmo with type, if bshow==false, type is ignored */
		void		show(bool bShow, bool enable, EGizmoType type = GT_MOVE, GEOM_SPACE space = GS_WORLD);

	protected:
		AxisGizmoState*	mState;
		AxisGizmoState* mStates[GT_COUNT];
		ISpaceContent*	mTarget;
	};//class AxisGizmo
	
}//namespace Blade

#endif//__Blade_AxisGizmo_h__