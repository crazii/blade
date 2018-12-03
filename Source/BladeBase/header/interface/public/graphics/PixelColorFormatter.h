/********************************************************************
	created:	2014/05/12
	filename: 	PixelColorFormatter.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_PixelColorFormatter_h__
#define __Blade_PixelColorFormatter_h__
#include "Color.h"
#include "PixelFormat.h"
#include <math/Half.h>

namespace Blade
{
	/************************************************************************/
	/* simple converter between 32 BPP rgba byte orders                                                                     */
	/************************************************************************/
	class RGBAConverter : public NonAllocatable
	{
	protected:
		typedef Color::COLOR (*FnConvertRGBA)(Color::COLOR);
	public:
		inline RGBAConverter() :mOrder(PO_RGBA)				{}
		inline RGBAConverter(PIXEL_ORDER layout) :mOrder(layout){}
		inline RGBAConverter& operator=(PIXEL_ORDER layout)		{mOrder = layout;return *this;}
		inline bool operator==(PIXEL_ORDER rhs) const			{return mOrder == rhs;}
		inline bool operator==(const RGBAConverter rhs) const				{return mOrder == rhs.mOrder;}
		inline bool operator!=(PIXEL_ORDER rhs) const			{return mOrder != rhs;}
		inline bool operator!=(const RGBAConverter rhs) const				{return mOrder != rhs.mOrder;}
		inline operator PIXEL_ORDER() const						{return mOrder;}

		/** @brief RGBA */
		inline Color::COLOR	convertFromRGBA(Color::COLOR rgba) const
		{
			return msFunList[mOrder][PO_RGBA][0](rgba);
		}
		inline Color::COLOR	convertToRGBA(Color::COLOR myColor) const
		{
			return msFunList[mOrder][PO_RGBA][1](myColor);
		}

		/** @brief ARGB */
		inline Color::COLOR	convertFromARGB(Color::COLOR argb) const
		{
			return msFunList[mOrder][PO_ARGB][0](argb);
		}

		inline Color::COLOR	convertToARGB(Color::COLOR myColor) const
		{
			return msFunList[mOrder][PO_ARGB][1](myColor);
		}

		/** @brief ABGR */
		inline Color::COLOR	convertFromABGR(Color::COLOR abgr) const
		{
			return msFunList[mOrder][PO_ABGR][0](abgr);
		}
		inline Color::COLOR	convertToABGR(Color::COLOR myColor) const
		{
			return msFunList[mOrder][PO_ABGR][1](myColor);
		}

		/** @brief BGRA */
		inline Color::COLOR	convertFromBGRA(Color::COLOR bgra) const
		{
			return msFunList[mOrder][PO_BGRA][0](bgra);
		}
		inline Color::COLOR	convertToBGRA(Color::COLOR myColor) const
		{
			return msFunList[mOrder][PO_BGRA][1](myColor);
		}

		/** @brief Dynamic */
		inline Color::COLOR convertFrom(PIXEL_ORDER layout, Color::COLOR color) const
		{
			return msFunList[mOrder][layout][0](color);
		}

		inline Color::COLOR convertTo(PIXEL_ORDER layout, Color::COLOR color) const
		{
			return msFunList[mOrder][layout][1](color);
		}

		/** @brief  */
		Color::COLOR	packColor(const Color c) const
		{
			switch (mOrder)
			{
			case PO_RGBA:
				return c.getRGBA();
			case PO_ARGB:
				return c.getARGB();
			case PO_ABGR:
				return c.getABGR();
			case PO_BGRA:
				return c.getBGRA();
			default:
				BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("bad enum value."));
				//return 0;
			}
		}

		/** @brief  */
		Color			unpackColor(const Color::COLOR c) const
		{
			Color color;
			switch (mOrder)
			{
			case PO_RGBA:
				color.setRGBA(c);
				break;
			case PO_ARGB:
				color.setARGB(c);
				break;
			case PO_ABGR:
				color.setABGR(c);
				break;
			case PO_BGRA:
				color.setBGRA(c);
				break;
			default:
				BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("bad enum value."));
				//return Color::BLACK;
			}
			return color;
		}

		/** @brief channel to byte-order index */
		inline const int*	getChannelIndexList() const
		{
			return msIndexList[mOrder];
		}

		/** @brief  byte order index: you can use this to decide the real byte order index of the channel
		 i.e. char color[4];  color[ getChannelIndex(Color::R) ]= "red color"  */
		inline const int	getChannelIndex(Color::EChannel c) const
		{
			return this->getChannelIndexList()[c];
		}

	protected:
		PIXEL_ORDER		mOrder;
		BLADE_BASE_API static FnConvertRGBA msFunList[PO_COUNT][PO_COUNT][2];
		BLADE_BASE_API static const int msIndexList[PO_COUNT][4];
		friend class RGBAConverter2;
	};//class RGBAConverter


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class RGBAConverter2 : public NonCopyable, public NonAllocatable
	{
	public:
		RGBAConverter2(PIXEL_ORDER from, PIXEL_ORDER to)
		{
			mFunc = (from == to) ? NULL : RGBAConverter::msFunList[from][to][1];
		}

		/** @brief  */
		inline bool needConvert() const
		{
			return mFunc != NULL;
		}

		/** @brief  */
		inline Color::COLOR convertTo(Color::COLOR color) const
		{
			return mFunc(color);
		}

		/** @brief  */
		inline void convert(Color::COLOR& color) const
		{
			color = mFunc(color);
		}
	protected:
		RGBAConverter::FnConvertRGBA mFunc;
	};

	/************************************************************************/
	/* generic color formatter for pixel formats                                                                     */
	/************************************************************************/
