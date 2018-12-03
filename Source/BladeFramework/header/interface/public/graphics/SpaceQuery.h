/********************************************************************
	created:	2011/05/19
	filename: 	SpaceQuery.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SpaceQuery_h__
#define __Blade_SpaceQuery_h__
#include <math/BladeMath.h>
#include <math/Ray.h>
#include <math/AxisAlignedBox.h>
#include <math/Sphere.h>
#include <math/Box2.h>
#include <math/Vector4.h>
#include <utility/TList.h>
#include <BladeFramework.h>
#include <Pimpl.h>
#include <StaticHandle.h>
#include "GraphicsDefs.h"


namespace Blade
{
	namespace Impl
	{
		class QueryResultImpl;
	}//namespace Impl

	class IElement;
	class GraphicsGeometry;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class SpaceQuery : public NonAssignable, public TempAllocatable
	{
	public:
		enum ETargetFilter
		{
			TF_3D	=	0x01,		//only query 3d objects
			TF_2D	=	0x02,		//only query 2d objects
		};
		//add query type enum for implementations?

		typedef struct SResult
		{
			SResult(IElement* elem, scalar dist) :element(elem),distance(dist) {}
			IElement*	element;
			scalar		distance;
			/** @brief  */
			inline bool operator<(const SResult& rhs) const { return distance < rhs.distance; }
			inline bool operator==(const SResult& rhs) const {return element == rhs.element;}
		}RESULT;

		typedef TList<RESULT, TempAllocator<RESULT>, TempAllocatable> ResultList;

		class ITriangleBuffer
		{
		public:
			virtual void	add(const POINT3& p0, const POINT3& p1, const POINT3& p2) = 0;
		};

		class ICallback
		{
		public:
			/** @brief return value indicates whether to continue querying */
			virtual bool	onQuery(IElement* element, scalar distance, size_t total) = 0;
			/** @brief return value indicates element should be added to result or not */
			virtual bool	validate(IElement* element, scalar distance) = 0;
		};

	public:
		SpaceQuery(	scalar distLimit = FLT_MAX, AppFlag appflag = FULL_APPFLAG, ITriangleBuffer* triangles = NULL, ICallback* cb = NULL)
			:mLimit(distLimit)
			,mFlag(appflag)
			,mTriangles(triangles)
			,mCallback(cb)
			,mDirection(Vector3::UNIT_ALL)
			,mFilter(TF_3D)
			,mPrecise(false)
			,mNearestOnly(false) {}

		virtual ~SpaceQuery() {}

		/** @brief  */
		BLADE_FRAMEWORK_API void		sort();

		/* @note the result data is temporary, if you need to store the result, copy the out from this object
		@brief  */
		void				push_back(scalar distance, IElement* elem)
		{
			if(mResult == NULL)
				mResult.constructInstance<ResultList>();
			mResult->push_back( RESULT(elem, distance) );
		}

		/** @brief  */
		size_t				size() const {return mResult != NULL ? mResult->size() : 0;}

		/** @brief  */
		RESULT*				getResult(index_t index) const
		{
			return mResult != NULL && index < mResult->size() ? &((*mResult)[index]) : NULL;
		}

		/** @brief  */
		void				clear()
		{
			if(mResult != NULL)
				mResult->clear();
		}

		/** @brief  */
		index_t				find(IElement* element) const
		{
			return mResult != NULL ? mResult->find(RESULT(element, 0)) : INVALID_INDEX;
		}

		/** @brief  */
		inline RESULT& operator[](size_t index) const
		{
			return *this->getResult(index);
		}

		/** @brief  */
		inline IElement*	at(index_t index) const
		{
			RESULT* result = this->getResult(index);
			return result->element;
		}

		/** @brief  */
		inline const scalar	getDistanceLimit() const { return mLimit; }

		/** @brief  */
		inline uint32		getAppFlag() const { return mFlag.getMask(); }

		/** @brief need triangles that an object is interests the query */
		bool				needTriangles() const { return mTriangles != NULL; }

		inline ITriangleBuffer* getTriangles() const { return mTriangles; }

		/** @brief  */
		bool				addTriangle(const POINT3& p0, const POINT3& p1, const POINT3& p2)
		{
			return (mTriangles != NULL) ? (mTriangles->add(p0, p1, p2), true) : false;
		}

		/** @brief  */
		inline ICallback*	getCallback() const { return mCallback; }

		/** @brief  */
		ETargetFilter	getFilter() const { return mFilter; }

		/** @brief whether use precise check on distance, default: false */
		inline void		setPrecise(bool precise) { mPrecise = precise; }

		/** @brief  */
		inline bool		isPrecise() const { return mPrecise; }

		/** @brief  */
		inline void		setQueryNearest(bool nearest) { mNearestOnly = nearest; }
		
		/** @brief  */
		inline bool		isQueringNearest() const { return mNearestOnly; }

		/** @brief get intersection dir of triangles
		only valid if intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit = FLT_MAX) 
		or intersect(const GraphicsGeometry& geom, scalar& distance, scalar limit = FLT_MAX) returns true */
		const Vector3&	getInsectionDirection() const { return mDirection; }

		/**
		@describe test whether AABB interests query
		@param
		@return
		*/
		virtual POS_VOL	intersect(const AABB& aab, scalar& distance) const = 0;

		/**
		@describe test whether sphere interests query
		@param
		@return
		*/
		virtual POS_VOL	intersect(const Sphere&	sphere, scalar& distance) const = 0;

		/**
		@describe test whether triangle intersects the query
		@param
		@return
		*/
		virtual bool	intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit = FLT_MAX) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	intersect(const GraphicsGeometry& geom, scalar& distance, scalar limit = FLT_MAX) const = 0;
		/**
		@describe
		@param
		@return
		*/
		virtual StaticHandle<SpaceQuery>	clone(const Matrix44& transform) const = 0;

	protected:
		mutable StaticHandle<ResultList>	mResult;
		scalar				mLimit;
		AppFlag				mFlag;
		ITriangleBuffer*	mTriangles;
		ICallback*			mCallback;
		mutable Vector3		mDirection;
		ETargetFilter		mFilter;
		bool				mPrecise;
		bool				mNearestOnly;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT RaycastQuery : public SpaceQuery
	{
	public:
		/** @brief  */
		RaycastQuery(const Ray& ray, scalar raylimit = FLT_MAX, POS_SIDE side = PS_POSITIVE, AppFlag appflag = FULL_APPFLAG, ITriangleBuffer* triangles = NULL, ICallback* cb = NULL)
			:SpaceQuery(raylimit, appflag, triangles, cb)
			,mRay(ray)
			,mSide(side)
		{
			mDirection = mRay.getDirection();
		}

		/** @brief  */
		inline virtual ~RaycastQuery() {}

		/** @brief  */
		inline const Ray&	getRay() const			{return mRay;}

		/** @brief  */
		inline scalar		getRayLimit() const		{return mLimit;}

		/**
		@describe test whether AABB interests query
		@param
		@return
		*/
		virtual POS_VOL	intersect(const AABB& aab, scalar& distance) const
		{
			scalar max;
			POS_VOL pv = mRay.intersect(aab, distance, max) ? PV_INTERSECTED : PV_OUTSIDE;
			if (pv == PV_INTERSECTED && distance < 0)
				distance = max;
			return pv;
		}

		/**
		@describe test whether sphere interests query
		@param
		@return
		*/
		virtual POS_VOL	intersect(const Sphere&	sphere, scalar& distance) const
		{
			return mRay.intersect(sphere, distance) ? PV_INTERSECTED : PV_OUTSIDE;
		}

		/**
		@describe test whether triangle interests query
		@param
		@return
		*/
		virtual bool	intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit = FLT_MAX) const
		{
			return mRay.intersect(p0, p1, p2, distance, mSide, limit);
		}

		/**
		@describe 
		@param
		@return
		*/
		BLADE_FRAMEWORK_API virtual bool	intersect(const GraphicsGeometry& geom, scalar& distance, scalar limit = FLT_MAX) const;

		/**
		@describe
		@param
		@return
		*/
		virtual StaticHandle<SpaceQuery>	clone(const Matrix44& transform) const
		{
			Ray ray = mRay * transform;
			return StaticHandle<SpaceQuery>(BLADE_NEW RaycastQuery(ray, mLimit, mSide, mFlag, mTriangles, mCallback));
		}

	protected:
		Ray		mRay;
		POS_SIDE mSide;
	};//class RaycastQuery


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT AABBQuery : public SpaceQuery
	{
	public:
		AABBQuery(const AABB& aab, AppFlag appflag = FULL_APPFLAG, ITriangleBuffer* triangles = NULL, ICallback* cb = NULL)
			:SpaceQuery(FLT_MAX, appflag, triangles, cb)
			,mBounding(aab)
		{
			assert(!aab.isNull() && !aab.isInfinite());
		}

		virtual ~AABBQuery() {}

		/** @brief  */
		inline const AABB&	getBounding() const { return mBounding; }

		/**
		@describe test whether AABB interests query
		@param 
		@return 
		*/
		virtual POS_VOL	intersect(const AABB& aab, scalar& distance) const
		{
			POS_VOL ret = mBounding.intersect(aab);
			if (ret)
			{
				if (aab.isInfinite() || mBounding.isInfinite())
					distance = 0;
				else
					distance = mBounding.getCenter().getDistance(aab.getCenter());
			}
			return ret;
		}

		/**
		@describe test whether sphere interests query
		@param 
		@return 
		*/
		virtual POS_VOL	intersect(const Sphere&	sphere, scalar& distance) const
		{
			POS_VOL ret = mBounding.intersect(sphere);
			if (ret)
				distance = mBounding.getCenter().getDistance(sphere.getCenter());
			return ret;
		}

		/**
		@describe 
		@param
		@return
		*/
		BLADE_FRAMEWORK_API virtual bool	intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit = FLT_MAX) const;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	intersect(const GraphicsGeometry& /*geom*/, scalar& /*distance*/, scalar /*limit = FLT_MAX*/) const { return false; };

		/**
		@describe 
		@param 
		@return 
		*/
		virtual StaticHandle<SpaceQuery>	clone(const Matrix44& transform) const
		{
			AABB newAABB = mBounding*transform;
			return StaticHandle<SpaceQuery>(BLADE_NEW AABBQuery(newAABB, mFlag, mTriangles, mCallback));
		}

	protected:
		AABB mBounding;
	};//class AABBQuery


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_VCLASS_EXPORT SphereQuery : public SpaceQuery
	{
	public:
		SphereQuery(const Sphere& sphere, AppFlag appflag = FULL_APPFLAG, ITriangleBuffer* triangles = NULL, ICallback* cb = NULL)
			:SpaceQuery(FLT_MAX, appflag, triangles, cb)
			,mSphere(sphere)
		{
		}

		virtual ~SphereQuery() {}

		/** @brief  */
		inline const Sphere&		getSphere() const { return mSphere; }

		/**
		@describe test whether AABB interests query
		@param 
		@return 
		*/
		virtual POS_VOL	intersect(const AABB& aab, scalar& distance) const
		{
			POS_VOL ret = mSphere.intersect(aab);
			if (ret)
			{
				if (aab.isInfinite() || aab.isNull())
					distance = 0;
				else
					distance = mSphere.getCenter().getDistance(aab.getCenter());
			}
			return ret;
		}

		/**
		@describe test whether sphere interests query
		@param 
		@return 
		*/
		virtual POS_VOL	intersect(const Sphere&	sphere, scalar& distance) const
		{
			POS_VOL ret = mSphere.intersect(sphere);
			if (ret)
				distance = mSphere.getCenter().getDistance(sphere.getCenter());
			return ret;
		}

		/**
		@describe test whether triangle interests query
		@param 
		@return 
		*/
		virtual bool	intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit = FLT_MAX) const;

		/**
		@describe
		@param
		@return
		*/
		virtual bool	intersect(const GraphicsGeometry& /*geom*/, scalar& /*distance*/, scalar /*limit = FLT_MAX*/) const { return false; };

		/**
		@describe 
		@param 
		@return 
		*/
		virtual StaticHandle<SpaceQuery>	clone(const Matrix44& transform) const
		{
			const Vector4 center = Vector4(mSphere.getCenter(), 1)*transform;
			Sphere newSphere(center, mSphere.getRadius());
			return StaticHandle<SpaceQuery>(BLADE_NEW SphereQuery(newSphere, mFlag, mTriangles, mCallback));
		}
	

	protected:
		Sphere		mSphere;
	};//class SphereQuery


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class IGraphicsCamera;
	class IGraphicsView;
	class BLADE_VCLASS_EXPORT ScreenQuery : public SpaceQuery
	{
	public:
		ScreenQuery(const Box2& rect, IGraphicsCamera* camera, IGraphicsView* view,
			AppFlag appFlag = FULL_APPFLAG, ITriangleBuffer* triangles = NULL, ICallback* cb = NULL)

			:SpaceQuery(FLT_MAX, appFlag, triangles, cb)
			,mRect(rect)
			,mCamera(camera)
			,mView(view)
		{
			mFilter = TF_2D;
		}

		virtual ~ScreenQuery() {}

		/** @brief  */
		inline void setFilter(ETargetFilter filter) { mFilter = filter; }

		/** @brief  */
		inline const Box2& getBoudingRect() const { return mRect; }

		/** @brief  */
		inline IGraphicsCamera*	getCamera() const { return mCamera; }

		/** @brief  */
		inline IGraphicsView* getView() const { return mView; }


		/**
		@describe test whether AABB interests query
		@param
		@return
		*/
		BLADE_FRAMEWORK_API virtual POS_VOL	intersect(const AABB& aab, scalar& distance) const;

		/**
		@describe test whether sphere interests query
		@param
		@return
		*/
		virtual POS_VOL	intersect(const Sphere&	sphere, scalar& distance) const
		{
			return this->intersect(AABB(sphere), distance);
		}

		/**
		@describe test whether triangle interests query
		@param
		@return
		*/
		virtual bool	intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar /*limit = FLT_MAX*/) const
		{
			AABB aab(BT_NULL);
			aab.merge(p0);
			aab.merge(p1);
			aab.merge(p2);
			return this->intersect(aab, distance) != PV_OUTSIDE;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool	intersect(const GraphicsGeometry& /*geom*/, scalar& /*distance*/, scalar /*limit = FLT_MAX*/) const { return false; };

		/**
		@describe 
		@param
		@return
		*/
		virtual StaticHandle<SpaceQuery>	clone(const Matrix44& /*transform*/) const
		{
			return StaticHandle<SpaceQuery>(BLADE_NEW ScreenQuery(mRect, mCamera, mView, mFlag, mTriangles, mCallback));
		}

	private:
		Box2				mRect;
		IGraphicsCamera*	mCamera;	//camera used for projection
		IGraphicsView*		mView;
	};

}//namespace Blade



#endif // __Blade_SpaceQuery_h__