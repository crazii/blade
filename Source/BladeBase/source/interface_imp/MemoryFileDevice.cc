/********************************************************************
	created:	2014/04/02
	filename: 	MemoryFileDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "MemoryFileDevice.h"

namespace Blade
{
	const TString MemoryFileDevice::MEMORY_FILE_TYPE = BTString("MemoryFileDevice");


	//////////////////////////////////////////////////////////////////////////
	MemoryFileDevice::MemoryFileDevice()
		:mRoot( BTString("/") )
		,mLoaded(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	MemoryFileDevice::~MemoryFileDevice()
	{

	}

	/************************************************************************/
	/* IDevice interfaces                                                                     */
	/************************************************************************/
	/** @brief open a device */
	bool	MemoryFileDevice::open()
	{
		mLoaded = true;
		return true;
	}

	/** @brief check if it is open */
	bool	MemoryFileDevice::isOpen() const
	{
		return mLoaded;
	}

	/** @brief close the device */
	void	MemoryFileDevice::close()
	{
		mPathName = TString::EMPTY;
		mLoaded = false;
		mRoot.clear();
	}

	/** @brief reset the device */
	bool	MemoryFileDevice::reset()
	{
		this->close();
		return this->open();
	}

	/************************************************************************/
	/* IFileDevice interfaces                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const TString&	MemoryFileDevice::getType() const noexcept
	{
		return MEMORY_FILE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MemoryFileDevice::isReadOnly() const noexcept
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MemoryFileDevice::existFile(const TString& filepathname) const noexcept
	{
		ScopedLock lock(mLock);

		TString fileName;
		MemoryNode* node = this->locateParent(filepathname, fileName);
		return node != NULL && node->mFiles.find( fileName ) != node->mFiles.end();
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			MemoryFileDevice::openFile(const TString& filepathname,IStream::EAccessMode mode/* = IStream::AM_READ*/) const noexcept
	{
		ScopedLock lock(mLock);

		TString fileName;
		MemoryNode* node = this->locateParent(filepathname, fileName);
		if( node != NULL )
		{
			MemoryNode::FileList::iterator i = node->mFiles.find(fileName);
			if( i != node->mFiles.end() )
			{
				return HSTREAM( i->second->clone(mPathName + BTString("/") + filepathname, mode, true) );
			}
		}
		return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			MemoryFileDevice::createFile(const TString& filepathname, IStream::EAccessMode mode) noexcept
	{
		ScopedLock lock(mLock);

		TString fileName;
		MemoryNode* node = this->locateParent(filepathname, fileName);
		if( node != NULL )
		{
			std::pair<MemoryNode::FileList::iterator, bool> ret = node->mFiles.insert( std::make_pair(fileName, (MemoryStream*)NULL ) );
			if( ret.second )
			{
				//create prototype
				ret.first->second = BLADE_NEW MemoryStream(filepathname, Memory::getResourcePool() );
				return HSTREAM(ret.first->second->clone(mPathName + BTString("/") + filepathname, mode, true));
			}
		}
		return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MemoryFileDevice::deleteFile(const TString& filepathname) noexcept
	{
		ScopedLock lock(mLock);

		TString fileName;

		index_t subPos = 0;
		if(TStringHelper::isStartWith(filepathname, mPathName))
			subPos = mPathName.size();

		MemoryNode* node = this->locateParent(filepathname.substr_nocopy(subPos), fileName);
		if( node != NULL )
		{
			MemoryNode::FileList::iterator i = node->mFiles.find( fileName );
			if( i != node->mFiles.end() )
			{
				//delete MemoryStream prototype
				BLADE_DELETE i->second;
				node->mFiles.erase(i);
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			MemoryFileDevice::findFile(TStringParam& result, const TString& pattern, int findFlag/* = FF_DIR|FF_FILE*/)
	{
		ScopedLock lock(mLock);
		this->findFileImpl(result, pattern, findFlag);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MemoryFileDevice::createDirectory(const TString& name, bool bRecursive/* = false*/)
	{
		TStringTokenizer tokenizer;
		tokenizer.tokenize( name, TEXT("/\\") );
		if( tokenizer.size() == 0 )
			return false;

		ScopedLock lock(mLock);
		MemoryNode* node = &mRoot;
		//find parent node:
		for(size_t i = 0; i < tokenizer.size(); ++i)
		{
			if( tokenizer.size() == 1 || i == tokenizer.size() - 1 )
				break;

			const TString& subNodeName = tokenizer[i];
			//find sub node by subNodeName
			MemoryNode::SubFolder::const_iterator iter = node->mSubFolders.find( subNodeName );
			if( iter == node->mSubFolders.end() )
			{
				if( !bRecursive )
					return false;
				MemoryNode* newNode = BLADE_NEW MemoryNode(subNodeName);
				iter = node->mSubFolders.insert( std::make_pair(subNodeName, newNode) ).first;
			}
			node = iter->second;
		}

		if( node == NULL )
			return false;

		const TString& dirName = tokenizer[ tokenizer.size() - 1];
		std::pair<MemoryNode::SubFolder::iterator, bool> ret = node->mSubFolders.insert( std::make_pair(dirName, (MemoryNode*)NULL) );
		//already exist
		if( !ret.second )
			return !ret.second;
		//add new node
		ret.first->second = BLADE_NEW MemoryNode(dirName);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MemoryFileDevice::deleteDirectory(const TString& name)
	{
		ScopedLock lock(mLock);

		TString dirName;
		MemoryNode* node = this->locateParent(name, dirName);
		if( node != NULL )
		{
			MemoryNode::SubFolder::iterator i = node->mSubFolders.find( dirName );
			if( i != node->mSubFolders.end() && i->second->mFiles.size() != 0 && i->second->mSubFolders.size() != 0 )
			{
				BLADE_DELETE i->second;
				node->mSubFolders.erase(i);
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MemoryFileDevice::existDirectory(const TString& name) const noexcept
	{
		ScopedLock lock(mLock);

		TString dirName;
		MemoryNode* node = this->locateParent(name, dirName);
		return node != NULL && node->mSubFolders.find( dirName ) != node->mSubFolders.end();
	}

	//////////////////////////////////////////////////////////////////////////
	MemoryFileDevice::MemoryNode*	MemoryFileDevice::locateParent(const TString& path, TString& name, TString* prefix/* = NULL*/) const
	{
		if( path == TString::EMPTY )
			return NULL;

		TStringTokenizer tokenizer;
		tokenizer.tokenize( path, TEXT("/\\") );
		if( tokenizer.size() == 0 )
			return NULL;

		MemoryNode* node = &mRoot;

		//find parent node:
		for(size_t i = 0; i < tokenizer.size(); ++i)
		{
			if( tokenizer.size() == 1 || i == tokenizer.size() - 1 )
				break;

			const TString& subNodeName = tokenizer[i];
			//find sub node by subNodeName
			MemoryNode::SubFolder::const_iterator iter = node->mSubFolders.find( subNodeName );
			if( iter == node->mSubFolders.end() )
				return NULL;
			node = iter->second;
		}

		name = tokenizer[ tokenizer.size() - 1];

		if( prefix != NULL )
		{
			TStringConcat concat( tokenizer[0] );
			for(size_t i = 1; i < tokenizer.size()-1; ++i)
				concat += BTString("/")+ tokenizer[i];
			*prefix = concat;
		}
		return node;
	}

	//////////////////////////////////////////////////////////////////////////
	void	MemoryFileDevice::findFileImpl(TStringParam& result, const TString& pattern, int findFlag)
	{
		if( (findFlag&(FF_DIR|FF_FILE)) == 0 )
			return;

		TString directory;
		TString fileWildcard;
		TStringHelper::getFilePathInfo(pattern, directory, fileWildcard);
		TString dummy;
		MemoryNode* node = this->locateParent( directory + BTString("/*"), dummy);
		if( node != NULL )
		{
			for(MemoryNode::SubFolder::const_iterator i = node->mSubFolders.begin(); i != node->mSubFolders.end(); ++i)
			{
				MemoryNode* subNode = i->second;
				TStringConcat nodeFullPath = directory + BTString("/") + subNode->mName; 

				if( (findFlag&FF_DIR) && TStringHelper::wildcardMatch(fileWildcard, subNode->mName) )
					result.push_back( nodeFullPath );

				if( (findFlag&FF_RECURSIVE) )
					this->findFileImpl(result, nodeFullPath + BTString("/") + fileWildcard, findFlag);
			}
			
			for(MemoryNode::FileList::const_iterator i = node->mFiles.begin(); i != node->mFiles.end(); ++i)
			{
				MemoryStream* stream = i->second;
				const TString& nodeFullPath = stream->getName();
				TString prefix, fileName;
				TStringHelper::getFilePathInfo(stream->getName(), prefix, fileName);
#if BLADE_DEBUG
				assert( nodeFullPath == directory + BTString("/") + fileName );
#endif
				if( (findFlag&FF_FILE) && TStringHelper::wildcardMatch(fileWildcard, fileName) )
					result.push_back( nodeFullPath );
			}
		}
	}
	
}//namespace Blade
