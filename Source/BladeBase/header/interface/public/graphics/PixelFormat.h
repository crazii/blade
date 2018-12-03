/********************************************************************
	created:	2009/02/22
	filename: 	PixelFormat.h
	author:		Crazii
	
	purpose:	pixel format definition
*********************************************************************/
#ifndef __Blade_PixelFormat_h__
#define __Blade_PixelFormat_h__
#include <utility/String.h>

namespace Blade
{

	enum EPixelFormat
	{
		PF_UNDEFINED	= 0,

		///size
		PF_FLAG_8BIT		=	0x0001,
		PF_FLAG_16BIT		=	0x0002,
		PF_FLAG_24BIT		=	0x0003,
		PF_FLAG_32BIT		=	0x0004,
		PF_FLAG_48BIT		=	0x0006,
		PF_FLAG_64BIT		=	0x0008,
		PF_FLAG_96BIT		=	0x000C,
		PF_FLAG_128BIT		=	0x0010,

		///components
		PF_FLAG_1COMPONENT	=	0x00C0,
		PF_FLAG_2COMPONENT	=	0x0080,
		PF_FLAG_3COMPONENT	=	0x0040,
		PF_FLAG_4COMPONENT	=	0x0000,

		PF_FLAG_RGBA		=	0x0100,
		PF_FLAG_RGBX		=	0x0200,
		PF_FLAG_RGB			=	0x0300,
		PF_FLAG_RG			=	0x0400,
		PF_FLAG_R			=	0x0500,
		PF_FLAG_A			=	0x0600,
		PF_FLAG_DEPTH		=	0x0700,
		PF_FLAG_STENCIL		=	0x0800,
		PF_FLAG_DEPTHSTENCIL=	0x0900,

		///fields
		PF_FLAG_U8			=	0x1000,
		PF_FLAG_I8			=	0x2000,
		PF_FLAG_U16			=	0x3000,
		PF_FLAG_I16			=	0x4000,
		PF_FLAG_F16			=	0x5000,
		PF_FLAG_F32			=	0x6000,

		PF_FLAG_COMPRESSED	=	0x7000,
		PF_FLAG_PACKED		=	0x8000,
		PF_FLAG_4444		=	PF_FLAG_PACKED,	///4bit for each channel(R4G4B4A4)
		PF_FLAG_565			=	0x9000,
		PF_FLAG_5551		=	0xA000,
		PF_FLAG_XXX2		=	0xB000,	///2,10,10,10

		///order
		///note: order are in byte order. (OpenGL/FreeImage/DX11)
		PF_FLAG_ORDER_NONE	=	0x00000000,
		PF_FLAG_ORDER_RGBA	=	0x00010000,
		PF_FLAG_ORDER_BGRA	=	0x00020000,
		PF_FLAG_ORDER_ARGB	=	0x00030000,
		PF_FLAG_ORDER_ABGR	=	0x00040000,

		///S3TC block compression
		PF_FLAG_BC1 = 0x00100000,
		PF_FLAG_BC2 = 0x00200000,
		PF_FLAG_BC3 = 0x00300000,
		PF_FLAG_BC4 = 0x00400000,
		PF_FLAG_BC5 = 0x00500000,
		PF_FLAG_BC6H = 0x00600000,

		///ETC2/EAC block compression
		PF_FLAG_ETC2 = 0x00700000,
		PF_FLAG_ETC2EAC = 0x00800000,
		PF_FLAG_EACR11 = 0x00900000,
		PF_FLAG_EACR11S = 0x00A00000,	///signed
		PF_FLAG_EACRG11 = 0x00B00000,
		PF_FLAG_EACRG11S = 0x00C00000,	///signed

		//compression max: 0x03F00000

		PF_FLAG_SPECIAL_PREMULTIPLIED_ALPHA = 0x04000000,
		
		//////////////////////////////////////////////////////////////////////////
		PF_MASK_SIZE			=	0x0000003F,

		PF_MASK_COMPONENT_SIZE	=	0x000000C0,
		PF_SHIFT_COMPONENT_SIZE	=	6,
		PF_MASK_COMPONENT_TYPE	=	0x00000F00,

		PF_MASK_FIELDS			=	0x0000F000,

		PF_MASK_ORDER			=	0x000F0000,
		PF_SHIFT_ORDER			=	16,

		PF_MASK_NO_TYPEORDER	=	PF_MASK_SIZE | PF_MASK_COMPONENT_SIZE | PF_MASK_FIELDS,

