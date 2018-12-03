/********************************************************************
	created:	2013/01/13
	filename: 	UIWidgetLayout.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WidgetLayout_h__
#define __Blade_WidgetLayout_h__
#include <BladeFramework.h>
#include <utility/String.h>
#include <utility/TList.h>
#include <math/Vector2i.h>
#include "UITypes.h"

namespace Blade
{
	class IXmlNode;
	class IUIWidget;

	namespace Impl
	{
		class UILayoutNodeListImpl;
	}//namespace Impl

	class BLADE_FRAMEWORK_API UILayoutNode : public Allocatable
	{
	public:
		struct SWidgetInfo
		{
			TString name;
			POINT2I	pos;
			SIZE2I	size;
			uint32	flags;
			bool operator==(const SWidgetInfo& rhs) const
			{
				return name == rhs.name;
			}
			bool operator==(const TString& rhs) const
			{
				return name == rhs;
			}
		};
		enum ELayoutType
		{
			LT_UNDEFINED,
			LT_HORIZONTAL,
			LT_VERTICAL,
		};
	public:
		UILayoutNode();
		~UILayoutNode();

		UILayoutNode& operator=(const UILayoutNode& rhs);

		/** @brief  */
		const fp32*		getSize() const { return mSize; }
		/** @brief  */
		void				setSize(float x, float y) { mSize[0] = x; mSize[1] = y; }
		/** @brief  */
		inline ELayoutType	getLayoutType() const { return mLayoutType; }
		/** @brief  */
		inline void			setLayoutType(ELayoutType type) { mLayoutType = type; }
		/** @brief  */
		inline bool			isVisible() const { return mVisible; }
		/** @brief  */
		inline void			setVisible(bool visible) { mVisible = visible; }
		/** @brief  */
		inline index_t		getActiveWidget() const { return mActiveIndex; }
		/** @brief  */
		inline bool			hasWidget(const TString& name) const { return this->findWidget(name) != INVALID_INDEX; }
		/** @brief  */
		inline bool			hasChild(const UILayoutNode* child) const { return this->findChild(child) != INVALID_INDEX; }
		/** @brief  */
		inline const TString&	getWidget(index_t index) const
		{
			const SWidgetInfo* info = this->getWidgetInfo(index);
			return info != NULL ? info->name : TString::EMPTY;
		}
		/** @brief  */
		bool			getWidgetVisibility(index_t index) const
		{
			const SWidgetInfo* info = this->getWidgetInfo(index);
			return info != NULL ? (info->flags&WMF_VISIBLE) != 0 : false;
		}

		/** @brief  */
		void			prepareWidgets(size_t count);

		/** @brief  */
		index_t			addWidget(const TString& name, POINT2I pos, SIZE2I size, uint32 flags);

		/** @brief  */
		index_t			addWidget(IUIWidget* widget);

		/** @brief  */
		bool			removeWidget(const TString& name);

		/** @brief  */
		void			clearWidgets();

		/** @brief  */
		index_t			findWidget(const TString& name) const;

		/** @brief  */
		bool			setActiveWidget(index_t index);

		/** @brief  */
		bool			setWidgetVisibility(index_t index, bool visible);

		/** @brief  */
		size_t			getWidgetCount() const;

		/** @brief  */
		const SWidgetInfo* getWidgetInfo(index_t index) const;

		/** @brief  */
		bool			getVisibleWidgets(TPointerParam<SWidgetInfo>& outList);

		/** @brief  */
		void			prepareChildren(size_t count);

		/** @brief  */
		index_t			findChild(const UILayoutNode* child) const;

		/** @brief  */
		size_t			getChildCount() const;

		/** @brief  */
		size_t			getVisibleChildCount() const;

		/** @brief  */
		UILayoutNode*	getChild(index_t index) const;

		/** @brief  */
		index_t			addChild(UILayoutNode* child);

		/** @brief  */
		index_t			insertChild(UILayoutNode* child, index_t index = 0);
		
		/** @brief  */
		bool			removeChild(index_t index);

		/** @brief  */
		bool			removeChild(UILayoutNode* child);

		/** @brief  */
		void			clearChildren();

		/** @brief  */
		SIZE2I			adjustSize(SIZE2I parentAbsSize);

	protected:
		fp32			mSize[2];		//size in rate of rang [0,1]
		ELayoutType		mLayoutType;	//horizontal or vertical
		uint16			mActiveIndex;	//real visible Widget index
		bool			mVisible;
	private:
		Pimpl<Impl::UILayoutNodeListImpl>	mData;
		friend class UIWidgetLayout;
	};//class UILayoutNode

	class IXmlNode;

	class BLADE_FRAMEWORK_API UIWidgetLayout : public Allocatable
	{
	public:
		UIWidgetLayout();
		~UIWidgetLayout();

		/** @brief  */
		inline UILayoutNode&	getRoot() { return mRoot; }
		inline const UILayoutNode&	getRoot() const { return mRoot; }
		/** @brief  */
		inline void	clear() { mRoot.clearChildren(); mRoot.clearWidgets(); }

		/*
		@describe read layout config file
		@param
		@return true if config loaded; false if no config found
		*/
		bool		load(const TString& layoutConfig);

		/*
		@describe save layout config file
		@param
		@return
		*/
		bool		save(const TString& layoutConfig) const;

		/*
		@describe walk node tree
		@param
		@return
		*/
		typedef		TPointerList<UILayoutNode> OutputBranch;

		/** @brief  */
		bool		findWidget(const TString& WidgetName, OutputBranch& outBranch) const;

		/** @brief  */
		bool		updateWidget(IUIWidget* widget);

		/** @brief  */
		bool		removeWidget(IUIWidget* widget);

		/** @brief  */
		POINT2I		getRootPosition() const { return mRootPos; }

		/** @brief  */
		void		setRootPosition(POINT2I pos) { mRootPos = pos; }

	protected:

		/** @brief  */
		bool		loadNode(IXmlNode* node, UILayoutNode& layout);
		/** @brief  */
		bool		saveNode(IXmlNode* node, const UILayoutNode& layout) const;

		mutable UILayoutNode	mRoot;
		POINT2I			mRootPos;
	};
	
}//namespace Blade

#endif//__Blade_WidgetLayout_h__