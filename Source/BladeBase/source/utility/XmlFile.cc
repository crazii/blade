/********************************************************************
	created:	2010/04/07
	filename: 	XmlFile.cc
	author:		Crazii
	
	purpose:	
	note:		XML file use in-place parsing and creat temp-const strings.
				if those strings will be copied, they'll auto allocating memories for copies.
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	/************************************************************************/
	/* XmlAttribute                                                                     */
	/************************************************************************/
	class  XmlAttribute : public IXmlAttribute
	{
	public:
		TString mName;
		TString mValue;
		inline bool		operator<(const XmlAttribute& rhs) const
		{
			FnTStringFastLess op;
			return op( mName , rhs.mName);
		}
		inline bool	operator==(const XmlAttribute& rhs) const
		{
			return mName == rhs.mName;
		}

		/** @brief  */
		inline virtual const TString&	getName() const
		{
			return mName;
		}
		/** @brief  */
		inline virtual const TString&	getValue() const
		{
			return mValue;
		}
	};


	/************************************************************************/
	/* XmlNode                                                                     */
	/************************************************************************/
	class XmlNode : public IXmlNode, public TempAllocatable
	{
		typedef class XmlNodeList : public IXmlNodeList
		{
		public:
			/** @brief  */
			virtual index_t		find(IXmlNode* node) const
			{
				if( node == NULL )
					return INVALID_INDEX;

				NodeListImpl::const_iterator i = std::find( mList.begin(), mList.end(), node);
				if( i == mList.end() )
					return INVALID_INDEX;

				index_t index = 0;
				while (i != mList.begin())
				{
					--i;
					++index;
				}
				return index;
			}

			/** @brief  */
			virtual bool		remove(index_t index)
			{
				if(index < mList.size() )
				{
					NodeListImpl::iterator i = mList.begin();
					std::advance(i, index);
					mList.erase( i );
					return true;
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
			}

			/** @brief  */
			virtual size_t		getSize() const
			{
				return mList.size();
			}

			/** @brief  */
			virtual IXmlNode*	getAt(index_t index) const
			{
				if (index < mList.size())
				{
					NodeListImpl::const_iterator i = mList.begin();
					std::advance(i, index);
					return *i;
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
			}

			/** @brief  */
			inline void push_back(XmlNode* node)
			{
				mList.push_back(node);
			}
		protected:
			typedef TempList<XmlNode*>	NodeListImpl;
			NodeListImpl	mList;
		}NodeList;

		typedef TempList<XmlAttribute>	AttributeSet;
		typedef TempTStringMap<NodeList>	GroupedNodeList;
	public:
		XmlNode();
		~XmlNode();

		/*
		@describe get the attribute value, if attribute not exist return NULL pointer.
		@param 
		@return 
		*/
		virtual const TString*	getAttributeValue(const TString& attribName) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getAttributeCount() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const XML_ATTRIB&	getAttribute(index_t index) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addAttribute(const TString& attribName, const TString& attribValue)
		{
			return this->addAttributeImpl(attribName, attribValue);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeAttribute(const TString& attribName);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			hasChild() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getChildCount();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IXmlNode*		getChild(index_t index);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			hasChild(const TString& nameTypes) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IXmlNodeList*	getChildList(const TString& nameType) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IXmlNode*		addChild(const TString& name, const TString& text = TString::EMPTY)
		{
			return this->addChildImpl(name, text);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeChild(IXmlNode* child);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getText() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setText(const TString& text);


		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setName(const TString& name);

		/** @brief  */
		//////////////////////////////////////////////////////////////////////////
		inline XmlNode*			addChildImpl(const TString& name, const TString& text = TString::EMPTY)
		{
			XmlNode* node = BLADE_NEW XmlNode();
			node->setName(name);
			node->setText(text);
			mNodesByName[node->getName()].push_back(node);
			mNodes.push_back(node);
			return node;
		}

		/** @brief  */
		inline bool			addAttributeImpl(const TString& attribName, const TString& attribValue)
		{
			//if (this->getAttributeValue(attribName) != NULL)
			//	return false;

			mAttributes.push_back(XmlAttribute());
			mAttributes.back().mName = attribName;
			mAttributes.back().mName.make_const_temp();

			mAttributes.back().mValue = attribValue;	
			mAttributes.back().mValue.make_const_temp();
			return true;
		}

	protected:
		NodeList				mNodes;
		mutable GroupedNodeList	mNodesByName;
		AttributeSet			mAttributes;
		TString				mName;
		TString				mText;
	};//class XmlNode

	//////////////////////////////////////////////////////////////////////////
	IXmlNode*		IXmlNode::createNode()
	{
		return BLADE_NEW XmlNode();
	}


	//////////////////////////////////////////////////////////////////////////
	XmlNode::XmlNode()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	XmlNode::~XmlNode()
	{
		for(size_t i = 0; i < mNodes.getSize(); ++i)
		{
			XmlNode* pNode = (XmlNode*)mNodes.getAt(i);
			BLADE_DELETE pNode;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const TString*	XmlNode::getAttributeValue(const TString& attribName) const
	{
		XmlAttribute attrib;
		attrib.mName = attribName;

		AttributeSet::const_iterator it = std::find(mAttributes.begin(), mAttributes.end(),attrib);
		if( it != mAttributes.end() )
			return &(it->getValue()); 
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			XmlNode::getAttributeCount() const
	{
		return mAttributes.size();
	}


	//////////////////////////////////////////////////////////////////////////
	const XML_ATTRIB&	XmlNode::getAttribute(index_t index) const
	{
		if( index < mAttributes.size() )
		{
			AttributeSet::const_iterator it = mAttributes.begin();
			std::advance(it, index);
			return *it;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			XmlNode::removeAttribute(const TString& attribName)
	{
		XmlAttribute attrib;
		attrib.mName = attribName;
		AttributeSet::iterator it = std::find(mAttributes.begin(), mAttributes.end(), attrib);
		if (it != mAttributes.end())
		{
			mAttributes.erase(it);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			XmlNode::hasChild() const
	{
		return mNodes.getSize() != 0;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			XmlNode::getChildCount()
	{
		return mNodes.getSize();
	}

	//////////////////////////////////////////////////////////////////////////
	IXmlNode*		XmlNode::getChild(index_t index)
	{
		return mNodes.getAt(index);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			XmlNode::hasChild(const TString& nameTypes) const
	{
		GroupedNodeList::const_iterator it = mNodesByName.find( nameTypes );
		if( it == mNodesByName.end() )
			return false;
		else
			return true;
	}

	//////////////////////////////////////////////////////////////////////////
	XmlNode::IXmlNodeList*	XmlNode::getChildList(const TString& nameType) const
	{
		GroupedNodeList::iterator it = mNodesByName.find( nameType);
		if( it == mNodesByName.end() )
			return NULL;
		else
			return &(it->second);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			XmlNode::removeChild(IXmlNode* child)
	{
		if( child == NULL )
			return false;

		const TString& name = child->getName();
		GroupedNodeList::iterator i = mNodesByName.find(name);
		NodeList& nodeList = i->second;
		index_t index = nodeList.find(child);
		if( index == INVALID_INDEX )
		{
			assert( mNodes.find(child) == INVALID_INDEX );
			return false;
		}

		nodeList.remove(index);
		if( nodeList.getSize() == 0)
			mNodesByName.erase(i);

		index = mNodes.find(child);
		assert( index != INVALID_INDEX );
		mNodes.remove(index);
		BLADE_DELETE child;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	XmlNode::getText() const
	{
		return mText;
	}

	//////////////////////////////////////////////////////////////////////////
	void			XmlNode::setText(const TString& text)
	{
		mText = text;
		mText.make_const_temp();
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	XmlNode::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	void			XmlNode::setName(const TString& name)
	{
		mName = name;
		mName.make_const_temp();
	}

	/************************************************************************/
	/* XmlFile                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	XmlFile::XmlFile()
		:mRoot(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	XmlFile::~XmlFile()
	{
		this->close();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		XmlFile::open(const TString& pathname,IStream::EAccessMode mode)
	{
		if( !TextFile::open(pathname, mode) )
			return false;
		return this->parseXml();
	}


	//////////////////////////////////////////////////////////////////////////
	bool		XmlFile::open(const HSTREAM& stream )
	{
		if( stream == NULL || !TextFile::open(stream) )
			return false;
		return this->parseXml();
	}

	//////////////////////////////////////////////////////////////////////////
	void		XmlFile::close()
	{
		TextFile::close();
		BLADE_DELETE mRoot;
		mRoot = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IXmlNode*	XmlFile::getRootNode()
	{
		return mRoot;
	}

	//////////////////////////////////////////////////////////////////////////
	void		XmlFile::saveXml(IXmlNode* root)
	{
		TEXT_ENCODING encoding = this->getDestEncoding();
		if( encoding <= TE_START || encoding >= TE_COUNT )
			encoding = TE_UTF8;

		TString encodingString[] =
		{
			BTString("utf-8"),	//force MBCS to utf-8
			BTString("utf-8"),
			BTString("utf-16le"),
			BTString("utf-16be"),
			BTString("utf-32le"),
			BTString("utf-32be"),
			BTString("utf-8"),
		};
		//update encoding
		this->setDestEncoding(encoding);
		//write header
		this->write(BTString("<?xml version=\"1.0\" encoding=\"") + encodingString[encoding] + BTString("\"?>\n"));
		this->writeNode(root,0);
	}

	#define THROW_EXCEPTION(desc) BLADE_EXCEPT(EXC_FILE_READ,mFileStream->getName()+BTString(":invalid xml file - ")+BTString(desc))
	#define THROW_EXCEPTION_LINE(desc,line) BLADE_EXCEPT(EXC_FILE_READ,mFileStream->getName()+BTString(":invalid xml file - ") + \
								BTString(desc)+ BTString(" at  \n\n") + line + BTString("\n"))

	//////////////////////////////////////////////////////////////////////////
	bool		XmlFile::parseXml()
	{
		if (mRoot != NULL)
			THROW_EXCEPTION("file already parsed.");

		const TString content = TextFile::getText();
	
		if( !content.empty() )
			this->readNode(NULL,content);
		else
			//just allocate one empty root
			mRoot = BLADE_NEW XmlNode();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		XmlFile::writeNode(IXmlNode* node,uint indentLevel)
	{
		if( node == NULL )
			return;

		for(uint i = 0; i < indentLevel; ++i)
			this->write(BTString("\t"));

		this->write(BTString("<"));
		this->write( node->getName() );

		for( size_t i = 0; i < node->getAttributeCount(); ++i )
		{
			const XML_ATTRIB& attrib = node->getAttribute(i);

			this->write(BTString(" "));
			this->write( attrib.getName() );
			this->write(BTString("="));
			this->write(BTString("\""));
			this->write( attrib.getValue() );
			this->write(BTString("\""));
		}

		if( !node->hasChild() )
		{
			 if( node->getText() == NULL )
				this->write(BTString("/>\n"));
			 else
			 {
				 this->write(BTString(">"));
				 this->write( node->getText() );
				 this->write(BTString("</"));
				 this->write( node->getName() );
				 this->write(BTString(">\n"));
			 }
		}
		else
		{
			this->write(BTString(">\n"));
			//TempList<XmlNode*>	nodeCache;

			for( size_t i = 0; i < node->getChildCount(); ++i )
			{
				IXmlNode* child = node->getChild(i);
				this->writeNode(child,indentLevel + 1);
			}

			for(uint i = 0; i < indentLevel; ++i)
				this->write(BTString("\t"));

			this->write( node->getText() );

			this->write(BTString("</"));
			this->write( node->getName() );
			this->write(BTString(">\n"));
		}
	}

	//TODO : exceptions with detailed information about text in xml file.
	namespace Impl
	{
		extern int tableWhiteSpace[128];
		extern int tableNameStarts[128];
		extern int tableNameChars[128];

		struct FnIsWhiteSpace
		{
			inline const bool operator()(const tchar tc) const
			{
				return (uint8)tc >= 128 ? false : tableWhiteSpace[(int)tc] != 0;
			}
		};

		struct FnIsValidNameStart
		{
			inline const bool operator()(const tchar tc) const
			{
				return check(tc);
			}
			static bool check(const tchar tc)
			{
				return (uint8)tc >= 128 ? true : tableNameStarts[(int)tc] != 0;
			}
		};

		struct FnIsValidNameChars
		{
			inline const bool operator()(const tchar tc) const
			{
				return (uint8)tc >= 128 ? true : tableNameChars[(int)tc] != 0;
			}
		};
	};

	static const TString headerEnd = BTString("?>");
	static const TString commentEnd = BTString("-->");

	//////////////////////////////////////////////////////////////////////////
	size_t		XmlFile::readNode(IXmlNode* _node, const TString& content)
	{
		XmlNode* node = static_cast<XmlNode*>(_node);

		index_t cursor = 0;
		while( cursor < content.size() )
		{
			//find sub node
			cursor = content.find_first_of(TEXT('<'),cursor);
			if( cursor == INVALID_INDEX )
				break;
			if( cursor+1 >= content.size() || content[cursor+1]==TEXT(' ') )
				THROW_EXCEPTION("missing node name after '<'.");
			//parsing header info
			else if( mRoot == NULL && content[cursor+1] == TEXT('?' ) )
			{
				//avoid cursor overflow
				index_t end = content.find(headerEnd, cursor+1);
				if(end == INVALID_INDEX)
					THROW_EXCEPTION("missing ?>.");
				cursor = end + 2;
				continue;
			}
			//parsing comment
			else if( content[cursor+1] == TEXT('!') && content[cursor+2] == TEXT('-') && content[cursor+3] == TEXT('-') )
			{
				//avoid cursor overflow
				index_t end = content.find(commentEnd, cursor + 4);
				if(end == INVALID_INDEX)
					THROW_EXCEPTION("missing -->.");
				cursor = end + 3;
				continue;
			}

			index_t ClosingTag = content.find_first_of(TEXT('>'),cursor);
			if( ClosingTag == INVALID_INDEX )
				THROW_EXCEPTION("missing '>'.");

			//recursive end check : end of node
			//simplify:ignoring node text
			if( content[cursor+1] == TEXT('/')  )
			{
				if (node == NULL /*&& mRoot == NULL*/)
					THROW_EXCEPTION("end tag \"</>\" without start tag \"<>\".");

				if(cursor+2+node->getName().size() >= content.size())
					THROW_EXCEPTION_LINE("name mismatch.", content.substr_nocopy(cursor + 2));
				
				if( Char_Traits<tchar>::compare(node->getName().c_str(), &content[cursor+2], node->getName().size()) == 0 )
				{
					// </name
					size_t leftoff = cursor+2+node->getName().size();

					// </name INVALIDSTRING >
					if(ClosingTag > leftoff && content.find_if_not(Impl::FnIsWhiteSpace(), leftoff, ClosingTag - leftoff) )
						THROW_EXCEPTION("invalid characters found after end tag.");

					return ClosingTag+1;
				}
				//<name></mismatch_name>
				else
					THROW_EXCEPTION_LINE("name mismatch.", content.substr_nocopy(cursor + 2, node->getName().size()) );
			}

			//<name ... />
			tchar closingChar = content[ClosingTag - 1];
			if(closingChar == TEXT('/') )
				--ClosingTag;

			const TString nodeContent = content.substr_nocopy(cursor+1,ClosingTag-(cursor+1) );
			
			index_t nameStart = nodeContent.find_if_not(Impl::FnIsWhiteSpace(), 0);
			if (nameStart == INVALID_INDEX || !Impl::FnIsValidNameStart::check(nodeContent[nameStart]) )
				THROW_EXCEPTION_LINE("no valid name tag found for node", nodeContent);

			index_t nameEnd = nodeContent.find_if_not(Impl::FnIsValidNameChars(), nameStart+1);
			const TString name = nodeContent.substr_nocopy(nameStart, nameEnd - nameStart);

			//check root:
			XmlNode* newNode;
			if( node == NULL )
			{
				if( mRoot != NULL )
					THROW_EXCEPTION("only one root allowed.");
				newNode = BLADE_NEW XmlNode();
				newNode->setName( name );
				mRoot = newNode;
			}
			else
				newNode = node->addChildImpl( name );

			//read attribute
			if (nameEnd != INVALID_INDEX)
			{
				const TString attributes = nodeContent.substr_nocopy(nameEnd);
				size_t pos = attributes.find_if_not(Impl::FnIsWhiteSpace());
				while (pos < attributes.size())
				{
					if (!Impl::FnIsValidNameStart::check(attributes[pos]))
						THROW_EXCEPTION_LINE("invalid attribute name", attributes);

					size_t posEnd = attributes.find_if_not(Impl::FnIsValidNameChars(), pos);
					if (posEnd == INVALID_INDEX)
						THROW_EXCEPTION_LINE("missing '='", attributes);

					const TString attribName = attributes.substr_nocopy(pos, posEnd-pos);
					pos = attributes.find_if_not(Impl::FnIsWhiteSpace(), posEnd);
					if (pos == INVALID_INDEX || attributes[pos] != TEXT('='))
						THROW_EXCEPTION_LINE("missing '='", attributes);

					pos = attributes.find_if_not(Impl::FnIsWhiteSpace(), pos + 1);

					if (attributes[pos] != TEXT('\'') && attributes[pos] != TEXT('\"'))
						THROW_EXCEPTION_LINE("missing beginning \' or \"", attributes);


					posEnd = attributes.find_first_of(attributes[pos], pos + 1);
					if (posEnd == INVALID_INDEX)
						THROW_EXCEPTION_LINE("missing ending \' or \"", attributes);
					++pos;
					const TString attribValue = attributes.substr_nocopy(pos, posEnd - pos);

					newNode->addAttributeImpl(attribName, attribValue);
					pos = attributes.find_if_not(Impl::FnIsWhiteSpace(), posEnd+1);
				}
			}

			cursor = ClosingTag+1;
			
			//end of sub node directly, no sub nodes
			if(closingChar == TEXT('/') )
				//try left content to find a sibling node of newNode
				cursor++;
			//read sub nodes recursively
			else
			{
				index_t SubNodeBegin = content.find(TEXT('<'),cursor);
				if(SubNodeBegin != INVALID_INDEX)
					SubNodeBegin += this->readNode(newNode, content.substr_nocopy(SubNodeBegin) );
				cursor = SubNodeBegin;
			}

		}//while
		return content.size();
	}

	
}//namespace Blade