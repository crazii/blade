/********************************************************************
	created:	2010/05/09
	filename: 	ISpaceData.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ISpaceData_h__
#define __Blade_ISpaceData_h__
#include <utility/String.h>

namespace Blade
{
	class ISpaceContent;

	class ISpaceData
	{
	public:
		virtual ~ISpaceData()	{}

		/*
		@describe get the name of the data
		@param 
		@return 
		*/
		virtual const TString&	getSpaceTypeName() = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual uint32			getPartitionMask() const = 0;

		/*
		@describe when attach to a camera
		@param 
		@return 
		*/
		virtual void			onAttach(ISpaceContent* content) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onDetach(ISpaceContent* content) = 0;

	};//class ISpaceData

	
}//namespace Blade


#endif //__Blade_ISpaceData_h__