		PF_MASK_COMPRESSTYPE	=	0x03F00000,
		PF_MASK_SPECIAL			=	0x7C000000,

		//////////////////////////////////////////////////////////////////////////
		///layout order independent format
		PF_U8			=	PF_FLAG_8BIT | PF_FLAG_1COMPONENT | PF_FLAG_U8,
		PF_U8X2			=	PF_FLAG_16BIT | PF_FLAG_2COMPONENT | PF_FLAG_U8,
		PF_U8X3			=	PF_FLAG_24BIT | PF_FLAG_3COMPONENT | PF_FLAG_U8,
		PF_U8X4			=	PF_FLAG_32BIT | PF_FLAG_4COMPONENT | PF_FLAG_U8,

		PF_I8			=	PF_FLAG_8BIT | PF_FLAG_1COMPONENT | PF_FLAG_I8,
		PF_I8X2			=	PF_FLAG_16BIT | PF_FLAG_2COMPONENT | PF_FLAG_I8,
		PF_I8X3			=	PF_FLAG_24BIT | PF_FLAG_3COMPONENT | PF_FLAG_I8,
		PF_I8X4			=	PF_FLAG_32BIT | PF_FLAG_4COMPONENT | PF_FLAG_I8,

		PF_U16			=	PF_FLAG_16BIT | PF_FLAG_1COMPONENT | PF_FLAG_U16,
		PF_U16X2		=	PF_FLAG_32BIT | PF_FLAG_2COMPONENT | PF_FLAG_U16,
		PF_U16X3		=	PF_FLAG_48BIT | PF_FLAG_3COMPONENT | PF_FLAG_U16,
		PF_U16X4		=	PF_FLAG_64BIT | PF_FLAG_4COMPONENT | PF_FLAG_U16,

		PF_I16			=	PF_FLAG_16BIT | PF_FLAG_1COMPONENT | PF_FLAG_I16,
		PF_I16X2			=	PF_FLAG_32BIT | PF_FLAG_2COMPONENT | PF_FLAG_I16,
		PF_I16X3			=	PF_FLAG_48BIT | PF_FLAG_3COMPONENT | PF_FLAG_I16,
		PF_I16X4			=	PF_FLAG_64BIT | PF_FLAG_4COMPONENT | PF_FLAG_I16,

		PF_F16			=	PF_FLAG_16BIT | PF_FLAG_1COMPONENT | PF_FLAG_F16,
		PF_F16X2		=	PF_FLAG_32BIT | PF_FLAG_2COMPONENT | PF_FLAG_F16,
		PF_F16X3		=	PF_FLAG_48BIT | PF_FLAG_3COMPONENT | PF_FLAG_F16,
		PF_F16X4		=	PF_FLAG_64BIT | PF_FLAG_4COMPONENT | PF_FLAG_F16,

		PF_F32			=	PF_FLAG_32BIT | PF_FLAG_1COMPONENT | PF_FLAG_F32,
		PF_F32X2		=	PF_FLAG_64BIT | PF_FLAG_2COMPONENT | PF_FLAG_F32,
		PF_F32X3		=	PF_FLAG_96BIT | PF_FLAG_3COMPONENT | PF_FLAG_F32,
		PF_F32X4		=	PF_FLAG_128BIT| PF_FLAG_4COMPONENT | PF_FLAG_F32,

		PF_UXXX2		=	PF_FLAG_32BIT | PF_FLAG_4COMPONENT | PF_FLAG_XXX2,
		PF_U4X4			=	PF_FLAG_16BIT | PF_FLAG_4COMPONENT | PF_FLAG_4444,
		PF_U5551		=	PF_FLAG_16BIT | PF_FLAG_4COMPONENT | PF_FLAG_5551,
		PF_U565			=	PF_FLAG_16BIT | PF_FLAG_3COMPONENT | PF_FLAG_565,

		//////////////////////////////////////////////////////////////////////////
		PF_A8			=	PF_U8	| PF_FLAG_A,

		PF_R8			=	PF_U8	| PF_FLAG_R,	///used as R8 or L8
		PF_R8G8			=	PF_U8X2 | PF_FLAG_RG | PF_FLAG_ORDER_RGBA,	///used as R8G8 or A8L8

