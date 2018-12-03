/********************************************************************
	created:	2014/04/02
	filename: 	MemoryFileDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_MemoryFileDevice_h__
#define __Blade_MemoryFileDevice_h__
#include <interface/public/file/IFileDevice.h>
#include <utility/IOBuffer.h>
#include <utility/MemoryStream.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class MemoryFileDevice : public IFileDevice , public Allocatable
	{
	public:
		static const TString MEMORY_FILE_TYPE;
	public:
		MemoryFileDevice();
		~MemoryFileDevice();

		/************************************************************************/
		/* IDevice interfaces                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update()		{return true;}

		/************************************************************************/
		/* IFileDevice interfaces                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReadOnly() const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			existFile(const TString& filepathname) const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			openFile(const TString& filepathname,IStream::EAccessMode mode = IStream::AM_READ) const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			createFile(const TString& filepathname, IStream::EAccessMode mode) noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteFile(const TString& filepathname) noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			findFile(TStringParam& result, const TString& pattern, int findFlag = FF_DIR|FF_FILE);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			createDirectory(const TString& name, bool bRecursive = false);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteDirectory(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			existDirectory(const TString& name) const noexcept;

	protected:

		/** @brief internal memory node */
		class MemoryNode : public Allocatable
		{
		public:
			typedef Map<TString, MemoryNode*> SubFolder;
			typedef Map<TString, MemoryStream*> FileList;
		public:
			TString mName;
			SubFolder	mSubFolders;
			//file prototypes
			FileList	mFiles;

			inline void clear()
			{
				//clear sub folders
				for(SubFolder::iterator i = mSubFolders.begin(); i != mSubFolders.end(); ++i)
					BLADE_DELETE i->second;
				//delete prototypes
				for(FileList::iterator i = mFiles.begin(); i != mFiles.end(); ++i)
					BLADE_DELETE i->second;
			}

			MemoryNode(const TString& name) :mName(name)	{}
			~MemoryNode()									{this->clear();}
			
		};

		/** @brief  */
		MemoryNode*		locateParent(const TString& path, TString& name, TString* prefix = NULL) const;
		/** @brief  */
		void			findFileImpl(TStringParam& result, const TString& pattern, int findFlag);


	protected:
		mutable MemoryNode	mRoot;
		mutable Lock		mLock;
		bool				mLoaded;
	};//class MemoryFileDevice
}//namespace Blade


#endif // __Blade_MemoryFileDevice_h__
