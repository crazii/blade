/********************************************************************
	created:	2014/05/15
	filename: 	BC.h
	author:		Crazii
	
	purpose:	block compression
	reference:	http://msdn.microsoft.com/en-us/library/windows/desktop/bb694531(v=vs.85).aspx
*********************************************************************/
#ifndef __Blade_BC_h__
#define __Blade_BC_h__
#include <interface/public/graphics/PixelFormat.h>
#include <interface/IPlatformManager.h>

namespace Blade
{

	namespace BC
	{
		
		/************************************************************************/
		/* compression                                                                     */
		/************************************************************************/

		/** @brief compress input 4x4 colors in given format, to BC1 */
		void compressBlockBC1(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief  */
		void compressBlockBC2(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief  */
		void compressBlockBC3(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief  */
		void compressBlockBC4(uint8 *outBlock, const uint8* colors, PixelFormat format);

		/** @brief  */
		void compressBlockBC5(uint8 *outBlock, const uint8* colors, PixelFormat format, bool normalize);


		/** @brief  */
		size_t compressImageBC1(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageBC2(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageBC3(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageBC4(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format);

		/** @brief  */
		size_t compressImageBC5(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format, bool normalize);


		/************************************************************************/
		/* decompression                                                                     */
		/************************************************************************/
		/** @brief  */
		void decompressBlockBC1(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockBC2(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockBC3(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockBC4(uint8* outColors, const uint8 *block, PixelFormat format);

		/** @brief  */
		void decompressBlockBC5(uint8* outColors, const uint8 *block, PixelFormat format);


		/** @brief return output bytes */
		/* @note width & height are in original source image dimensions */
		size_t decompressImageBC1(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageBC2(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageBC3(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageBC4(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);

		/** @brief  */
		size_t decompressImageBC5(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format);


	}//namespace BC
	
}//namespace Blade


#endif // __Blade_BC_h__