#define CF_U32MSB0(_u32) ((uint8)(((_u32)>>24)&0x000000FF))
#define CF_U32MSB1(_u32) ((uint8)(((_u32)&0x00FF0000) >> 16))
#define CF_U32MSB2(_u32) ((uint8)(((_u32)&0x0000FF00) >> 8))
#define CF_U32MSB3(_u32) ((uint8)((_u32)&0x000000FF))
//MSB|0|1|2|3|LSB
#define CF_U16MSB0(_u16) ((uint16)(((_u16)>>12)&0xFu))
#define CF_U16MSB1(_u16) ((uint16)(((_u16)>>8)&0xFu))
#define CF_U16MSB2(_u16) ((uint16)(((_u16)>>4)&0xFu))
#define CF_U16MSB3(_u16) ((uint16)((_u16)&0xF))
#define CF_UNPACK4TO8(b4) ((uint8)((b4)*0xFFu/0xFu))
#define CF_PACK8TO4(b8)  ((uint8)((b8)*0xFu/0xFFu))

#define CF_MAKEU32(_msb0,_msb1,_msb2,_msb3) ((uint32)( ((uint32)((_msb0)&0xFF)) << 24 | ((uint32)((_msb1)&0xFF)) << 16 | ((uint32)((_msb2)&0xFF)) << 8 | ((uint32)((_msb3)&0xFF)) ))
#define CF_MAKEU16(_msb0,_msb1,_msb2,_msb3) ((uint16)( ((uint16)((_msb0)&0xF)) << 12 | ((uint16)((_msb1)&0xF)) << 8 | ((uint16)((_msb2)&0xF)) << 4 | ((uint16)((_msb3)&0xF)) ))

	class ColorFormatter : public NonCopyable, public NonAllocatable
	{
	private:
		ColorFormatter() {}

	public:

		/** @brief  */
		static inline bool	readColor(const void* unit, size_t bytes, Color& outColor, PixelFormat format);

		/** @brief  */
		static inline bool	writeColor(void* unit, size_t bytes, const Color& color, PixelFormat format);

		/** @brief  */
		static inline bool	readColorLDR(const void* color, Color::RGBA& outColor, PixelFormat format);

		/** @brief  */
		static inline bool	writeColorLDR(void* outColor, const Color::RGBA& color, PixelFormat format);

	};//class ColorFormatter


	//////////////////////////////////////////////////////////////////////////
	inline bool	ColorFormatter::readColor(const void* unit, size_t bytes, Color& outColor, PixelFormat format)
	{
		if( format.getSizeBytes() == bytes && unit != NULL)
		{
			switch (format)
			{
			case PF_A8:
				{
					const uint8 val = *(const uint8*)unit;
					outColor.setComponent8U(0, 0, 0, val);
					break;
				}
			case PF_R8:
				{
					const uint8* val = (const uint8*)unit;
					uint8 r = val[0];
					outColor.setComponent8U(r, 0, 0);
					break;
				}
			case PF_R8G8: //byte order RG
				{
					const uint8* val = (const uint8*)unit;
					uint8 r = val[0];
					uint8 g = val[1];
					uint8 b = 0;
					outColor.setComponent8U(r, g, b);
					break;
				}
			case PF_R8G8B8: //byte order RGB
				{
					const uint8* val = (const uint8*)unit;
					uint8 r = val[0];
					uint8 g = val[1];
					uint8 b = val[2];
					outColor.setComponent8U(r, g, b);
					break;
				}
			case PF_B8G8R8: //byte order BGR
				{
					const uint8* val = (const uint8*)unit;
					uint8 b = val[0];
					uint8 g = val[1];
					uint8 r = val[2];
					outColor.setComponent8U(r, g, b);
					break;
				}
				//bit orders
			case PF_RGB565:
				{
					uint16 val = *(const uint16*)unit;
					uint8 r = (uint8)((val >> 11)*0xFF/0x1F);
					uint8 g = (uint8)(((val >> 5)&0x3F)*0xFF/0x3F);
					uint8 b = (uint8)((val&0x1F)*0xFF/0x1F);
					outColor.setComponent8U(r, g, b);
					break;
				}
			case PF_BGR565:
				{
					uint16 val = *(const uint16*)unit;
					uint8 b = uint8((val >> 11)*0xFF/0x1F);
					uint8 g = uint8(((val >> 5)&0x3F)*0xFF/0x3F);
					uint8 r = uint8((val&0x1F)*0xFF/0x1F);
					outColor.setComponent8U(r, g, b);
					break;
				}
			case PF_RGBA:
				{
					uint32 val = *(const uint32*)unit;
					uint8 r = CF_U32MSB0(val);
					uint8 g = CF_U32MSB1(val);
					uint8 b = CF_U32MSB2(val);
					uint8 a = CF_U32MSB3(val);
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_BGRA:
				{
					uint32 val = *(const uint32*)unit;
					uint8 b = CF_U32MSB0(val);
					uint8 g = CF_U32MSB1(val);
					uint8 r = CF_U32MSB2(val);
					uint8 a = CF_U32MSB3(val);
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_ARGB:
			case PF_XRGB:
				{
					uint32 val = *(const uint32*)unit;
					uint8 a = CF_U32MSB0(val);
					uint8 r = CF_U32MSB1(val);
					uint8 g = CF_U32MSB2(val);
					uint8 b = CF_U32MSB3(val);
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_ABGR:
			case PF_XBGR:
				{
					uint32 val = *(const uint32*)unit;
					uint8 a = CF_U32MSB0(val);
					uint8 b = CF_U32MSB1(val);
					uint8 g = CF_U32MSB2(val);
					uint8 r = CF_U32MSB3(val);
					outColor.setComponent8U(r, g, b, a);
					break;
				}
				//note: byte order for A2R10G10B10 & PF_A2B10G10R10 is swapped(incorrect) for history reason
			case PF_B10G10R10A2:
				{
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE
					uint32 val = *(const uint32*)unit;
					fp32 a = (fp32)((val>>30)&0x00000003) / 3.0f;
					fp32 b = (fp32)((val>>20)&0x000003FF) / 1023.0f;
					fp32 g = (fp32)((val>>10)&0x000003FF) / 1023.0f;
					fp32 r = (fp32)(val&0x000003FF) / 1023.0f;
#else
					uint32 val = *(const uint32*)unit;
					fp32 r = (fp32)((val>>22)&0x000003FF) / 1023.0f;
					fp32 g = (fp32)((val>>12)&0x000003FF) / 1023.0f;
					fp32 b = (fp32)((val>>2)&0x000003FF) / 1023.0f;
					fp32 a = (fp32)(val&0x00000003) / 3.0f;
#endif
					outColor.setComponent32F(r, g, b, a);
					break;
				}
				//case PF_R10G10B10A2:
				//	{
				//		uint32 val = *(const uint32*)unit;
				//		fp32 a = (fp32)((val>>30)&0x00000003) / 3.0f;
				//		fp32 r = (fp32)((val>>20)&0x000003FF) / 1023.0f;
				//		fp32 g = (fp32)((val>>10)&0x000003FF) / 1023.0f;
				//		fp32 b = (fp32)(val&0x000003FF) / 1023.0f;
				//		outColor.setComponent32F(r, g, b, a);
				//		break;
				//	}
			case PF_B5G5R5A1:
				{
					//Direct3D type
					uint16 val = *(const uint16*)unit;
					uint8 a = (uint8)((val>>15)&0x00000001);
					uint8 r = (uint8)(((val>>10)&0x0000001F)*0xFF/0x1F);
					uint8 g = (uint8)(((val>>5)&0x0000001F)*0xFF/0x1F);
					uint8 b = (uint8)((val&0x0000001F)*0xFF/0x1F);
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_R5G5B5A1:
				{
					//OpenGL type
					uint16 val = *(const uint16*)unit;
					uint8 r = (uint8)((val>>11)&0x0000001F);
					uint8 g = (uint8)(((val>>6)&0x0000001F)*0xFF/0x1F);
					uint8 b = (uint8)(((val>>1)&0x0000001F)*0xFF/0x1F);
					uint8 a = (uint8)((val&0x00000001)*0xFF/0x1F);
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_B4G4R4A4:
				{
					//Direct3D type
					uint16 val = *(const uint16*)unit;
					uint8 a = CF_UNPACK4TO8(CF_U16MSB0(val));
					uint8 r = CF_UNPACK4TO8(CF_U16MSB1(val));
					uint8 g = CF_UNPACK4TO8(CF_U16MSB2(val));
					uint8 b = CF_UNPACK4TO8(CF_U16MSB3(val));
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_R4G4B4A4:
				{
					//OpenGL type
					uint16 val = *(const uint16*)unit;
					uint8 r = CF_UNPACK4TO8(CF_U16MSB0(val));
					uint8 g = CF_UNPACK4TO8(CF_U16MSB1(val));
					uint8 b = CF_UNPACK4TO8(CF_U16MSB2(val));
					uint8 a = CF_UNPACK4TO8(CF_U16MSB3(val));
					outColor.setComponent8U(r, g, b, a);
					break;
				}
			case PF_R16:
				{
					uint16 val = *(const uint16*)unit;
					fp32 r = val*(1.0f/65535.0f);
					outColor.setComponent32F(r, 0, 0);
					break;
				}
			case PF_R16G16:
				{
					const uint16* val = (const uint16*)unit;
					fp32 r = val[0]*(1.0f/65535.0f);
					fp32 g = val[1]*(1.0f/65535.0f);
					outColor.setComponent32F(r, g, 0);
					break;
				}
			case PF_R16G16B16A16:
				{
					const uint16* val = (const uint16*)unit;
					fp32 r = val[0]*(1.0f/65535.0f);
					fp32 g = val[1]*(1.0f/65535.0f);
					fp32 b = val[2]*(1.0f/65535.0f);
					fp32 a = val[3]*(1.0f/65535.0f);
					outColor.setComponent32F(r, g, b, a);
					break;
				}
			case PF_R16F:
				{
					fp16 r = *(const fp16*)unit;
					outColor.setComponent32F(r.getFloat(), 0, 0);
					break;
				}
			case PF_R16G16F:
				{
					const fp16* val = (const fp16*)unit;
					fp16 r = val[0];
					fp16 g = val[1];
					outColor.setComponent32F(r.getFloat(), g.getFloat(), 0);
					break;
				}
			case PF_R16G16B16F:
				{
					const fp16* val = (const fp16*)unit;
					fp16 r = val[0];
					fp16 g = val[1];
					fp16 b = val[2];
					outColor.setComponent32F(r.getFloat(), g.getFloat(), b.getFloat());
					break;
				}
			case PF_R16G16B16A16F:
				{
					const fp16* val = (const fp16*)unit;
					fp16 r = val[0];
					fp16 g = val[1];
					fp16 b = val[2];
					fp16 a = val[3];
					outColor.setComponent32F(r.getFloat(), g.getFloat(), b.getFloat(), a.getFloat());
					break;
				}
			case PF_R32F:
				{
					const fp32* val = (const fp32*)unit;
					fp32 r = val[0];
					outColor.setComponent32F(r, 0, 0);
					break;
				}
			case PF_R32G32F:
				{
					const fp32* val = (const fp32*)unit;
					fp32 r = val[0];
					fp32 g = val[1];
					outColor.setComponent32F(r, g, 0);
					break;
				}
			case PF_R32G32B32F:
				{
					const fp32* val = (const fp32*)unit;
					fp32 r = val[0];
					fp32 g = val[1];
					fp32 b = val[2];
					outColor.setComponent32F(r, g, b);
					break;
				}
			case PF_R32G32B32A32F:
				{
					const fp32* val = (const fp32*)unit;
					fp32 r = val[0];
					fp32 g = val[1];
					fp32 b = val[2];
					fp32 a = val[3];
					outColor.setComponent32F(r, g, b, a);
					break;
				}
			default:
				assert(false);
				return false;
			}//switch
			return true;
		}
		assert(false);
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	inline bool	ColorFormatter::writeColor(void* unit, size_t bytes, const Color& color, PixelFormat format)
	{
		if( format.getSizeBytes() == bytes && unit != NULL )
		{
			switch (format)
			{
			case PF_A8:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint8* val = (uint8*)unit;
					*val = a;
					break;
				}
			case PF_R8:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint8 * val = (uint8*)unit;
					*val = r;
					break;
				}
			case PF_R8G8: //byte order RG
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint8* val = (uint8*)unit;
					val[0] = r;
					val[1] = g;
					break;
				}
			case PF_R8G8B8: //byte order RGB
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint8* val = (uint8*)unit;
					val[0] = r;
					val[1] = g;
					val[2] = b;
					break;
				}
			case PF_B8G8R8: //byte order BGR
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint8* val = (uint8*)unit;
					val[0] = b;
					val[1] = g;
					val[2] = r;
					break;
				}
				//bit order
			case PF_RGB565:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint16* val = (uint16*)unit;
					uint16 r16 = uint16((r*0x1F+7)/0xFF);
					uint16 g16 = uint16((g*0x3F+3)/0xFF);
					uint16 b16 = uint16((b*0x1F+7)/0xFF);
					*val = uint16( (r16&0x1F) << 11 | (g16&0x3F) << 5 | (b16&0x1F) );
					break;
				}
			case PF_BGR565:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint16* val = (uint16*)unit;
					uint16 r16 = uint16( (r*0x1F+7)/0xFF );
					uint16 g16 = uint16( (g*0x3F+3)/0xFF );
					uint16 b16 = uint16( (b*0x1F+7)/0xFF );
					*val = uint16( (b16&0x1F) << 11 | (g16&0x3F) << 5 | (r16&0x1F) );
					break;
				}
			case PF_RGBA:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint32* val = (uint32*)unit;
					*val = CF_MAKEU32(r,g,b,a);
					break;
				}
			case PF_BGRA:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint32* val = (uint32*)unit;
					*val = CF_MAKEU32(b,g,r,a);
					break;
				}
			case PF_ARGB:
			case PF_XRGB:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint32* val = (uint32*)unit;
					*val = CF_MAKEU32(a,r,g,b);
					break;
				}
			case PF_ABGR:
			case PF_XBGR:
				{
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint32* val = (uint32*)unit;
					*val = CF_MAKEU32(a,b,g,r);
					break;
				}
				//note: byte order for A2R10G10B10 & PF_A2B10G10R10 is swapped(incorrect) for history reason
			case PF_B10G10R10A2:
				{
					fp32 r32f,g32f,b32f,a32f;
					color.getComponent32F(r32f,g32f,b32f,a32f);
					uint32* val = (uint32*)unit;

					uint32 a = uint32(a32f*3.0f) & 0x00000003;
					uint32 r = uint32(r32f*1023.0f) & 0x000003FF;
					uint32 g = uint32(g32f*1023.0f) & 0x000003FF;
					uint32 b = uint32(b32f*1023.0f) & 0x000003FF;
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE
					*val = (a << 30) | ( b << 20 ) | (g << 10) | r;
#else
#check out!
#endif
					break;
				}
				//case PF_A2B10G10R10:
				//	{
				//		fp32 r32f,g32f,b32f,a32f;
				//		color.getComponent32F(r32f,g32f,b32f,a32f);
				//		uint32* val = (uint32*)unit;

				//		uint32 a = uint32(a32f*3.0f) & 0x00000003;
				//		uint32 r = uint32(r32f*1023.0f) & 0x000003FF;
				//		uint32 g = uint32(g32f*1023.0f) & 0x000003FF;
				//		uint32 b = uint32(b32f*1023.0f) & 0x000003FF;

				//		*val = (a << 30) | ( r << 20 ) | (g << 10) | b;
				//		break;
				//	}
			case PF_B5G5R5A1:
				{
					//Direct3D type
					uint8 r8,g8,b8,a8;
					color.getComponent8U(r8,g8,b8,a8);
					uint16* val = (uint16*)unit;

					uint16 a = uint16(a8 > 0 ? 1: 0);
					uint16 r = uint16( ((uint16(r8)*0x1F)/0xFF)&0x1F );
					uint16 g = uint16( ((uint16(g8)*0x1F)/0xFF)&0x1F );
					uint16 b = uint16( ((uint16(b8)*0x1F)/0xFF)&0x1F );
					*val = uint16( (a<<15) | (r<<10) | (g<<5) | b );
					break;
				}
			case PF_R5G5B5A1:
				{
					//OpenGL type
					uint8 r8,g8,b8,a8;
					color.getComponent8U(r8,g8,b8,a8);
					uint16* val = (uint16*)unit;

					uint16 a = uint16(a8 > 0 ? 1: 0);
					uint16 r = uint16( ((uint16(r8)*0x1F)/0xFF)&0x1F );
					uint16 g = uint16( ((uint16(g8)*0x1F)/0xFF)&0x1F );
					uint16 b = uint16( ((uint16(b8)*0x1F)/0xFF)&0x1F );
					*val = uint16( (r<<11) | (g<<6) | (b<<1) | a );
					break;
				}
			case PF_B4G4R4A4:
				{
					//Direct3D type
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint16* val = (uint16*)unit;
					*val = CF_MAKEU16(uint16(a)*0xF/0xFF, uint16(r)*0xF/0xFF, uint16(g)*0xF/0xFF, uint16(b)*0xF/0xFF);
					break;
				}
			case PF_R4G4B4A4:
				{
					//OpenGL type
					uint8 r,g,b,a;
					color.getComponent8U(r,g,b,a);
					uint16* val = (uint16*)unit;
					*val = CF_MAKEU16(uint16(r)*0xF/0xFF, uint16(g)*0xF/0xFF, uint16(b)*0xF/0xFF, uint16(a)*0xF/0xFF);
					break;
				}
			case PF_R16:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					uint16 * val = (uint16*)unit;
					*val = uint16(r * 65535.0);
					break;
				}
			case PF_R16G16:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					uint16 * val = (uint16*)unit;
					val[0] = uint16(r * 65535.0);
					val[1] = uint16(g * 65535.0);
					break;
				}
			case PF_R16G16B16A16:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					uint16 * val = (uint16*)unit;
					val[0] = uint16(r * 65535.0);
					val[1] = uint16(g * 65535.0);
					val[2] = uint16(b * 65535.0);
					val[3] = uint16(a * 65535.0);
					break;
				}
			case PF_R16F:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					fp16* val = (fp16*)unit;
					*val = fp16(r);
					break;
				}
			case PF_R16G16F:
				{
					fp32 r32f,g32f,b32f,a32f;
					color.getComponent32F(r32f,g32f,b32f,a32f);
					fp16 r = fp16(r32f);
					fp16 g = fp16(g32f);

					fp16* val = (fp16*)unit;
					val[0] = r;
					val[1] = g;
					break;
				}
			case PF_R16G16B16F:
				{
					fp32 r32f,g32f,b32f,a32f;
					color.getComponent32F(r32f,g32f,b32f,a32f);
					fp16 r = fp16(r32f);
					fp16 g = fp16(g32f);
					fp16 b = fp16(b32f);

					fp16* val = (fp16*)unit;
					val[0] = r;
					val[1] = g;
					val[2] = b;
					break;
				}
			case PF_R16G16B16A16F:
				{
					fp32 r32f,g32f,b32f,a32f;
					color.getComponent32F(r32f,g32f,b32f,a32f);
					fp16 r = fp16(r32f);
					fp16 g = fp16(g32f);
					fp16 b = fp16(b32f);
					fp16 a = fp16(a32f);

					fp16* val = (fp16*)unit;
					val[0] = r;
					val[1] = g;
					val[2] = b;
					val[3] = a;
					break;
				}
			case PF_R32F:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					fp32* val = (fp32*)unit;
					val[0] = r;
					break;
				}
			case PF_R32G32F:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					fp32* val = (fp32*)unit;
					val[0] = r;
					val[1] = g;
					break;
				}
			case PF_R32G32B32F:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					fp32* val = (fp32*)unit;
					val[0] = r;
					val[1] = g;
					val[2] = b;
					break;
				}
			case PF_R32G32B32A32F:
				{
					fp32 r,g,b,a;
					color.getComponent32F(r,g,b,a);
					fp32* val = (fp32*)unit;
					val[0] = r;
					val[1] = g;
					val[2] = b;
					val[3] = a;
					break;
				}
			default:
				assert(false);
				return false;
			}//switch
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	inline bool	ColorFormatter::readColorLDR(const void* color, Color::RGBA& outColor, PixelFormat format)
	{
		if( color != NULL)
		{
			switch (format)
			{
			case PF_A8:
				{
					const uint8 val = *(const uint8*)color;
					outColor.r = outColor.g = outColor.b = 0;
					outColor.a = val;
					break;
				}
			case PF_R8:
				{
					const uint8 val = *(const uint8*)color;
					outColor.g = outColor.b = outColor.a = 0;
					outColor.r = val;
					break;
				}
			case PF_R8G8: //byte order RG
				{
					const uint8* val = (const uint8*)color;
					outColor.r = val[0];
					outColor.g = val[1];
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R8G8B8: //byte order RGB
				{
					const uint8* val = (const uint8*)color;
					outColor.r = val[0];
					outColor.g = val[1];
					outColor.b = val[2];
					outColor.a = 0xFF;
					break;
				}
			case PF_B8G8R8: //byte order BGR
				{
					const uint8* val = (const uint8*)color;
					outColor.b = val[0];
					outColor.g = val[1];
					outColor.r = val[2];
					outColor.a = 0xFF;
					break;
				}
				//bit orders
			case PF_RGB565:
				{
					uint16 val = *(const uint16*)color;
					outColor.r = (val >> 11)*0xFFu/0x1Fu;
					outColor.g = ((val >> 5)&0x3Fu)*0xFFu/0x3Fu;
					outColor.b = (val&0x1Fu)*0xFFu/0x1Fu;
					outColor.a = 0xFFu;
					break;
				}
			case PF_BGR565:
				{
					uint16 val = *(const uint16*)color;
					outColor.b = (val >> 11)*0xFFu/0x1Fu;
					outColor.g = ((val >> 5)&0x3Fu)*0xFFu/0x3Fu;
					outColor.r = (val&0x1F)*0xFFu/0x1Fu;
					outColor.a = 0xFFu;
					break;
				}
			case PF_RGBA:
				{
					uint32 val = *(const uint32*)color;
					outColor.r = CF_U32MSB0(val);
					outColor.g = CF_U32MSB1(val);
					outColor.b = CF_U32MSB2(val);
					outColor.a = CF_U32MSB3(val);
					break;
				}
			case PF_BGRA:
				{
					uint32 val = *(const uint32*)color;
					outColor.b = CF_U32MSB0(val);
					outColor.g = CF_U32MSB1(val);
					outColor.r = CF_U32MSB2(val);
					outColor.a = CF_U32MSB3(val);
					break;
				}
			case PF_ARGB:
			case PF_XRGB:
				{
					uint32 val = *(const uint32*)color;
					outColor.a = CF_U32MSB0(val);
					outColor.r = CF_U32MSB1(val);
					outColor.g = CF_U32MSB2(val);
					outColor.b = CF_U32MSB3(val);
					break;
				}
			case PF_ABGR:
			case PF_XBGR:
				{
					uint32 val = *(const uint32*)color;
					outColor.a = CF_U32MSB0(val);
					outColor.b = CF_U32MSB1(val);
					outColor.g = CF_U32MSB2(val);
					outColor.r = CF_U32MSB3(val);
					break;
				}
			case PF_B10G10R10A2:
				{
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE
					uint32 val = *(const uint32*)color;
					fp32 a = (fp32)((val>>30)&0x00000003) / 3.0f * 255.0f;
					fp32 b = (fp32)((val>>20)&0x000003FF) / 1023.0f * 255.0f;
					fp32 g = (fp32)((val>>10)&0x000003FF) / 1023.0f * 255.0f;
					fp32 r = (fp32)(val&0x000003FF) / 1023.0f * 255.0f;
#else
					uint32 val = *(const uint32*)color;
					fp32 r = (fp32)((val>>22)&0x000003FF) / 1023.0f * 255.0f;
					fp32 g = (fp32)((val>>12)&0x000003FF) / 1023.0f * 255.0f;
					fp32 b = (fp32)((val>>2)&0x000003FF) / 1023.0f * 255.0f;
					fp32 a = (fp32)(val&0x00000003) / 3.0f * 255.0f;
#endif
					outColor.r = (uint8)r;
					outColor.g = (uint8)g;
					outColor.b = (uint8)b;
					outColor.a = (uint8)a;
					break;
				}
				//case PF_R10G10B10A2:
				//	{
				//		uint32 val = *(const uint32*)color;
				//		fp32 a = (fp32)((val>>30)&0x00000003) / 3.0f;
				//		fp32 r = (fp32)((val>>20)&0x000003FF) / 1023.0f;
				//		fp32 g = (fp32)((val>>10)&0x000003FF) / 1023.0f;
				//		fp32 b = (fp32)(val&0x000003FF) / 1023.0f;
				//		outColor.setComponent32F(r, g, b, a);
				//		break;
				//	}
			case PF_B5G5R5A1:
				{
					//Direct3D type
					uint16 val = *(const uint16*)color;
					uint8 a = (uint8)((val>>15)&0x00000001);
					uint8 r = (uint8)(((val>>10)&0x0000001F)*0xFF/0x1F);
					uint8 g = (uint8)(((val>>5)&0x0000001F)*0xFF/0x1F);
					uint8 b = (uint8)((val&0x0000001F)*0xFF/0x1F);
					outColor.r = r;
					outColor.g = g;
					outColor.b = b;
					outColor.a = a;
					break;
				}
			case PF_R5G5B5A1:
				{
					//OpenGL type
					uint16 val = *(const uint16*)color;
					uint8 r = (uint8)((val>>11)&0x0000001F);
					uint8 g = (uint8)(((val>>6)&0x0000001F)*0xFF/0x1F);
					uint8 b = (uint8)(((val>>1)&0x0000001F)*0xFF/0x1F);
					uint8 a = (uint8)((val&0x00000001)*0xFF/0x1F);
					outColor.r = r;
					outColor.g = g;
					outColor.b = b;
					outColor.a = a;
					break;
				}

			case PF_B4G4R4A4:
				{
					uint16 val = *(const uint16*)color;
					uint8 a = CF_UNPACK4TO8(CF_U16MSB0(val));
					uint8 r = CF_UNPACK4TO8(CF_U16MSB1(val));
					uint8 g = CF_UNPACK4TO8(CF_U16MSB2(val));
					uint8 b = CF_UNPACK4TO8(CF_U16MSB3(val));
					outColor.r = r;
					outColor.g = g;
					outColor.b = b;
					outColor.a = a;
					break;
				}
			case PF_R4G4B4A4:
				{
					uint16 val = *(const uint16*)color;
					uint8 r = CF_UNPACK4TO8(CF_U16MSB0(val));
					uint8 g = CF_UNPACK4TO8(CF_U16MSB1(val));
					uint8 b = CF_UNPACK4TO8(CF_U16MSB2(val));
					uint8 a = CF_UNPACK4TO8(CF_U16MSB3(val));
					outColor.r = r;
					outColor.g = g;
					outColor.b = b;
					outColor.a = a;
					break;
				}
			case PF_R16:
				{
					uint16 val = *(const uint16*)color;
					outColor.r = uint8(val*255/65535);
					outColor.g = 0;
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R16G16:
				{
					const uint16* val = (const uint16*)color;
					outColor.r = uint8(val[0]*255/65535);
					outColor.g = uint8(val[1]*255/65535);
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R16G16B16A16:
				{
					const uint16* val = (const uint16*)color;
					outColor.r = uint8((int32)val[0]*255/65535);
					outColor.g = uint8((int32)val[1]*255/65535);
					outColor.b = uint8((int32)val[2]*255/65535);
					outColor.a = uint8((int32)val[3]*255/65535);
					break;
				}
			case PF_R16F:
				{
					fp16 r = *(const fp16*)color;
					outColor.r = uint8(r.getFloat()*255);
					outColor.g = 0;
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R16G16F:
				{
					const fp16* val = (const fp16*)color;
					outColor.r = uint8(val[0].getFloat()*255);
					outColor.g = uint8(val[1].getFloat()*255);
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R16G16B16F:
				{
					const fp16* val = (const fp16*)color;
					outColor.r = uint8(val[0].getFloat()*255);
					outColor.g = uint8(val[1].getFloat()*255);
					outColor.b = uint8(val[2].getFloat()*255);
					outColor.a = 0xFF;
					break;
				}
			case PF_R16G16B16A16F:
				{
					const fp16* val = (const fp16*)color;
					outColor.r = uint8(val[0].getFloat()*255);
					outColor.g = uint8(val[1].getFloat()*255);
					outColor.b = uint8(val[2].getFloat()*255);
					outColor.a = uint8(val[3].getFloat()*255);
					break;
				}
			case PF_R32F:
				{
					const fp32* val = (const fp32*)color;
					outColor.r = uint8(val[0]*255);
					outColor.g = 0;
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R32G32F:
				{
					const fp32* val = (const fp32*)color;
					outColor.r = uint8(val[0]*255);
					outColor.g = uint8(val[1]*255);
					outColor.b = 0;
					outColor.a = 0xFF;
					break;
				}
			case PF_R32G32B32F:
				{
					const fp32* val = (const fp32*)color;
					outColor.r = uint8(val[0]*255);
					outColor.g = uint8(val[1]*255);
					outColor.b = uint8(val[2]*255);
					outColor.a = 0xFF;
					break;
				}
			case PF_R32G32B32A32F:
				{
					const fp32* val = (const fp32*)color;
					outColor.r = uint8(val[0]*255);
					outColor.g = uint8(val[1]*255);
					outColor.b = uint8(val[2]*255);
					outColor.a = uint8(val[3]*255);
					break;
				}
			default:
				assert(false);
				return false;
			}//switch
			return true;
		}//if
		assert(false);
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	inline bool	ColorFormatter::writeColorLDR(void* outColor, const Color::RGBA& color, PixelFormat format)
	{
		if( outColor != NULL)
		{
			switch (format)
			{
			case PF_A8:
				{
					uint8 a = color.a;
					uint8* val = (uint8*)outColor;
					*val = a;
					break;
				}
			case PF_R8:
				{
					uint8 r = color.r;
					uint8 * val = (uint8*)outColor;
					*val = r;
					break;
				}
			case PF_R8G8: //byte order RG
				{
					uint8 r = color.r;
					uint8 g = color.g;
					uint8* val = (uint8*)outColor;
					val[0] = r;
					val[1] = g;
					break;
				}
			case PF_R8G8B8: //byte order RGB
				{
					uint8 r = color.r, g = color.g, b = color.b;
					uint8* val = (uint8*)outColor;
					val[0] = r;
					val[1] = g;
					val[2] = b;
					break;
				}
			case PF_B8G8R8: //byte order BGR
				{
					uint8 r = color.r, g = color.g, b = color.b;
					uint8* val = (uint8*)outColor;
					val[0] = b;
					val[1] = g;
					val[2] = r;
					break;
				}
				//bit order
			case PF_RGB565:
				{
					uint8 r = color.r, g = color.g, b = color.b;
					uint16* val = (uint16*)outColor;
					uint16 r16 = (r*0x1Fu+7u)/0xFFu;
					uint16 g16 = (g*0x3Fu+3u)/0xFFu;
					uint16 b16 = (b*0x1Fu+7u)/0xFFu;
					*val = (r16&0x1Fu) << 11 | (g16&0x3Fu) << 5 | (b16&0x1Fu);
					break;
				}
			case PF_BGR565:
				{
					uint8 r = color.r, g = color.g, b = color.b;
					uint16* val = (uint16*)outColor;
					uint16 r16 = (r*0x1Fu+7u)/0xFFu;
					uint16 g16 = (g*0x3Fu+3u)/0xFFu;
					uint16 b16 = (b*0x1Fu+7u)/0xFFu;
					*val = (b16&0x1Fu) << 11 | (g16&0x3Fu) << 5 | (r16&0x1Fu);
					break;
				}
			case PF_RGBA:
				{
					uint8 r = color.r, g = color.g, b = color.b, a = color.a;
					uint32* val = (uint32*)outColor;
					*val = CF_MAKEU32(r,g,b,a);
					break;
				}
			case PF_BGRA:
				{
					uint8 r = color.r, g = color.g, b = color.b, a = color.a;
					uint32* val = (uint32*)outColor;
					*val = CF_MAKEU32(b,g,r,a);
					break;
				}
			case PF_ARGB:
			case PF_XRGB:
				{
					uint8 r = color.r, g = color.g, b = color.b, a = color.a;
					uint32* val = (uint32*)outColor;
					*val = CF_MAKEU32(a,r,g,b);
					break;
				}
			case PF_ABGR:
			case PF_XBGR:
				{
					uint8 r = color.r, g = color.g, b = color.b, a = color.a;
					uint32* val = (uint32*)outColor;
					*val = CF_MAKEU32(a,b,g,r);
					break;
				}
				//note: byte order for A2R10G10B10 & PF_A2B10G10R10 is swapped(incorrect) for history reason
			case PF_B10G10R10A2:
				{
					uint32* val = (uint32*)outColor;
					uint32 a = uint32(color.a*3/255) & 0x00000003;
					uint32 r = uint32(color.r*1023/255) & 0x000003FF;
					uint32 g = uint32(color.g*1023/255) & 0x000003FF;
					uint32 b = uint32(color.b*1023/255) & 0x000003FF;
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE
					*val = (a << 30) | ( b << 20 ) | (g << 10) | r;
#else
#check out!
#endif
					break;
				}
				//case PF_A2B10G10R10:
				//	{
				//		fp32 r32f,g32f,b32f,a32f;
				//		color.getComponent32F(r32f,g32f,b32f,a32f);
				//		uint32* val = (uint32*)outColor;

				//		uint32 a = uint32(a32f*3.0f) & 0x00000003;
				//		uint32 r = uint32(r32f*1023.0f) & 0x000003FF;
				//		uint32 g = uint32(g32f*1023.0f) & 0x000003FF;
				//		uint32 b = uint32(b32f*1023.0f) & 0x000003FF;

				//		*val = (a << 30) | ( r << 20 ) | (g << 10) | b;
				//		break;
				//	}
			case PF_B5G5R5A1:
				{
					//Direct3D type
					uint16* val = (uint16*)outColor;
					uint16 a = uint16(color.a > 0 ? 1: 0);
					uint16 r = ((uint16(color.r)*0x1Fu)/0xFFu)&0x1Fu;
					uint16 g = ((uint16(color.g)*0x1Fu)/0xFFu)&0x1Fu;
					uint16 b = ((uint16(color.b)*0x1Fu)/0xFFu)&0x1Fu;
					*val = uint16( (a<<15) | (r<<10) | (g<<5) | b );
					break;
				}
			case PF_R5G5B5A1:
				{
					//OpenGL type
					uint16* val = (uint16*)outColor;
					uint16 a = uint16(color.a > 0 ? 1: 0);
					uint16 r = ((uint16(color.r)*0x1Fu)/0xFFu)&0x1Fu;
					uint16 g = ((uint16(color.g)*0x1Fu)/0xFFu)&0x1Fu;
					uint16 b = ((uint16(color.b)*0x1Fu)/0xFFu)&0x1Fu;
					*val = uint16( (r<<11) | (g<<6) | (b<<1) | a );
					break;
				}
			case PF_B4G4R4A4:
				{
					uint16* val = (uint16*)outColor;
					//Direct3D's format: A4R4G4B4
					*val = CF_MAKEU16( uint16(color.a)*0xF/0xFF, uint16(color.r)*0xF/0xFF, uint16(color.g)*0xF/0xFF, uint16(color.b)*0xF/0xFF);
					break;
				}
			case PF_R4G4B4A4:
				{
					uint16* val = (uint16*)outColor;
					//OpenGL's format: R4G4B4A4
					*val = CF_MAKEU16( uint16(color.r)*0xF/0xFF, uint16(color.g)*0xF/0xFF, uint16(color.b)*0xF/0xFF, uint16(color.a)*0xF/0xFF);
					break;
				}
			case PF_R16:
				{
					uint16* val = (uint16*)outColor;
					*val = uint16( (int32)color.r * 65535 / 255);
					break;
				}
			case PF_R16G16:
				{
					uint16* val = (uint16*)outColor;
					val[0] = uint16( (int32)color.r * 65535 / 255);
					val[1] = uint16( (int32)color.g * 65535 / 255);
					break;
				}
			case PF_R16G16B16A16:
				{
					uint16 * val = (uint16*)outColor;
					val[0] = uint16( (int32)color.r * 65535 / 255);
					val[1] = uint16( (int32)color.g * 65535 / 255);
					val[2] = uint16( (int32)color.b * 65535 / 255);
					val[3] = uint16( (int32)color.a * 65535 / 255);
					break;
				}
			case PF_R16F:
				{
					fp16* val = (fp16*)outColor;
					val[0] = fp16( fp32(color.r)/255 );
					break;
				}
			case PF_R16G16F:
				{
					fp16* val = (fp16*)outColor;
					val[0] = fp16( fp32(color.r)/255 );
					val[1] = fp16( fp32(color.g)/255 );
					break;
				}
			case PF_R16G16B16F:
				{
					fp16* val = (fp16*)outColor;
					val[0] = fp16( fp32(color.r)/255 );
					val[1] = fp16( fp32(color.g)/255 );
					val[2] = fp16( fp32(color.b)/255 );
					break;
				}
			case PF_R16G16B16A16F:
				{
					fp16* val = (fp16*)outColor;
					val[0] = fp16( fp32(color.r)/255 );
					val[1] = fp16( fp32(color.g)/255 );
					val[2] = fp16( fp32(color.b)/255 );
					val[3] = fp16( fp32(color.a)/255 );					
					break;
				}
			case PF_R32F:
				{
					fp32* val = (fp32*)outColor;
					val[0] = fp32(color.r)/255;
					break;
				}
			case PF_R32G32F:
				{
					fp32* val = (fp32*)outColor;
					val[0] = fp32(color.r)/255;
					val[1] = fp32(color.g)/255;
					break;
				}
			case PF_R32G32B32F:
				{
					fp32* val = (fp32*)outColor;
					val[0] = fp32(color.r)/255;
					val[1] = fp32(color.g)/255;
					val[2] = fp32(color.b)/255;
					break;
				}
			case PF_R32G32B32A32F:
				{
					fp32* val = (fp32*)outColor;
					val[0] = fp32(color.r)/255;
					val[1] = fp32(color.g)/255;
					val[2] = fp32(color.b)/255;
					val[3] = fp32(color.a)/255;
					break;
				}
			default:
				assert(false);
				return false;
			}//switch
			return true;
		}//if
		assert(false);
		return false;
	}
	
}//namespace Blade


#endif // __Blade_PixelColorFormatter_h__