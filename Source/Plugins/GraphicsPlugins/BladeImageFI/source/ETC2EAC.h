/********************************************************************
	created:	2015/01/28
	filename: 	ETC2EAC.h
	author:		Crazii
	
	purpose:	ETC2/EAC block compression
	reference:	
*********************************************************************/
#ifndef __Blade_ETC2EAC_h__
#define __Blade_ETC2EAC_h__
#include <interface/public/graphics/PixelFormat.h>

namespace Blade
{

	namespace ETC2EAC
	{

		/************************************************************************/
		/* compression                                                                     */
		/************************************************************************/

		/** @brief compress input 4x4 colors in given format, to ETC2 - RGB  */
		void compressBlockETC2(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief compress input 4x4 colors in given format, to ETC2 - RGBA  */
		void compressBlockETC2EAC(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief compress input 4x4 colors in given format, to EAC, using only the R channel  */
		void compressBlockR11EAC(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief compress input 4x4 colors in given format, to EAC, using only the R,G channel  */
		void compressBlockRG11EAC(uint8 *outBlock, const uint8* colors, PixelFormat format, bool normalize);

		/** @brief  */
		size_t compressImageETC2(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageETC2EAC(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageR11EAC(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageRG11EAC(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format, bool normalize);


		/************************************************************************/
		/* decompression                                                                     */
		/************************************************************************/
		/** @brief  */
		void decompressBlockETC2(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockETC2EAC(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockR11EAC(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockRG11EAC(uint8* outColors, const uint8 *block, PixelFormat format);


		/** @brief return output bytes */
		/* @note width & height are in original source image dimensions */
		size_t decompressImageETC2(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageETC2EAC(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageR11EAC(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageRG11EAC(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);
		
	}//namespace ETC2EAC

}//namespace Blade

#endif//__Blade_ETC2EAC_h__