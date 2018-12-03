/********************************************************************
	created:	2011/04/23
	filename: 	StageResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_StageResource_h__
#define __Blade_StageResource_h__
#include <interface/public/IResource.h>
#include "interface_imp/Stage.h"

namespace Blade
{
	class IPage;

	class StageResource : public IResource, public Allocatable
	{
	public:
		StageResource();
		~StageResource();

		/************************************************************************/
		/* IResource interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void					setStage(IStage* stage)		{mStage = stage;}

		/** @brief  */
		IStage*					getStage() const			{return mStage;}

		/** @brief  */
		void					setPage(IPage* page) { mPage = page; }

		/** @brief  */
		IPage*					getPage() const { return mPage; }

	protected:
		IStage*	mStage;
		IPage*	mPage;
		friend class StageSerializer;
	};//class StageResource


}//namespace Blade



#endif // __Blade_StageResource_h__