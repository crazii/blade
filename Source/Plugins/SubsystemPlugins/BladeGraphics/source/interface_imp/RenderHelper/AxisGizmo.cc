/********************************************************************
	created:	2013/06/13
	filename: 	AxisGizmo.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "AxisGizmo.h"
#include "utility/StringHelper.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/GraphicsUtility.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <RenderUtility.h>
#include <interface/IRenderQueue.h>

#include <Material.h>
#include <Technique.h>
#include <Pass.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	static const scalar GIZMO_SIZE = 1.0f;
	const scalar AXIS_ERROR = GIZMO_SIZE*0.06f;
	const scalar AXIS_CENTER = GIZMO_SIZE*0.1f;

	//line
	static const scalar LINE_LENGTH = GIZMO_SIZE*0.8f;
	//line bound
	static const scalar BOUND_SECTION_SIZE = GIZMO_SIZE*0.04f;
	//axis plane
	static const scalar AXIS_PLANE_SIZE = GIZMO_SIZE*0.2f;
	//arrow
	static const scalar HEAD_LENGTH = GIZMO_SIZE - LINE_LENGTH;
	static const scalar HEAD_RADIUS = GIZMO_SIZE*0.04f;
	static const size_t HEAD_DENSITY = 127;
	//box
	static const scalar BOX_SIZE = GIZMO_SIZE*0.12f;
	//circle & sphere
	const size_t CIRCLE_DENSITY = 180;
	//const scalar CYLINDER_WIDTH = 0.05f;
	static const scalar ROTATE_HELPER_RADIUS = GIZMO_SIZE*0.8f;
	static const scalar INNER_RING_RADIUS = GIZMO_SIZE*0.8f;
	static const scalar OUTTER_RING_RADIUS = GIZMO_SIZE;
	const size_t SPHERE_DENSITY = 50;

	//////////////////////////////////////////////////////////////////////////
	ISpaceContent*			AxisRenderable::getSpaceContent() const
	{
		return mContent;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class AxisGizmoData : public Singleton<AxisGizmoData>
	{
	public:
		AxisGizmoData()
		{
			mInitialized = false;
			IEventManager::getSingleton().addEventHandler(RenderDeviceReadyEvent::NAME, EventDelegate(this, &AxisGizmoData::onRenderDeviceOpen) );
			IEventManager::getSingleton().addEventHandler(RenderDeviceCloseEvent::NAME, EventDelegate(this, &AxisGizmoData::onRenderDeviceClose) );
		}
		~AxisGizmoData()
		{
			IEventManager::getSingleton().removeEventHandlers(RenderDeviceReadyEvent::NAME, this);
			IEventManager::getSingleton().removeEventHandlers(RenderDeviceCloseEvent::NAME, this);
		}

		/** @brief  */
		void	onRenderDeviceOpen(const Event& /*data*/)
		{
			//TODO: create shared geometry buffers
			//re-create data
			if( mInitialized )
				this->initialize();
		}

		/** @brief  */
		void		onRenderDeviceClose(const Event& /*data*/)
		{
			for(size_t i = 0; i < AxisGizmoState::COLOR_COUNT; ++i)
			{
				mArrowHead[i].reset();
				mLine[i].reset();
				mLineBound[i].reset();
				mPlane[i].reset();
				mBox[i].reset();
				mRing[i].reset();

				mArrowBuffer[i].clear();
				mArrowIndex[i].clear();
				mLineBuffer[i].clear();
				mLineBoundBuffer[i].clear();
				mLineBoundIndex[i].clear();
				mPlaneBuffer[i].clear();
				mBoxBuffer[i].clear();
				mBoxIndex[i].clear();
				mRingBuffer[i].clear();
				mRingIndex[i].clear();

				mMaterial[i].clear();
				mHighLightMaterial[i].clear();

				mPlaneMaterial[i].clear();
				mPlaneHighLightMaterial[i].clear();

				mRingMaterial[i].clear();
				mHilightRingMaterial[i].clear();
			}
			mSphere.reset();
			mSphereBuffer.clear();
			mSphereIndex.clear();
			mSphereMaterial.clear();

			mInnerRing.reset();
			mInnerRingBuffer.clear();
			mOutterRing.reset();
			mOutterRingBuffer.clear();

			mSphereMaterial.clear();
			mInnerRingMaterial.clear();
			mOutterRingMaterial.clear();
			mHilightOutterRingMaterial.clear();

			mDisabledMaterial.clear();
		}

		/** @brief  */
		void initialize()
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			//not created?
			if( mArrowHead[0].mVertexSource != NULL )
			{
				for(size_t i = 0; i < AxisGizmoState::COLOR_COUNT; ++i)
				{
					assert( mArrowBuffer[i] != NULL);
					assert( mLineBuffer[i] != NULL);
					assert( mBoxBuffer[i] != NULL);
					assert( mRingBuffer[i] != NULL);
					assert( mMaterial[i] != NULL);
				}
				assert( mSphereBuffer != NULL );
				return;
			}

			mInitialized = true;
			IGraphicsResourceManager& softManger = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_SOFT);

			//line bound size
			Vector3 LINE_BOUND_SIZE[AxisGizmoState::COLOR_COUNT] = 
			{ 
				Vector3(LINE_LENGTH,BOUND_SECTION_SIZE,BOUND_SECTION_SIZE),
				Vector3(BOUND_SECTION_SIZE,LINE_LENGTH,BOUND_SECTION_SIZE),
				Vector3(BOUND_SECTION_SIZE,BOUND_SECTION_SIZE,LINE_LENGTH)
			};
			//plane side vector
			const Vector3 SIDE_VECTOR[AxisGizmoState::COLOR_COUNT][2] = 
			{
				Vector3::UNIT_Y,Vector3::UNIT_Z,
				Vector3::UNIT_Z,Vector3::UNIT_X,
				Vector3::UNIT_X,Vector3::UNIT_Y,
			};

			Vector3 AXIS[AxisGizmoState::COLOR_COUNT] = {Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z};
			HIBUFFER dummy;

			//create all
			for(size_t i = 0; i < AxisGizmoState::COLOR_COUNT; ++i)
			{
				RenderUtility::GEOMETRY GeomArrow(mArrowHead[i],mArrowIndex[i],mArrowBuffer[i], true);
				RenderUtility::GEOMETRY GeomTail(mLine[i],mArrowIndex[i],mLineBuffer[i], true);
				bool ret = RenderUtility::createArrow(GeomArrow, GeomTail, LINE_LENGTH, HEAD_LENGTH, HEAD_RADIUS, HEAD_DENSITY,
					Vector3::ZERO, AXIS[i]);
				assert(ret);

				//line bound for intersect
				RenderUtility::GEOMETRY GeomBound(mLineBound[i], mLineBoundIndex[i], mLineBoundBuffer[i], true, softManger);
				ret = RenderUtility::createAABox(GeomBound, LINE_BOUND_SIZE[i].x, LINE_BOUND_SIZE[i].y, LINE_BOUND_SIZE[i].z, 
					RenderUtility::BE_POSITION, RenderUtility::FF_OUTSIDE, AXIS[i]*LINE_LENGTH*0.5);
				assert(ret);

				//axis plane
				RenderUtility::GEOMETRY GeomPlane(mPlane[i], dummy, mPlaneBuffer[i], true);
				ret = RenderUtility::createPlane(GeomPlane, AXIS_PLANE_SIZE, AXIS_PLANE_SIZE, RenderUtility::BE_POSITION, 
					RenderUtility::FF_OUTSIDE, Vector3::ZERO, SIDE_VECTOR[i][0], SIDE_VECTOR[i][1]);
				assert(ret);

				//shared line created, create scale box
				RenderUtility::GEOMETRY GeomBox(mBox[i], mBoxIndex[i], mBoxBuffer[i], true);
				ret = RenderUtility::createAABox(GeomBox, BOX_SIZE, BOX_SIZE, BOX_SIZE, RenderUtility::BE_POSITION, RenderUtility::FF_OUTSIDE, AXIS[i]*LINE_LENGTH);
				assert(ret);

				//create rotate axis circle
				RenderUtility::GEOMETRY GeomRing(mRing[i], mRingIndex[i], mRingBuffer[i]);
				ret = RenderUtility::createRing(GeomRing, ROTATE_HELPER_RADIUS, CIRCLE_DENSITY, Vector3::ZERO, AXIS[i]);
				assert(ret);

				//create material instance
				Material* mat = IMaterialManager::getSingleton().getMaterial(BTString("AxisGizmo"));
				assert(mat != NULL);
				mMaterial[i].bind( BLADE_NEW MaterialInstance(mat) );
				mMaterial[i]->enableColor(true, AxisGizmoState::NORMAL_COLORS[i]);
				mHighLightMaterial[i] = mMaterial[i]->clone(MIS_NONE);
				mHighLightMaterial[i]->enableColor(true, AxisGizmoState::HILIGHT_COLORS[i]);

				//const TString name = mat->getName()+BTString("_cloned_CULL_NONE") + TStringHelper::fromInt(i);
				//Material* planeMaterial = mat->clone(name, MIS_ALL&(~MS_PROPERTY) );
				//planeMaterial->getActiveTechnique()->getPass(0)->getRenderProperty()->setCullMode(HCM_NONE);

				mPlaneMaterial[i].bind( BLADE_NEW MaterialInstance( mat ) );
				mPlaneMaterial[i]->enableColor(true, AxisGizmoState::PLANE_NORMAL_COLORS[i]);
				mPlaneHighLightMaterial[i] = mPlaneMaterial[i]->clone(MIS_NONE);
				mPlaneHighLightMaterial[i]->enableColor(true, AxisGizmoState::PLANE_HILIGHT_COLORS[i]);

				mRingMaterial[i].bind( BLADE_NEW MaterialInstance( mat ) );
				mRingMaterial[i]->enableColor(true, AxisGizmoState::NORMAL_COLORS[i]);
				mRingMaterial[i]->setShaderGroup(BTString("ring_circle"));
				mHilightRingMaterial[i].bind( BLADE_NEW MaterialInstance( mat ) );
				mHilightRingMaterial[i]->enableColor(true, AxisGizmoState::HILIGHT_COLORS[i]);
				mHilightRingMaterial[i]->setShaderGroup(BTString("ring_circle"));

				mMaterial[i]->load();
				mHighLightMaterial[i]->load();

				mPlaneMaterial[i]->load();
				mPlaneHighLightMaterial[i]->load();

				mRingMaterial[i]->load();
				mHilightRingMaterial[i]->load();
			}
			//create rotate sphere
			RenderUtility::GEOMETRY GeomShpere(mSphere, mSphereIndex, mSphereBuffer, true);
			bool ret = false;
			ret = RenderUtility::createSphere(GeomShpere, ROTATE_HELPER_RADIUS, SPHERE_DENSITY);
			assert(ret);
			Material* mat = IMaterialManager::getSingleton().getMaterial(BTString("AxisGizmo"));
			mSphereMaterial.bind( BLADE_NEW MaterialInstance(mat) );
			mSphereMaterial->enableColor(true, Color((scalar)0.8,(scalar)0.8,(scalar)0.8,(scalar)0.2) );
			mSphereMaterial->load();

			RenderUtility::GEOMETRY GeomInnerRing(mInnerRing, dummy, mInnerRingBuffer);
			ret = RenderUtility::createRing(GeomInnerRing, INNER_RING_RADIUS, CIRCLE_DENSITY, Vector3::ZERO, Vector3::UNIT_Z);
			assert(ret);

			RenderUtility::GEOMETRY GeomOutterRing(mOutterRing, dummy, mOutterRingBuffer);
			ret = RenderUtility::createRing(GeomOutterRing, OUTTER_RING_RADIUS, CIRCLE_DENSITY, Vector3::ZERO, Vector3::UNIT_Z);
			assert(ret);

			mInnerRingMaterial.bind( BLADE_NEW MaterialInstance(mat) );
			mInnerRingMaterial->enableColor(true, Color((scalar)0.9,(scalar)0.9,(scalar)0.9,(scalar)0.8));
			mInnerRingMaterial->load();

			mOutterRingMaterial.bind( BLADE_NEW MaterialInstance(mat) );
			mOutterRingMaterial->enableColor(true, Color::GRAY);
			mOutterRingMaterial->load();

			mHilightOutterRingMaterial.bind( BLADE_NEW MaterialInstance(mat) );
			mHilightOutterRingMaterial->enableColor(true, Color::WHITE);
			mHilightOutterRingMaterial->load();

			mDisabledMaterial.bind( BLADE_NEW MaterialInstance(mat) );
			mDisabledMaterial->enableColor(true, AxisGizmoState::DISABLED_COLOR);
			mDisabledMaterial->load();
		}

		/** @brief  */
		const GraphicsGeometry*	getArrowHead()
		{
			 this->initialize();
			 return mArrowHead;
		}
		
		/** @brief  */
		const GraphicsGeometry* getLine()
		{
			this->initialize();
			return mLine;
		}

		/** @brief  */
		const GraphicsGeometry* getLineBound()
		{
			this->initialize();
			return mLineBound;
		}

		/** @brief  */
		const GraphicsGeometry* getAxisPlane()
		{
			this->initialize();
			return mPlane;
		}

		/** @brief  */
		const GraphicsGeometry* getBox()
		{
			this->initialize();
			return mBox;
		}

		/** @brief  */
		const GraphicsGeometry* getRing()
		{
			this->initialize();
			return mRing;
		}

		/** @brief  */
		const GraphicsGeometry& getSphere()
		{
			this->initialize();
			return mSphere;
		}

		/** @brief  */
		const HMATERIALINSTANCE& getSphereMaterial()
		{
			this->initialize();
			return mSphereMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE* getMaterial()
		{
			this->initialize();
			return mMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE* getHighLightMaterial()
		{
			this->initialize();
			return mHighLightMaterial;
		}
		
		/** @brief  */
		const HMATERIALINSTANCE* getPlaneMaterial()
		{
			this->initialize();
			return mPlaneMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE* getPlaneHighLightMaterial()
		{
			this->initialize();
			return mPlaneHighLightMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE* getRingMaterial()
		{
			this->initialize();
			return mRingMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE* getRingHighLightMaterial()
		{
			this->initialize();
			return mHilightRingMaterial;
		}

		/** @brief  */
		const GraphicsGeometry&	getInnerRing()
		{
			this->initialize();
			return mInnerRing;
		}

		/** @brief  */
		const HMATERIALINSTANCE& getInnerRingMaterial()
		{
			this->initialize();
			return mInnerRingMaterial;
		}

		/** @brief  */
		const GraphicsGeometry&	getOutterRing()
		{
			this->initialize();
			return mOutterRing;
		}

		/** @brief  */
		const HMATERIALINSTANCE& getOutterRingMaterial()
		{
			this->initialize();
			return mOutterRingMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE& getHilightOutterRingMaterial()
		{
			this->initialize();
			return mHilightOutterRingMaterial;
		}

		/** @brief  */
		const HMATERIALINSTANCE& getDisabledmaterial()
		{
			this->initialize();
			return mDisabledMaterial;
		}

	protected:
		GraphicsGeometry	mArrowHead[AxisGizmoState::COLOR_COUNT];
		GraphicsGeometry	mLine[AxisGizmoState::COLOR_COUNT];	//shared by translate/scale
		GraphicsGeometry	mLineBound[AxisGizmoState::COLOR_COUNT];
		GraphicsGeometry	mPlane[AxisGizmoState::COLOR_COUNT];
		GraphicsGeometry	mBox[AxisGizmoState::COLOR_COUNT];
		GraphicsGeometry	mRing[AxisGizmoState::COLOR_COUNT];
		GraphicsGeometry	mInnerRing;
		GraphicsGeometry	mOutterRing;
		GraphicsGeometry	mSphere;

		HVERTEXSOURCE		mArrowBuffer[AxisGizmoState::COLOR_COUNT];
		HIBUFFER			mArrowIndex[AxisGizmoState::COLOR_COUNT];

		HVERTEXSOURCE		mLineBuffer[AxisGizmoState::COLOR_COUNT];

		HVERTEXSOURCE		mLineBoundBuffer[AxisGizmoState::COLOR_COUNT];
		HIBUFFER			mLineBoundIndex[AxisGizmoState::COLOR_COUNT];

		HVERTEXSOURCE		mPlaneBuffer[AxisGizmoState::COLOR_COUNT];

		HVERTEXSOURCE		mBoxBuffer[AxisGizmoState::COLOR_COUNT];
		HIBUFFER			mBoxIndex[AxisGizmoState::COLOR_COUNT];

		HVERTEXSOURCE		mRingBuffer[AxisGizmoState::COLOR_COUNT];
		HIBUFFER			mRingIndex[AxisGizmoState::COLOR_COUNT];

		HVERTEXSOURCE		mInnerRingBuffer;
		HVERTEXSOURCE		mOutterRingBuffer;

		HVERTEXSOURCE		mSphereBuffer;
		HIBUFFER			mSphereIndex;

		HMATERIALINSTANCE	mMaterial[AxisGizmoState::COLOR_COUNT];
		HMATERIALINSTANCE	mHighLightMaterial[AxisGizmoState::COLOR_COUNT];

		HMATERIALINSTANCE	mPlaneMaterial[AxisGizmoState::COLOR_COUNT];
		HMATERIALINSTANCE	mPlaneHighLightMaterial[AxisGizmoState::COLOR_COUNT];

		HMATERIALINSTANCE	mRingMaterial[AxisGizmoState::COLOR_COUNT];
		HMATERIALINSTANCE	mHilightRingMaterial[AxisGizmoState::COLOR_COUNT];

		HMATERIALINSTANCE	mSphereMaterial;
		HMATERIALINSTANCE	mInnerRingMaterial;
		HMATERIALINSTANCE	mOutterRingMaterial;
		HMATERIALINSTANCE	mHilightOutterRingMaterial;

		HMATERIALINSTANCE	mDisabledMaterial;

		bool				mInitialized;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static const scalar ALPHA = (scalar)0.8;
	static const scalar INTENSITY = (scalar)0.6;//normal state intensity
	const Color AxisGizmoState::DISABLED_COLOR = Color((scalar)0.2, (scalar)0.2, (scalar)0.2, ALPHA);
	const Color AxisGizmoState::NORMAL_COLORS[COLOR_COUNT] = {Color(INTENSITY, 0, 0, ALPHA), Color(0, INTENSITY, 0, ALPHA), Color(0, 0, INTENSITY, ALPHA)};
	const Color AxisGizmoState::HILIGHT_COLORS[COLOR_COUNT] = {Color(1, 0.2f, 0.2f, ALPHA), Color(0.2f, 1, 0.2f, ALPHA), Color(0.2f, (scalar)0.2, 1, ALPHA)};
	const EAxis	AxisGizmoState::AXIS[COLOR_COUNT] = {GA_X, GA_Y, GA_Z,};

	const EAxis	AxisGizmoState::AXIS_PLANE[COLOR_COUNT] = {GA_YZ, GA_XZ, GA_XY,};
	const Color	AxisGizmoState::PLANE_NORMAL_COLORS[COLOR_COUNT] = { Color(0, INTENSITY, INTENSITY, ALPHA), Color(INTENSITY, 0, INTENSITY, (scalar)0.8), Color(INTENSITY, INTENSITY, 0, ALPHA) };
	const Color	AxisGizmoState::PLANE_HILIGHT_COLORS[COLOR_COUNT] = { Color(0, 1, 1, ALPHA), Color(1, 0, 1, ALPHA), Color(1, 1, 0, ALPHA) };

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class MoveGizmoState : public AxisGizmoState, public Allocatable
	{
	public:
		MoveGizmoState(AxisGizmo* gizmo)
			:AxisGizmoState(GT_MOVE)
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				mArrowHead[i] = BLADE_NEW AxisRenderable();
				mArrowTail[i] = BLADE_NEW AxisRenderable();
				mAxisPlane[i] = BLADE_NEW AxisRenderable();

				AxisGizmoState::initGizmoContent(*mArrowHead[i], gizmo);
				AxisGizmoState::initGizmoContent(*mArrowTail[i], gizmo);
				AxisGizmoState::initGizmoContent(*mAxisPlane[i], gizmo);
			}
			mAxisPlaneMaterial = NULL;
			mAxisPlaneHighLightMaterial = NULL;
		}
		~MoveGizmoState()
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				BLADE_DELETE mArrowHead[i];
				BLADE_DELETE mArrowTail[i];
				BLADE_DELETE mAxisPlane[i];
			}
		}		/** @brief  */
		virtual void	rebindMaterial()
		{
			if( mEnabled )
			{
				mAxisMaterial = AxisGizmoData::getSingleton().getMaterial();
				mAxisPlaneMaterial = AxisGizmoData::getSingleton().getPlaneMaterial();

				for(size_t i = 0; i < COLOR_COUNT; ++i)
				{
					this->bindMaterial( *mArrowHead[i], mAxisMaterial, i);
					this->bindMaterial( *mArrowTail[i], mAxisMaterial, i);
					this->bindMaterial( *mAxisPlane[i], mAxisPlaneMaterial, i);
				}
			}
			else
			{
				mAxisMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				mAxisPlaneMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				for(size_t i = 0; i < COLOR_COUNT; ++i)
				{
					this->bindMaterial( *mArrowHead[i], mAxisMaterial, 0);
					this->bindMaterial( *mArrowTail[i], mAxisMaterial, 0);
					this->bindMaterial( *mAxisPlane[i], mAxisPlaneMaterial, 0);
				}
			}
		}

		/** @brief  */
		virtual bool		highLightAxis(EAxis axis)
		{
			if( !mEnabled )
				return false;

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( mAxisMaterial[i] == NULL || mAxisHighLightMaterial == NULL )
					return false;
				if( (axis&AXIS[i]) )
				{
					this->bindMaterial(*mArrowHead[i], mAxisHighLightMaterial, i);
					this->bindMaterial(*mArrowTail[i], mAxisHighLightMaterial, i);
				}
				else
				{
					this->bindMaterial(*mArrowHead[i], mAxisMaterial, i);
					this->bindMaterial(*mArrowTail[i], mAxisMaterial, i);
				}
			}

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( mAxisPlaneMaterial == NULL || mAxisPlaneHighLightMaterial == NULL)
					return false;

				if(axis == AXIS_PLANE[i])
					this->bindMaterial(*mAxisPlane[i], mAxisPlaneHighLightMaterial, i);
				else
					this->bindMaterial(*mAxisPlane[i], mAxisPlaneMaterial, i);
			}
			return true;
		}

		/** @brief  */
		virtual void	enqueueRenderable(IRenderQueue* queue)
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				queue->addRenderable(mArrowHead[i]);
				queue->addRenderable(mArrowTail[i]);
				queue->addRenderable(mAxisPlane[i]);
			}			
		}

		/** @brief  */
		virtual void	initializeImpl()
		{
			mAxisHighLightMaterial = AxisGizmoData::getSingleton().getHighLightMaterial();
			mAxisPlaneHighLightMaterial = AxisGizmoData::getSingleton().getPlaneHighLightMaterial();
			this->rebindMaterial();

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				this->bindGeometry( *mArrowHead[i], AxisGizmoData::getSingleton().getArrowHead()[i] );
				this->bindGeometry( *mArrowTail[i], AxisGizmoData::getSingleton().getLine()[i] );
				this->bindGeometry( *mAxisPlane[i], AxisGizmoData::getSingleton().getAxisPlane()[i]);

				this->bindTransform( *mArrowHead[i], &mTransform );
				this->bindTransform( *mArrowTail[i], &mTransform );
				this->bindTransform( *mAxisPlane[i], &mTransform );
			}
		}

		/** @brief  */
		virtual EAxis hitTest(const Ray& ray, scalar* dist/* = NULL*/) const
		{
			EAxis axis = GA_NONE;
			Ray localRay = ray*mTransform.getInverse();
			scalar fDist = FLT_MAX;
			if( dist != NULL )
				fDist = *dist;
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( GraphicsUtility::rayIntersectTriangles(localRay, mArrowHead[i]->getGeometry(), fDist , PS_BOTH) )
					axis = AXIS[i];
			}

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( GraphicsUtility::rayIntersectTriangles(localRay, AxisGizmoData::getSingleton().getLineBound()[i], fDist , PS_BOTH) )
					axis = AXIS[i];
			}

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( GraphicsUtility::rayIntersectTriangles(localRay, mAxisPlane[i]->getGeometry(), fDist , PS_BOTH) )
					axis = AXIS_PLANE[i];
			}

			if( fDist != FLT_MAX )
			{
				POINT3 p = localRay.getRayPoint(fDist);
				if( Math::Equal(p.x, 0.0f, AXIS_CENTER) && Math::Equal(p.y, 0.0f, AXIS_CENTER) && Math::Equal(p.z, 0.0f, AXIS_CENTER) )
					axis = GA_XYZ;
			}

			if( dist != NULL )
				*dist = fDist;
			return axis;
		}
		
	protected:
		AxisRenderable*		mArrowHead[COLOR_COUNT];
		AxisRenderable*		mArrowTail[COLOR_COUNT];
		AxisRenderable*		mAxisPlane[COLOR_COUNT];

		const HMATERIALINSTANCE*	mAxisPlaneMaterial;
		const HMATERIALINSTANCE*	mAxisPlaneHighLightMaterial;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ScaleGizmoState : public AxisGizmoState, public Allocatable
	{
	public:
		ScaleGizmoState(AxisGizmo* gizmo)
			:AxisGizmoState(GT_SCALE)
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				mScaleBox[i] = BLADE_NEW AxisRenderable();
				mScaleLine[i] = BLADE_NEW AxisRenderable();
				mAxisPlane[i] = BLADE_NEW AxisRenderable();

				AxisGizmoState::initGizmoContent(*mScaleBox[i], gizmo);
				AxisGizmoState::initGizmoContent(*mScaleLine[i], gizmo);
				AxisGizmoState::initGizmoContent(*mAxisPlane[i], gizmo);
			}
			mAxisPlaneMaterial = NULL;
			mAxisPlaneHighLightMaterial = NULL;
		}
		~ScaleGizmoState()
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				BLADE_DELETE mScaleBox[i];
				BLADE_DELETE mScaleLine[i];
				BLADE_DELETE mAxisPlane[i];
			}
		}
		/** @brief  */
		virtual void	rebindMaterial()
		{
			if( mEnabled )
			{
				mAxisMaterial = AxisGizmoData::getSingleton().getMaterial();
				mAxisPlaneMaterial = AxisGizmoData::getSingleton().getPlaneMaterial();

				for(size_t i = 0; i < COLOR_COUNT; ++i)
				{
					this->bindMaterial( *mScaleBox[i], mAxisMaterial, i);
					this->bindMaterial( *mScaleLine[i], mAxisMaterial, i);
					this->bindMaterial( *mAxisPlane[i], mAxisPlaneMaterial, i);
				}
			}
			else
			{
				mAxisMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				mAxisPlaneMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				for(size_t i = 0; i < COLOR_COUNT; ++i)
				{
					this->bindMaterial( *mScaleBox[i], mAxisMaterial, 0);
					this->bindMaterial( *mScaleLine[i], mAxisMaterial, 0);
					this->bindMaterial( *mAxisPlane[i], mAxisPlaneMaterial, 0);
				}
			}
		}

		/** @brief  */
		virtual bool		highLightAxis(EAxis axis)
		{
			if( !mEnabled )
				return false;

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( mAxisMaterial[i] == NULL || mAxisHighLightMaterial == NULL )
					return false;
				if( (axis&AXIS[i]) )
				{
					this->bindMaterial(*mScaleBox[i], mAxisHighLightMaterial, i);
					this->bindMaterial(*mScaleLine[i], mAxisHighLightMaterial, i);
				}
				else
				{
					this->bindMaterial(*mScaleBox[i], mAxisMaterial, i);
					this->bindMaterial(*mScaleLine[i], mAxisMaterial, i);
				}
			}

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( mAxisPlaneMaterial == NULL || mAxisPlaneHighLightMaterial == NULL)
					return false;

				if(axis == AXIS_PLANE[i])
					this->bindMaterial(*mAxisPlane[i], mAxisPlaneHighLightMaterial, i);
				else
					this->bindMaterial(*mAxisPlane[i], mAxisPlaneMaterial, i);
			}
			return true;
		}

		/** @brief  */
		virtual void	enqueueRenderable(IRenderQueue* queue)
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				queue->addRenderable(mScaleBox[i]);
				queue->addRenderable(mScaleLine[i]);
				queue->addRenderable(mAxisPlane[i]);
			}				
		}

		/** @brief  */
		virtual void	initializeImpl()
		{
			mAxisHighLightMaterial = AxisGizmoData::getSingleton().getHighLightMaterial();
			mAxisPlaneHighLightMaterial = AxisGizmoData::getSingleton().getPlaneHighLightMaterial();
			this->rebindMaterial();

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				this->bindGeometry( *mScaleBox[i], AxisGizmoData::getSingleton().getBox()[i] );
				this->bindGeometry( *mScaleLine[i], AxisGizmoData::getSingleton().getLine()[i] );
				this->bindGeometry( *mAxisPlane[i], AxisGizmoData::getSingleton().getAxisPlane()[i]);

				this->bindTransform( *mScaleBox[i], &mTransform );
				this->bindTransform( *mScaleLine[i], &mTransform );
				this->bindTransform( *mAxisPlane[i], &mTransform );
			}
		}

		/** @brief  */
		virtual EAxis hitTest(const Ray& ray, scalar* dist/* = NULL*/) const
		{
			EAxis axis = GA_NONE;
			Ray localRay = ray*mTransform.getInverse();

			scalar fDist = FLT_MAX;
			if( dist != NULL )
				fDist = *dist;
			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( GraphicsUtility::rayIntersectTriangles(localRay, mScaleBox[i]->getGeometry(), fDist, PS_BOTH) )
					axis = AXIS[i];
			}

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( GraphicsUtility::rayIntersectTriangles(localRay, AxisGizmoData::getSingleton().getLineBound()[i], fDist, PS_BOTH) )
					axis = AXIS[i];
			}

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( GraphicsUtility::rayIntersectTriangles(localRay, mAxisPlane[i]->getGeometry(), fDist, PS_BOTH) )
					axis = AXIS_PLANE[i];
			}

			if( fDist != FLT_MAX )
			{
				POINT3 p = localRay.getRayPoint(fDist);
				if( Math::Equal(p.x, 0.0f, AXIS_CENTER) && Math::Equal(p.y, 0.0f, AXIS_CENTER) && Math::Equal(p.z, 0.0f, AXIS_CENTER) )
					axis = GA_XYZ;
			}

			if( dist != NULL )
				*dist = fDist;
			return axis;
		}

	protected:
		AxisRenderable*		mScaleBox[COLOR_COUNT];
		AxisRenderable*		mScaleLine[COLOR_COUNT];
		AxisRenderable*		mAxisPlane[COLOR_COUNT];
		const HMATERIALINSTANCE*	mAxisPlaneMaterial;
		const HMATERIALINSTANCE*	mAxisPlaneHighLightMaterial;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class RotateGizmoState : public AxisGizmoState, public Allocatable
	{
	public:
		RotateGizmoState(AxisGizmo* gizmo)
			:AxisGizmoState(GT_ROTATE)
			,mShowSphere(false)
		{
			for (size_t i = 0; i < COLOR_COUNT; ++i)
			{
				mRing[i] = BLADE_NEW AxisRenderable();
				AxisGizmoState::initGizmoContent(*mRing[i], gizmo);
			}
			mSphere = BLADE_NEW AxisRenderable();
			mInnerRing = BLADE_NEW AxisRenderable();
			mOutterRing = BLADE_NEW AxisRenderable();

			AxisGizmoState::initGizmoContent(*mSphere, gizmo);
			AxisGizmoState::initGizmoContent(*mInnerRing, gizmo);
			AxisGizmoState::initGizmoContent(*mOutterRing, gizmo);
		}

		~RotateGizmoState()
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
				BLADE_DELETE mRing[i];
			BLADE_DELETE mSphere;
			BLADE_DELETE mInnerRing;
			BLADE_DELETE mOutterRing;
		}

		/** @brief  */
		virtual void	rebindMaterial()
		{
			const HMATERIALINSTANCE* innerRingMaterial;
			const HMATERIALINSTANCE* outterRingMaterial;
			const HMATERIALINSTANCE* sphereMaterial;
			if( mEnabled )
			{
				mAxisMaterial = AxisGizmoData::getSingleton().getRingMaterial();
				innerRingMaterial = &AxisGizmoData::getSingleton().getInnerRingMaterial();
				outterRingMaterial = &AxisGizmoData::getSingleton().getOutterRingMaterial();
				sphereMaterial = &AxisGizmoData::getSingleton().getSphereMaterial();
				for(size_t i = 0; i < COLOR_COUNT; ++i)
					this->bindMaterial( *mRing[i], mAxisMaterial, i);
			}
			else
			{
				mAxisMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				innerRingMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				outterRingMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				sphereMaterial = &AxisGizmoData::getSingleton().getDisabledmaterial();
				for(size_t i = 0; i < COLOR_COUNT; ++i)
					this->bindMaterial( *mRing[i], mAxisMaterial, 0);
			}
			this->bindMaterial(*mInnerRing, innerRingMaterial, 0);
			this->bindMaterial(*mOutterRing, outterRingMaterial, 0);
			this->bindMaterial(*mSphere, sphereMaterial, 0);
		}

		/** @brief  */
		virtual bool		highLightAxis(EAxis axis)
		{
			if( !mEnabled )
				return false;

			if(axis == GA_VIEWZ )
				this->bindMaterial(*mOutterRing, &AxisGizmoData::getSingleton().getHilightOutterRingMaterial(), 0);
			else
				this->bindMaterial(*mOutterRing, &AxisGizmoData::getSingleton().getOutterRingMaterial(), 0);

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				if( mAxisMaterial[i] == NULL || mAxisHighLightMaterial == NULL )
					return false;
				if( (axis&AXIS[i]) )
					this->bindMaterial(*mRing[i], mAxisHighLightMaterial, i);
				else
					this->bindMaterial(*mRing[i], mAxisMaterial, i);
			}

			mShowSphere = (axis == GA_VIEWXY);
			if( mShowSphere == false)
			{
				for(size_t i = 0; i < COLOR_COUNT; ++i)
				{
					if(axis == AXIS_PLANE[i])
					{
						mShowSphere = true;
						break;
					}
				}
			}
			return true;
		}

		/** @brief  */
		virtual void	enqueueRenderable(IRenderQueue* queue)
		{
			for(size_t i = 0; i < COLOR_COUNT; ++i)
				queue->addRenderable(mRing[i]);
			queue->addRenderable(mInnerRing);
			queue->addRenderable(mOutterRing);

			if( mShowSphere )
				queue->addRenderable(mSphere);
		}

		/** @brief  */
		virtual void	initializeImpl()
		{
			mAxisHighLightMaterial = AxisGizmoData::getSingleton().getRingHighLightMaterial();
			this->rebindMaterial();

			for(size_t i = 0; i < COLOR_COUNT; ++i)
			{
				this->bindGeometry( *mRing[i], AxisGizmoData::getSingleton().getRing()[i] );
				this->bindTransform( *mRing[i], &mTransform );
			}

			this->bindGeometry(*mInnerRing, AxisGizmoData::getSingleton().getInnerRing() );
			this->bindTransform(*mInnerRing, &mViewSpaceRingTransform);
			this->bindGeometry(*mOutterRing, AxisGizmoData::getSingleton().getOutterRing() );
			this->bindTransform(*mOutterRing, &mViewSpaceRingTransform);

			this->bindGeometry(*mSphere, AxisGizmoData::getSingleton().getSphere() );
			this->bindTransform(*mSphere, &mTransform);
		}

		/** @brief  */
		virtual EAxis hitTest(const Ray& ray, scalar* dist/* = NULL*/) const
		{
			int axis = GA_NONE;
			Ray localRay = ray*mTransform.getInverse();

			scalar fDist = FLT_MAX;
			if( dist != NULL )
				fDist = *dist;
			const scalar ZERO = 0.0f;
			if( GraphicsUtility::rayIntersectTriangles(localRay, mSphere->getGeometry(), fDist, PS_BOTH) )
			{
				Vector3 point = localRay.getRayPoint(fDist);
				if( Math::Equal(point.x, ZERO, AXIS_ERROR) )
					axis |= GA_X;
				else if( Math::Equal(point.y, ZERO, AXIS_ERROR) )
					axis |= GA_Y;
				else if( Math::Equal(point.z, ZERO, AXIS_ERROR) )
					axis |= GA_Z;
			}

			if(fDist != FLT_MAX)
			{
				if( axis == GA_NONE )
					axis = GA_VIEWXY;
			}
			else
			{
				//view circle intersection check
				Plane p(Vector3::UNIT_Z,0);
				Ray viewSpaceRay = ray*mViewSpaceRingTransform.getInverse();
				if( viewSpaceRay.intersect(p, fDist) )
				{
					Vector3 point = localRay.getRayPoint(fDist);
					scalar fDist2Center = point.getDistance(Vector3::ZERO);
					if( Math::Equal(fDist2Center, GIZMO_SIZE, AXIS_ERROR) )
						axis = GA_VIEWZ;
				}
			}

			if( dist != NULL )
				*dist = fDist;
			return (EAxis)axis;
		}

		virtual void	setWorldTransform(const Vector3& pos,  const Vector3& scale, const Quaternion& rotation, const Quaternion& cameraRotation)
		{
			AxisGizmoState::setWorldTransform(pos, scale, rotation, cameraRotation);
			//TODO: update view space ring transform

			////note: the rings are created to face the Vector3::UNIT_Z by default (vertices are in view space already)
			////now just apply the rotation = inverse(viewMatrix33) = camera rotation, to make world*view == identity
			Matrix44::generateTransform(mViewSpaceRingTransform, pos, scale, cameraRotation);
		}

	protected:
		//use thin cylinder as ring to enable back face culling
		AxisRenderable*		mRing[COLOR_COUNT];
		AxisRenderable*		mSphere;

		//view space ring
		AxisRenderable*		mInnerRing;
		AxisRenderable*		mOutterRing;

		Matrix44			mViewSpaceRingTransform;
		bool				mShowSphere;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	AxisGizmo::AxisGizmo()
	{
		mSpaceFlags = CSF_INDEPENDENT;
		mUpdateFlags = CUF_VISIBLE_UPDATE | CUF_DEFAULT_VISIBLE;
		mWorldAABB.setInfinite();
		mLocalAABB.setInfinite();

		mStates[GT_MOVE] = BLADE_NEW MoveGizmoState(this);
		mStates[GT_SCALE] = BLADE_NEW ScaleGizmoState(this);
		mStates[GT_ROTATE] = BLADE_NEW RotateGizmoState(this);
		mState = mStates[0];
		mTarget = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	AxisGizmo::~AxisGizmo()
	{
		for(int i = 0; i < GT_COUNT; ++i)
			BLADE_DELETE mStates[i];
	}

	//////////////////////////////////////////////////////////////////////////
	void		AxisGizmo::visibleUpdate(const ICamera* current, const ICamera* /*main*/, const ICamera::VISIBILITY /*visibility*/)
	{
		//update gizmo transform

		//note: keep a constant distance form camera so that gizmo size looks unchanged
		//static const scalar DISTANCE = 8;
		//Vector3 pos = mTarget->getPosition();
		//pos = cameraPos + (pos - cameraPos).getNormalizedVector()*DISTANCE;
		//const Vector3& scale = Vector3::UNIT_ALL;

		//now we use scale to make hit test can get the exact hit point
		static const scalar PROJECTED_SIZE = 0.125;
		const Vector3& pos = mTarget->getPosition();
		float dist = current->getEyePosition().getDistance(pos);
		//WorldRadius*GIZMO_SIZE/Distance = ProjectedSize / 1 (project plane is at 1.0)
		scalar worldRadius = PROJECTED_SIZE*dist/GIZMO_SIZE;
		Vector3 scale = Vector3::UNIT_ALL*worldRadius;

		mState->setWorldTransform(pos, scale, mTarget->getRotation(), current->getOrientation());
	}

	//////////////////////////////////////////////////////////////////////////
	void		AxisGizmo::show(bool bShow, bool enable, EGizmoType type, GEOM_SPACE space/* = GS_WORLD*/)
	{
		this->setVisible(bShow);

		if( !bShow )
		{
			if( mState->getType() != type )
				mState = mStates[type];
			mState->setEnable(enable);
			return;
		}

		assert( type >= GT_BEGIN && type < GT_COUNT );
		if( !(type >= GT_BEGIN && type < GT_COUNT) )
			return;

		if( mState->getType() != type )
			mState = mStates[type];

		//
		mState->setEnable(enable);

		if( mState->getSpaceMode() != space )
			mState->setSpaceMode(space);

		if(!mState->isInited() )
			mState->initialize();
	}

}//namespace Blade