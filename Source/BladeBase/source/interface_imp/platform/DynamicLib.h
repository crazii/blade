/********************************************************************
	created:	2010/04/02
	filename: 	DynamicLlib.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_DynamicLlib_h__
#define __Blade_DynamicLlib_h__
#include <BladePlatform.h>
#include <BladeBase.h>
#include <utility/String.h>

namespace Blade
{
	class DynamicLib : public Allocatable
	{
	public:
		DynamicLib();
		//set the name ,but not load
		DynamicLib(const TString& name);
		~DynamicLib();

		/*
		@describe get the dynamic link lib 's name
		@param none
		@return name string
		*/
		const TString&	getName() const;

		/*
		@describe load the lib
		@param name the name of shared library
		@param bPersistant do not unload the library ever.
		@return 
		@if this instance has already loaded a lib,\n
		it will return false
		*/
		bool			load(const TString& name, bool except = true, bool persistent = false);

		/*
		@describe unload the lib
		@param 
		@return 
		*/
		void			unload();

		/*
		@describe get the export symbol address
		@param symbol symbol string to find
		@return address pointer
		*/
		void*			getExport(const char* symbol);


		//for STL access
		bool			operator<(const DynamicLib& rhs) const;

		//
		bool			operator==(const DynamicLib& rhs) const;

	protected:
		typedef void* DLHandle;

		/** @brief name of the lib */
		TString		mName;

		/** @brief handle of the lib */
		DLHandle	mLibHandle;

		bool		mPersistent;
	};//class DynamicLib
	
}//namespace Blade


#endif //__Blade_DynamicLlib_h__