/********************************************************************
	created:	2015/01/26
	filename: 	GLESPixelFormat.h
	author:		Crazii
	
	purpose:	pixel format converter: shared between runtime & file formats(KTX texture compressor)
*********************************************************************/
#ifndef __Blade_GLESPixelFormat_h__
#define __Blade_GLESPixelFormat_h__

#include "../GLESHeader.h"
#if BLADE_USE_GLES

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GLESPixelFormat
	{
	public:
		GLint	internalFormat;
		GLenum	format;
		GLenum	type;
	};
	class GLESPixelFormatConverter
	{
	public:
		GLESPixelFormatConverter(const GLESPixelFormat& GLformat)
			:mGLformat(GLformat)
		{
			switch(GLformat.internalFormat)
			{
			case GL_LUMINANCE:	//L8
				mFormat = PF_R8;
				break;
			case GL_ALPHA:
				mFormat = PF_A8;
				break;
			case GL_R8:
				mFormat = PF_R8;
				break;
			case GL_RG8:
				mFormat = PF_R8G8;
				break;
			case GL_RGB10_A2:
				mFormat = PF_B10G10R10A2;
				break;
			case GL_RGBA4:
				mFormat = PF_R4G4B4A4;
				break;
			case GL_RGB5_A1:
				mFormat = PF_R5G5B5A1;
				break;
			case GL_RGB565:
				mFormat = PF_RGB565;
				break;
			case GL_RGB:
			case GL_SRGB8:
			case GL_RGB8:
				if( GLformat.internalFormat != GL_RGB8 && GLformat.type == GL_UNSIGNED_SHORT_5_6_5 )
					mFormat = PF_RGB565;
				else
					mFormat = PF_R8G8B8;
				mFormat.setLinearColor(GLformat.internalFormat == GL_SRGB8);
				break;
			case GL_RGBA:
			case GL_SRGB8_ALPHA8:
			case GL_RGBA8:
				if (GLformat.internalFormat == GL_RGBA8)
					mFormat = PF_R8G8B8A8;
				else if (GLformat.type == GL_UNSIGNED_SHORT_4_4_4_4)
					mFormat = PF_B4G4R4A4;
				else if (GLformat.type == GL_UNSIGNED_SHORT_5_5_5_1)
					mFormat = PF_B5G5R5A1;
				else
					assert(false);
				mFormat.setLinearColor(GLformat.internalFormat == GL_SRGB8_ALPHA8);
				break;
#if 0
			case GL_R8I:
				break;
			case GL_R8UI:
				break;
			case GL_RG8I:
				break;
			case GL_RG8UI:
				break;
			case GL_RGB8I:
				break;
			case GL_RGB8UI:
				break;
			case GL_RGBA8I:
				break;
			case GL_RGBA8UI:
				break;
#endif
			case GL_R16I:
				break;
			case GL_R16UI:
				mFormat = PF_R16;
				break;
			case GL_RG16I:
				break;
			case GL_RG16UI:
				mFormat = PF_R16G16;
				break;
			case GL_RGBA16I:
				break;
			case GL_RGBA16UI:
				mFormat = PF_R16G16B16A16;
				break;

			case GL_R16F:
				mFormat = PF_R16F;
				break;
			case GL_RG16F:
				mFormat = PF_R16G16F;
				break;
			case GL_RGB16F:
				mFormat = PF_R16G16B16F;
				break;
			case GL_RGBA16F:
				mFormat = PF_R16G16B16A16F;
				break;

			case GL_R32F:
				mFormat = PF_R32F;
				break;
			case GL_RG32F:
				mFormat = PF_R32G32F;
				break;
			case GL_RGB32F:
				mFormat = PF_R32G32B32F;
				break;
			case GL_RGBA32F:
				mFormat = PF_R32G32B32A32F;
				break;

			case GL_COMPRESSED_RGB8_ETC2:
			case GL_COMPRESSED_SRGB8_ETC2:
				mFormat = PF_RGB_ETC2;
				mFormat.setLinearColor(GLformat.internalFormat == GL_COMPRESSED_SRGB8_ETC2);
				break;
			case GL_COMPRESSED_RGBA8_ETC2_EAC:
			case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
				mFormat = PF_RGBA_ETC2EAC;
				mFormat.setLinearColor(GLformat.internalFormat == GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC);
				break;
			case GL_COMPRESSED_R11_EAC:
				mFormat = PF_R_EAC;
				break;
			case GL_COMPRESSED_RG11_EAC:
				mFormat = PF_RG_EAC;
				break;
			default:
				assert(false && "unsupported format.");
				mFormat = PF_UNDEFINED;
				break;
			}
		}

		GLESPixelFormatConverter(GLint internalFormat, GLenum format, GLenum type)
		{
			GLESPixelFormat glesFormat = {internalFormat, format, type};
			*this = GLESPixelFormatConverter(glesFormat);
		}

		GLESPixelFormatConverter(PixelFormat fmt)
			:mFormat(fmt)
		{
			GLint	internalFormat;
			GLenum	format;
			GLenum	type;
			bool linear = fmt.isLinearColor();
			if(fmt.hasByteOrder())
				fmt.setByteOrder(PO_RGBA);

			switch( fmt )
			{
			case PF_R8:
				internalFormat = GL_R8; format = GL_RED; type = GL_UNSIGNED_BYTE;
				break;
			case PF_A8:
				internalFormat = GL_ALPHA; format = GL_ALPHA; type = GL_UNSIGNED_BYTE;
				break;
			case PF_R8G8:
				internalFormat = GL_RG8; format = GL_RG; type = GL_UNSIGNED_BYTE;
				break;
			case PF_R8G8B8:
				internalFormat = linear ? GL_RGB8 : GL_SRGB8; format = GL_RGB; type = GL_UNSIGNED_BYTE;
				break;
			case PF_R8G8B8A8:
				internalFormat = linear ? GL_RGBA8 : GL_SRGB8_ALPHA8; format = GL_RGBA; type = GL_UNSIGNED_BYTE;
				break;
			case PF_B10G10R10A2:
				internalFormat = GL_RGB10_A2; format = GL_RGBA; type = GL_UNSIGNED_INT_2_10_10_10_REV;
				break;
			case PF_B4G4R4A4:
			case PF_R4G4B4A4:
				internalFormat = GL_RGBA4; format = GL_RGBA; type = GL_UNSIGNED_SHORT_4_4_4_4;
				break;
			case PF_B5G5R5A1:
			case PF_R5G5B5A1:
				internalFormat = GL_RGB5_A1; format = GL_RGBA; type = GL_UNSIGNED_SHORT_5_5_5_1;
				break;
			case PF_RGB565:
				internalFormat = GL_RGB565; format = GL_RGB; type = GL_UNSIGNED_SHORT_5_6_5;
				break;
			case PF_DEPTH16:
				internalFormat = GL_DEPTH_COMPONENT16; format = GL_DEPTH_COMPONENT; type = GL_UNSIGNED_SHORT;
				break;
			case PF_DEPTH32:
				internalFormat = GL_DEPTH_COMPONENT32F; format = GL_DEPTH_COMPONENT; type = GL_FLOAT;
				break;
			case PF_DEPTH24S8:
				internalFormat = GL_DEPTH24_STENCIL8; format = GL_DEPTH_STENCIL; type = GL_UNSIGNED_INT_24_8;
				break;
			case PF_DEPTH32F:
				internalFormat = GL_DEPTH_COMPONENT32F; format = GL_DEPTH_COMPONENT; type = GL_FLOAT;
				break;
			case PF_R16F:
				internalFormat = GL_R16F; format = GL_RED; type = GL_HALF_FLOAT;
				break;
			case PF_R16G16F:
				internalFormat = GL_RG16F; format = GL_RG; type = GL_HALF_FLOAT;
				break;
			case PF_R16G16B16F:
				internalFormat = GL_RGB16F; format = GL_RGB; type = GL_HALF_FLOAT;
				break;
			case PF_R16G16B16A16F:
				internalFormat = GL_RGBA16F; format = GL_RGBA; type = GL_HALF_FLOAT;
				break;
			case PF_R32F:
				internalFormat = GL_R32F; format = GL_RED; type = GL_FLOAT;
				break;
			case PF_R32G32F:
				internalFormat = GL_RG32F; format = GL_RG; type = GL_FLOAT;
				break;
			case PF_R32G32B32F:
				internalFormat = GL_RGB32F; format = GL_RGB; type = GL_FLOAT;
				break;
			case PF_R32G32B32A32F:
				internalFormat = GL_RGBA32F; format = GL_RGBA; type = GL_FLOAT;
				break;
				//note:for compressed format, only internalFormat is used, format & type is not used, fill them with any proper value.

#if 0	//ETC2 is standard and why bother other format?
				//support BCn format, just in case for WindowsPhone / Tegra chipset
				//https://www.khronos.org/registry/gles/extensions/EXT/texture_compression_s3tc.txt
			case PF_BC1:	//DXT1
				internalFormat = COMPRESSED_RGB_S3TC_DXT1_EXT; format = GL_RGB; type = 0;
				break;
			case PF_BC2:	//DXT3
				internalFormat = COMPRESSED_RGBA_S3TC_DXT3_EXT; format = GL_RGBA; type = 0;
				break;
			case PF_BC3:	//DXT5
				internalFormat = COMPRESSED_RGBA_S3TC_DXT5_EXT; format = GL_RGBA; type = 0;
				break;
			case PF_BC4:
				break;
			case PF_BC5:
				break;
#endif

			case PF_RGB_ETC2:
				internalFormat = linear ? GL_COMPRESSED_RGB8_ETC2 : GL_COMPRESSED_SRGB8_ETC2; format = GL_RGB; type = 0;
				break;
			case PF_RGBA_ETC2EAC:
				internalFormat = linear ? GL_COMPRESSED_RGBA8_ETC2_EAC : GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC; format = GL_RGBA; type = 0;
				break;
			case PF_R_EAC:
				internalFormat = GL_COMPRESSED_R11_EAC; format = GL_RED; type = 0;
				break;
			case PF_RG_EAC:
				internalFormat = GL_COMPRESSED_RG11_EAC; format = GL_RG; type = 0;
				break;
			default:
				if( fmt.isCompressed() || fmt.isDepth() || fmt.isDepthStencil() )
				{
					assert(false && "unsupported format.");
					internalFormat = 0; format = 0; type = 0;
				}
				if( fmt.hasByteOrder() && fmt.getByteOrder() != PO_RGBA )	//OGL/ES is RGBA order
				{
					assert(false && "unsupported byte order.");
					internalFormat = 0; format = 0; type = 0;
				}
				else
				{
					if(fmt == PF_U8 )
					{internalFormat = GL_R8UI; format = GL_RED_INTEGER; type = GL_UNSIGNED_BYTE;}
					else if(fmt == PF_U8X2)
					{internalFormat = GL_RG8UI; format = GL_RG_INTEGER; type = GL_UNSIGNED_BYTE;}
					else if(fmt == PF_U8X4)
					{internalFormat = GL_RGBA8UI; format = GL_RGBA_INTEGER; type = GL_UNSIGNED_BYTE;}
					else if(fmt == PF_I8)
					{internalFormat = GL_R8I; format = GL_RED_INTEGER; type = GL_UNSIGNED_BYTE;}
					else if(fmt == PF_I8X2)
					{internalFormat = GL_RG8I; format = GL_RG_INTEGER; type = GL_UNSIGNED_BYTE;}
					else if(fmt == PF_I8X4)
					{internalFormat = GL_RGBA8I; format = GL_RGBA_INTEGER; type = GL_UNSIGNED_BYTE;}

					else if(fmt == PF_U16)
					{internalFormat = GL_R16UI; format = GL_RED_INTEGER; type = GL_UNSIGNED_SHORT;}
					else if(fmt == PF_U16X2)
					{internalFormat = GL_RG16UI; format = GL_RG_INTEGER; type = GL_UNSIGNED_SHORT;}
					else if(fmt == PF_U16X4)
					{internalFormat = GL_RGBA16UI; format = GL_RGBA_INTEGER; type = GL_UNSIGNED_SHORT;}
					else if(fmt == PF_I16)
					{internalFormat = GL_R16I; format = GL_RED_INTEGER; type = GL_UNSIGNED_SHORT;}
					else if(fmt == PF_I16X2)
					{internalFormat = GL_RG16I; format = GL_RG_INTEGER; type = GL_UNSIGNED_SHORT;}
					else if(fmt == PF_I16X4)
					{internalFormat = GL_RGBA16I; format = GL_RGBA_INTEGER; type = GL_UNSIGNED_SHORT;}

					else if(fmt == PF_F16)
					{internalFormat = GL_R16F; format = GL_RED; type = GL_HALF_FLOAT;}
					else if(fmt == PF_F16X2)
					{internalFormat = GL_RG16F; format = GL_RG; type = GL_HALF_FLOAT;}
					else if(fmt == PF_F16X3)
					{internalFormat = GL_RGB16F; format = GL_RGB; type = GL_HALF_FLOAT;}
					else if(fmt == PF_F16X4)
					{internalFormat = GL_RGBA16F; format = GL_RGBA; type = GL_HALF_FLOAT;}

					else if(fmt == PF_F32)
					{internalFormat = GL_R32F; format = GL_RED; type = GL_FLOAT;}
					else if(fmt == PF_F32X2)
					{internalFormat = GL_RG32F; format = GL_RG; type = GL_FLOAT;}
					else if(fmt == PF_F32X3)
					{internalFormat = GL_RGB32F; format = GL_RGB; type = GL_FLOAT;}
					else if(fmt == PF_F32X4)
					{internalFormat = GL_RGBA32F; format = GL_RGBA; type = GL_FLOAT;}
					else
					{
						assert(false && "unsupported format.");
						internalFormat = 0; format = 0; type = 0;
					}
				}
				break;
			}

			GLESPixelFormat glesFormat = {internalFormat, format, type};
			mGLformat = glesFormat;
		}

		/** @brief  */
		inline operator PixelFormat() const				{return mFormat;}
		/** @brief  */
		inline operator const GLESPixelFormat&() const	{return mGLformat;}

		/** @brief  */
		static GLESPixelFormat getBackBufferFormat()
		{
			return msBackBufferFormat;
		}

		/** @brief  */
		static void setBackBufferFormat(GLESPixelFormat format)
		{
			msBackBufferFormat = format;
		}

	protected:
		GLESPixelFormat	mGLformat;
		PixelFormat		mFormat;

		static GLESPixelFormat msBackBufferFormat;
	};//GLESPixelFormatConverter

}//namespace Blade

#endif

#endif//__Blade_GLESPixelFormat_h__