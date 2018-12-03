/********************************************************************
	created:	2012/12/30
	filename: 	GraphicsEffect.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/GraphicsEffect.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/public/graphics/IGraphicsScene.h>

namespace Blade
{
	const TString IBoundingEffect::TYPE = BTString("AABBEffect");
	const TString IDecalEffect::TYPE = BTString("DecalEffect");
	const TString IBrushDecalEffect::TYPE = BTString("BrushDecalEffect");
	const TString IHighLightEffect::TYPE = BTString("HighLightEffect");
	const TString ITransparentEffect::TYPE = BTString("TransparentEffect");
	const TString IEditGizmoEffect::TYPE = BTString("AxisGizmoEffect");
	const TString IHUDEffect::TYPE = BTString("HUDEffect");

	//////////////////////////////////////////////////////////////////////////
	size_t IBrushDecalEffect::queryElement(PointerParam& outList, AppFlag appFlag/* = INVALID_APPFLAG*/, const TString& spaceName/* = TString::EMPTY*/)
	{
		if( appFlag == INVALID_APPFLAG )
			appFlag = mAppFlag;

		AABBQuery query(this->getAABB(), appFlag);
		IGraphicsScene* scene = this->getManager()->getScene();
		scene->queryElements(query, spaceName);
		//outList.clear();
		size_t elementCount = query.size();
		for(size_t i = 0; i < elementCount; ++i)
		{
			IElement* elem = query.at(i);
			assert(elem != NULL);
			outList.push_back(elem);
		}
		return elementCount;
	}
	
}//namespace Blade