// ==========================================================
// Upsampling / downsampling routine
//
// Design and implementation by
// - Herv?Drolon (drolon@infonie.fr)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#include "Resize.h"
#include "../MemHelper.h"

FIBITMAP * DLL_CALLCONV 
FreeImage_Rescale(FreeImageMem* mem,FIBITMAP *src, int dst_width, int dst_height, FREE_IMAGE_FILTER filter) {
	FIBITMAP *dst = NULL;

	if (!FreeImage_HasPixels(src) || (dst_width <= 0) || (dst_height <= 0) || (FreeImage_GetWidth(src) <= 0) || (FreeImage_GetHeight(src) <= 0)) {
		return NULL;
	}

	// select the filter
	CGenericFilter *pFilter = NULL;
	switch(filter) {
		case FILTER_BOX:
			pFilter = new( FreeImage_Malloc(mem->temp_mem,sizeof(CBoxFilter)) ) CBoxFilter();
			break;
		case FILTER_BICUBIC:
			pFilter = new( FreeImage_Malloc(mem->temp_mem,sizeof(CBicubicFilter)) ) CBicubicFilter();
			break;
		case FILTER_BILINEAR:
			pFilter = new( FreeImage_Malloc(mem->temp_mem,sizeof(CBilinearFilter)) ) CBilinearFilter();
			break;
		case FILTER_BSPLINE:
			pFilter = new( FreeImage_Malloc(mem->temp_mem,sizeof(CBSplineFilter)) ) CBSplineFilter();
			break;
		case FILTER_CATMULLROM:
			pFilter = new( FreeImage_Malloc(mem->temp_mem,sizeof(CCatmullRomFilter)) ) CCatmullRomFilter();
			break;
		case FILTER_LANCZOS3:
			pFilter = new( FreeImage_Malloc(mem->temp_mem,sizeof(CLanczos3Filter)) ) CLanczos3Filter();
			break;
	}

	if(!pFilter) {
		return NULL;
	}

	CResizeEngine Engine(pFilter);

	// perform upsampling or downsampling
	FreeImageMem tmem;
	FreeImage_MakeTempMem(&tmem,mem);

	if((FreeImage_GetBPP(src) == 4) || (FreeImage_GetColorType(src) == FIC_PALETTE)) {
		// special case for 4-bit images or color map indexed images ...

		if(FreeImage_IsTransparent(src) == FALSE) {
			FIBITMAP *src24 = NULL;
			FIBITMAP *dst24 = NULL;
			try {
				// transparent conversion to 24-bit (any transparency table will be destroyed)
				src24 = FreeImage_ConvertTo24Bits(&tmem,src);
				if(!src24) throw(1);
				// perform upsampling or downsampling
				dst24 = Engine.scale(&tmem,src24, dst_width, dst_height);
				if(!dst24) throw(1);
				// color quantize to 8-bit
				dst = FreeImage_ColorQuantize(mem,dst24, FIQ_NNQUANT);
				// free and return
				FreeImage_Unload(&tmem,src24);
				FreeImage_Unload(&tmem,dst24);
			} catch(int) {
				if(src24) FreeImage_Unload(&tmem,src24);
				if(dst24) FreeImage_Unload(&tmem,dst24);
			}
		} else {
			FIBITMAP *src32 = NULL;
			try {
				// transparent conversion to 32-bit (keep transparency)
				src32 = FreeImage_ConvertTo32Bits(&tmem,src);
				if(!src32) throw(1);
				// perform upsampling or downsampling
				dst = Engine.scale(mem,src32, dst_width, dst_height);
				if(!dst) throw(1);
				// free and return
				FreeImage_Unload(&tmem,src32);
			} catch(int) {
				if(src32) FreeImage_Unload(&tmem,src32);
				if(dst) FreeImage_Unload(mem,dst);
			}
		}
	}
	else if((FreeImage_GetBPP(src) == 16) && (FreeImage_GetImageType(src) == FIT_BITMAP)) {
		// convert 16-bit RGB to 24-bit
		FIBITMAP *src24 = NULL;
		try {
			// transparent conversion to 24-bit (any transparency table will be destroyed)
			src24 = FreeImage_ConvertTo24Bits(&tmem,src);
			if(!src24) throw(1);
			// perform upsampling or downsampling
			dst = Engine.scale(mem,src24, dst_width, dst_height);
			if(!dst) throw(1);
			// free and return
			FreeImage_Unload(&tmem,src24);
		} catch(int) {
			if(src24) FreeImage_Unload(&tmem,src24);
			if(dst) FreeImage_Unload(mem,dst);
		}
	}
	else {
		// normal case : 
		// 1- or 8-bit greyscale, 24- or 32-bit RGB(A) images
		// 16-bit greyscale, 48- or 64-bit RGB(A) images
		// 32-bit float, 96- or 128-bit RGB(A) float images
		dst = Engine.scale(mem,src, dst_width, dst_height);
	}

	pFilter->~CGenericFilter();
	FreeImage_Free(mem->temp_mem,pFilter);

	// copy metadata from src to dst
	FreeImage_CloneMetadata(mem,dst, src);
	
	return dst;
}

