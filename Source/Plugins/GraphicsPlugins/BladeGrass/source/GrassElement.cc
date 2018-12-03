/********************************************************************
created:	2017/10/12
filename: 	GrassElement.cc
author:		Crazii
purpose:
*********************************************************************/
#include "GrassElement.h"
#include "GrassResource.h"
#include "GrassContent.h"
#include <utility/StringHelper.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	GrassElement::GrassElement()
		:GraphicsElement(GrassConsts::GRASS_ELEMENT_TYPE)
	{
		//TODO: add global element to manage dynamic sized paging (subpage size can be changed at runtime/startup time, thus grass element bounding changes)
		//the global element create all grass elements on page loading time, those elements are not saved to page.
		mX = mZ = mOffsetX = mOffsetZ = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	GrassElement::~GrassElement()
	{

	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GrassElement::prepareSave()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassElement::postSave()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassElement::postProcess(const ProgressNotifier& notifier)
	{
		assert(mContent == NULL);

		GrassResource* res = static_cast<GrassResource*>(this->getBoundResource());
		if (res != NULL)
		{
			GrassContent* content = BLADE_NEW GrassContent();
			assert(content->getSpace() == NULL);
			content->setElement(this);
			content->initialize(mPosition, res->getLocalBounding(), res);
			mContent = content;
		}
		else
			assert(false);

		GraphicsElement::postProcess(notifier);
	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GrassElement::getResourceParams(ParamList& outParams, TString& /*outResType*/, TString& /*outSerializer*/)
	{
		outParams[GrassConsts::GRASS_PARAM_X] = (uint32)mOffsetX;
		outParams[GrassConsts::GRASS_PARAM_Z] = (uint32)mOffsetZ;
		outParams[GrassConsts::GRASS_PARAM_RANGE] = (uint32)mBounding->getSize().x;

		outParams[GrassConsts::GRASS_PARAM_SHARED_DATA] = BTString("path:") + TStringHelper::fromUInt(mX) + BTString("_") + TStringHelper::fromUInt(mZ);

		size_t meshCount = mGrassMesh.size();
		outParams[GrassConsts::GRASS_PARAM_MESH_COUNT] = (uint32)meshCount;
		for (size_t i = 0; i < meshCount; ++i)
		{
			outParams[GrassConsts::GRASS_PARAM_MESH_PREFIX + TStringHelper::fromUInt(i)] = mGrassMesh[i];
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GrassElement::onResourceUnload()
	{
		GraphicsElement::onResourceUnload();
		BLADE_DELETE mContent;
		mContent = NULL;
	}

	/************************************************************************/
	/* GraphicsElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GrassElement::onInitialize()
	{
		GrassConfig::getSingleton().buildInstancingPartitions(this->getRenderScene());
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	void GrassElement::onConfigChange(void* data)
	{
		if (data == &mGrassMesh)
		{
			if (mGrassMesh.size() > GrassConfig::MAX_LAYERS)
				mGrassMesh.resize(GrassConfig::MAX_LAYERS);
		}
	}
}