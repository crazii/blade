/********************************************************************
	created:	2011/01/28
	filename: 	SchemeParser.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderSchemeParser.h"

#include <interface/public/graphics/IRenderDevice.h>
#include <interface/IRenderTypeManager.h>
#include <interface/IMaterialManager.h>
#include <Material.h>


namespace Blade
{
	namespace
	{
		static const TString SCHEME_ROOT		= BTString("render_scheme");
		static const TString NAME				= BTString("name");
		static const TString PROFILE			= BTString("profiles");
		
		static const TString OPERATOR_ROOT		= BTString("render_operators");
		static const TString OPERATOR			= BTString("operator");
		static const TString PREFIX				= BTString("prefix");
		static const TString RENDER_TYPE_ROOT	= BTString("render_types");
		static const TString TYPE				= BTString("type");
		static const TString MATERIAL			= BTString("material");

		static const TString RENDER_BUFFER_ROOT	= BTString("render_buffers");
		static const TString BUFFER				= BTString("buffer");
		static const TString FORMAT				= BTString("format");
		static const TString WIDTH				= BTString("width");
		static const TString HEIGHT				= BTString("height");
		static const TString DEFAULT_SIZE		= BTString("%100");

		static const TString ACCESS				= BTString("access");
		static const TString ACCESS_READ		= BTString("read");
		static const TString ACCESS_WRITE		= BTString("write");
		static const TString ACCESS_READ_WRITE	= BTString("read_write");
		static const TString DEFAULT_ACCESS		= ACCESS_READ_WRITE;

		static const TString TYPE_COLOR			= BTString("color");
		static const TString TYPE_DEPTH			= BTString("depth");

		static const TString OUTPUT_NODE		= BTString("render_output");
		static const TString OUTPUT_TARGET		= BTString("target");
		static const TString OUTPUT_DEPTH		= BTString("target_depth");
		static const TString FINAL_BUFFER		= BTString("FINAL");
		static const TString OUTPUT_INPUT		= BTString("input");
		static const TString OUTPUT_SORTER		= BTString("sorter");

		static const TString REFERENCE			= BTString("reference");

		static const TString CAMERA				= BTString("camera");
		static const TString CAMERA_MAIN		= BTString("MAIN");

		static const TString CAMERA_CUSTOM		= BTString("custom");
		static const TString CAMERA_PERSPECTIVE	= BTString("PERSPECTIVE");
		static const TString CAMERA_ORTHOGRAPHIC= BTString("ORTHOGRAPHIC");
		static const TString CAMERA_NEAR		= BTString("near");
		static const TString CAMERA_FAR			= BTString("far");
		static const TString CAMERA_ORTHO_WIDTH	= BTString("width");
		static const TString CAMERA_ORTHO_HEIGHT= BTString("height");
		static const TString CAMERA_FOV			= BTString("horiz_fov");
		static const TString CAMERA_ASPECT		= BTString("aspect");

		static const TString VIEW				= BTString("view");
		static const TString VIEW_FINAL			= BTString("FINAL");
		static const TString LEFT				= BTString("left");
		static const TString TOP				= BTString("top");
		static const TString RIGHT				= BTString("right");
		static const TString BOTTOM				= BTString("bottom");
		static const TString VIEW_CLEAR			= BTString("clear");
		static const TString CLEAR_COLOR		= BTString("color");
		static const TString CLEAR_DEPTH		= BTString("depth");
		static const TString CLEAR_STENCIL		= BTString("stencil");

		static const TString STEP_NODE			= BTString("render_step");
		static const TString STEP_PASS			= BTString("pass");
		static const TString STEP_SORTER		= OUTPUT_SORTER;
		static const TString STEP_GROUPS		= BTString("group");
		static const TString STEP_TEXTURING			= BTString("texturing");


		static const TString NONE				= BTString("NONE");
		static const TString EMPTY				= BTString("EMPTY");
		static const TString DEFAULT			= BTString("default");
		static const TString ENABLE				= BTString("enable");
		static const TString DISABLE			= BTString("disable");

		static const TString IMAGE_EFFECT		= BTString("image_effect");
	}//namespace SchemeKeyString

	//////////////////////////////////////////////////////////////////////////
	SchemeParser::SchemeParser()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	SchemeParser::~SchemeParser()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool			SchemeParser::parseFile(const HSTREAM& stream, SCHEME_DESC& outSchemeDesc)
	{
		BLADE_LOG(Information, TEXT("Parsing RenderScheme config file: ") + BTString(" - ") + stream->getName());

		XmlFile file;
		if( !file.open(stream) )
		{
			BLADE_LOG(Warning, TEXT("Open RenderScheme config file failed:") + BTString(" - ") + stream->getName());
			return false;
		}

		IXmlNode* root = file.getRootNode();

		if( root->getName() != SCHEME_ROOT )
		{
			BLADE_LOG(Warning, TEXT("Invalid RenderScheme config file:") + BTString(" - ") + stream->getName());
			return false;
		}

		//read name
		{
			const TString* name = this->readAttribute(root,NAME,stream);
			if( name == NULL )
				return false;
			outSchemeDesc.mName = *name;
		}

		//read profiles
		{
			const TString* profiles = this->readAttribute(root, PROFILE, stream);
			if( profiles == NULL )
				profiles = &TString::EMPTY;

			TStringTokenizer profToken;
			profToken.tokenize(*profiles,TEXT(","));

			if(profToken.size() == 0)
			{
				BLADE_LOG(Warning, TEXT("Missing attribute:") + PROFILE + BTString(" - ") + stream->getName());
				return false;
			}

			outSchemeDesc.mProfileList.reserve( profToken.size() );
			for(size_t i = 0; i < profToken.size(); ++i)
				outSchemeDesc.mProfileList.push_back(profToken[i]);

			std::sort(outSchemeDesc.mProfileList.begin(), outSchemeDesc.mProfileList.end());
		}

		outSchemeDesc.mUseFinalTarget = false;

		//operators
		{
			IXmlNode::IXmlNodeList* operatorNodeList = root->getChildList(OPERATOR_ROOT);
			if (operatorNodeList->safeSize() > 1)
			{
				BLADE_LOG(Warning, TEXT("multiple node:") + OPERATOR_ROOT + BTString(" - not allowed. ") + stream->getName());
				return false;
			}

			if (operatorNodeList->safeSize() == 1)
			{
				IXmlNode* operatorRootNode = operatorNodeList->getAt(0);
				TempVector<OPERATOR_DESC> operatorDescList;
				for (size_t i = 0; i < operatorRootNode->getChildCount(); ++i)
				{
					IXmlNode* operatorNode = operatorRootNode->getChild(i);
					if (operatorNode->getName() != OPERATOR)
					{
						BLADE_LOG(Warning, TEXT("invalid type node :") + operatorNode->getName() + BTString(" - ") + stream->getName());
						continue;
					}
					const TString* type = this->readAttribute(operatorNode, TYPE, stream);
					if (type == NULL)
						continue;
					const TString* prefix = this->readAttribute(operatorNode, PREFIX, stream);
					if (prefix == NULL)
						continue;
					OPERATOR_DESC desc = { *type, *prefix };
					operatorDescList.push_back(desc);
				}
				outSchemeDesc.mOperatorList.reserve(operatorDescList.size());
				outSchemeDesc.mOperatorList.insert(outSchemeDesc.mOperatorList.end(), operatorDescList.begin(), operatorDescList.end());
			}
		}

		//read types
		{
			IXmlNode::IXmlNodeList* TypesNodeList = root->getChildList( RENDER_TYPE_ROOT );
			if( TypesNodeList->safeSize() != 1)
			{
				BLADE_LOG(Error, TEXT("Missing node:") + RENDER_TYPE_ROOT + BTString(" - ") + stream->getName());
				assert(false);
				return false;
			}

			IXmlNode* TypesRootNode = TypesNodeList->getAt(0);
			for(size_t i = 0; i < TypesRootNode->getChildCount(); ++i)
			{
				IXmlNode* TypeNode= TypesRootNode->getChild(i);
				if( TypeNode->getName() != TYPE )
				{
					BLADE_LOG(Warning, TEXT("invalid type node :") + TypeNode->getName() + BTString(" - ") + stream->getName());
					continue;
				}
				const TString* name = this->readAttribute( TypeNode, NAME, stream );
				if( name == NULL )
					continue;

				//const TString* materialName = this->readAttribute( TypeNode, MATERIAL, stream );
				//if( materialName == NULL )
				//	continue;

				if( IRenderTypeManager::getSingleton().getRenderType(*name)  == NULL )
				{
					BLADE_LOG(Warning, TEXT("render type not found: ") + *name << BTString(" - ") + stream->getName());
					continue;
				}

				//Material* material = IMaterialManager::getSingleton().getMaterial( *materialName);
				//if( material == NULL || material->getTechnique(outSchemeDesc.mTechniqueName) == NULL )
				//{
				//	BLADE_LOG(Warning, TEXT("material not found:") << *materialName << BTString(" - ") << stream->getName());
				//	continue;
				//}

				TYPE_DESC typeDesc;
				typeDesc.mRenderTypeName = *name;
				//typeDesc.mMterialName = *materialName;
				outSchemeDesc.mTypeList.insert( typeDesc );
			}
		}

		if( outSchemeDesc.mTypeList.size() == 0 )
		{
			BLADE_LOG(Warning, TEXT("no render types defined") + BTString(" - ") + stream->getName());
			return false;
		}

		//read buffers
		{
			//add pre-defined buffer
			{
				BUFFER_DESC desc;
				desc.mName = FINAL_BUFFER;
				desc.mBufferRef = BR_FINAL;
				desc.mGpuAccess = IGraphicsBuffer::GBAF_READ;
				desc.mFormat = PF_UNDEFINED;
				desc.mWidth = 0;
				desc.mHeight = 0;
				desc.mWidthScale = 1;
				desc.mHeightScale = 1;
				desc.mType = RBT_NONE;
				outSchemeDesc.mBufferList.insert(desc);

				desc.mName = NONE;
				desc.mBufferRef = BR_EMPTY;
				desc.mBufferUsage = BU_NONE;
				outSchemeDesc.mBufferList.insert(desc);

				desc.mName = EMPTY;
				outSchemeDesc.mBufferList.insert(desc);
			}

			IXmlNode::IXmlNodeList* BufferNodeList = root->getChildList( RENDER_BUFFER_ROOT );
			if(BufferNodeList->safeSize() == 0 )
			{
				BLADE_LOG(Warning, TEXT("Missing node:") + RENDER_BUFFER_ROOT << BTString(" - ") + stream->getName());
				return false;
			}

			if( BufferNodeList->safeSize() != 1)
				BLADE_LOG(Warning, TEXT("One and only one node allowed:") + RENDER_BUFFER_ROOT << BTString(" - ") + stream->getName());

			IXmlNode* TargetsRootNode = BufferNodeList->getAt(0);

			for(size_t i = 0; i < TargetsRootNode->getChildCount(); ++i)
			{
				IXmlNode* BufferNode= TargetsRootNode->getChild(i);
				if( BufferNode->getName() != BUFFER )
				{
					BLADE_LOG(Warning, TEXT("Invalid type node :") + BufferNode->getName() + BTString(" - ") + stream->getName());
					continue;
				}
				const TString* name = this->readAttribute( BufferNode, NAME, stream );
				if( name == NULL )
				{
					BLADE_LOG(Warning, TEXT("Missing pixel buffer name:") + BufferNode->getName() << BTString(" - ") + stream->getName());
					continue;
				}

				if( *name == FINAL_BUFFER )	//preserved keyword
				{
					BLADE_LOG(Warning, TEXT("pixel buffer name is preserved, it should not be:") + FINAL_BUFFER << BTString(" - ") + stream->getName());
					continue;
				}

				const TString* format = this->readAttribute( BufferNode, FORMAT, stream );
				if( format == NULL )
				{
					BLADE_LOG(Warning, TEXT("Missing buffer pixel format definition:") << BufferNode->getName() << BTString(" - ") << stream->getName());
					continue;
				}

				PixelFormat fmt( *format );
				if( fmt == PF_UNDEFINED )
				{
					BLADE_LOG(Warning, TEXT("Invalid pixel format:") + *format << BTString(" - ") + stream->getName());
					continue;
				}

				const TString* type = this->readAttribute( BufferNode, TYPE, stream );
				if( type == NULL )
				{
					BLADE_LOG(Warning, TEXT("Missing buffer type definition:") + BufferNode->getName() << BTString(" - ") + stream->getName());
					continue;
				}

				const TString* width = this->readAttribute( BufferNode, WIDTH, stream );
				const TString* height = this->readAttribute( BufferNode, HEIGHT, stream );
				BUFFER_DESC bufferDesc;
				bufferDesc.mName = *name;
				bufferDesc.mBufferRef = BR_CUSTOM;
				bufferDesc.mBufferUsage = BU_NONE;

				//parsing width and height
				{
					bufferDesc.mHeightScale = bufferDesc.mWidthScale = 1.0f;
					bufferDesc.mWidth = bufferDesc.mHeight = 0;
					this->parseSize( width, bufferDesc.mWidthScale, bufferDesc.mWidth );
					this->parseSize( height, bufferDesc.mHeightScale, bufferDesc.mHeight );
					if( bufferDesc.mWidthScale != 0 || bufferDesc.mWidthScale != 0 )
						outSchemeDesc.mUseFinalTarget = true;
				}

				//parsing access
				{
					const TString* access = this->readAttribute( BufferNode, ACCESS, stream );
					if( access == NULL )
						access = &DEFAULT_ACCESS;

					if( *access == ACCESS_READ )
						bufferDesc.mGpuAccess = IGraphicsBuffer::GBAF_READ;
					else if( *access == ACCESS_WRITE )
						bufferDesc.mGpuAccess = IGraphicsBuffer::GBAF_WRITE;
					else if( *access == ACCESS_READ_WRITE )
						bufferDesc.mGpuAccess = IGraphicsBuffer::GBAF_READ_WRITE;
					else
					{
						BLADE_LOG(Warning, TEXT("Invalid access :") + *access << BTString(" - ") + stream->getName());
						continue;
					}
				}

				if( *type == TYPE_COLOR )
					bufferDesc.mType = RBT_COLOR;
				else if( *type == TYPE_DEPTH )
					bufferDesc.mType = RBT_DEPTH;
				else
				{
					BLADE_LOG(Warning, TEXT("Invalid buffer type :") + *type << BTString(" - ") + stream->getName());
					continue;
				}

				bufferDesc.mFormat = fmt;
				outSchemeDesc.mBufferList.insert( bufferDesc );
			}//for
		}

		//read outputs
		IXmlNode::IXmlNodeList* outputList = root->getChildList( OUTPUT_NODE );
		if( outputList == NULL )
		{
			BLADE_LOG(Warning, TEXT("Missing node:") + OUTPUT_NODE << BTString(" - ") + stream->getName());
			return false;
		}

		for(size_t i = 0; i < outputList->getSize(); ++i )
		{
			IXmlNode* output_child = outputList->safeAt(i);

			Handle<OUTPUT_DESC> outputDesc(BLADE_NEW OUTPUT_DESC());

			bool hasReference = false;
			//output reference (clone source)
			{
				const TString* reference = this->readAttribute(output_child, REFERENCE, stream);
				if (reference != NULL)
				{
					for (OutputDescList::iterator iter = outSchemeDesc.mOutputList.begin(); iter != outSchemeDesc.mOutputList.end(); ++iter)
					{
						if ((*iter)->mName == *reference)
						{
							*outputDesc = *(*iter);
							hasReference = true;
							//copy steps
							for (size_t j = 0; j < outputDesc->mStepDescList.size(); ++j)
								outputDesc->mStepDescList[j] = BLADE_NEW STEP_DESC(*outputDesc->mStepDescList[j]);
							break;
						}
					}
				}
			}

			//read output name
			{
				const TString* name = this->readAttribute(output_child, NAME, stream);
				if( name == NULL )
				{
					BLADE_LOG(Warning, TEXT("Missing attribute:") + NAME << BTString(" - ") + stream->getName());
					continue;
				}
				outputDesc->mName = *name;
			}

			//read post effect
			outputDesc->mHasImageEfect = false;
			{
				const TString* post_effect = this->readAttribute(output_child, IMAGE_EFFECT, stream);
				if (post_effect != NULL)
				{
					outputDesc->mImageEffect = *post_effect;
					outputDesc->mHasImageEfect = true;
				}
			}

			//read output sorter
			{
				const TString* sorters = this->readAttribute(output_child, OUTPUT_SORTER, stream);
				if (sorters == NULL)
					sorters = &NONE;
				else
					outputDesc->mSorters.clear();
				TStringTokenizer token;
				token.tokenize(*sorters, TEXT(","));
				outputDesc->mSorters.reserve( token.size() );
				for(size_t j = 0; j < token.size(); ++j)
				{
					if(token[j] != NONE )
						outputDesc->mSorters.push_back( token[j] );
				}
			}

			//read output buffer
			const BUFFER_DESC* depthDesc = NULL;
			this->readOutputBuffer(output_child, BU_DEFAULT, outputDesc->mTargetDescList, depthDesc, outputDesc->mOutputBufferRef, outSchemeDesc.mBufferList, stream, &outSchemeDesc.mUseFinalTarget, outputDesc->mHasImageEfect);
			if (depthDesc != NULL || !hasReference)
				outputDesc->mDepthTarget = depthDesc;

			//read input buffer
			this->readInputBuffer(output_child, BU_DEFAULT, outputDesc->mInputDescList, outSchemeDesc.mBufferList, stream);

			//read output view
			{
				VIEW_DESC viewDesc;
				IXmlNode::IXmlNodeList* viewNodeList = output_child->getChildList( VIEW );
				if( viewNodeList->safeSize() > 1 )
				{
					BLADE_LOG(Warning, TEXT("one and only one view reference needed per output:") + stream->getName());
					continue;
				}
				IXmlNode* viewNode = viewNodeList->safeAt(0);
				if (viewNode != NULL)
				{
					const TString* crString = this->readAttribute(viewNode, REFERENCE, stream);

					if (crString == NULL)
						viewDesc.mViewRef = hasReference ? outputDesc->mViewDesc.mViewRef : VR_FINAL;
					else
					{
						if (*crString == NONE)
							viewDesc.mViewRef = VR_MANUAL;
						else if (*crString == VIEW_FINAL)
							viewDesc.mViewRef = VR_FINAL;
						else
						{
							BLADE_LOG(Warning, TEXT("Invalid view reference:") + stream->getName());
							continue;
						}
					}

					IXmlNode::IXmlNodeList* clearNodeList = viewNode->getChildList(VIEW_CLEAR);

					viewDesc.mClearMode = hasReference ? outputDesc->mViewDesc.mClearMode : FBT_NONE;
					if (clearNodeList->safeSize() > 1)
						BLADE_LOG(Warning, TEXT("only one clear param needed per view:") + stream->getName());

					IXmlNode* clearNode = clearNodeList->safeAt(0);
					if (clearNode != NULL)
					{
						const TString* color = this->readAttribute(clearNode, CLEAR_COLOR, stream);
						if (color == NULL || *color == NONE)
						{ }
						else
						{
							Variant var = Color::BLACK;
							var.fromTString(*color);
							viewDesc.mClearColor = var;
							viewDesc.mClearMode |= FBT_COLOR;
						}

						const TString* depth = this->readAttribute(clearNode, CLEAR_DEPTH, stream);
						if (depth == NULL || *depth == NONE)
						{ }
						else
						{
							viewDesc.mClearMode |= FBT_DEPTH;
							viewDesc.mClearDepth = TStringHelper::toScalar(*depth);
						}

						const TString* stencil = this->readAttribute(clearNode, CLEAR_STENCIL, stream);
						if (stencil == NULL || *stencil == NONE)
						{ }
						else
						{
							viewDesc.mClearMode |= FBT_STENCIL;
							viewDesc.mClearStencil = (uint16)TStringHelper::toUInt(*stencil);
						}
					}

				}
				else
				{
					viewDesc.mViewRef = VR_FINAL;
					viewDesc.mClearMode = FBT_NONE;
				}

				if( viewDesc.mViewRef == VR_MANUAL )
				{
					{
						const TString* left = this->readAttribute(viewNode, LEFT, stream);
						if( !this->parseSize(left,viewDesc.mLeft, viewDesc.mLeftPixel, BTString("0%")) )
							BLADE_LOG(Warning, BTString("unable to parse view dimension") );
					}

					{
						const TString* top = this->readAttribute(viewNode, TOP, stream);
						if( !this->parseSize(top, viewDesc.mTop, viewDesc.mTopPixel, BTString("0%")) )
							BLADE_LOG(Warning, BTString("unable to parse view dimension") );
					}

					{
						const TString* right = this->readAttribute(viewNode, RIGHT, stream);
						if( !this->parseSize(right, viewDesc.mRight, viewDesc.mRightPixel) )
							BLADE_LOG(Warning, BTString("unable to parse view dimension") );
					}

					{
						const TString* bottom = this->readAttribute(viewNode, BOTTOM, stream);
						if( !this->parseSize(bottom, viewDesc.mBottom, viewDesc.mBottomPixel) )
							BLADE_LOG(Warning, BTString("unable to parse view dimension") );
					}
				}//if viewDesc.mViewRef == VR_MANUAL

				outputDesc->mViewDesc = viewDesc;
			}

			//read output cameras
			{
				IXmlNode::IXmlNodeList* camNodeList = output_child->getChildList(CAMERA);
				if( camNodeList->safeSize() > 1)
				{
					BLADE_LOG(Warning, TEXT("one and only one camera reference needed per output:") + stream->getName());
					continue;
				}

				CAMERA_DESC cameraDesc;
				IXmlNode* camNode = camNodeList->safeAt(0);
				if (camNode != NULL)
				{
					const TString* crString = this->readAttribute(camNode, REFERENCE, stream);
					if (crString == NULL)
						cameraDesc.mCameraRef = hasReference ? outputDesc->mCameraDesc.mCameraRef : CR_MAIN;
					else
					{
						if (*crString == CAMERA_MAIN)
							cameraDesc.mCameraRef = CR_MAIN;
						else
						{
							cameraDesc.mCameraName = *crString;
							cameraDesc.mCameraRef = CR_BYNAME;
						}
					}
					const TString* customString = this->readAttribute(camNode, CAMERA_CUSTOM, stream);
					if (customString == NULL || *customString != ENABLE)
						cameraDesc.mFlags.clearBits(CDF_CUSTOM);
					else
						cameraDesc.mFlags.raiseBits(CDF_CUSTOM);

					if (cameraDesc.mFlags.checkBits(CDF_CUSTOM))
					{
						cameraDesc.mFlags.raiseBits(CDF_CUSTOM_ALL);
						//read camera attribute
						const TString* attrib = this->readAttribute(camNode, TYPE, stream);
						if (attrib == NULL)
							cameraDesc.mFlags.clearBits(CDF_CUSTOM_TYPE);
						else if (*attrib == CAMERA_PERSPECTIVE)
							cameraDesc.mCameraType = PT_PERSPECTIVE;
						else
							cameraDesc.mCameraType = PT_ORTHOGRAPHIC;

						attrib = this->readAttribute(camNode, CAMERA_NEAR, stream);
						if (attrib == NULL)
						{
							cameraDesc.mFlags.clearBits(CDF_CUSTOM_NEAR);
							cameraDesc.mNear = 0;
						}
						else
							cameraDesc.mNear = TStringHelper::toScalar(*attrib);

						attrib = this->readAttribute(camNode, CAMERA_FAR, stream);
						if (attrib == NULL)
						{
							cameraDesc.mFlags.clearBits(CDF_CUSTOM_FAR);
							cameraDesc.mFar = 0;
						}
						else
							cameraDesc.mFar = TStringHelper::toScalar(*attrib);

						attrib = this->readAttribute(camNode, CAMERA_FOV, stream);
						if (attrib == NULL)
						{
							cameraDesc.mFlags.clearBits(CDF_CUSTOM_FOV);
							cameraDesc.mFOV = 0;
						}
						else
							cameraDesc.mFOV = Math::Degree2Radian(TStringHelper::toScalar(*attrib));

						attrib = this->readAttribute(camNode, CAMERA_ASPECT, stream);
						if (attrib == NULL)
						{
							cameraDesc.mFlags.clearBits(CDF_CUSTOM_ASPECT);
							cameraDesc.mAspect = 0;
						}
						else
							cameraDesc.mAspect = TStringHelper::toScalar(*attrib);

						bool isDefaultWidth = false;
						attrib = this->readAttribute(camNode, CAMERA_ORTHO_WIDTH, stream);
						if (attrib == NULL)
							isDefaultWidth = true;
						if (!this->parseSize(attrib, cameraDesc.mWidth, cameraDesc.mWidthPixel))
							BLADE_LOG(Warning, BTString("unable to parse camera dimension"));

						attrib = this->readAttribute(camNode, CAMERA_ORTHO_HEIGHT, stream);
						if (attrib == NULL && isDefaultWidth)
							cameraDesc.mFlags.clearBits(CDF_CUSTOM_ORTHO);

						if (!this->parseSize(attrib, cameraDesc.mHeight, cameraDesc.mHeightPixel))
							BLADE_LOG(Warning, BTString("unable to parse camera dimension"));
					}
				}
				else if (hasReference)
					cameraDesc = outputDesc->mCameraDesc;
				else
				{
					cameraDesc.mCameraRef = CR_MAIN;
					cameraDesc.mFlags.clearBits(CDF_CUSTOM);
					cameraDesc.mFlags.clearBits(CDF_CUSTOM_ALL);
				}
				outputDesc->mCameraDesc = cameraDesc;
			}

			//read output step
			this->readSteps(output_child, outputDesc->mStepDescList, outSchemeDesc.mTypeList, stream, outputDesc->mHasImageEfect || hasReference);

			const TString* enable = this->readAttribute( output_child, ENABLE, stream );
			if (enable == NULL)
			{
				if(!hasReference)
					outputDesc->mEnable = true;
			}
			else
				outputDesc->mEnable = TStringHelper::toBool( *enable, false );

			outSchemeDesc.mOutputList.push_back( outputDesc.unbind() );
		}//for output

		return !outSchemeDesc.mOutputList.empty() && !outSchemeDesc.mTypeList.empty();
	}


	//////////////////////////////////////////////////////////////////////////
	const TString*	SchemeParser::readAttribute(IXmlNode* node,const TString& attribute,const HSTREAM& stream)
	{
		assert( node != NULL );
		assert( stream != NULL );
		BLADE_UNREFERENCED(stream);

		const TString* ret = node->getAttributeValue(attribute);
		//if( ret == NULL )
		//{
		//	BLADE_LOG(Warning, TEXT("Missing \"" << attribute << "\" attribute in entry:") << node->getName()
		//		<< TEXT(" in RenderScheme config file -") << stream->getName());
		//}
		return ret;
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool			SchemeParser::parseSize(const TString* psizeString,scalar& scale,size_t& offset, const TString& defaultVal/* = BTString("100%") */)
	{
		if( psizeString == NULL || *psizeString == DEFAULT )
			psizeString = &defaultVal;
		const TString& sizeString = *psizeString;

		TStringTokenizer token;
		token.tokenize( sizeString, TEXT("+ ") );
		if( token.size() == 2 )
		{
			const TString& s0 = token[0];
			const TString& s1 = token[1];
			scale = 0;
			offset = 0;
			if( s0.size() > 2 &&  s0[s0.size()-1] == TEXT('%') )
				scale = TStringHelper::toScalar( TString( &s0[0], s0.size()-1, TString::CONST_HINT) ) / 100;
			else
				offset = TStringHelper::toSizeT(s0);

			if( s1.size() > 2 &&  s1[s1.size()-1] == TEXT('%') )
				scale += TStringHelper::toScalar( TString( &s1[0], s1.size()-1, TString::CONST_HINT) ) / 100;
			else
				offset += TStringHelper::toSizeT(s1);
		}
		else if( token.size() == 1)
		{
			const TString& s = token[0];
			scale = 0;
			offset = 0;
			if( s.size() > 2 &&  s[s.size()-1] == TEXT('%') )
				scale = TStringHelper::toScalar( TString( &s[0], s.size()-1, TString::CONST_HINT) ) / 100;
			else
				offset = TStringHelper::toSizeT(s);
		}
		else
		{
			assert(false);
			return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SchemeParser::readInputBuffer(IXmlNode* node, int bufferUsage, InputDescList& outDesc, const BufferDescList& buffers, const HSTREAM& stream)
	{
		TempSet<TString> inputSet;
		for (size_t j = 0; j < MAX_INPUT_COUNT; ++j)
		{
			TString InputIndex = OUTPUT_INPUT + TStringHelper::fromUInt(j);
			const TString* input_i = this->readAttribute(node, InputIndex, stream);

			if (input_i == NULL)
			{
				if (j == 0)
				{
					input_i = this->readAttribute(node, OUTPUT_INPUT, stream);
					if (input_i == NULL)
						break;
				}
				else
					break;
			}

			const BUFFER_DESC* desc;
			BUFFER_DESC targetDesc;
			targetDesc.mName = *input_i;
			BufferDescList::const_iterator iter = buffers.find(targetDesc);
			if (iter == buffers.end())
			{
				BLADE_LOG(Warning, TEXT("input reference not found:") + *input_i + BTString(" - ") + stream->getName());
				return false;
			}
			desc = &*(iter);
			iter->mBufferUsage |= bufferUsage;

			if (!inputSet.insert(*input_i).second)
			{
				BLADE_LOG(Warning, TEXT("multiple input buffer not allowed:") + *input_i + BTString(" - ") + stream->getName());
				return false;
			}
			outDesc.push_back(desc);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	SchemeParser::readOutputBuffer(IXmlNode* node, int bufferUsage, 
		TargetDescList& outDesc, const BUFFER_DESC*& outDepthDesc, BUFFER_REF& bufferRef,
		const BufferDescList& buffers, const HSTREAM& stream, bool* useFinalTarget, bool optional/* = false*/)
	{
		TempSet<TString> targetSet;
		BUFFER_REF bref = BR_CUSTOM;

		for (size_t j = 0; j < MAX_TARGET_COUNT; ++j)
		{
			TString TargetIndex = OUTPUT_TARGET + TStringHelper::fromUInt(j);
			const TString* target_i = this->readAttribute(node, TargetIndex, stream);

			if (target_i == NULL)
			{
				if (j == 0)
				{
					target_i = this->readAttribute(node, OUTPUT_TARGET, stream);
					if (target_i == NULL)
						break;
				}
				else
					break;
			}

			const BUFFER_DESC* desc = NULL;
			{
				BUFFER_DESC targetDesc;
				targetDesc.mName = *target_i;
				BufferDescList::const_iterator iter = buffers.find(targetDesc);
				if (iter == buffers.end())
				{
					BLADE_LOG(Warning, TEXT("target reference not found:") + *target_i << BTString(" - ") + stream->getName());
					return false;
				}
				desc = &*(iter);
				iter->mBufferUsage |= bufferUsage;
			}

			if (!targetSet.insert(*target_i).second)
			{
				BLADE_LOG(Warning, TEXT("multiple output target not allowed:") + *target_i << BTString(" - ") + stream->getName());
				return false;
			}

			if (desc == NULL)
			{
				BLADE_LOG(Warning, TEXT("target not defined:") + *target_i + BTString(" - ") + stream->getName());
				continue;
			}
			else
				outDesc.push_back(desc); //NULL means using final buffer
		}

		if (outDesc.size() == 0)
		{
			if(!optional)
				BLADE_LOG(Warning, TEXT("no target defined:") + OUTPUT_TARGET + BTString(" - ") + stream->getName());
			return optional;
		}

		for (size_t i = 0; i < outDesc.size(); ++i)
		{
			const BUFFER_DESC* desc = outDesc[i];
			if (desc->mBufferRef != BR_CUSTOM)
				bref = desc->mBufferRef;
			if (desc->mBufferRef == BR_FINAL)
			{
				if (useFinalTarget != NULL)
					*useFinalTarget = true;
			}
		}

		//currently, if final target is specified or an empty target is specified, then there should not be any other output targets
		if ((bref != BR_CUSTOM) && outDesc.size() > 1)
		{
			BLADE_LOG(Warning, TEXT("final buffer cannot be used with any other target buffer:") + OUTPUT_TARGET + BTString(" - ") + stream->getName());
			return false;
		}
		bufferRef = bref;

		//read depth output
		outDepthDesc = NULL;
		const TString* output_depth = this->readAttribute(node, OUTPUT_DEPTH, stream);
		if (output_depth == NULL)
		{
			//BLADE_LOG(Warning, TEXT("no output depth reference defined:") << OUTPUT_TARGET << BTString(" - ") << stream->getName());
			//continue;
			outDepthDesc = NULL;
		}
		else if (*output_depth == NONE)
			outDepthDesc = NULL;
		else
		{
			BUFFER_DESC targetDesc;
			targetDesc.mName = *output_depth;
			BufferDescList::const_iterator iter = buffers.find(targetDesc);
			if (iter == buffers.end())
			{
				BLADE_LOG(Warning, TEXT("target reference not found:") + *output_depth + BTString(" - ") + stream->getName());
				return false;
			}
			if (bufferRef == BR_FINAL)
				iter->mBufferUsage |= BU_FINAL;

			outDepthDesc = &*(iter);
			iter->mBufferUsage |= bufferUsage;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SchemeParser::readSteps(IXmlNode* node, StepDescList& outSteps, const TypeDescList& typeList, const HSTREAM& stream, bool optional/* = false*/)
	{
		IXmlNode::IXmlNodeList* stepListNode = node->getChildList(STEP_NODE);
		if (stepListNode == NULL || stepListNode->getSize() == 0)
		{
			if (!optional)
			{
				BLADE_LOG(Warning, TEXT("Missing node:") + STEP_NODE + BTString(" for node '") + node->getName() + BTString("' - ") + stream->getName());
				return false;
			}
			return optional;
		}

		for (size_t n = 0; n < stepListNode->getSize(); ++n)
		{
			IXmlNode* stepNode = stepListNode->getAt(n);

			const TString* type = this->readAttribute(stepNode, TYPE, stream);
			if (type == NULL)
			{
				BLADE_LOG(Warning, TEXT("Missing attribute:") + TYPE + BTString("for '") + STEP_NODE + BTString("' - ") + stream->getName());
				continue;
			}

			TYPE_DESC typeDesc;
			typeDesc.mRenderTypeName = *type;
			//BLADE_LOG(Information, TEXT("render type :") + *type + BTString(" - ") + stream->getName());
			TypeDescList::const_iterator iter = typeList.find(typeDesc);
			if (iter == typeList.end())
			{
				BLADE_LOG(Warning, TEXT("render type reference not found: ") + *type + BTString(" - ") + stream->getName());
				continue;
			}

			bool bEnable = true;
			const TString* enable = this->readAttribute(stepNode, ENABLE, stream);
			if (enable != NULL)
				bEnable = TStringHelper::toBool(*enable, false);

			const TString* pass = this->readAttribute(stepNode, STEP_PASS, stream);
			if (pass == NULL)
			{
				BLADE_LOG(Warning, TEXT("empty pass for step type ") << typeDesc.mRenderTypeName
					<< BTString(", now use dynamic pass. - ") << stream->getName());
				pass = &TString::EMPTY;
			}

			const TString* sorter = this->readAttribute(stepNode, STEP_SORTER, stream);
			if (sorter == NULL)
				sorter = &NONE;

			STEP_DESC* stepDesc = BLADE_NEW STEP_DESC();
			stepDesc->mEnable = bEnable;
			stepDesc->mTexturing = true;
			stepDesc->mPassName = *pass;
			stepDesc->mTypeDesc = &*iter;

			TStringTokenizer sorters;
			sorters.tokenize(*sorter, TEXT(","));
			stepDesc->mSorters.reserve(sorters.size());
			for (size_t st = 0; st < sorters.size(); ++st)
			{
				if (sorters[st] != NONE)
					stepDesc->mSorters.push_back(sorters[st]);
			}

			//step groups
			if (stepDesc->mPassName.empty())
			{
				const TString* groups = this->readAttribute(stepNode, STEP_GROUPS, stream);
				if (groups != NULL)
				{
					TStringTokenizer token;
					token.tokenize(*groups, TEXT(","));
					stepDesc->mGroups.reserve(token.size());
					for (size_t gp = 0; gp < token.size(); ++gp)
						stepDesc->mGroups.push_back(token[gp]);
				}
			}

			//step texturing
			const TString* texturing = this->readAttribute(stepNode, STEP_TEXTURING, stream);
			if (texturing != NULL)
			{
				if (texturing->compare_no_case(BTString("FALSE")) == 0)
					stepDesc->mTexturing = false;
				else
					stepDesc->mTexturing = true;
			}
			else
				stepDesc->mTexturing = true;

			outSteps.push_back(stepDesc);
		}//for step

		return true;
	}

}//namespace Blade