		PF_R8G8B8		=	PF_U8X3	| PF_FLAG_RGB | PF_FLAG_ORDER_RGBA,
		PF_B8G8R8		=	PF_U8X3	| PF_FLAG_RGB | PF_FLAG_ORDER_BGRA,

		PF_R8G8B8A8		=	PF_U8X4 | PF_FLAG_RGBA | PF_FLAG_ORDER_RGBA,
		PF_R8G8B8X8		=	PF_U8X4 | PF_FLAG_RGBX | PF_FLAG_ORDER_RGBA,
		PF_B8G8R8A8		=	PF_U8X4 | PF_FLAG_RGBA | PF_FLAG_ORDER_BGRA,
		PF_B8G8R8X8		=	PF_U8X4 | PF_FLAG_RGBX | PF_FLAG_ORDER_BGRA,
		PF_A8R8G8B8		=	PF_U8X4 | PF_FLAG_RGBA | PF_FLAG_ORDER_ARGB,
		PF_A8B8G8R8		=	PF_U8X4 | PF_FLAG_RGBA | PF_FLAG_ORDER_ABGR,

		PF_R16			=	PF_U16	| PF_FLAG_R,	///used as R16 or L16
		PF_R16G16		=	PF_U16X2| PF_FLAG_RG,
		PF_R16G16B16A16 =	PF_U16X4| PF_FLAG_RGBA,

		PF_R16F			=	PF_F16	| PF_FLAG_R,
		PF_R16G16F		=	PF_F16X2| PF_FLAG_RG,
		PF_R16G16B16F	=	PF_F16X3| PF_FLAG_RGB,
		PF_R16G16B16A16F=	PF_F16X4| PF_FLAG_RGBA,

		PF_R32F			=	PF_F32	| PF_FLAG_R,
		PF_R32G32F		=	PF_F32X2| PF_FLAG_RG,
		PF_R32G32B32F	=	PF_F32X3| PF_FLAG_RGB,
		PF_R32G32B32A32F=	PF_F32X4| PF_FLAG_RGBA,

		PF_B10G10R10A2	=	PF_UXXX2| PF_FLAG_RGBA | PF_FLAG_ORDER_ABGR,	///yes, PF_FLAG_ORDER_ABGR. this is workaround for most libs & exiting images.
		PF_B4G4R4A4		=	PF_U4X4 | PF_FLAG_RGBA | PF_FLAG_ORDER_BGRA,
		PF_R4G4B4A4		=	PF_U4X4 | PF_FLAG_RGBA | PF_FLAG_ORDER_RGBA,
		PF_B5G5R5A1		=	PF_U5551| PF_FLAG_RGBA | PF_FLAG_ORDER_BGRA,
		PF_R5G5B5A1		=	PF_U5551| PF_FLAG_RGBA | PF_FLAG_ORDER_RGBA,

		PF_R5G6B5		=	PF_U565	| PF_FLAG_RGB | PF_FLAG_ORDER_ARGB,
		PF_B5G6R5		=	PF_U565	| PF_FLAG_RGB | PF_FLAG_ORDER_RGBA,

		//////////////////////////////////////////////////////////////////////////
		PF_DEPTH16		=	PF_FLAG_16BIT | PF_FLAG_1COMPONENT | PF_FLAG_DEPTH,
		PF_DEPTH24S8	=	PF_FLAG_24BIT | PF_FLAG_2COMPONENT | PF_FLAG_DEPTHSTENCIL,
		PF_DEPTH32		=	PF_FLAG_32BIT | PF_FLAG_1COMPONENT | PF_FLAG_DEPTH,
		PF_DEPTH32F		=	PF_FLAG_32BIT | PF_FLAG_1COMPONENT | PF_FLAG_DEPTH | PF_FLAG_F32,

		///@note the size of compressed format is the bytes of a block, not pixel
		PF_BC1			=	PF_FLAG_COMPRESSED | PF_FLAG_BC1 | PF_FLAG_64BIT | PF_FLAG_3COMPONENT,
		PF_BC2			=	PF_FLAG_COMPRESSED | PF_FLAG_BC2 | PF_FLAG_128BIT | PF_FLAG_4COMPONENT,
		PF_BC3			=	PF_FLAG_COMPRESSED | PF_FLAG_BC3 | PF_FLAG_128BIT | PF_FLAG_4COMPONENT,
		PF_BC4			=	PF_FLAG_COMPRESSED | PF_FLAG_BC4 | PF_FLAG_64BIT | PF_FLAG_1COMPONENT,
		PF_BC5			=	PF_FLAG_COMPRESSED | PF_FLAG_BC5 | PF_FLAG_128BIT | PF_FLAG_2COMPONENT,
		PF_BC6H			=	PF_FLAG_COMPRESSED | PF_FLAG_BC6H | PF_FLAG_128BIT | PF_FLAG_3COMPONENT,

