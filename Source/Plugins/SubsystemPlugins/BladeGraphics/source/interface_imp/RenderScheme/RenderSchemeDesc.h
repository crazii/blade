/********************************************************************
	created:	2012/02/20
	filename: 	SchemeDesc.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SchemeDesc_h__
#define __Blade_SchemeDesc_h__
#include <utility/String.h>
#include <utility/BladeContainer.h>
#include <utility/FixedArray.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/PixelFormat.h>
#include <interface/public/graphics/IGraphicsBuffer.h>
#include <interface/ICamera.h>
#include <utility/Mask.h>

namespace Blade
{
	typedef enum EBufferReference
	{
		BR_CUSTOM	= 0x01,
		BR_FINAL	= 0x02,
		BR_EMPTY	= 0x03,
	}BUFFER_REF;

	typedef enum EBufferUsage
	{
		BU_NONE		= 0x00,
		BU_DEFAULT	= 0x01,
		BU_FINAL	= 0x02,
		BU_IMAGEFX	= 0x04,
	}BUFFER_USAGE;

	typedef enum ECameraReference
	{
		CR_MAIN,		//main camera specified by app
		CR_BYNAME,
	}CAMREA_REF;

	typedef enum EViewReference
	{
		VR_FINAL,		//main view specified by app, attached by final target output
		VR_MANUAL,
	}VIEW_REF;


	typedef struct SRenderTypeDesc
	{
		TString		mRenderTypeName;

		bool operator<(const SRenderTypeDesc& rhs) const
		{
			return mOP.operator() (mRenderTypeName,rhs.mRenderTypeName); 
		}
		FnTStringFastLess mOP;
	}TYPE_DESC;

	typedef Set<TYPE_DESC> TypeDescList;

	enum ERenderBufferType
	{
		RBT_NONE = 0,
		RBT_COLOR,
		RBT_DEPTH,
	};

	typedef struct SRenderBufferDesc
	{
		TString		mName;
		size_t		mWidth;
		size_t		mHeight;
		scalar		mWidthScale;
		scalar		mHeightScale;
		PixelFormat	mFormat;
		ERenderBufferType	mType;
		IGraphicsBuffer::ACCESS mGpuAccess;
		BUFFER_REF	mBufferRef : 16;
		mutable int	mBufferUsage : 16;

		bool operator<(const SRenderBufferDesc& rhs) const
		{
			return mOP.operator() (mName,rhs.mName); 
		}
		FnTStringFastLess	mOP;
	}BUFFER_DESC;

	typedef Set<BUFFER_DESC> BufferDescList;

	typedef struct SViewDesc
	{
		VIEW_REF	mViewRef;
		scalar		mLeft;
		scalar		mTop;
		scalar		mRight;
		scalar		mBottom;
		size_t		mLeftPixel;
		size_t		mTopPixel;
		size_t		mRightPixel;
		size_t		mBottomPixel;
		uint32		mClearMode;		//see @EFrameBufferType
		Color		mClearColor;
		scalar		mClearDepth;
		uint16		mClearStencil;

		SViewDesc()
		{
			mLeft = mTop = 0;
			mRight = mBottom = 1;
			mLeftPixel = mTopPixel = mRightPixel = mBottomPixel = 0;
			mViewRef = VR_FINAL;
			mClearMode = FBT_ALL;
			mClearColor = Color::BLACK_ALPHA;
			mClearDepth = 1.0f;
			mClearStencil = 0;
		}
	}VIEW_DESC;

	typedef enum ECameraDescFlag
	{
		CDF_CUSTOM				= 0x100,	//using custom fields

		CDF_CUSTOM_TYPE			= 0x001,
		CDF_CUSTOM_NEAR			= 0x002,	//separated field flags
		CDF_CUSTOM_FAR			= 0x004,
		CDF_CUSTOM_FOV			= 0x008,
		CDF_CUSTOM_ASPECT		= 0x010,
		CDF_CUSTOM_ORTHO		= 0x020,

		CDF_CUSTOM_ALL			= CDF_CUSTOM_TYPE|CDF_CUSTOM_NEAR|CDF_CUSTOM_FAR|CDF_CUSTOM_FOV|CDF_CUSTOM_ASPECT|CDF_CUSTOM_ORTHO,
	}CAMERA_FLAG;

	typedef struct SCameraDesc
	{
		CAMREA_REF	mCameraRef;
		TString		mCameraName;
		PROJECTION	mCameraType;
		scalar		mNear;
		scalar		mFar;
		//perspective param
		scalar		mFOV;
		scalar		mAspect;
		//orthographic param
		scalar		mWidth;
		scalar		mHeight;
		size_t		mWidthPixel;
		size_t		mHeightPixel;
		//TODO: flag to reference near,far,fov.etc to main camera?
		//calculate aspect by view size
		Mask		mFlags;
	}CAMERA_DESC;

	typedef struct SRenderStepDesc : public Allocatable
	{
		const TYPE_DESC*	mTypeDesc;
		TString				mPassName;
		TStringList			mGroups;
		TStringList			mSorters;
		bool				mEnable;
		bool				mTexturing;	//optimization for texture bounding
	}STEP_DESC;

	typedef Vector<const STEP_DESC*>	StepDescList;

	static const size_t MAX_TARGET_COUNT = 4;
	static const size_t MAX_INPUT_COUNT = 8;

	typedef FixedVector<const BUFFER_DESC*,MAX_TARGET_COUNT> TargetDescList;
	typedef FixedVector<const BUFFER_DESC*,MAX_INPUT_COUNT> InputDescList;
	typedef struct SRenderOutputDesc : public Allocatable
	{
		TString				mName;
		TString				mImageEffect;
		TStringList			mSorters;
		VIEW_DESC			mViewDesc;
		CAMERA_DESC			mCameraDesc;
		TargetDescList		mTargetDescList;
		const BUFFER_DESC*	mDepthTarget;
		InputDescList		mInputDescList;
		StepDescList		mStepDescList;
		BUFFER_REF			mOutputBufferRef;
		bool				mEnable;
		bool				mHasImageEfect;
	}OUTPUT_DESC, IMAGEFX_DESC;

	typedef Vector<const OUTPUT_DESC*>	OutputDescList;

	typedef struct SOperatorDesc
	{
		TString	mType;		//factory type
		TString	mPrefix;	//target prefix
	}OPERATOR_DESC;
	typedef Vector<OPERATOR_DESC> OperatorDescList;

	typedef struct SRenderSchemeDesc : public Allocatable
	{
		TString			mName;
		TStringList		mProfileList;
		TypeDescList	mTypeList;
		BufferDescList	mBufferList;
		OutputDescList	mOutputList;
		OperatorDescList mOperatorList;
		bool			mUseFinalTarget;	//the final buffer is needed to render, or the final buffer's dimension(width/height) is needed
	}SCHEME_DESC;

}//namespace Blade


#endif //__Blade_SchemeDesc_h__