/********************************************************************
	created:	2011/08/22
	filename: 	MediaLibrary.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MediaLibrary_h__
#define __Blade_MediaLibrary_h__
#include <interface/IMediaLibrary.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class MediaLibrary : public IMediaLibrary, public Singleton<MediaLibrary>
	{
	public:
		MediaLibrary();
		~MediaLibrary();

		/************************************************************************/
		/* IMediaLibrary interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getMediaTypesCount() const		{return mMediaTypes.size();}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const MEDIA_TYPE&	getMediaTypeByIndex(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const MEDIA_TYPE&	getMediaType(const TString& type) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual	size_t	getMediaPaths(index_t mediaTypeIndex,TStringParam& outPathList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t	getMediaFileCount(index_t mediaTypeIndex) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const MediaFile*	getMediaFile(index_t mediaTypeIndex,index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const MediaFile*	getMediaFile(index_t mediaTypeIndex,const TString& mediaFileName) const;

		/*
		@describe register media types
		@param 
		@return the media type's index filled by MediaLibrary, same as MediaType::mIndex
		@note you should register you media type at plugin installing time, before IMediaLibrary::initialize
		*/
		virtual index_t	registerMediaType(const MEDIA_TYPE& mediaType,const TString& path = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	addMediaFilePath(index_t mediaTypeIndex,const TString& path);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	clearImageCaches();

		/**
		@describe
		@param
		@return
		*/
		virtual bool			getMediaFileInfo(const TString& type, const TString& mediaFileName,
			ISelectionReceiver& receiver);

	protected:
		typedef Vector<MEDIA_TYPE>	MediaTypeList;
		typedef Vector<TStringList>	TypedMediaPaths;
		typedef StaticVector<MediaFile>	FileList;
		typedef StaticVector<FileList>	TypedFileList;
		typedef StaticVector<TString>	MediaTypeNameList;


		MediaTypeList		mMediaTypes;
		TypedMediaPaths		mMediaPaths;
		TypedFileList		mFiles;
		bool				mInitialized;
	};
	

}//namespace Blade



#endif // __Blade_MediaLibrary_h__