/********************************************************************
	created:	2013/01/13
	filename: 	UIWidgetLayout.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/ui/UIWidgetLayout.h>
#include <interface/public/ui/IUIWidget.h>
#include <interface/IResourceManager.h>

namespace Blade
{
	typedef Vector<UILayoutNode*>				ChildNodeList;
	typedef Vector<UILayoutNode::SWidgetInfo>	WidgetList;

	namespace Impl
	{
		class UILayoutNodeListImpl : public Allocatable
		{
		public:
			ChildNodeList	mSubLayouts;
			WidgetList		mWidgetList;
		};
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	UILayoutNode::UILayoutNode()
		:mLayoutType(LT_UNDEFINED)
		,mActiveIndex(0)
		,mVisible(false)
		,mData(BLADE_NEW Impl::UILayoutNodeListImpl() )
	{
		mSize[0] = mSize[1] = 1;
	}

	//////////////////////////////////////////////////////////////////////////
	UILayoutNode::~UILayoutNode()
	{
		this->clearChildren();
	}

	//////////////////////////////////////////////////////////////////////////
	UILayoutNode& UILayoutNode::operator=(const UILayoutNode& rhs)
	{
		mActiveIndex = rhs.mActiveIndex;
		mLayoutType = rhs.mLayoutType;
		mVisible = rhs.mVisible;
		mSize[0] = rhs.mSize[0];
		mSize[1] = rhs.mSize[1];

		mData->mWidgetList.resize(rhs.mData->mWidgetList.size());
		for (size_t i = 0; i < rhs.mData->mWidgetList.size(); ++i)
			mData->mWidgetList[i] = rhs.mData->mWidgetList[i];

		this->clearChildren();
		mData->mSubLayouts.resize(rhs.mData->mSubLayouts.size());
		for (size_t i = 0; i < rhs.mData->mSubLayouts.size(); ++i)
		{
			mData->mSubLayouts[i] = BLADE_NEW UILayoutNode();
			*mData->mSubLayouts[i] = *rhs.mData->mSubLayouts[i];
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void			UILayoutNode::prepareWidgets(size_t count)
	{
		mData->mWidgetList.reserve(count); 
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			UILayoutNode::addWidget(const TString& name, POINT2I pos, SIZE2I size, uint32 flags)
	{
		WidgetList::iterator iter;
		if (name == TString::EMPTY
			|| (iter = std::find(mData->mWidgetList.begin(), mData->mWidgetList.end(), name)) != mData->mWidgetList.end())
		{
			iter->pos = pos;
			iter->size = size;
			iter->flags = flags;

			index_t index = (index_t)(iter - mData->mWidgetList.begin());

			//update active index
			if (!(iter->flags&WMF_VISIBLE) && mActiveIndex == index)
				if (++mActiveIndex >= mData->mWidgetList.size())
					mActiveIndex = (uint16)(mData->mWidgetList.size() != 0 ? mData->mWidgetList.size() - 1u : 0);
			return index;
		}

		SWidgetInfo info;
		info.name = name;
		info.pos = pos;
		info.size = size;
		info.flags = flags;
		mData->mWidgetList.push_back(info);
		return mData->mWidgetList.size() - 1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	index_t			UILayoutNode::addWidget(IUIWidget* widget)
	{
		if (widget == NULL)
		{
			assert(false);
			return false;
		}

		const TString& name = widget->getName();
		POINT2I pos = widget->getPosition();
		SIZE2I size = widget->getSize();
		uint32 flags = widget->getFlag();

		return this->addWidget(name, pos, size, flags);
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool			UILayoutNode::removeWidget(const TString& name)
	{
		index_t index = this->findWidget(name);
		if (index == INVALID_INDEX)
			return false;
		mData->mWidgetList.erase(mData->mWidgetList.begin() + (indexdiff_t)index);
		if (mActiveIndex == index)
		{
			if (++mActiveIndex > mData->mWidgetList.size())
				mActiveIndex = (uint16)(mData->mWidgetList.size() != 0 ? mData->mWidgetList.size() - 1u : 0);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			UILayoutNode::clearWidgets()
	{
		mData->mWidgetList.clear(); 
	}
	
	//////////////////////////////////////////////////////////////////////////
	index_t			UILayoutNode::findWidget(const TString& name) const
	{
		WidgetList::const_iterator i = std::find(mData->mWidgetList.begin(), mData->mWidgetList.end(), name);
		return (i == mData->mWidgetList.end() || name == TString::EMPTY) ? INVALID_INDEX : (i - mData->mWidgetList.begin());
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UILayoutNode::setActiveWidget(index_t index)
	{
		if (index < mData->mWidgetList.size())
		{
			mActiveIndex = (uint16)index;
			mData->mWidgetList[index].flags |= WMF_VISIBLE;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UILayoutNode::setWidgetVisibility(index_t index, bool visible)
	{
		if (index < mData->mWidgetList.size())
		{
			if(visible)
				mData->mWidgetList[index].flags |= WMF_VISIBLE;
			else
				mData->mWidgetList[index].flags &= ~WMF_VISIBLE;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			UILayoutNode::getWidgetCount() const
	{
		return mData->mWidgetList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	const UILayoutNode::SWidgetInfo* UILayoutNode::getWidgetInfo(index_t index) const
	{
		return index < mData->mWidgetList.size() ? &mData->mWidgetList[index] : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UILayoutNode::getVisibleWidgets(TPointerParam<SWidgetInfo>& outList)
	{
		for (size_t i = 0; i < mData->mWidgetList.size(); ++i)
		{
			if ((mData->mWidgetList[i].flags&WMF_VISIBLE))
				outList.push_back(&mData->mWidgetList[i]);
		}
		return outList.size() > 1;
	}

	//////////////////////////////////////////////////////////////////////////
	void			UILayoutNode::prepareChildren(size_t count)
	{
		mData->mSubLayouts.reserve(count);
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			UILayoutNode::findChild(const UILayoutNode* child) const
	{
		if (child == NULL)
			return INVALID_INDEX;
		ChildNodeList::const_iterator i = std::find(mData->mSubLayouts.begin(), mData->mSubLayouts.end(), child);
		if (i == mData->mSubLayouts.end())
			return INVALID_INDEX;
		else
			return index_t(i - mData->mSubLayouts.begin());
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		UILayoutNode::getChildCount() const
	{
		return mData->mSubLayouts.size();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		UILayoutNode::getVisibleChildCount() const
	{
		size_t n = 0;
		for (index_t i = 0; i < mData->mSubLayouts.size(); ++i)
			if (mData->mSubLayouts[i]->isVisible())
				++n;
		return n;
	}

	//////////////////////////////////////////////////////////////////////////
	UILayoutNode*		UILayoutNode::getChild(index_t index) const
	{
		return index > mData->mSubLayouts.size() ? NULL : mData->mSubLayouts[index];
	}

	//////////////////////////////////////////////////////////////////////////
	index_t				UILayoutNode::addChild(UILayoutNode* child)
	{
		ChildNodeList::iterator i;
		if (child == NULL || (i = std::find(mData->mSubLayouts.begin(), mData->mSubLayouts.end(), child)) != mData->mSubLayouts.end())
			return INVALID_INDEX;
		mData->mSubLayouts.push_back(child);
		return mData->mSubLayouts.size() - 1;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t				UILayoutNode::insertChild(UILayoutNode* child, index_t index/* = 0*/)
	{
		ChildNodeList::iterator i;
		if (child == NULL || (i = std::find(mData->mSubLayouts.begin(), mData->mSubLayouts.end(), child)) != mData->mSubLayouts.end() || index > mData->mSubLayouts.size())
		{
			assert(false);
			return INVALID_INDEX;
		}
		mData->mSubLayouts.insert(mData->mSubLayouts.begin() + (indexdiff_t)index, child);
		return index;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				UILayoutNode::removeChild(index_t index)
	{
		if (index >= mData->mSubLayouts.size())
			return false;
		UILayoutNode* node = mData->mSubLayouts[index];
		mData->mSubLayouts.erase(mData->mSubLayouts.begin() + (indexdiff_t)index);
		BLADE_DELETE node;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				UILayoutNode::removeChild(UILayoutNode* child)
	{
		if (child == NULL)
			return false;
		ChildNodeList::iterator i = std::find(mData->mSubLayouts.begin(), mData->mSubLayouts.end(), child);
		if (i != mData->mSubLayouts.end())
		{
			BLADE_DELETE *i;
			mData->mSubLayouts.erase(i);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void				UILayoutNode::clearChildren()
	{
		for (ChildNodeList::iterator i = mData->mSubLayouts.begin(); i != mData->mSubLayouts.end(); ++i)
		{
			UILayoutNode* node = *i;
			assert(node != NULL);
			BLADE_DELETE node;
		}
		mData->mSubLayouts.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	SIZE2I				UILayoutNode::adjustSize(SIZE2I parentAbsSize)
	{
		int absX = int(mSize[0] * parentAbsSize.x + 0.5f);
		int absY = int(mSize[1] * parentAbsSize.y + 0.5f);

		for (size_t i = 0; i < mData->mSubLayouts.size(); ++i)
		{
			SIZE2I adjustedSize = mData->mSubLayouts[i]->adjustSize(SIZE2I(absX, absY));
			absX = std::max(adjustedSize.x, absX);
			absY = std::max(adjustedSize.y, absY);
		}

		for (size_t i = 0; i < mData->mWidgetList.size(); ++i)
		{
			SWidgetInfo& widgetInfo = mData->mWidgetList[i];
			if ((widgetInfo.flags&WMF_FIXED))
			{
				if(this->getLayoutType() == LT_HORIZONTAL )
					absX = std::max(absX, widgetInfo.size.x);
				else
					absY = std::max(absY, widgetInfo.size.y);
			}
			else
			{
				widgetInfo.size.x = absX;
				widgetInfo.size.y = absY;
			}
		}

		for (size_t i = 0; i < mData->mSubLayouts.size(); ++i)
		{
			SIZE2I adjustedSize = mData->mSubLayouts[i]->adjustSize(SIZE2I(absX, absY));
			fp32 rateX = (fp32)adjustedSize.x / (fp32)absX;
			fp32 rateY = (fp32)adjustedSize.y / (fp32)absY;
			mData->mSubLayouts[i]->setSize(rateX, rateY);
		}

		return SIZE2I(absX, absY);
	}

	namespace LayoutString
	{
		const TString NODE_NAME = BTString("layout_node");
		const TString ATTRIB_VISIBLE = BTString("visible");
		const TString ATTRIB_SIZE = BTString("size");
		const TString ATTRIB_LAYOUT = BTString("layout");
		const TString ATTRIB_ACTIVE = BTString("active_widget");	//active index

		const TString WIDGET_NODE = BTString("widget");
		const TString WIDGET_ATTRIB_NAME = BTString("name");
		const TString WIDGET_ATTRIB_SIZE = BTString("size");
		const TString WIDGET_ATTRIB_POS = BTString("pos");
		const TString WIDGET_ATTRIB_FLAGS = BTString("flags");

		const TString DEFAULT_POS_SIZE_STRING = BTString("(0,0)");

		const TString NODE_ROOT_ATTRIB_POS = BTString("pos");
	}

	//////////////////////////////////////////////////////////////////////////
	UIWidgetLayout::UIWidgetLayout()
	{
		mRootPos = POINT2I::ZERO;
	}

	//////////////////////////////////////////////////////////////////////////
	UIWidgetLayout::~UIWidgetLayout()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::load(const TString& layoutConfig)
	{
		XmlFile file;
		HSTREAM stream = IResourceManager::getSingleton().loadStream(layoutConfig, IStream::AM_READ);
		if (stream == NULL || stream->getSize() == 0 )
			return false;

		if (!file.open(stream))
			return false;

		this->clear();
		IXmlNode* root = file.getRootNode();

		//load root attributes
		const TString* rootPos = root->getAttributeValue(LayoutString::NODE_ROOT_ATTRIB_POS);
		if (rootPos == NULL)
			rootPos = &LayoutString::DEFAULT_POS_SIZE_STRING;
		TStringStream ss;
		ss.str(*rootPos);
		ss.scanFormat(TEXT("(%d,%d)"), &mRootPos.x, &mRootPos.y);

		return this->loadNode(root, mRoot);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::save(const TString& layoutConfig) const
	{
		XmlFile file;
		HSTREAM stream = IResourceManager::getSingleton().openStream(layoutConfig, false, IStream::AM_OVERWRITE);
		if (stream == NULL)
		{
			assert(false);
			return false;
		}

		if (!file.open(stream))
		{
			assert(false);
			return false;
		}

		//save root attributes
		TStringStream ss;
		ss.format(TEXT("(%d,%d)"), mRootPos.x, mRootPos.y);
		file.getRootNode()->addAttribute(LayoutString::NODE_ROOT_ATTRIB_POS, ss.str());

		bool ret = this->saveNode(file.getRootNode(), mRoot);
		if (ret)
			file.saveXml(file.getRootNode());
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::loadNode(IXmlNode* node, UILayoutNode& layout)
	{
		//parsing Widget list
		IXmlNode::IXmlNodeList* widgets = node->getChildList(LayoutString::WIDGET_NODE);
		//one node must have at least one Widget, or the node has layout type information
		if (widgets == NULL)
		{
			assert(node->getAttributeValue(LayoutString::ATTRIB_LAYOUT) != NULL);
		}
		else
		{
			size_t count = widgets->getSize();
			layout.prepareWidgets(count);
			for (size_t i = 0; i < count; ++i)
			{
				IXmlNode* widgetNode = widgets->getAt(i);
				const TString* widgetName = widgetNode->getAttributeValue(LayoutString::WIDGET_ATTRIB_NAME);
				const TString* widgetPos = widgetNode->getAttributeValue(LayoutString::WIDGET_ATTRIB_POS);
				const TString* widgetSize = widgetNode->getAttributeValue(LayoutString::WIDGET_ATTRIB_SIZE);
				const TString* widgetFlag = widgetNode->getAttributeValue(LayoutString::WIDGET_ATTRIB_FLAGS);

				//pos
				POINT2I pos = POINT2I::ZERO;
				TStringStream ss;
				ss.str(widgetPos != NULL ? *widgetPos : LayoutString::DEFAULT_POS_SIZE_STRING);
				ss.scanFormat(TEXT("(%d,%d)"), &pos.x, &pos.y);

				//size
				POINT2I size = POINT2I::ZERO;
				ss.str(widgetSize != NULL ? *widgetSize : LayoutString::DEFAULT_POS_SIZE_STRING);
				ss.scanFormat(TEXT("(%d,%d)"), &size.x, &size.y);

				//
				uint32 flags = widgetFlag == NULL ? 0 : TStringHelper::toHex32(*widgetFlag);

				if (widgetName == NULL)
					assert(false);
				else
					layout.addWidget(*widgetName, pos, size, flags);
			}
		}

		//size
		const TString* sizeString = node->getAttributeValue(LayoutString::ATTRIB_SIZE);
		if (sizeString == NULL)
		{
			assert(false);
			sizeString = &LayoutString::DEFAULT_POS_SIZE_STRING;
		}
		TStringStream ss;
		ss.str(*sizeString);
		float size[2] = { 0, 0 };
		ss.scanFormat(TEXT("(%f,%f)"), &size[0], &size[1]);
		layout.setSize(size[0], size[1]);

		//visible
		const TString* visibleString = node->getAttributeValue(LayoutString::ATTRIB_VISIBLE);
		if (visibleString == NULL)
			assert(false);
		else
			layout.setVisible(TStringHelper::toBool(*visibleString));

		//layout type
		const TString* layoutString = node->getAttributeValue(LayoutString::ATTRIB_LAYOUT);
		if (layoutString != NULL)
			layout.setLayoutType((UILayoutNode::ELayoutType)TStringHelper::toInt(*layoutString));

		//active widget index
		const TString* activeString = node->getAttributeValue(LayoutString::ATTRIB_ACTIVE);
		if (activeString != NULL && layout.getWidgetCount() > 0)
			layout.setActiveWidget(TStringHelper::toSizeT(*activeString));

		//recursive child
		IXmlNode::IXmlNodeList* children = node->getChildList(LayoutString::NODE_NAME);
		if (children == NULL)
			return true;

		size_t count = children->getSize();
		layout.prepareChildren(count);
		for (size_t i = 0; i < count; ++i)
		{
			IXmlNode* child = children->getAt(i);
			UILayoutNode* subLayout = BLADE_NEW UILayoutNode();
			bool ret = this->loadNode(child, *subLayout);
			if (!ret)
				assert(false);
			layout.addChild(subLayout);
		}
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::saveNode(IXmlNode* node, const UILayoutNode& layout) const
	{
		if (layout.getWidgetCount() == 0)
			assert(layout.getLayoutType() != UILayoutNode::LT_UNDEFINED);

		node->setName(LayoutString::NODE_NAME);
		//visible
		node->addAttribute(LayoutString::ATTRIB_VISIBLE, TStringHelper::fromBool(layout.isVisible()));
		//layout type
		if (layout.getLayoutType() != UILayoutNode::LT_UNDEFINED)
			node->addAttribute(LayoutString::ATTRIB_LAYOUT, TStringHelper::fromInt(layout.getLayoutType()));
		//active index
		index_t activeIndex = layout.getActiveWidget();
		node->addAttribute(LayoutString::ATTRIB_ACTIVE, TStringHelper::fromUInt(activeIndex));
		//size
		TStringStream ss;
		ss.format(TEXT("(%f,%f)"), std::min(1.0f, layout.mSize[0]), std::min(1.0f, layout.mSize[1]));
		node->addAttribute(LayoutString::ATTRIB_SIZE, ss.str());

		//widgets
		for (WidgetList::const_iterator i = layout.mData->mWidgetList.begin(); i != layout.mData->mWidgetList.end(); ++i)
		{
			IXmlNode* widgetNode = node->addChild(LayoutString::WIDGET_NODE);
			widgetNode->addAttribute(LayoutString::WIDGET_ATTRIB_NAME, (*i).name);
			ss.format(TEXT("(%d,%d)"), (*i).pos.x, (*i).pos.y);
			widgetNode->addAttribute(LayoutString::WIDGET_ATTRIB_POS, ss.str());
			ss.format(TEXT("(%d,%d)"), (*i).size.x, (*i).size.y);
			widgetNode->addAttribute(LayoutString::WIDGET_ATTRIB_SIZE, ss.str());
			widgetNode->addAttribute(LayoutString::WIDGET_ATTRIB_FLAGS, TStringHelper::fromHex32((*i).flags));
		}

		//children
		for (ChildNodeList::const_iterator i = layout.mData->mSubLayouts.begin(); i != layout.mData->mSubLayouts.end(); ++i)
		{
			UILayoutNode* subLayout = *i;
			IXmlNode* child = node->addChild(LayoutString::NODE_NAME);
			bool ret = this->saveNode(child, *subLayout);
			assert(ret);
			BLADE_UNREFERENCED(ret);
		}
		return true;
	}

	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		typedef struct SNodeItem
		{
			UILayoutNode*	mNode;
			index_t		mIndex;

			SNodeItem() {}
			SNodeItem(UILayoutNode* node, index_t index)
				:mNode(node), mIndex(index) {}
		}NodeItem;

		template<typename FN>
		static bool findNode(UILayoutNode* root, UIWidgetLayout::OutputBranch& outBranch, const FN& fn)
		{
			typedef TempList< NodeItem  > NodeList;
			NodeList nodeList;
			nodeList.push_back(NodeItem(root, 0));

			bool found = false;
			while (!nodeList.empty() && !found)
			{
				NodeItem item = nodeList.back();
				nodeList.pop_back();

				UILayoutNode* node = item.mNode;
				index_t childIndex = item.mIndex;

				if (childIndex == 0)
				{
					found = fn((const UILayoutNode*)node);
					if (found)
					{
						nodeList.push_back(item);
						break;
					}
				}

				if (childIndex < node->getChildCount())
				{
					if (childIndex < node->getChildCount())
						nodeList.push_back(NodeItem(node, childIndex + 1));

					UILayoutNode* subNode = node->getChild(childIndex);
					nodeList.push_back(NodeItem(subNode, 0));
				}
			}

			outBranch.clear();
			if (found)
			{
				UILayoutNode* last = NULL;
				for (NodeList::iterator i = nodeList.begin(); i != nodeList.end(); ++i)
				{
					UILayoutNode* node = (*i).mNode;
					if (node != last)
					{
						if (last != NULL)
							assert(last->hasChild(node));
						outBranch.push_back(node);
					}
					last = node;
				}
			}
			return found;
		}

		struct FnFindWidget : public NonCopyable, public NonAssignable
		{
			FnFindWidget(const TString& WidgetName) :mWidgetNameRef(WidgetName) {}
			bool operator()(const UILayoutNode* node) const { return node->getChildCount() == 0 && node->hasWidget(mWidgetNameRef); }
			const TString& mWidgetNameRef;
		};
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::findWidget(const TString& WidgetName, OutputBranch& outBranch) const
	{
		return Impl::findNode(&mRoot, outBranch, Impl::FnFindWidget(WidgetName));
	}

	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::updateWidget(IUIWidget* widget)
	{
		if (widget == NULL)
		{
			assert(false);
			return false;
		}

		OutputBranch output;
		bool ret = this->findWidget(widget->getName(), output);
		if (!ret)
			return ret;

		UILayoutNode* node = output[output.size() - 1];
		const TString& name = widget->getName();
		POINT2I pos = widget->getPosition();
		SIZE2I size = widget->getSize();
		uint32 flags = widget->getFlag();

		//update info
		node->addWidget(name, pos, size, flags);

		//update node visibility
		{
			//becomes invisible if all widgets invisible
			bool allInvisible = true;
			for (size_t i = 0; allInvisible && i < node->mData->mWidgetList.size(); ++i)
			{
				if ((node->mData->mWidgetList[i].flags&WMF_VISIBLE))
					allInvisible = false;
			}

			if (allInvisible)
				node->setVisible(false);
			else
				node->setVisible(true);
		}

		//update cascade visibility
		int index = (int)output.size() - 2;
		while (index > 0)
		{
			UILayoutNode* parent = output[(index_t)index];
			bool allInvisible = true;
			for (size_t j = 0; allInvisible && j < parent->getChildCount(); ++j)
			{
				if (parent->getChild(j)->isVisible())
					allInvisible = false;
			}

			if (allInvisible)
				parent->setVisible(false);
			else
				parent->setVisible(true);

			node = parent;
			--index;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		UIWidgetLayout::removeWidget(IUIWidget* widget)
	{
		UIWidgetLayout::OutputBranch output;
		bool result = this->findWidget(widget->getName(), output);
		if (!result)
			return result;

		UILayoutNode* node = output[output.size() - 1];
		result = node->removeWidget(widget->getName());
		if (!result)
			return result;
		
		//remove empty parents
		int index = (int)output.size() - 2;
		while (index >= 0 && node->getChildCount() == 0 && node->getWidgetCount() == 0)
		{
			UILayoutNode* parent = output[(index_t)index];
			parent->removeChild(node);
			node = parent;
			--index;
		}
		return true;
	}

}//namespace Blade