/********************************************************************
	created:	2011/08/22
	filename: 	MediaLibrary.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "MediaLibrary.h"
#include <ExceptionEx.h>
#include <interface/IResourceManager.h>
#include <interface/public/IStartupOutput.h>
#include <BladeFramework_blang.h>

namespace Blade
{
	template class Factory<IMediaLibrary>;
	template class Factory<IMediaLibrary::IMediaPreviewer>;

	static const size_t INIT_MEDIA_COUNT = 16;
	static const size_t INIT_PATH_COUNT = 16;
	static const size_t MEDIA_PREVIEW_SIZE = 256;

	//////////////////////////////////////////////////////////////////////////
	MediaLibrary::MediaLibrary()
		:mInitialized(false)
	{
		mMediaPaths.reserve(INIT_MEDIA_COUNT);
	}

	//////////////////////////////////////////////////////////////////////////
	MediaLibrary::~MediaLibrary()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	MediaLibrary::initialize()
	{
		if( mInitialized )
		{
			assert(false);
			return false;
		}

		IStartupOutput* output = NULL;
		if (Factory<IStartupOutput>::getSingleton().getNumRegisteredClasses() > 0)
			output = IStartupOutput::getSingletonPtr();

		mFiles.resize( mMediaTypes.size() );

		//read files from paths and sort them
		for(size_t mediaTypeIndex = 0; mediaTypeIndex < mMediaTypes.size(); ++mediaTypeIndex )
		{
			TStringList& pathList = mMediaPaths[mediaTypeIndex];
			for(size_t n = 0; n < pathList.size(); ++n )
			{
				const TString& path = pathList[n];

				//setup file table: find & record all files.
				HFILEDEVICE hFolder = IResourceManager::getSingleton().loadFolder( path );

				const MEDIA_TYPE& mediaType = mMediaTypes[mediaTypeIndex];
				FileList& list = mFiles[mediaTypeIndex];

				if (hFolder == NULL)
					continue;

				//calculate total size
				size_t totalSize = 0;
				for( size_t i = 0; i < mediaType.mExtensions.size(); ++i )
				{
					TStringParam files;
					hFolder->findFile(files, BTString("*.")+mediaType.mExtensions[i], IFileDevice::FF_FILE);
					totalSize += files.size();
				}
				list.reserve( totalSize );

				//all supported file types
				for( size_t i = 0; i < mediaType.mExtensions.size(); ++i )
				{
					TStringParam files;
					hFolder->findFile(files, BTString("*.")+mediaType.mExtensions[i], IFileDevice::FF_FILE);

					//process every file
					for(size_t j = 0; j < files.size(); ++j)
					{
						const TString& filePath = files[j];
						const TString& schemePath = path + BTString("/") + files[j];
						bool isValid = mediaType.mFilter == NULL ? true : mediaType.mFilter->filterFile(schemePath);
						if( !isValid )
							continue;

						list.push_back(MediaFile());
						MediaFile& file = *list.rbegin();
						file.mSchemePath = schemePath;
						hFolder->getFullPath(filePath,file.mRawPath);

						TString dir,filename;
						TStringHelper::getFilePathInfo(filePath,dir,filename);
						file.mName = filename;

						if(output != NULL)
							output->outputProgress(BTLang(BLANG_CACHE_MEDIA) + BTString(" \"")+ file.mSchemePath + BTString("\"..."));

						//TODO: hard code for now, maybe need preview icon size config
						HIMAGE img = mediaType.mPreviewer->generatePreview(schemePath, MEDIA_PREVIEW_SIZE, IP_DEFAULT);
						file.mPreview = img;
					}
				}

				std::sort(list.begin(),list.end());
				for(size_t i = 0; i < list.size(); ++i)
					list[i].mIndex = i;

			}

		}

		mInitialized = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MediaLibrary::shutdown()
	{
		if( !mInitialized )
			return false;

		mMediaTypes.clear();
		mMediaPaths.clear();
		mFiles.clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const IMediaLibrary::MEDIA_TYPE&	MediaLibrary::getMediaTypeByIndex(index_t index) const
	{
		if( index < mMediaTypes.size() )
			return  mMediaTypes[index];
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range."));
	}

	//////////////////////////////////////////////////////////////////////////
	struct FnMediaTypeFinder
	{
		FnMediaTypeFinder(const TString& type) :mTypeRef(type)	{}

		bool	operator()(const IMediaLibrary::MEDIA_TYPE& target) const
		{
			return target.mType == mTypeRef;
		}
		const TString& mTypeRef;
	private:
		FnMediaTypeFinder&	operator=(const FnMediaTypeFinder&)	{return *this;}
	};
	const IMediaLibrary::MEDIA_TYPE&	MediaLibrary::getMediaType(const TString& type) const
	{
		MediaTypeList::const_iterator i = std::find_if(mMediaTypes.begin(),mMediaTypes.end(),FnMediaTypeFinder(type) );
		if( i == mMediaTypes.end() )
			BLADE_EXCEPT(EXC_NEXIST,BTString("media type \"") + type + BTString("\" not found."));
		else
			return *i;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	MediaLibrary::getMediaPaths(index_t mediaTypeIndex,TStringParam& outPathList) const
	{
		if( mediaTypeIndex >= mMediaTypes.size() )
			return 0;
		else
		{
			assert( mediaTypeIndex < mMediaPaths.size() );

			const TStringList& PathList = mMediaPaths[mediaTypeIndex];
			for(size_t i = 0; i < PathList.size(); ++i )
				outPathList.push_back(PathList[i]);
			
			return outPathList.size();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	MediaLibrary::getMediaFileCount(index_t mediaTypeIndex) const
	{
		if( mediaTypeIndex >= mMediaTypes.size() )
			return 0;
		assert( mediaTypeIndex < mFiles.size() );

		return mFiles[mediaTypeIndex].size();
	}

	//////////////////////////////////////////////////////////////////////////
	const IMediaLibrary::MediaFile*
		MediaLibrary::getMediaFile(index_t mediaTypeIndex,index_t index) const
	{
		if( mediaTypeIndex >= mMediaTypes.size() )
			return NULL;

		assert( mediaTypeIndex < mFiles.size() );

		if( index >= mFiles[mediaTypeIndex].size() )
			return NULL;

		return &( mFiles[mediaTypeIndex][index] );
	}

	//////////////////////////////////////////////////////////////////////////
	const IMediaLibrary::MediaFile*
		MediaLibrary::getMediaFile(index_t mediaTypeIndex,const TString& mediaFileName) const
	{
		if( mediaTypeIndex >= mMediaTypes.size() )
			return NULL;

		assert( mediaTypeIndex < mFiles.size() );
		const FileList& list = mFiles[mediaTypeIndex];

		MediaFile finder;
		TString dir;
		//get the file name without directory
		TStringHelper::getFilePathInfo(mediaFileName,dir,finder.mName);
		//FileList::const_iterator i = std::lower_bound(list.begin(),list.end(),finder);
		FileList::const_iterator i = std::find(list.begin(), list.end(), finder);
		if( i != list.end() )
			return &(*i);
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t	MediaLibrary::registerMediaType(const MEDIA_TYPE& mediaType,const TString& path/* = TString::EMPTY*/)
	{
		MediaTypeList::iterator i = std::find(mMediaTypes.begin(), mMediaTypes.end(), mediaType);

		if( i != mMediaTypes.end() )
			return INVALID_INDEX;

		mMediaTypes.push_back(mediaType);
		mMediaPaths.push_back(TStringList());
		mMediaPaths.back().reserve(INIT_PATH_COUNT);

		index_t index = mMediaTypes.size() - 1;
		(*mMediaTypes.rbegin()).mIndex = index;

		if( path != TString::EMPTY )
			this->addMediaFilePath(index,path);

		return index;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MediaLibrary::addMediaFilePath(index_t mediaTypeIndex,const TString& path)
	{
		if( mediaTypeIndex >= mMediaTypes.size() )
			return false;

		assert( mediaTypeIndex < mMediaPaths.size() );

		TStringList& pathList = mMediaPaths[mediaTypeIndex];

		if( pathList.find(path) != INVALID_INDEX )
			return false;

		pathList.push_back(path);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MediaLibrary::clearImageCaches()
	{
		for(size_t i = 0; i < mFiles.size(); ++i )
		{
			FileList& list = mFiles[i];
			for(size_t n = 0; n < list.size(); ++n )
			{
				MediaFile& file = list[n];
				file.mPreview.clear();
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MediaLibrary::getMediaFileInfo(const TString& type, const TString& mediaFileName,
		ISelectionReceiver& receiver)
	{
		try
		{
			const IMediaLibrary::MEDIA_TYPE& mediaType = this->getMediaType(type);
			index_t mediaIndex = mediaType.mIndex;
			const IMediaLibrary::MediaFile* file = this->getMediaFile(mediaIndex, mediaFileName);
			if (file != NULL)
			{
				receiver.setSelectedTarget(mediaIndex, *file);
				return true;
			}
			else
			{
				file = this->getMediaFile(mediaIndex, 0);
				assert(file != NULL);
				receiver.setSelectedTarget(mediaIndex, *file);
				return false;
			}
		}

		catch (...)
		{
			return false;
		}
	}

}//namespace Blade
