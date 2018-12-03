/********************************************************************
created:	2017/10/12
filename: 	GrassElement.h
author:		Crazii
purpose:	grass loading unit, for streaming
*********************************************************************/
#ifndef __Blade_GrassElement_h__
#define __Blade_GrassElement_h__
#include <Element/GraphicsElement.h>

namespace Blade
{
	class GrassElement : public GraphicsElement, public Allocatable
	{
	public:
		GrassElement();
		~GrassElement();

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called before saving data  */
		virtual void			prepareSave();
		/** @brief  */
		virtual void			postSave();
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual void			getResourceParams(ParamList& outParams, TString& outResType, TString& outSerializer);

		/**
		@describe
		@param
		@return
		*/
		virtual void			onResourceUnload();

		/************************************************************************/
		/* GraphicsElement interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			onInitialize();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline index_t getOffsetX() const { return mOffsetX; }
		/** @brief  */
		inline index_t getOffsetZ() const { return mOffsetZ; }

	protected:
		//those data won't change after first-time creation
		uint16	mX;			//index to locate resource
		uint16	mZ;
		uint16	mOffsetX;	//offset x resource
		uint16	mOffsetZ;	//offset z resource

		//data bindings
		TStringList	mGrassMesh;
		/** @brief  */
		void onConfigChange(void* data);

		friend class BladeGrassPlugin;
	};
}

#endif