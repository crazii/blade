/********************************************************************
	created:	2011/01/28
	filename: 	SchemeParser.h
	author:		Crazii
	purpose:	render scheme parser
*********************************************************************/

#ifndef __Blade_SchemeParser_h__
#define __Blade_SchemeParser_h__
#include <interface/public/file/IFileDevice.h>
#include <utility/BladeContainer.h>
#include <utility/String.h>
#include "RenderOutput.h"
#include "RenderSchemeDesc.h"

namespace Blade
{
	class IXmlNode;

	class SchemeParser
	{
	public:
		SchemeParser();
		~SchemeParser();

		/*
		@describe parse the render scheme from a XML file stream
		@param 
		@return 
		*/
		bool			parseFile(const HSTREAM& stream,SCHEME_DESC& outSchemeDesc);

	protected:

		/** @brief  */
		const TString*	readAttribute(IXmlNode* node,const TString& attribute,const HSTREAM& stream);

		/** @brief  */
		bool			parseSize(const TString* psizeString,scalar& scale,size_t& offset, const TString& defaultVal = BTString("100%") );

		/** @brief  */
		bool			readInputBuffer(IXmlNode* node, int bufferUsage, InputDescList& outDesc, const BufferDescList& buffers, const HSTREAM& stream);

		/** @brief  */
		bool			readOutputBuffer(IXmlNode* node, int bufferUsage, 
			TargetDescList& outDesc, const BUFFER_DESC*& outDepthDesc, BUFFER_REF& bufferRef,
			const BufferDescList& buffers, const HSTREAM& stream, bool* useFinalTarget, bool optional = false);

		/** @brief  */
		bool			readSteps(IXmlNode* node, StepDescList& outSteps, const TypeDescList& typeList, const HSTREAM& stream, bool  optional = false);

	};//class SchemeParser

}//namespace Blade


#endif // __Blade_SchemeParser_h__