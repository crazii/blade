/********************************************************************
	created:	2014/02/15
	filename: 	BPKApplication.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "BPKApplication.h"
#include <BladeStdTypes.h>
#include <utility/BPKFile.h>
#include <utility/StringHelper.h>
#include <utility/StringList.h>
#include <interface/ILog.h>
#include <interface/IResourceManager.h>
#include <interface/IPlatformManager.h>
#include <iomanip>

namespace Blade
{
	class BPKCmdOutput : public IBPKCallback
	{
	public:
		virtual ~BPKCmdOutput()	{}
#define SINGLE_LINE 0

#if SINGLE_LINE
		BPKCmdOutput(bool progress) :mMaxLength(0),mShowProgress(progress){}
		/* @brief  */
		virtual void onProgress(const TString& name, scalar percent)
		{
			if( mShowProgress )
				std::tcout << TEXT("\r") << std::setiosflags(std::ios::fixed) << std::setprecision(3) << percent << TEXT(" : ");

			std::tcout << name.c_str() << std::setw(mMaxLength) << TEXT(" ");
			mMaxLength = std::max(mMaxLength, name.size());
		}
	protected:
		size_t mMaxLength;
#else
		BPKCmdOutput(bool progress) :mShowProgress(progress){}
		/* @brief  */
		virtual void onProgress(const TString& name, scalar percent)
		{
			if( mShowProgress )
				std::tcout << std::setiosflags(std::ios::fixed) << std::setprecision(3) << percent << TEXT(":");
			std::tcout << name.c_str() << std::endl;
		}
	protected:
		TString	mLastFile;
#endif
		bool	mShowProgress;
	};

	class BPKFileFilter : public IBPKFilter
	{
	public:
		BPKFileFilter(TStringTokenizer& tokenizer)
		{
			mFilters.reserve(tokenizer.size());
			for (size_t i = 0; i < tokenizer.size(); ++i)
				mFilters.push_back(tokenizer[i]);
		}
		virtual ~BPKFileFilter() {}

		/** @brief  */
		virtual bool filt(const TString& file)
		{
			for (size_t i = 0; i < mFilters.size(); ++i)
			{
				if (TStringHelper::wildcardMatch(mFilters[i], file))
					return false;
			}
			return true;
		}
	protected:
		TStringList mFilters;
	};

	//////////////////////////////////////////////////////////////////////////
	BPKApplication::BPKApplication()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BPKApplication::~BPKApplication()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKApplication::run(bool /*bModal*/)
	{
		const TStringList& files = ICmdParser::getSingleton().getFileList();
		const TString& destPath = ICmdParser::getSingleton().getOption(BTString("destpath"))->getValue();
		TString rootdir = ICmdParser::getSingleton().getOption(BTString("rootdir"))->getValue();
		const TString& output = ICmdParser::getSingleton().getOption(BTString("output"))->getValue();	
		const TString& filters = ICmdParser::getSingleton().getOption(BTString("filter"))->getValue();
		bool appendMode = ICmdParser::getSingleton().getOption(BTString("append"))->getValue();
		bool showProgress = ICmdParser::getSingleton().getOption(BTString("progress"))->getValue();
		bool verbose = ICmdParser::getSingleton().getOption(BTString("verbose"))->getValue();

		//no input files
		if(files.size() == 0)
			return true;

		BPKFile bpk;

		BPKCmdOutput cmdCallback(showProgress);
		IBPKCallback* cb = verbose ? &cmdCallback : NULL;

		TStringTokenizer tokenizer;
		tokenizer.tokenize(filters, TEXT("\"\""), TEXT(";"), false);
		BPKFileFilter fileFilter(tokenizer);
		IBPKFilter* filter = tokenizer.size() > 0 ? &fileFilter : NULL;

		HSTREAM stream = IResourceManager::getSingleton().openStream(output, true, IStream::AM_READWRITE);
		if( bpk.openFile(stream) )
		{
			if( !appendMode )
			{
				if( !bpk.eraseAll() )
				{
					assert(false);
					return false;
				}
			}

			const TString& cwd = IPlatformManager::getSingleton().getCurrentWorkingPath();
			if (rootdir == BTString("./"))
				rootdir = cwd;

			for(size_t i = 0; i < files.size(); ++i)
			{
				TString source = TStringHelper::standardizePath(files[i]);
				if (!TStringHelper::isAbsolutePath(source))
					source = rootdir + BTString("/") + source;

				TString dest = destPath;

				HFILEDEVICE folder = IResourceManager::getSingleton().loadFolder(source);
				stream = IResourceManager::getSingleton().loadStream(source);
				bool isFolder = folder != NULL && folder->isLoaded();

				if( dest == TString::EMPTY )
				{
					dest = source;

					TString dir, file;
					TStringHelper::getFilePathInfo(dest, dir, file);
					if( !TStringHelper::isAbsolutePath(dest))
					{
						if( !isFolder )
							dest = dir;
					}
					else
						dest = file;
				}
				dest = TStringHelper::standardizePath(BTString("/") + dest);
				bool result;
				if (isFolder)
					result = bpk.addEntry(folder, dest, filter, cb);
				else
					result = bpk.addEntry(stream, BPKA_READONLY, dest, true, filter, cb) != NULL;

				if(!result)
					BLADE_LOGU(ToolLog::Error, BTString("package '") + source + BTString("' failed.") );
			}
		}
		else
			BLADE_LOGU(ToolLog::Error, BTString("open file failed.") );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKApplication::doInitialize()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	BPKApplication::initPlugins()
	{

	}
	
}//namespace Blade