		PF_DXT1			=	PF_BC1,
		PF_DXT3			=	PF_BC2,
		PF_DXT5			=	PF_BC3,
		PF_DXT2			=	PF_DXT3 | PF_FLAG_SPECIAL_PREMULTIPLIED_ALPHA,
		PF_DXT4			=	PF_DXT5 | PF_FLAG_SPECIAL_PREMULTIPLIED_ALPHA,

		PF_RGB_ETC2		=	PF_FLAG_COMPRESSED | PF_FLAG_ETC2 | PF_FLAG_64BIT | PF_FLAG_3COMPONENT,
		PF_RGBA_ETC2EAC	=	PF_FLAG_COMPRESSED | PF_FLAG_ETC2EAC | PF_FLAG_128BIT | PF_FLAG_4COMPONENT,
		PF_R_EAC		=	PF_FLAG_COMPRESSED | PF_FLAG_EACR11 | PF_FLAG_64BIT | PF_FLAG_1COMPONENT,
		PF_RG_EAC		=	PF_FLAG_COMPRESSED | PF_FLAG_EACRG11 | PF_FLAG_128BIT | PF_FLAG_2COMPONENT,

		///bit order
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE

		PF_RGB = PF_B8G8R8,
		PF_RGB565 = PF_B5G6R5,
		PF_BGR = PF_R8G8B8,
		PF_BGR565 = PF_R5G6B5,

		PF_ARGB = PF_B8G8R8A8,
		PF_BGRA = PF_A8R8G8B8,
		PF_RGBA = PF_A8B8G8R8,
		PF_ABGR	= PF_R8G8B8A8,

		PF_XRGB	= PF_B8G8R8X8,
		PF_XBGR	= PF_R8G8B8X8,
#else
		PF_RGB = PF_R8G8B8,
		PF_RGB565 = PF_R5G6B5,
		PF_BGR = PF_B8G8R8,
		PF_BGR565 = PF_B5G6R5,

		PF_ARGB = PF_A8R8G8B8,
		PF_BGRA = PF_B8G8R8A8,
		PF_RGBA = PF_A8B8G8R8,
		PF_ABGR = PF_A8B8G8R8,

		PF_XRGB	= PF_R8G8B8X8,
		PF_XBGR	= PF_B8G8R8X8,
#endif
	};

	///@note different render API has different texture coordinates,
	///in order to keep the uv stream independent,
	///while loading a texture, we vertically flip the texture if needed, due to this layout
	typedef enum EImageOrientation
	{
		IMGO_DEFAULT,	//image's default
		IMGO_TOP_DOWN,	
		IMGO_BOTTOM_UP,
	}IMG_ORIENT;

	///pixel byte order enum for different platform / graphics APIs.
	typedef enum EPixelByteOrder
	{
		PO_NONE	= PF_FLAG_ORDER_NONE >> PF_SHIFT_ORDER,
		PO_RGBA = PF_FLAG_ORDER_RGBA >> PF_SHIFT_ORDER,
		PO_BGRA = PF_FLAG_ORDER_BGRA >> PF_SHIFT_ORDER,
		PO_ARGB = PF_FLAG_ORDER_ARGB >> PF_SHIFT_ORDER,
		PO_ABGR = PF_FLAG_ORDER_ABGR >> PF_SHIFT_ORDER,

		PO_START	= PO_NONE,
		PO_COUNT	= PO_ABGR + 1,
	}PIXEL_ORDER;

	///PixelFormat class wrapper for enum, this class uses bitfield and not suitable for direct serialization,
	///since bitfield 's order (MSB or LSB) is impl-defined
	class BLADE_BASE_API PixelFormat
	{
	public:
		PixelFormat(const TString& fmtString);

		PixelFormat()					:mFormatEnum(PF_UNDEFINED), mSRGB(EPixelFormat(1))	{}
		PixelFormat(EPixelFormat format):mFormatEnum(format), mSRGB(EPixelFormat(1))		{}
		inline ~PixelFormat()									{}

