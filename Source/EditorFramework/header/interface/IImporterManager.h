/********************************************************************
	created:	2015/09/24
	filename: 	IImporterManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IImporterManager_h__
#define __Blade_IImporterManager_h__
#include "public/IImporter.h"
#include <interface/InterfaceSingleton.h>

namespace Blade
{

	class IImporterManager : public InterfaceSingleton<IImporterManager>
	{
	public:
		virtual ~IImporterManager() {}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	registerImporter(const SEditorImporterInfo& impInfo) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getImporterCount() const = 0;

		/**
		@describe get importer info by index, ranged from [0, getImporterCount())
		@param
		@return
		*/
		virtual const SEditorImporterInfo* getImporterInfo(index_t index) const = 0;

		/**
		@describe get impoerter info by importer type index: SEditorImporterInfo::mTypeIndex
		@param
		@return
		*/
		virtual const SEditorImporterInfo* getImporterInfoByType(FileTypeID typeID) const = 0;
	};//class IImporterManager

	extern template class BLADE_EDITOR_API Factory<IImporterManager>;

	
}//namespace Blade


#endif // __Blade_IImporterManager_h__