FIBITMAP * DLL_CALLCONV 
FreeImage_MakeThumbnail(FreeImageMem* mem,FIBITMAP *dib, int max_pixel_size, BOOL convert) {
	FIBITMAP *thumbnail = NULL;
	int new_width, new_height;

	if(!FreeImage_HasPixels(dib) || (max_pixel_size <= 0)) return NULL;

	int width	= FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	if(max_pixel_size == 0) max_pixel_size = 1;

	if((width < max_pixel_size) && (height < max_pixel_size)) {
		// image is smaller than the requested thumbnail
		return FreeImage_Clone(mem,dib);
	}

	FreeImageMem tmem;
	FreeImage_MakeTempMem(&tmem,mem);

	if(width > height) {
		new_width = max_pixel_size;
		// change image height with the same ratio
		double ratio = ((double)new_width / (double)width);
		new_height = (int)(height * ratio + 0.5);
		if(new_height == 0) new_height = 1;
	} else {
		new_height = max_pixel_size;
		// change image width with the same ratio
		double ratio = ((double)new_height / (double)height);
		new_width = (int)(width * ratio + 0.5);
		if(new_width == 0) new_width = 1;
	}

	const FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(dib);

	// perform downsampling using a bilinear interpolation

	switch(image_type) {
		case FIT_BITMAP:
		case FIT_UINT16:
		case FIT_RGB16:
		case FIT_RGBA16:
		case FIT_FLOAT:
		case FIT_RGBF:
		case FIT_RGBAF:
		{
			FREE_IMAGE_FILTER filter = FILTER_BILINEAR;
			thumbnail = FreeImage_Rescale(&tmem,dib, new_width, new_height, filter);
		}
		break;

		case FIT_INT16:
		case FIT_UINT32:
		case FIT_INT32:
		case FIT_DOUBLE:
		case FIT_COMPLEX:
		default:
			// cannot rescale this kind of image
			thumbnail = NULL;
			break;
	}

	if((thumbnail != NULL) && (image_type != FIT_BITMAP) && convert) {
		// convert to a standard bitmap
		FIBITMAP *bitmap = NULL;
		switch(image_type) {
			case FIT_UINT16:
				bitmap = FreeImage_ConvertTo8Bits(mem,thumbnail);
				break;
			case FIT_RGB16:
				bitmap = FreeImage_ConvertTo24Bits(mem,thumbnail);
				break;
			case FIT_RGBA16:
				bitmap = FreeImage_ConvertTo32Bits(mem,thumbnail);
				break;
			case FIT_FLOAT:
				bitmap = FreeImage_ConvertToStandardType(mem,thumbnail, TRUE);
				break;
			case FIT_RGBF:
				bitmap = FreeImage_ToneMapping(mem,thumbnail, FITMO_DRAGO03);
				break;
			case FIT_RGBAF:
				// no way to keep the transparency yet ..

				FIBITMAP *rgbf = FreeImage_ConvertToRGBF(&tmem,thumbnail);
				bitmap = FreeImage_ToneMapping(&tmem,rgbf, FITMO_DRAGO03);
				FreeImage_Unload(&tmem,rgbf);
				break;
		}
		if(bitmap != NULL) {
			FreeImage_Unload(&tmem,thumbnail);
			thumbnail = bitmap;
		}
	}

	{
		FIBITMAP* temp = thumbnail;
		thumbnail = FreeImage_Clone(mem,temp);
		FreeImage_Unload(&tmem,temp);
	}

	// copy metadata from src to dst
	FreeImage_CloneMetadata(mem,thumbnail, dib);
	
	return thumbnail;
}
