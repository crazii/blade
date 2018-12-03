/********************************************************************
	created:	2010/05/06
	filename: 	RenderScheme.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderScheme_h__
#define __Blade_RenderScheme_h__
#include <interface/IRenderScheme.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IRenderTarget.h>
#include "RenderOutput.h"
#include "RenderChannel.h"

namespace Blade
{	
	class ImageEffectHelper;

	class RenderScheme : public IRenderScheme, public IRenderTarget::IListener, public Allocatable
	{
	public:
		RenderScheme(const SCHEME_DESC* desc);
		~RenderScheme();

		/************************************************************************/
		/* IRenderScheme interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const
		{
			return mDesc->mName;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual const TStringList& getProfiles() const
		{
			return mDesc->mProfileList;
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual const TString&	getProfile() const
		{
			return mDesc->mProfileList[mProfileIndex];
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setProfile(index_t index)
		{
			if( index < mDesc->mProfileList.size() )
			{
				mProfileIndex = (uint16)index;
				return true;
			}
			return false;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getBufferCount() const
		{
			return this->getBuffers().size();
		}

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getBuffer(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getBuffer(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			updateCulling(ICamera* mainCamera = NULL) const;

		/*
		@describe
		@param
		@return
		*/
		virtual void			render(IRenderDevice* device,bool swapBuffer,
			IRenderTarget* target = NULL, IGraphicsView* view = NULL, ICamera* camera = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual void			setupRender(IRenderDevice* device,IRenderTarget* target,IGraphicsView* view, ICamera* camera,
			ILightManager* lightManager,ICameraManager* cameraManager);

		/************************************************************************/
		/* render/output control                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderPipeline*	createPipeline();

		/************************************************************************/
		/* IRenderTarget::IListener interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			onTargetSizeChange(IRenderTarget* target);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		IRenderTarget*	getRenderTarget(IRenderDevice* device, const RenderOutput* output, size_t width, size_t height);

		/** @brief  */
		bool			addRenderOutput(RenderOutput* output);

		/** @brief  */
		void			rebindBuffers(bool checkIFXBufferOnly = false);

		/** @brief post effect enable/disable event handler */
		void			onImageEffectEnable(void* data);

		/** @brief  */
		inline IRenderSchemeOperation*	getOperation(const TString& name) const
		{
			OperatorList::const_iterator i = mOperators.find(name);
			return i != mOperators.end() ? i->second : NULL;
		}

		/** @brief  */
		inline ImageEffectHelper* getIFXHelper() const { return mIFXHelper; }

		/** @brief  */
		static inline void		clearSharedBuffers()
		{
			msSharedBuffers.clear();
		}

		/** @brief  */
		static inline void		setUseSharedBuffer(bool isShared)
		{
			msUseShareBuffer = isShared;
		}

	protected:

		typedef TStringMap<HTEXTURE>		BufferMap;
		typedef Vector<RenderOutput*>		OutputList;
		typedef TStringMap<Material*>		TypeMaterialMap;
		typedef Map<const SCHEME_DESC*,BufferMap>		SharedBufferMap;
		typedef TStringMap<IRenderSchemeOperation*>	OperatorList;

		BufferMap		mBufferMap;
		OutputList		mOutputList;
		TypeMaterialMap	mMaterialMap;
		OperatorList	mOperators;

		IRenderTarget::IListener* mPrevListener;
		const SCHEME_DESC*	mDesc;
		ILightManager*	mLightManager;
		ICameraManager* mCameraManager;
		IRenderTarget*	mFinalOutput;
		IGraphicsView*	mFinalView;
		ICamera*		mMainCamera;
		ImageEffectHelper*	mIFXHelper;
		uint16			mMaxMRT;
		uint16			mProfileIndex;
		//note: outputs share render targets if outputs are the same
		TStringList		mInputBufferID;
		typedef Map<Mask64, IRenderTarget*> RenderTargetMap;
		RenderTargetMap	mRenderTargets;

		//used as cache for image effects
		typedef struct SStorage
		{
			const HTEXTURE*	first;
			const HTEXTURE*	second;
			HTEXTURE	storage;
		}STORAGE;
		typedef TStringMap<STORAGE>		BufferStorage;
		BufferStorage	mStorage;

		/** @brief get shared buffer for editor mode, otherwise use own buffer */
		BufferMap&	getBuffers();
		const BufferMap&	getBuffers() const { BufferMap& map = const_cast<RenderScheme*>(this)->getBuffers(); return map; }

		static SharedBufferMap	msSharedBuffers;
		static size_t			msViewID;
		static bool				msUseShareBuffer;
	};//class RenderScheme
	
}//namespace Blade


#endif //__Blade_RenderScheme_h__