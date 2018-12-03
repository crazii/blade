/********************************************************************
	created:	2016/6/2
	filename: 	IUIWidget.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IUIWidget_h__
#define __Blade_IUIWidget_h__
#include <BladeFramework.h>
#include "UITypes.h"
#include <Factory.h>

namespace Blade
{
	class IUIWidget
	{
	public:
		virtual ~IUIWidget() {}

		/** @brief  */
		virtual const TString&	getWidgetType() const = 0;

		/** @brief  */
		virtual bool		initWidget(const WIDGET_DATA& data) = 0;

		/** @brief  */
		virtual bool		setParent(IUIWidget* parent) = 0;
		/** @brief  */
		virtual IUIWidget*	getParent() const = 0;

		/** @brief  */
		virtual const WIDGET_DATA& getWidgetData() const = 0;

		/** @brief  */
		virtual size_t		getChildCount() const = 0;

		/** @brief  */
		virtual IUIWidget*	getChild(index_t index) const = 0;

		/** @brief  */
		virtual IUIWidget*	getChild(const TString& name) const = 0;

		/** @brief  */
		virtual bool		addChild(IUIWidget* child) = 0;

		/** @brief  */
		virtual bool		removeChild(const TString& name) = 0;

		/** @brief  */
		virtual void		setCaption(const TString& caption) { this->getWidgetDataW().mCaption = caption; }

		/** @brief  */
		virtual void		setPosition(POINT2I pos) { this->getWidgetDataW().mPos = pos; }

		/** @brief  */
		virtual void		setSize(SIZE2I size) { this->getWidgetDataW().mSize = size; }

		/** @brief  */
		virtual void		setFlag(uint32 flag) { this->getWidgetDataW().mFlags = flag; }

		/** @brief  */
		virtual bool		setVisible(bool visibile)
		{
			uint32 flag = this->getFlag();
			uint32 nflag = flag;
			nflag |= visibile ? WMF_VISIBLE : 0;
			nflag &= visibile ? uint32(-1) : (~WMF_VISIBLE);
			if (nflag != flag)
			{
				this->setFlag(nflag);
				return true;
			}
			return false;
		}

		/** @brief get unique name */
		virtual const TString&	getName() const { return this->getWidgetData().mName; }
		inline const TString&	getCaption() const { return this->getWidgetData().mCaption; }
		inline IconIndex	getIcon() const { return this->getWidgetData().mIcon; }
		inline POINT2I		getPosition() const { return this->getWidgetData().mPos; }
		inline SIZE2I		getSize() const { return this->getWidgetData().mSize; }
		inline uint32		getFlag() const { return this->getWidgetData().mFlags; }

		inline bool	removeChild(IUIWidget* child)
		{
			return child != NULL && this->removeChild(child->getName());
		}
		inline uint32		getDockFlag() const
		{
			return (this->getFlag() & WDF_MASK);
		}
		inline bool			isFixedSize() const
		{
			return (this->getFlag() & WMF_FIXED) != 0;
		}
		inline bool			isFloating() const
		{
			return (this->getDockFlag() & WDF_FLOATING) != 0;
		}
		inline bool			setFloating(bool floating)
		{
			uint32 flag = this->getFlag();
			if (!(flag&WDF_FLOATABLE))
				return false;

			uint32 nflag = flag;
			nflag |= floating ? WDF_FLOATING : 0;
			nflag &= floating ? uint32(-1) : (~WDF_FLOATING);
			if (nflag != flag)
			{
				this->setFlag(nflag);
				return true;
			}
			return false;
		}
		inline bool			setActive(bool active)
		{
			uint32 flag = this->getFlag();
			uint32 nflag = flag;
			nflag |= active ? WMF_ACTIVE : 0;
			nflag &= active ? uint32(-1) : (~WMF_ACTIVE);
			if (nflag != flag)
			{
				this->setFlag(nflag);
				return true;
			}
			return false;
		}
		inline bool			isActive() const
		{
			return (this->getFlag()&WMF_ACTIVE) != 0;
		}
		inline bool		isVisible() const
		{ 
			return (this->getFlag()&WMF_VISIBLE) != 0;
		}

	protected:

		/** @brief get writable data */
		virtual WIDGET_DATA& getWidgetDataW() = 0;

	};//class IUIWidget

	struct FnWidgetNameLess
	{
		inline bool operator()(const IUIWidget* lhs, const IUIWidget* rhs) const
		{
			return FnTStringFastLess::compare(lhs->getName(), rhs->getName());
		}
	};

	class IUIWidgetLeaf : public IUIWidget
	{
	public:
		virtual ~IUIWidgetLeaf() {}

		/** @brief  */
		virtual size_t		getChildCount() const { return 0; }

		/** @brief  */
		virtual IUIWidget*	getChild(index_t /*index*/) const { return NULL; }

		/** @brief  */
		virtual IUIWidget*	getChild(const TString& /*name*/) const { return NULL; }

		/** @brief  */
		virtual bool		addChild(IUIWidget* /*child*/) { return false; }

		/** @brief  */
		virtual bool		removeChild(const TString& /*name*/) { return false; }
	};

	class UIWidgetFinder : public IUIWidgetLeaf, public NonAllocatable, public NonAssignable
	{
	public:
		const TString& mNameRef;
		UIWidgetFinder(const TString& name) :mNameRef(name) {}

		/** @brief  */
		virtual const TString&	getWidgetType() const { return TString::EMPTY; }

		/** @brief  */
		virtual bool		initWidget(const WIDGET_DATA& /*data*/)
		{ return false; }

		/** @brief  */
		virtual bool		setParent(IUIWidget* /*parent*/) { return false; }

		/** @brief  */
		virtual IUIWidget*	getParent() const { return NULL; }

		/** @brief  */
		virtual const WIDGET_DATA& getWidgetData() const { BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("should not go here")); }

		/** @brief  */
		virtual WIDGET_DATA& getWidgetDataW() { BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("should not go here")); }

		/** @brief  */
		virtual const TString&	getName() const { return mNameRef; }
	};

	extern template class BLADE_FRAMEWORK_API Factory<IUIWidget>;
	typedef Factory<IUIWidget> UIWidgetFactory;
	
}//namespace Blade

#endif//__Blade_IUIWidget_h__