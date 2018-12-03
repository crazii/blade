/********************************************************************
	created:	2016/07/23
	filename: 	QtIconManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtIconManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QtIconManager::QtIconManager()
	{
		mSizeList.resize(IS_COUNT);
		mSizeList[IS_8] = 8;
		mSizeList[IS_16] = 16;
		mSizeList[IS_24] = 24;
		mSizeList[IS_32] = 32;
	}

	//////////////////////////////////////////////////////////////////////////
	QtIconManager::~QtIconManager()
	{
		for(IconList::iterator i = mIconList.begin(); i != mIconList.end(); ++i)
			BLADE_DELETE *i;
	}

	/************************************************************************/
	/* IIconManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		QtIconManager::initialize(const TString& path)
	{
		mIconPath = path;
	}

	//////////////////////////////////////////////////////////////////////////
	IconIndex	QtIconManager::loadSingleIconImage(const TString& image)
	{
		QtIcon*  icon = BLADE_NEW QtIcon();

		if( this->fillIcon(icon, image) )
		{		
			mIconList.push_back(icon);
			return (IconIndex)mIconList.size()-1u;
		}
		else
		{
			BLADE_DELETE icon;
			return INVALID_ICON_INDEX;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtIconManager::loadIconsImage(const TString& /*image*/, size_t /*iconSize*/, size_t /*validCount*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	IconIndex	QtIconManager::loadIconForFileType(const TString& extension)
	{
		//http://stackoverflow.com/questions/4617981/how-to-get-qt-icon-qicon-given-a-file-extension
		ExtensionMap::iterator iter = mExtIcon.find(extension);
		if( iter != mExtIcon.end() )
			return iter->second;

		//note: QFileIconProvider only recognize existing file icons
		QFileInfo info( TString2QString(BTString("*.") + extension) );
		QIcon icon = mProvider.icon(info);
		mIconList.push_back( BLADE_NEW QtIcon(icon) );

		IconIndex index = (IconIndex)mIconList.size()-1u;
		mExtIcon[extension] = index;
		return index;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		QtIconManager::getIconCount() const
	{
		return mIconList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		QtIconManager::getIconSize(ICONSIZE is) const
	{
		if( is >= IS_COUNT )
		{
			assert(false);
			return 0;
		}
		return mSizeList[is];
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtIconManager::fillIcon(QIcon* target, const HIMAGE& image)
	{
		Handle<QImage> qimg = toQImage(image);

		if (qimg == NULL)
		{
			assert(false);
			return false;
		}

		QPixmap pixmap;
		bool ret = pixmap.convertFromImage(*qimg);
		assert(ret);

		*target = QIcon(pixmap);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtIconManager::fillIcon(QIcon* target, const TString& image)
	{
		assert(target != NULL);

		HSTREAM stream = IResourceManager::getSingleton().loadStream(mIconPath + image);
		if (stream == NULL)
			stream = IResourceManager::getSingleton().loadStream(image);
		if (stream == NULL)
		{
			assert(false);
			return INVALID_ICON_INDEX;
		}

		HIMAGE img = IImageManager::getSingleton().loadImage(stream, IP_TEMPORARY, PO_NONE, IMGO_TOP_DOWN);
		return this->fillIcon(target, img);
	}
	
}//namespace Blade