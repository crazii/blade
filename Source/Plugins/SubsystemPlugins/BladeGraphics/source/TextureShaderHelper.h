/********************************************************************
	created:	2017/7/18
	filename: 	TextureShaderHelper.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TextureShaderHelper_h__
#define __Blade_TextureShaderHelper_h__
#include <interface/public/graphics/ITexture.h>

namespace Blade
{

	namespace Impl
	{
		static const size_t UVSPACE_DATA_COUNT = 4;

		//////////////////////////////////////////////////////////////////////////
		inline const Vector4* updateTextureSize(Vector4& size, const HTEXTURE& texture)
		{
			if (texture == NULL)
				size = Vector4::UNIT_ALL;
			else
			{
				size_t width = texture->getWidth();
				size_t height = texture->getHeight();

				size[0] = (scalar)width;
				size[1] = (scalar)height;
				reinterpret_cast<Vector2&>(size[2]) = Vector2::UNIT_ALL / reinterpret_cast<const Vector2&>(size[0]);
			}
			return &size;
		}

		//////////////////////////////////////////////////////////////////////////
		inline const Vector4*	updateUVSpace(Vector4 uvSpace[UVSPACE_DATA_COUNT], const HTEXTURE& texture)
		{
			for (size_t i = 0; i < UVSPACE_DATA_COUNT; ++i)
				uvSpace[i] = Vector4(0, 0, 1, 1);

			if (texture == NULL)
				return uvSpace;


			if (!texture->getTextureUsage().isRenderTarget() && !texture->getTextureUsage().isDepthStencil())
			{
				assert(false);
				return uvSpace;
			}

			Vector4 offsetScale;
			static_cast<IRenderTexture*>(texture)->getViewRect(offsetScale.x, offsetScale.y, offsetScale.z, offsetScale.w);

			//apply half pixel offset
			Vector2 size;
			size.x = (scalar)texture->getWidth();
			size.y = (scalar)texture->getHeight();
			Vector2 halfPixelOffset = Vector2::UNIT_ALL*0.5f / size;
			Vector2 offset = Vector2::ZERO;
			if (IGraphicsResourceManager::getSingleton().getGraphicsConfig().HalfTexel2Pixel)
				offset = halfPixelOffset;

			//half pixel offset + view offset
			reinterpret_cast<Vector2&>(uvSpace[0].x) = reinterpret_cast<const Vector2&>(offsetScale.x) + offset;
			//scale
			reinterpret_cast<Vector2&>(uvSpace[0].z) = reinterpret_cast<const Vector2&>(offsetScale.z);
			//int shader : uv * scale + offset - transformUVSpace()

			//this is used for absolute sampling. i.e. tex2D(texture, transformUVSpace(float2(0.5, 0.5), space[1]) )
			//absolute offset scale without half pixel offset
			//apply half pixel offset to absolute texture coordinate is INCORRECT:
			//half pixel offset is used for texture coordinates bound to primitives.
			uvSpace[1] = offsetScale;

			//min, max. always use half pixel offset to prevent bilinear bleeding when using multiple samplings
			reinterpret_cast<Vector2&>(uvSpace[2].x) = reinterpret_cast<const Vector2&>(offsetScale.x) + halfPixelOffset;
			reinterpret_cast<Vector2&>(uvSpace[2].z) = reinterpret_cast<const Vector2&>(offsetScale.x) + reinterpret_cast<const Vector2&>(offsetScale.z) - halfPixelOffset;

			//original size without view offset
			reinterpret_cast<Vector2&>(uvSpace[3].x) = halfPixelOffset;
			reinterpret_cast<Vector2&>(uvSpace[3].z) = Vector2::UNIT_ALL;

			return uvSpace;
		}
	}

	
}//namespace Blade

#endif//__Blade_TextureShaderHelper_h__