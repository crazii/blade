/********************************************************************
	created:	2011/08/22
	filename: 	IMediaLibrary.h
	author:		Crazii
	purpose:	framework built in media library interface, holding data only
*********************************************************************/
#ifndef __Blade_IMediaLibrary_h__
#define __Blade_IMediaLibrary_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <utility/String.h>
#include <utility/StringList.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/graphics/IImage.h>

namespace Blade
{
	class IMediaLibrary : public InterfaceSingleton<IMediaLibrary>
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		class MediaFile
		{
		public:
			TString		mName;
			TString		mSchemePath;
			TString		mRawPath;
			index_t		mIndex;		//set by the media library
			HIMAGE		mPreview;

			/** @brief  */
			bool	operator<(const MediaFile& rhs) const
			{
				//FnTStringFastLess less;
				//return less(mName,rhs.mName);
				
				//note: use alphabet order
				return mName < rhs.mName;
			}
			/** @brief  */
			bool	operator==(const MediaFile& rhs) const
			{
				return mName == rhs.mName;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class BLADE_FRAMEWORK_API ISelectionReceiver
		{
		public:
			virtual ~ISelectionReceiver()	{}

			/** @brief  */
			virtual void	setSelectedTarget(index_t mediaIndex, const MediaFile& media) = 0;
			/** @brief  */
			virtual void	canceled() {};
		};

		//////////////////////////////////////////////////////////////////////////
		class BLADE_FRAMEWORK_API IMediaPreviewer
		{
		public:
			virtual ~IMediaPreviewer() {}

			/** @brief called for generating icon, or large image preview if it is not playable*/
			/** @brief if size == -1, then there is no scale, will use the original size */
			virtual HIMAGE	generatePreview(const TString& mediaFile,size_t size, IMAGE_POOL pool) = 0;

			/** @brief  */
			virtual bool	isPlayable() const { return false; }

			/** @brief start play the preview if it is playable */
			virtual void	play(IWindow* targetWindow,const TString& mediaFile) = 0;

			/** @brief  */
			virtual void	updatePlay() = 0;

			/** @brief  */
			virtual void	stop() = 0;
		};
		typedef Handle<IMediaPreviewer> HMEDIAPREVIEWER;

		//////////////////////////////////////////////////////////////////////////
		class BLADE_FRAMEWORK_API IMediaFilter
		{
		public:
			virtual ~IMediaFilter() {}
			/** @brief filter for media files */
			/* @remark return true means the input file is valid  */
			virtual bool	filterFile(const TString& file) const = 0;
		};
		typedef Handle<IMediaFilter> HMEDIAFILTER;

		//////////////////////////////////////////////////////////////////////////
		typedef struct SMediaType
		{
		public:
			TString				mType;
			TStringList			mExtensions;
			index_t				mIndex;			//index filled by MediaLibrary
			HMEDIAPREVIEWER		mPreviewer;
			HMEDIAFILTER		mFilter;		//this member can be NULL
			//all media files shares one icon(i.e. sounds may use one same icon like a speaker)
			bool				mSharedIcon;
			bool				mIsPlayable;
			
			inline	bool		operator<(const SMediaType& rhs) const
			{return FnTStringFastLess::compare(mType,rhs.mType);}
			inline	bool		operator==(const SMediaType& rhs) const	{return mType == rhs.mType;}
		}MEDIA_TYPE;

	public:
		virtual ~IMediaLibrary()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	initialize() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	shutdown() = 0;

		/**
		@describe actually types supported
		@param 
		@return 
		*/
		virtual size_t				getMediaTypesCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const MEDIA_TYPE&	getMediaTypeByIndex(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const MEDIA_TYPE&	getMediaType(const TString& type) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual	size_t	getMediaPaths(index_t mediaTypeIndex,TStringParam& outPathList) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t	getMediaFileCount(index_t mediaTypeIndex) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const MediaFile*	getMediaFile(index_t mediaTypeIndex,index_t index) const = 0;
		inline const MediaFile*		getMediaFile(const TString& type, index_t index) const
		{
			const MEDIA_TYPE& MediaType = this->getMediaType(type);
			return this->getMediaFile(MediaType.mIndex, index);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const MediaFile*	getMediaFile(index_t mediaTypeIndex, const TString& mediaFileName) const = 0;
		inline const MediaFile*		getMediaFile(const TString& type, const TString& mediaFileName) const
		{
			const MEDIA_TYPE& MediaType = this->getMediaType(type);
			return this->getMediaFile(MediaType.mIndex, mediaFileName);
		}

		/**
		@describe 
		@param 
		@return the media type's index filled by MediaLibrary, same as MediaType::mIndex
		@note you should register you media type at plugin installing time, before IMediaLibrary::initialize
		*/
		virtual index_t	registerMediaType(const MEDIA_TYPE& mediaType,const TString& path = TString::EMPTY) = 0;

		/**
		@describe 
		@param 
		@return 
		@note you should add custom media type/file paths at your plug-in installing time
		*/
		virtual bool	addMediaFilePath(index_t mediaTypeIndex,const TString& path) = 0;
		inline bool		addMediaFilePath(const TString& type,const TString& path)
		{
			const MEDIA_TYPE& mt = this->getMediaType(type);
			return this->addMediaFilePath(mt.mIndex,path);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	clearImageCaches() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			getMediaFileInfo(const TString& type, const TString& mediaFileName,
			ISelectionReceiver& receiver) = 0;

	};//class IMediaLibrary
	

	extern template class BLADE_FRAMEWORK_API Factory<IMediaLibrary>;
	extern template class BLADE_FRAMEWORK_API Factory<IMediaLibrary::IMediaPreviewer>;

}//namespace Blade



#endif // __Blade_IMediaLibrary_h__