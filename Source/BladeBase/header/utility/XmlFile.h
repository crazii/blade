/********************************************************************
	created:	2010/04/07
	filename: 	XmlFile.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_XmlFile_h__
#define __Blade_XmlFile_h__
#include "TextFile.h"

namespace Blade
{
	typedef class  BLADE_BASE_API IXmlAttribute
	{
	public:
		virtual ~IXmlAttribute()	{}
		/** @brief  */
		virtual const TString&	getName() const = 0;
		/** @brief  */
		virtual const TString&	getValue() const = 0;
	}XML_ATTRIB;

	class BLADE_BASE_API IXmlNode
	{
	public:
		virtual ~IXmlNode()		{}

		class IXmlNodeList
		{
		protected:
			virtual ~IXmlNodeList()		{}
		public:
			/** @brief  */
			virtual index_t		find(IXmlNode* node) const = 0;

			/** @brief  */
			virtual bool		remove(index_t index) = 0;

			/** @brief  */
			virtual size_t		getSize() const = 0;

			/** @brief  */
			virtual IXmlNode*	getAt(index_t index) const= 0;

			inline IXmlNode*	operator[](index_t index) const			{return getAt(index);}

#if BLADE_COMPILER == BLADE_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-undefined-compare"
#endif

			/** @brief  */
			inline size_t		safeSize() const
			{
				if( this == NULL )
					return 0;
				else
					return this->getSize();
			}

			/** @brief  */
			inline IXmlNode*	safeAt(index_t index) const
			{
				if( this == NULL )
					return NULL;
				else
					return getAt(index);
			}
		};
#if BLADE_COMPILER == BLADE_COMPILER_CLANG
#pragma clang diagnostic pop
#endif

		/**
		@describe get the attribute value, if attribute not exist return NULL pointer.
		@param 
		@return 
		*/
		virtual const TString*	getAttributeValue(const TString& attribName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getAttributeCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const XML_ATTRIB&	getAttribute(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			addAttribute(const TString& attribName,const TString& attribValue) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeAttribute(const TString& attribName) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			hasChild() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getChildCount() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual  IXmlNode*		getChild(index_t index) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			hasChild(const TString& nameTypes) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IXmlNodeList*	getChildList(const TString& nameType) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IXmlNode*		addChild(const TString& name,const TString& text = TString::EMPTY ) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeChild(IXmlNode* child) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getText() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setText(const TString& text) = 0;


		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setName(const TString& name) = 0;


		//static member
		static IXmlNode*		createNode();

	};//class IXmlNode


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API XmlFile : protected TextFile
	{
	public:
		XmlFile();
		~XmlFile();

		using TextFile::setDestEncoding;

		/**
		@describe open the config file and parse the content
		@param [in] pathname filepath of the xml file
		@param [in] mode access mode the opening file, valid mode is read/write
		@return true if load and parse successfully
		@exception invalid xml file
		*/
		bool		open(const TString& pathname,IStream::EAccessMode mode );

		/**
		@describe 
		@note the stream should be opened in binary mode
		@param 
		@return 
		*/
		bool		open(const HSTREAM& stream );


		/**
		@describe close the xml file
		@param none
		@return none
		*/
		void		close();

		/**
		@describe 
		@param 
		@return 
		*/
		IXmlNode*	getRootNode();

		/**
		@describe 
		@param 
		@return 
		*/
		void		saveXml(IXmlNode* root);

	protected:
		/**
		@describe 
		@param 
		@return 
		*/
		bool		parseXml();

		/**
		@describe 
		@param 
		@return 
		*/
		void		writeNode(IXmlNode* node,uint indentLevel);

		/**
		@describe 
		@param 
		@return 
		*/
		size_t		readNode(IXmlNode* node,const TString& text);

		/** @brief root node of a xml */
		IXmlNode*	mRoot;
	};//class XmlFile
	
}//namespace Blade


#endif //__Blade_XmlFile_h__