		/** @brief  */
		inline const PixelFormat& operator=(EPixelFormat format)
		{
			mFormatEnum = format;
			return *this;
		}

		/** @brief  */
		inline bool	operator==(EPixelFormat format) const
		{
			return mFormatEnum == format;
		}

		/** @brief  */
		inline operator EPixelFormat() const
		{
			return mFormatEnum;
		}

		/** @brief bytes per pixel */
		inline size_t	getSizeBytes()	const
		{
			return size_t(((mFormat&PF_MASK_SIZE))*sizeof(char) );
		}

		/** @brief  */
		inline bool	hasAlpha() const
		{
			return (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_A || (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_RGBA;
		}

		/** @brief  */
		inline bool	isFloat16() const
		{
			return (mFormat&PF_MASK_FIELDS) == PF_FLAG_F16;
		}

		/** @brief  */
		inline bool	isFloat32() const
		{
			return (mFormat&PF_MASK_FIELDS) == PF_FLAG_F32;
		}

		/** @brief  */
		inline bool	isFloat() const
		{
			return this->isFloat32() || this->isFloat16();
		}

		/** @brief  */
		inline bool isUint8() const
		{
			return (mFormat&PF_MASK_FIELDS) == PF_FLAG_U8;
		}

		/** @brief  */
		inline bool isUint16() const
		{
			return (mFormat&PF_MASK_FIELDS) == PF_FLAG_U16;
		}

		/** @brief  */
		inline bool	isDepth() const
		{
			return (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_DEPTH || (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_DEPTHSTENCIL;
		}

		/** @brief  */
		inline bool	isStencil() const
		{
			return (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_STENCIL || (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_DEPTHSTENCIL;;
		}

		/** @brief  */
		inline bool	isDepthStencil() const
		{
			return (mFormat&PF_MASK_COMPONENT_TYPE) == PF_FLAG_DEPTHSTENCIL;
		}

		/** @brief  */
		inline bool	isCompressed() const
		{
			return (mFormat&PF_MASK_FIELDS) == PF_FLAG_COMPRESSED;
		}

		/** @brief  */
		inline size_t getComponents() const
		{
			return 4 - ((mFormat&PF_MASK_COMPONENT_SIZE) >> PF_SHIFT_COMPONENT_SIZE);
		}

		/** @brief  */
		inline bool	hasByteOrder() const
		{
			return (mFormat&PF_MASK_ORDER) != PF_FLAG_ORDER_NONE;
		}

		/** @brief  */
		inline PIXEL_ORDER getByteOrder() const
		{
			int val = (int)( (mFormat&PF_MASK_ORDER)  >> PF_SHIFT_ORDER );
			assert( val >= PO_START && val < PO_COUNT );
			return (PIXEL_ORDER)val;
		}

		/** @brief  */
		inline bool		setByteOrder(PIXEL_ORDER bo)
		{
			if( this->hasByteOrder() && bo >= PO_START && bo < PO_COUNT )
			{
				mFormat &= ~PF_MASK_ORDER;	//clear
				mFormat |= bo << PF_SHIFT_ORDER;	//reset
				return true;
			}
			assert(false);
			return false;
		}

		/** @brief  */
		inline bool	isPacked() const
		{
			return (mFormat&PF_MASK_FIELDS) >= PF_FLAG_PACKED;
		}

		/** @brief  */
		uint operator&(int mask) const
		{
			return mFormat&mask;
		}

		/** @brief  */
		uint operator&(EPixelFormat format) const
		{
			return mFormat&format;
		}

		/** @brief linear color space. default: false */
		inline bool isLinearColor() const { return mSRGB == 0; }

		/** @brief set linear space. NOTE: you need to set linear space BEFORE creating texture/image */
		inline void setLinearColor(bool linear)
		{
			mSRGB = linear ? EPixelFormat(0) : EPixelFormat(1);
		}

		/** @brief  */
		const TString&	getString() const;

	private:
		union
		{
			struct
			{
				EPixelFormat	mFormatEnum : 31;	//debug watch
				int				mSRGB : 1;			//color space: linear/sRGB. default using sRGB (1). 
													//Note: sRGB only has effect BEFORE texture creation, nothing happens if changing sRGB after texture created
			};
			uint			mFormat;
		};
	};//class PixelFormat
	static_assert(sizeof(PixelFormat) == sizeof(uint), "pixel format size error");
	
}//namespace Blade


#endif // __Blade_PixelFormat_h__