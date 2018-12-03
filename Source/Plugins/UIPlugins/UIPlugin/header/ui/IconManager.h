/********************************************************************
	created:	2010/05/23
	filename: 	IconManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IconManager_h__
#define __Blade_IconManager_h__
#include <interface/public/ui/IIconManager.h>
#include <atlimage.h>

namespace Blade
{
	class UIImageList : public CImageList, public Allocatable
	{
	public:
		using Allocatable::operator new;
		using Allocatable::operator delete;
		using Allocatable::operator new[];
		using Allocatable::operator delete[];
	};

	class IconManager : public IIconManager , public Singleton<IconManager>
	{
	public:
		using Singleton<IconManager>::getSingleton;
		using Singleton<IconManager>::getSingletonPtr;
	public:
		IconManager();
		~IconManager();

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
		/* internal custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		CImageList*			getIconList(ICONSIZE iconSize, bool grayed = false);

		/** @brief  */
		HBITMAP				createBitmap(const HIMAGE& img);

		/** @brief  */
		void				shutdown();

	protected:

		/** @brief  */
		HBITMAP				createGrayedIcon(const HIMAGE& img);

		typedef		Vector<CImageList*>		SizeImageList;
		typedef		Vector<size_t>			SizeList;
		typedef		Vector<CImage*>			Images;
		typedef	Map<TString, IconIndex, FnTStringNCLess>	ExtensionMap;	//no case on extension

		TString			mIconPath;
		SizeImageList	mSizeIconList;
		SizeImageList	mGrayedIconList;

		SizeList		mSizeList;
		ExtensionMap	mExtIcon;
	};//class IconManager
	
}//namespace Blade


#endif //__Blade_IconManager_h__