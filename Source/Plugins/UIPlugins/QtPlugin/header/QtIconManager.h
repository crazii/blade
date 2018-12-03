/********************************************************************
	created:	2016/07/23
	filename: 	QtIconManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtIconManager_h__
#define __Blade_QtIconManager_h__
#include <interface/public/ui/IIconManager.h>

namespace Blade
{
	class QtIcon : public QIcon, public Allocatable
	{
	public:
		QtIcon() :QIcon() {}
		QtIcon(const QIcon& icon) :QIcon(icon){}
		QtIcon(const QPixmap& pm) :QIcon(pm) {}
		virtual ~QtIcon() {}
	};

	class QtIconManager : public IIconManager, public Singleton<QtIconManager>
	{
	public:
		using Singleton<QtIconManager>::getSingleton;
		using Singleton<QtIconManager>::getSingletonPtr;
	protected:
		typedef		Vector<size_t>			SizeList;
		typedef		List<QtIcon*>			IconList;
		typedef	Map<TString, IconIndex, FnTStringNCLess>	ExtensionMap;	//no case on extension
	public:
		QtIconManager();
		~QtIconManager();

		/************************************************************************/
		/* IIconManager interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void		initialize(const TString& path);

		/** @brief  */
		virtual const TString& getIconPath() const { return mIconPath; }

		/** @brief  */
		virtual IconIndex	loadSingleIconImage(const TString& image);

		/** @brief  */
		virtual bool		loadIconsImage(const TString& image, size_t iconSize, size_t validCount);

		/** @brief  */
		virtual IconIndex	loadIconForFileType(const TString& extension);

		/** @brief  */
		virtual size_t		getIconCount() const;

		/** @brief  */
		virtual size_t		getIconSize(ICONSIZE is) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool		fillIcon(QIcon* target, const HIMAGE& image);
		/** @brief  */
		bool		fillIcon(QIcon* target, const TString& image);

		/** @brief  */
		QIcon*	getIcon(IconIndex index)
		{
			if(index >= mIconList.size())
				return NULL;
			IconList::const_iterator i = mIconList.begin();
			std::advance(i, index);
			return *i;
		}

	protected:

		TString			mIconPath;
		SizeList		mSizeList;
		IconList		mIconList;
		ExtensionMap	mExtIcon;
		QFileIconProvider mProvider;
	};
	
}//namespace Blade


#endif // __Blade_QtIconManager_h__