/********************************************************************
	created:	2016/07/25
	filename: 	QtPropertyHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtPropertyHelper_h__
#define __Blade_QtPropertyHelper_h__

class QtAbstractPropertyBrowser;
class QtBrowserItem;
class QtProperty;

class QtDoublePropertyManager;
class QtStringPropertyManager;
class QtColorPropertyManager;
class QtBoolPropertyManager;
class QtEnumPropertyManager;
class FilePathManager;

class QtDoubleEditSliderFactory;
class QtCheckBoxFactory;
class QtIntEditSliderFactory;
class QtLineEditFactory;
class QtColorEditorFactory;
class QtEnumEditorFactory;
class FileEditFactory;
class QtEnumSliderFactory;
struct QColorHDR;

namespace Blade
{
	class QtPropertyHelper : public NonAssignable, public NonAllocatable, public QObject
	{
	protected:
		typedef Map<QtProperty*, IConfig*> PropertyMap;
	public:
		QtPropertyHelper(QWidget* parent);
		~QtPropertyHelper();

		/** @brief  */
		void initialize(QtAbstractPropertyBrowser* browser);

		/** @brief  */
		QtProperty*	createProperty(IConfig* config, IConfig* parentConfig);

		/** @brief  */
		void clear();

		/************************************************************************/
		/* standalone mode                                                                     */
		/************************************************************************/
		/** @brief  */
		void initialize();

		/** @brief  */
		QWidget* createEditor(IConfig* config, QWidget* parent);


		/************************************************************************/
		/* helpers                                                                     */
		/************************************************************************/
		/** @brief  */
		inline IConfig*	getConfig(QtProperty* prop) const
		{
			PropertyMap::const_iterator i = mMap.find(prop);
			if (i != mMap.end())
				return i->second;
			//assert(mPropCreating); //maybe auto created sub property
			return NULL;
		}
		
	protected:
		/** @brief  */
		virtual void timerEvent(QTimerEvent *evt);
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt);
		/** @brief  */
		void currentItemChanged(QtBrowserItem *);
		/** @brief  */
		void propertyRemoved(QtProperty* prop, QtProperty* parent);
		/** @brief  */
		void imageCallback(QtProperty* prop, QString& path, bool& done);
		/** @brief  */
		void listChanged(QtProperty *prop, int val);
		/** @brief  */
		void checkChanged(QtProperty *prop, bool val);
		/** @brief  */
		void colorChanged(QtProperty* prop, const QColorHDR &val);
		/** @brief  */
		void editChanged(QtProperty* prop, const QString& val);
		/** @brief  */
		void rangeChanged(QtProperty* prop, double value);
		/** @brief  */
		void fileChanged(QtProperty* prop, const QString& file);
		/** @brief  */
		void updateCollection(IConfig* config, QtProperty* prop);
		/** @brief  */
		void updateGroup(IConfig* config, QtProperty* prop);
		/** @brief  */
		void checkGroupChange(IConfig* config, QtProperty* prop);
		/** @brief recursively remove a property, browser can recursively remove a top level property but not for sub properties */
		static void removeProperty(QtProperty* prop, QtProperty* parent);

		QWidget*					mParent;
		QtAbstractPropertyBrowser*	mBrowser;
		QtDoublePropertyManager*	mDoubleManager;
		QtStringPropertyManager*	mStringManager;
		QtColorPropertyManager*		mColorManager;
		QtBoolPropertyManager*		mBoolManager;
		QtEnumPropertyManager*		mEnumManager;
		FilePathManager*			mFilePathManager;
		QtEnumPropertyManager*		mRangedEnumManager;

		QtDoubleEditSliderFactory*	mDoubleEditSliderFactory;
		QtLineEditFactory*			mLineEditFactory;
		QtCheckBoxFactory*			mCheckBoxFactory;
		QtColorEditorFactory*		mColorFactory;
		QtEnumEditorFactory*		mComboBoxFactory;
		FileEditFactory*			mFileEditFactory;
		QtEnumSliderFactory*		mSliderFactory;
		QtIntEditSliderFactory*		mIntEditSliderFactory;

		PropertyMap					mMap;
		PropertyMap					mGroupMap;	//for group update
		int							mTimerID;
		bool						mPropCreating;
		bool						mPropUpdating;
		bool						mInited;
	};//class QtPropertyHelper

	
}//namespace Blade


#endif // __Blade_QtPropertyHelper_h__