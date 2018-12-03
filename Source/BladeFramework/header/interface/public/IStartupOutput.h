/********************************************************************
	created:	2011/09/11
	filename: 	IStartupOutput.h
	author:		Crazii
	purpose:	this output not same as the log, it is user-oriented at startup time, such as splash
*********************************************************************/
#ifndef __Blade_IStartupOutput_h__
#define __Blade_IStartupOutput_h__
#include <BladeFramework.h>

namespace Blade
{

	class IStartupOutput : public InterfaceSingleton<IStartupOutput>
	{
	public:
		virtual ~IStartupOutput()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void initialize(const TString& softInfo) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void show(bool bShow) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void outputProgress(const TString& detail) = 0;

	};//class IStartupOutput

	extern template class BLADE_FRAMEWORK_API Factory<IStartupOutput>;

}//namespace Blade



#endif // __Blade_IStartupOutput_h__
