/********************************************************************
	created:	2013/04/23
	filename: 	ViewportManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ViewportManager.h"
#include "Viewport.h"
#include <interface/IEventManager.h>
#include <interface/public/ui/UIEvents.h>
#include <databinding/DataSourceWriter.h>
#include <BladeFramework_blang.h>

namespace Blade
{
	template class Factory<IViewportManager>;

	static const size_t RESERVED_LAYOUT_COUNT = 16;
	static const TString CFG_VIEWPORT = BXLang(BLANG_VIEWPORT);
	static const TString CFG_LAYOUT = BTString("Layout");
	static const TString CFG_CAMERA = BXLang(BLANG_CAMERA);
	static const TString CFG_CAMREA_POS = BXLang(BLANG_CAMERA_POS);
	static const TString CFG_CAMERA_SPEED = BXLang(BLANG_CAMERA_MOVE_SPEED);
	static const TString CFG_CAMERA_ROTATE_SPEED = BXLang(BLANG_CAMERA_ROTATE_SPEED);

	static const TString CFG_VIEWPORT_REALTIME = BXLang(BLANG_VIEW_VIEWREALTIME);
	static const TString CFG_VIEWPORT_MAXIMIZED = BXLang(BLANG_VIEW_VIEWMAX);

#define DEFAULT_CAMERA_MOVE_SPEED (50.0f)
#define DEFAULT_CAMERA_ROTATE_SPEED (45.0f)	//degrees

	//////////////////////////////////////////////////////////////////////////
	ViewportManager::ViewportManager()
	{
		mConfig.bind( BLADE_NEW ConfigGroup(CFG_VIEWPORT) );
		mCurrentLayout = 0;
		{
			DataSourceWriter<ViewportManager> writer( mConfig );
			writer.beginVersion(Version(0,1));
			writer << CFG_LAYOUT << &ViewportManager::onViewportConfigChange << static_cast<Bindable*>(this) << &ViewportManager::mCurrentLayout;
			writer.endVersion();
		}

		for(int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			mViewportCameraConfig[i].bind( BLADE_NEW ConfigGroup(CFG_CAMERA + TStringHelper::fromInt(i) ) );
			{
				//TODO: currently one delegate list for one single data member,
				//use one delegate list for one data source?
				DataSourceWriter<VIEWPORT_DESC> writer( mViewportCameraConfig[i] );
				writer.beginVersion(Version(0,1));
				//TODO: camera pos set by mini top view?
				
				writer << CFG_CAMERA_SPEED << ConfigAtom::Range(1.0f, 500.0f, 1.0f) << Delegate(this, &ViewportManager::onViewportConfigChange)
					<< &VIEWPORT_DESC::mCameraSpeed;

				writer << CFG_CAMERA_ROTATE_SPEED << ConfigAtom::Range(45.0f, 180.0f, 1.0f) << Delegate(this, &ViewportManager::onViewportConfigChange)
					<< &VIEWPORT_DESC::mCameraRotateSpeed;

				writer << CFG_VIEWPORT_REALTIME << CUIH_NONE << CAF_SERIALIZE << &VIEWPORT_DESC::mRealtime;
				writer << CFG_VIEWPORT_MAXIMIZED << CUIH_NONE << CAF_SERIALIZE << &VIEWPORT_DESC::mMaximized;

				writer.endVersion();
			}
			mConfig->addSubConfig( mViewportCameraConfig[i] );
		}
		
	}

	//////////////////////////////////////////////////////////////////////////
	ViewportManager::~ViewportManager()
	{
		
	}

	/************************************************************************/
	/* IViewportManager                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ViewportManager::initialize()
	{
		mLayouts.reserve(RESERVED_LAYOUT_COUNT);
		//built-in layout

		//1 perspective
		VIEWPORT_LAYOUT layout;

		//4 equal size	(default)
		layout.count = 4;
		for (size_t i = 0; i < layout.count; ++i)
		{
			VIEWPORT_DESC& desc = layout.layouts[i];

			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = (i % 2)*0.5f;
			desc.mTop = (i / 2)*0.5f;
			desc.mRight = (i % 2)*0.5f + 0.5f;
			desc.mBottom = (i / 2)*0.5f + 0.5f;
			desc.mType = (EViewportType)i;
			desc.mRealtime = desc.mType == VT_PERSPECTIVE;
			desc.mMaximized = false;
			desc.mActive = desc.mType == VT_PERSPECTIVE;
			desc.mFillMode = (desc.mType == VT_PERSPECTIVE) ? GFM_DEFAULT : GFM_WIREFRAME;
		}
		mLayouts.push_back(layout);

		layout.count = 1;
		{
			VIEWPORT_DESC& desc = layout.layouts[0];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = 0.0f;
			desc.mTop = 0.0f;
			desc.mRight = 1.0f;
			desc.mBottom = 1.0f;
			desc.mType = VT_PERSPECTIVE;
			desc.mFillMode = GFM_DEFAULT;
			desc.mMaximized = false;
			desc.mRealtime = true;
			desc.mActive = true;
		}
		//make other camera speed valid so that they save to config
		layout.layouts[1] = layout.layouts[2] = layout.layouts[3] = layout.layouts[0];
		mLayouts.push_back( layout );

		//1 large right, 3 small left
		layout.count = 4;
		const scalar rate = 1.0f/3.0f;
		for(size_t i = 0; i < 3; ++i)
		{
			VIEWPORT_DESC& desc = layout.layouts[i];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = 0;
			desc.mTop = i*rate;
			desc.mRight = rate;
			desc.mBottom = desc.mTop + rate;
			desc.mType = (EViewportType)i;
			desc.mRealtime = false;
			desc.mMaximized = false;
			desc.mActive = false;
			desc.mFillMode = GFM_WIREFRAME;
		}
		{
			VIEWPORT_DESC& desc = layout.layouts[3];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = rate;
			desc.mTop = 0.0f;
			desc.mRight = 1.0f;
			desc.mBottom = 1.0f;
			desc.mType = VT_PERSPECTIVE;
			desc.mRealtime = true;
			desc.mMaximized = false;
			desc.mActive = true;
			desc.mFillMode = GFM_SOLID;
		}
		mLayouts.push_back( layout );

		//1 large left, 3 small right
		layout.count = 4;
		for(size_t i = 0; i < 3; ++i)
		{
			VIEWPORT_DESC& desc = layout.layouts[i];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = 1.0f-rate;
			desc.mTop = i*rate;
			desc.mRight = 1.0f;
			desc.mBottom = desc.mTop + rate;
			desc.mType = (EViewportType)i;
			desc.mRealtime = false;
			desc.mMaximized = false;
			desc.mActive = false;
			desc.mFillMode = GFM_WIREFRAME;
		}
		{
			VIEWPORT_DESC& desc = layout.layouts[3];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = 0;
			desc.mTop = 0.0f;
			desc.mRight = 1.0f-rate;
			desc.mBottom = 1.0f;
			desc.mType = VT_PERSPECTIVE;
			desc.mRealtime = true;
			desc.mMaximized = false;
			desc.mActive = true;
			desc.mFillMode = GFM_SOLID;
		}
		mLayouts.push_back( layout );

		//1 large bottom, 3 small top
		layout.count = 4;
		for(size_t i = 0; i < 3; ++i)
		{
			VIEWPORT_DESC& desc = layout.layouts[i];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = i*rate;
			desc.mTop = 0;
			desc.mRight = desc.mLeft + rate;
			desc.mBottom = rate;
			desc.mType = (EViewportType)i;
			desc.mRealtime = false;
			desc.mMaximized = false;
			desc.mActive = false;
			desc.mFillMode = GFM_WIREFRAME;
		}
		{
			VIEWPORT_DESC& desc = layout.layouts[3];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = 0.0f;
			desc.mTop = rate;
			desc.mRight = 1.0f;
			desc.mBottom = 1.0f;
			desc.mType = VT_PERSPECTIVE;
			desc.mRealtime = true;
			desc.mMaximized = false;
			desc.mActive = true;
			desc.mFillMode = GFM_SOLID;
		}
		mLayouts.push_back( layout );


		//1 large top, 3 small bottom
		layout.count = 4;
		for(size_t i = 0; i < 3; ++i)
		{
			VIEWPORT_DESC& desc = layout.layouts[i];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = i*rate;
			desc.mTop = 1.0f-rate;
			desc.mRight = desc.mLeft + rate;
			desc.mBottom = 1.0f;
			desc.mType = (EViewportType)i;
			desc.mRealtime = false;
			desc.mMaximized = false;
			desc.mActive = false;
			desc.mFillMode = GFM_WIREFRAME;
		}

		{
			VIEWPORT_DESC& desc = layout.layouts[3];
			desc.mCameraSpeed = DEFAULT_CAMERA_MOVE_SPEED;
			desc.mCameraRotateSpeed = DEFAULT_CAMERA_ROTATE_SPEED;
			desc.mLeft = 0.0f;
			desc.mTop = 0.0f;
			desc.mRight = 1.0f;
			desc.mBottom = 1.0f-rate;
			desc.mType = VT_PERSPECTIVE;
			desc.mRealtime = true;
			desc.mMaximized = false;
			desc.mActive = true;
			desc.mFillMode = GFM_SOLID;
		}
		mLayouts.push_back( layout );
		this->setCurrentLayout(0);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ViewportManager::shutdown()
	{
		if( mCurrentLayout == (uint32)INVALID_INDEX )
		{
			assert(false);
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	ViewportManager::getLayoutCount() const
	{
		return mLayouts.size();
	}

	//////////////////////////////////////////////////////////////////////////
	index_t	ViewportManager::getCurrentLayout() const
	{
		assert( mCurrentLayout < mLayouts.size() );
		return mCurrentLayout;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ViewportManager::setCurrentLayout(index_t index)
	{
		if( index >= mLayouts.size() )
		{
			assert(false);
			return false;
		}
		mCurrentLayout = index;
		for(int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
			mViewportCameraConfig[i]->bindTarget( &(mLayouts[mCurrentLayout].layouts[i]) );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const VIEWPORT_LAYOUT&	ViewportManager::getLayout(index_t index) const
	{
		if( index < mLayouts.size() )
			return mLayouts[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	IViewportSet*	ViewportManager::createViewportSet(const VIEWPORT_LAYOUT& layout)
	{
		return BLADE_NEW ViewportSet(layout);
	}

	//////////////////////////////////////////////////////////////////////////
	void	ViewportManager::destroyViewportSet(IViewportSet* vps)
	{
		BLADE_DELETE vps;
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	ViewportManager::getViewportConfig(index_t index) const
	{
		if( index >= MAX_VIEWPORT_COUNT )
			return HCONFIG::EMPTY;
		return mViewportCameraConfig[index];
	}

	//////////////////////////////////////////////////////////////////////////
	void	ViewportManager::onViewportConfigChange(void* data)
	{
		if( data == &mCurrentLayout )
		{
			for(int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
				mViewportCameraConfig[i]->bindTarget( &(mLayouts[mCurrentLayout].layouts[i]) );
		}
		else
		{
			assert( mCurrentLayout < mLayouts.size() );
			//dispatch state(event)
			for(int i = 0; i < MAX_VIEWPORT_COUNT; ++i)
			{
				const VIEWPORT_DESC* desc = mLayouts[mCurrentLayout].layouts+i;
				if( data == &(desc->mCameraSpeed) || data == &(desc->mCameraRotateSpeed) )
				{
					IEventManager::getSingleton().dispatchEvent(ViewportCameraConfigEvent(i, desc->mCameraSpeed, Math::Degree2Radian(desc->mCameraRotateSpeed)));
					return;
				}
			}
			assert(false);
		}
	}
}//namespace Blade