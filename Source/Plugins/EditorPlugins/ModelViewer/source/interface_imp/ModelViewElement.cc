/********************************************************************
	created:	2014/09/26
	filename: 	ModelViewElement.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IEntity.h>
#include <interface/IModel.h>
#include "ModelViewElement.h"

namespace Blade
{
	const TString ModelViewElement::MODELVIEW_ELEMENT_TYPE = BTString("ModelView Element");

	//////////////////////////////////////////////////////////////////////////
	ModelViewElement::ModelViewElement()
		:EditorElement(MODELVIEW_ELEMENT_TYPE)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelViewElement::~ModelViewElement()
	{

	}

	/************************************************************************/
	/* IElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t	ModelViewElement::initParallelStates()
	{
		mParallelStates[ModelState::CURRENT_ANIMAION] = mPSOutAnimation;
		return EditorElement::initParallelStates();
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	ModelViewElement::postProcess(const ProgressNotifier& notifier)
	{
		EditorElement::postProcess(notifier);

		//postProcess order now controlled by priority, editor element has lowest priority
		//@see EditorElement
		//so at this time of postProcess, model element should be loaded already and
		//we can get the animations & bones
		//note: for a paged loading (which is not for model viewer but generally for other types), editor elements will
		//be added to page info later than model elements, so it still be loaded after model element. @see IStageConfig::addLayerElement
		TStringParam animationList;
		this->getEntity()->getInterface(IID_MODEL())->getAnimationList(animationList);
		mAnimationList = animationList;

		TStringParam boneList;
		this->getEntity()->getInterface(IID_MODEL())->getBoneList(boneList);
		std::sort(boneList.begin(), boneList.end());
		mBoneList = boneList;
	}

	/************************************************************************/
	/* EditorElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	ModelViewElement::onInitialize()
	{
		
	}
	
	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	ModelViewElement::setAnimation(const TString& animation)
	{
		mPSOutAnimation = animation;
	}

}//namespace Blade