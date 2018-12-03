/********************************************************************
	created:	2016/07/27
	filename: 	QtHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtHelper_h__
#define __Blade_QtHelper_h__

namespace Blade
{
	//need unicode build to bridge with QChar
	static_assert(sizeof(QChar) == sizeof(tchar), "need unicode build, define UNICODE or _UNICODE.");

	//////////////////////////////////////////////////////////////////////////
	static inline QString TString2QString(const TString& btstr)
	{
		return QString( (QChar*)btstr.c_str(), btstr.size() );
	}

	//////////////////////////////////////////////////////////////////////////
	static inline TString QString2TString(const QString& qstr)
	{
		return TString( (tchar*)qstr.constData() );
	}

	//////////////////////////////////////////////////////////////////////////
	static inline QString TranslateQString(const QString& qstr)
	{
		return TString2QString(BTString2Lang(QString2TString(qstr)));
	}

	//////////////////////////////////////////////////////////////////////////
	static inline QStringList TranslateQStrings(const QStringList& qstrList)
	{
		QStringList ret;
		ret.reserve(qstrList.count());
		for(int i = 0; i < qstrList.count(); ++i)
			ret.append(TranslateQString(qstrList[i]));
		return ret;
	}

	//functors for None QObject derived class's member function
	template<typename T, typename V>
	struct FnMember1
	{
	public:
		typedef void (T::*memFun)(V);

		T* mTarget;
		memFun mFn;

		FnMember1(T* target, memFun fn) :mTarget(target), mFn(fn) {}

		void operator()(V v)
		{
			(mTarget->*mFn)(v);
		}
	};

	template<typename T, typename V>
	static inline FnMember1<T, V> makeMemFn(T* target, void (T::*fn)(V))
	{
		return FnMember1<T, V>(target, fn);
	}

	template<typename T>
	struct FnMember1<T, void>
	{
		typedef void (T::*memFun)(void);
		T* mTarget;
		memFun mFn;

		FnMember1(T* target, memFun fn) :mTarget(target), mFn(fn) {}

		void operator()(void)
		{
			(mTarget->*mFn)();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	static inline FnMember1<T, void> makeMemFn(T* t, void (T::*fn)(void))
	{
		return FnMember1<T, void>(t, fn);
	}

	/** @brief  */
	QImage::Format toQImageFormat(PixelFormat format);
	
	/** @brief */
	Handle<QImage> toQImage(const HIMAGE& img);

}//namespace Blade

#define QT_NEW BLADE_NEW

#endif // __Blade_QtHelper_h__