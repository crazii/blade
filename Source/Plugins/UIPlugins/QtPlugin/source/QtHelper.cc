/********************************************************************
	created:	2016/07/27
	filename: 	QtHelper.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QImage::Format toQImageFormat(PixelFormat format)
	{
		if(format.isCompressed() || format.isDepth() || format.isFloat())
			return QImage::Format_Invalid;

		switch(format.getSizeBytes())
		{
		case 1:
			return QImage::Format_Grayscale8;
		case 2:
			if( format == PF_RGB565 )
				return QImage::Format_RGB16;
			else if(format == PF_R4G4B4A4)
				return QImage::Format_RGB444;
			else if(format == PF_B5G5R5A1)
				return QImage::Format_RGB555;
			else
				return QImage::Format_Invalid;
		case 3:
			return format == PF_RGB ? QImage::Format_RGB888 : QImage::Format_Invalid;
		case 4:
			return QImage::Format_ARGB32;
		}
		return QImage::Format_Invalid;
	}

	//////////////////////////////////////////////////////////////////////////
	Handle<QImage> toQImage(const HIMAGE& img)
	{
		if(img == NULL || img->isCompressed())
		{
			assert(false);
			return Handle<QImage>::EMPTY;
		}
		QImage::Format fmt = toQImageFormat(img->getFormat());
		if(fmt == QImage::Format_Invalid)
			return Handle<QImage>::EMPTY;

		QImage* qimg = QT_NEW QImage(img->getWidth(), img->getHeight(), fmt);
		assert(qimg->byteCount() == img->getBytes());
		::memcpy(qimg->bits(), img->getBits(), img->getBytes());

		return Handle<QImage>(qimg);
	}
	
}//namespace Blade