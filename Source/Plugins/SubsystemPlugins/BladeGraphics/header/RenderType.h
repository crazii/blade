/********************************************************************
	created:	2011/04/02
	filename: 	RenderType.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderType_h__
#define __Blade_RenderType_h__
#include <BladeGraphics.h>
#include <interface/public/graphics/IGraphicsType.h>

namespace Blade
{

	class Material;
	class Technique;
	class IRenderQueue;

	class BLADE_GRAPHICS_API RenderType : public IGraphicsType
	{
	public:
		RenderType(const TString& name);
		~RenderType();

		/************************************************************************/
		/* IGraphicsType interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getName() const
		{
			return mName;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual AppFlag			getAppFlag() const
		{
			return mAppFlag;
		}

		/************************************************************************/
		/* custom interface & methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			setAppFlag(AppFlag flag)
		{
			mAppFlag = flag;
		}

		/*
		@describe
		@param
		@return
		*/
		virtual Material*		getMaterial() const = 0;

		/*
		@describe update the material's profile. return a non NULL pointer indicates that
		the profile has been updated, otherwise the profile remains unchanged
		@param
		@return
		*/
		virtual Material*		updateProfile(const TString& profile, bool& updated);
		
		/*
		@describe process render buffer after culling & before rendering, i.e. batch combinations, delayed visible updates, etc.
		@param
		@return
		*/
		virtual void			processRenderQueue(IRenderQueue* queue)	{ BLADE_UNREFERENCED(queue); }

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceReady();

		/**
		@describe 
		@param
		@return
		*/
		virtual void			onRenderDeviceClose();

		/** @brief  */
		inline bool				isFixedFillMode() const					{return mFixedFillMode;}

		/** @brief 0 based id. id for render types are continuous */
		inline uint32			getTypeID() const						{ return mID; }

	private:
		AppFlag			mAppFlag;
		uint32			mID;
		TString			mName;
		IRenderQueue*	mRenderBuffer;
		Material*		mLastMat;
	protected:
		bool			mFixedFillMode;

		/** @brief  */
		inline void				setTypeID(uint32 id) { mID = id; }
		friend class RenderTypeManager;

	};//class RenderType
	

}//namespace Blade



#endif // __Blade_RenderType_h__