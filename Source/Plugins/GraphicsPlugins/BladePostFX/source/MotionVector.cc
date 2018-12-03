/********************************************************************
	created:	2017/01/26
	filename: 	MotionVector.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <RenderUtility.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/IRenderSchemeManager.h>
#include <interface/IMaterialManager.h>
#include "MotionVector.h"

namespace Blade
{
	const TString MotionVector::NAME = BTString("MotionVector");

	//////////////////////////////////////////////////////////////////////////
	MotionVector::MotionVector()
		:ImageEffectBase(NAME)
	{
		mMaterial = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	MotionVector::~MotionVector()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void MotionVector::initialize()
	{
		mMaterial = IMaterialManager::getSingleton().getMaterial(BTString("motion_material"));
		mMaterial->loadSync();
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mHandleReprojection, BTString("ReprojectionMatrix"), SCT_MATRIX44, 1);
	}

	//////////////////////////////////////////////////////////////////////////
	void MotionVector::shutdown()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void MotionVector::prepareImpl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool MotionVector::process_inputoutput(const HTEXTURE& /*chainIn*/, const HTEXTURE& /*chainOut*/, const HTEXTURE& /*depth*/,
		const HTEXTURE* /*extraInputs*/, size_t /*inputCount*/, const HTEXTURE* extraOuputs/**/, size_t outputCount)
	{
		if (extraOuputs == NULL || outputCount != 1)
		{
			BLADE_UNREFERENCED(outputCount);
			assert(false);
			return false;
		}

		const Matrix44& reProjection = this->getReprojectionMatrix();
		mHandleReprojection.setData(&reProjection, sizeof(reProjection));

		this->doProcess(extraOuputs[0], mMaterial);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void MotionVector::finalizeImpl()
	{

	}
	
}//namespace Blade