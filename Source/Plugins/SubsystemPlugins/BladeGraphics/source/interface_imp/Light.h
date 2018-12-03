/********************************************************************
	created:	2011/08/30
	filename: 	Light.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Light_h__
#define __Blade_Light_h__
#include <interface/ILight.h>
#include <interface/public/IRenderable.h>
#include <SpaceContent.h>
#include <interface/ILightManager.h>

namespace Blade
{
	class Light : public ILight, public SpaceContent, public IRenderable, public Allocatable
	{
	public:
		enum ELightRenderHelperMode
		{
			LRHM_NONE,
			LRHM_VOLUME	= 0x0001,
		};
	public:
		Light(ILightManager* lightsource, ELightType type = LT_DIRECTIONAL, bool notify = true);
		~Light();

		/************************************************************************/
		/* ILight interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const uint32&	getType() const		{return mType;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Vector3&	getPosition() const	{return this->SpaceContent::getPosition();}

		/**
		@describe 
		@param
		@return
		*/
		virtual const scalar&	getAttenuation() const	{return mAttenuation;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Vector3&	getDirection() const	{return mDirection;}

		/**
		@describe
		@param
		@return
		*/
		virtual const Quaternion& getOrientation() const { return mRotation; }

		/**
		@describe
		@param
		@return
		*/
		virtual const scalar&	getRange() const { return mRange; }

		/**
		@describe
		@param
		@return
		*/
		virtual const scalar&	getSpotInnerAngle() const { return mSpotInnerAngle; }

		/**
		@describe
		@param
		@return
		*/
		virtual const scalar&	getSpotOuterAngle() const { return mSpotOutAngle; }

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getDiffuse() const		{return mDiffuse;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const Color&	getSpecular() const		{return mSpecular;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ILightManager*	getLightManager() const	{return mLightManager;}

		/************************************************************************/
		/* custom interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool					setType(ELightType type, bool notify = true);

		/*
		@describe 
		@param 
		@return 
		*/
		void					setPosition(const Vector3& pos);

		/*
		@describe 
		@param 
		@return 
		*/
		void					setDirection(const Vector3& dir);

		/**
		@describe 
		@param
		@return
		*/
		void					enableVolumeHelper(bool enable, const Color& color);

		/** @brief  */
		inline void			setAttenuation(const scalar& attenuation)
		{
			mAttenuation = attenuation;
			this->updateAttenuation();
		}

		/** @brief  */
		inline void			setRange(const scalar& range)
		{
			mRange = range;
			if (mType == LT_POINT)
				this->setScale(Vector3::UNIT_ALL*range);
			else if (mType == LT_SPOT)
			{
				scalar radius = std::tan(Math::Degree2Radian(mSpotOutAngle/2)) * range;
				this->setScale(Vector3(radius, radius, range));
			}
		}

		/** @brief  */
		inline void			setSpotLightInnerAngle(const scalar& angle)
		{
			mSpotInnerAngle = angle;
			this->updateAttenuation();
		}

		/** @brief  */
		inline void			setSpotLightOuterAngle(const scalar& angle)
		{
			mSpotOutAngle = angle;
			if (mType == LT_SPOT)
			{
				scalar range = this->getRange();
				float radius = range * std::tan( Math::Degree2Radian(angle/2) );
				this->setScale(Vector3(radius, radius, range));
			}
		}

		/** @brief  */
		inline void			setDiffuse(const Color& diffuse)
		{
			mDiffuse = diffuse;
		}

		/** @brief  */
		inline void			setSpecular(const Color& spec)
		{
			mSpecular = spec;
		}

		/** @brief  */
		inline const Vector4&	getAttenuationParam() const
		{
			return mAttenuationParam;
		}

	protected:

		/************************************************************************/
		/* SpaceContent overrides                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			notifyPositionChange();

		/*
		@describe
		@param
		@return
		*/
		virtual void			notifyRotationChange();

		/**
		@describe
		@param
		@return
		*/
		virtual void			notifyScaleChange();

		/**
		@describe
		@param
		@return
		*/
		virtual	uint32			getAppFlag() const { return this->getRenderType().getAppFlag().getMask(); }

		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		TODO: add debug switch to show helpers
		*/
		virtual void			updateRender(IRenderQueue* queue);

		/************************************************************************/
		/* IRenderable interface                                                                     */
		/************************************************************************/
	public:
		/**
		@describe 
		@param
		@return
		*/
		virtual RenderType&				getRenderType() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual const GraphicsGeometry&	getGeometry() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual const MaterialInstance*	getMaterial() const;

		/**
		@describe 
		@param
		@return
		*/
		virtual const Matrix44&			getWorldTransform() const	{return mWorldTransform;}

		/**
		@describe get hosted content
		@param
		@return
		*/
		virtual ISpaceContent*			getSpaceContent() const { return const_cast<ISpaceContent*>(static_cast< const ISpaceContent*>(this)); }

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		//MaterialInstance*	getMaterial()  { return mMaterial; }

	protected:

		/** @brief  */
		void					updateAttenuation();

		ILightManager*	mLightManager;
		//HMATERIALINSTANCE	mMaterial;
		Matrix44	mWorldTransform;

		//shader parameters
		Vector4		mAttenuationParam;
		//note: the two members are used as a whole shader parameter
		Vector3		mDirection;
		scalar		mPadding;

		Color		mDiffuse;
		Color		mSpecular;
		//-shader parameters

		scalar		mRange;
		scalar		mSpotInnerAngle;	//in degrees
		scalar		mSpotOutAngle;		//in degrees
		scalar		mAttenuation;		//linear attenuations
		Mask		mMask;
		uint32		mType;


		class LightHelper : public IRenderable, public Allocatable
		{
		public:
			LightHelper(Light* light)
				:mLight(light)
			{
				mState = 0;
			}

			/** @brief  */
			virtual RenderType&				getRenderType() const;

			/** @brief  */
			virtual const GraphicsGeometry&	getGeometry() const;

			/** @brief  */
			virtual const MaterialInstance*	getMaterial() const { return mLight->getMaterial(); }

			/** @brief  */
			virtual const Matrix44&			getWorldTransform() const { return mLight->getWorldTransform(); }

			/** @brief  */
			virtual ISpaceContent*			getSpaceContent() const { return mLight; }

			/** @brief  */
			void enqueue(IRenderQueue* queue);

		protected:
			Light*					mLight;
			mutable index_t			mState;
		};
		LightHelper		mBoundingHelper;
	};

	//ambient light object only used for deferred shading. draw full screen quad like directional lights
	class AmbientLight : public Light
	{
	public:
		AmbientLight(ILightManager* mananger) :Light(mananger, LT_DIRECTIONAL, false) {}

		/** @brief  */
		virtual MaterialInstance* getMaterial() const;
	};


}//namespace Blade



#endif // __Blade_Light_h__