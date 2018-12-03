/********************************************************************
	created:	2013/10/29
	filename: 	LangTableFile.h
	author:		Crazii
	
	purpose:	object holding string table data (raw buffer of strings)
				this object should not be released until shutdown, because 
				all TString objects may keep reference to the string buffer here.
*********************************************************************/
#ifndef __Blade_LangTableFile_h__
#define __Blade_LangTableFile_h__
#include <interface/public/file/IStream.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class LangTableFile
	{
	public:
		typedef StaticVector<TString> LangStringTable;
		typedef StaticMap<TString, LangStringTable> MultiLangTable;

		//writable data
		typedef Vector<TString> WritableLangStringTable;
		typedef Map<TString, WritableLangStringTable> WritableMultiLangTable;

		LangTableFile();
		~LangTableFile();

		/*
		@describe 
		@param 
		@return 
		*/
		bool open(const HSTREAM& stream, MultiLangTable& table);

		/*
		@describe 
		@param 
		@return 
		*/
		bool isOpen() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void close();

		/*
		@describe 
		@param 
		@return 
		*/
		static bool save(const HSTREAM& stream, const WritableMultiLangTable& data);

	protected:
		tchar* mBuffer;
	};
	
}//namespace Blade


#endif // __LangTableFile_h__
