/********************************************************************
	created:	2014/09/26
	filename: 	ModeViewElement.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModeViewElement_h__
#define __Blade_ModeViewElement_h__
#include <interface/EditorElement.h>
#include <interface/ModelState.h>

namespace Blade
{
	class ModelViewElement : public EditorElement
	{
	public:
		static const TString MODELVIEW_ELEMENT_TYPE;
	public:
		ModelViewElement();
		~ModelViewElement();

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t	initParallelStates();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data in main synchronous state */
		virtual void	postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* EditorElement interface                                                                     */
		/************************************************************************/
		virtual void	onInitialize();

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		void			setAnimation(const TString& animation);

	protected:
		ParaTString			mPSOutAnimation;

		TStringList			mAnimationList;
		TStringList			mBoneList;
		friend class ModelViewerPlugin;
	};
	
}//namespace Blade


#endif // __Blade_ModeViewElement_h__