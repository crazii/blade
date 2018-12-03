/********************************************************************
	created:	2013/09/24
	filename: 	RenderUtility.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <RenderUtility.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IGraphicsSystem.h>
#include <interface/IRenderSchemeManager.h>
#include <MaterialInstance.h>

namespace Blade
{
	namespace RenderUtility
	{
		typedef StaticMap<int, HVDECL> DeclMap;
		static DeclMap VertexDeclaration;
		static StaticLock VertexDeclLock;
		static StaticLock SharedLock;

		static HVDECL			QuadDecl;
		static GraphicsGeometry QuadGeometry[2];
		static HVERTEXSOURCE	QuadSource[2];

		static const int ARROW_AXIS_COUNT = 6;
		static GraphicsGeometry ArrowHeadGeometry[ARROW_AXIS_COUNT];
		static HVERTEXSOURCE	ArrowHeadSource[ARROW_AXIS_COUNT];
		static HIBUFFER			ArrowHeadIndex[ARROW_AXIS_COUNT];
		static GraphicsGeometry ArrowTailGeometry[ARROW_AXIS_COUNT];
		static HVERTEXSOURCE	ArrowTailSource[ARROW_AXIS_COUNT];

		static HVDECL			ScreenTriangleDecl;
		static GraphicsGeometry	ScreenTriangleGeometry;
		static HVERTEXSOURCE	ScreenTriangleSource;

		//////////////////////////////////////////////////////////////////////////
		HVDECL getVertexDeclaration(int element)
		{
			HVDECL hDecl;
			{
				ScopedLock lock(VertexDeclLock);
				HVDECL& decl = VertexDeclaration[element];
				if( decl == NULL )
				{
					bool hasUV = (element&BE_UV) != 0;
					bool hasNormal = (element&BE_NORMAL) != 0;

					decl = IGraphicsResourceManager::getSingleton().createVertexDeclaration();
					decl->addElement(0, 0, VET_FLOAT3, VU_POSITION, 0);
					uint16 offset = sizeof(Vector3);
					if( hasUV )
					{
						decl->addElement(0, offset, VET_FLOAT2, VU_TEXTURE_COORDINATES, 0);
						offset += sizeof(Vector2);
					}
					if( hasNormal )
					{
						decl->addElement(0, offset, VET_FLOAT3, VU_NORMAL, 0);
						offset += sizeof(Vector3);
					}
				}
				hDecl = decl;
			}
			return hDecl;
		}

		//////////////////////////////////////////////////////////////////////////
		void	releaseResources()
		{
			VertexDeclaration.clear();
			QuadDecl.clear();
			QuadSource[0].clear();
			QuadSource[1].clear();
			for(int i = 0; i < ARROW_AXIS_COUNT; ++i)
			{
				ArrowHeadSource[i].clear();
				ArrowHeadIndex[i].clear();
				ArrowTailSource[i].clear();
			}

			ScreenTriangleDecl.clear();
			ScreenTriangleSource.clear();
		}

		//////////////////////////////////////////////////////////////////////////
		bool	createSphere(GEOMETRY& outGeom, scalar radius, size_t density,
			int element/* = BE_POSITION*/, FRONT_FACE facing /* = FF_OUTSIDE*/,
			scalar latitude/* = Math::PI*/, scalar longitude/* = Math::TWICE_PI*/, const POINT3& center/* = POINT3::ZERO*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}
			bool hasUV = (element&BE_UV) != 0;
			bool hasNormal = (element&BE_NORMAL) != 0;
			bool uvTopdown = outGeom.mManager.getGraphicsConfig().TextureDir == IMGO_TOP_DOWN;

			HVDECL hDecl = RenderUtility::getVertexDeclaration(element);

			size_t VertexCount = (density+1)*(density+1);
			size_t IndexCount = 6*(density)*(density+1);

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			HVBUFFER vertexBuffer = manager.createVertexBuffer(NULL, hDecl->getVertexSize(0),VertexCount,IGraphicsBuffer::GBU_DEFAULT);
			//
			HIBUFFER indexBuffer = manager.createIndexBuffer(NULL,
				IndexBufferHelper::calcIndexType(VertexCount), 
				IndexCount,
				IGraphicsBuffer::GBU_DEFAULT );

			fp32*	vertexData = (fp32*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);
			IndexBufferHelper indexData(indexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL), indexBuffer->getIndexType());

			scalar	stepY = latitude / density;
			scalar	stepXZ = longitude / density;
			uint16	indexStart = 0;

			scalar theta = 0;
			scalar UStep = (fp32)1/(fp32)density;
			scalar VStep = -UStep;
			scalar u = 0.0f;
			scalar v = 1.0f;
			if( uvTopdown )
			{
				v = 0.0f;
				VStep = -VStep;
			}

			if( facing == FF_INSIDE )
			{
				u = 1.0f;
				UStep = -UStep;
			}

			size_t vertexCounting = 0;
			size_t indexCounting = 0;
			for( size_t i = 0;i <= density;++i,theta += stepY)
			{
				scalar y = radius*std::cos(theta);
				scalar prjoctedXZ = radius*std::sin(theta);
				scalar phi = 0;
				for( size_t j = 0; j <= density; ++j,phi += stepXZ )
				{
					//phi start from Z+ axis, CCW
					scalar x = prjoctedXZ * std::sin(phi);
					scalar z = prjoctedXZ * std::cos(phi);

					//setup points
					*(vertexData++) = x + center.x;
					*(vertexData++) = y + center.y;
					*(vertexData++) = z + center.z;
					++vertexCounting;

					//UV
					if( hasUV )
					{
						*(vertexData++) = u;
						*(vertexData++) = v;
					}
					u += UStep;

					//normal
					if( hasNormal )
					{
						Vector3 normal(x,y,x);
						normal.normalize();
						if( facing == FF_INSIDE )
							normal = -normal;

						*(vertexData++) = normal.x;
						*(vertexData++) = normal.y;
						*(vertexData++) = normal.z;
					}

					//setup triangles
					if( i != density )
					{
						if( facing == FF_OUTSIDE )
						{
							*(indexData++) = (uint16)(indexStart + density + 1);
							*(indexData++) = (uint16)(indexStart);
							*(indexData++) = (uint16)(indexStart + density);

							*(indexData++) = (uint16)(indexStart + density + 1);
							*(indexData++) = (uint16)(indexStart + 1);
							*(indexData++) = (uint16)(indexStart);
						}
						else
						{
							*(indexData++) = (uint16)(indexStart + density + 1);
							*(indexData++) = (uint16)(indexStart + density);
							*(indexData++) = (uint16)(indexStart);

							*(indexData++) = (uint16)(indexStart + density + 1);
							*(indexData++) = (uint16)(indexStart);
							*(indexData++) = (uint16)(indexStart + 1);
						}
						indexCounting += 6;

						++indexStart;
					}

				}//for

				u = 0;
				v += VStep;
			}
			assert(vertexCounting == VertexCount);
			assert(indexCounting == IndexCount);

			vertexBuffer->unlock();
			indexBuffer->unlock();
			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			HIBUFFER ibuffer;
			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);
			outGeom.mManager.cloneIndexBuffer(ibuffer, *indexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mGeometry.reset();
			outGeom.mVertexSource->setSource(0, vbuffer);
			outGeom.mIndexBuffer = ibuffer;

			outGeom.mGeometry.mVertexDecl = hDecl;
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			outGeom.mGeometry.useIndexBuffer(true);
			outGeom.mGeometry.mIndexCount = (uint32)IndexCount;
			outGeom.mGeometry.mIndexBuffer = ibuffer;
			outGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;			
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	createAABox(GEOMETRY& outGeom, scalar sx, scalar sy, scalar sz, 
			int element/* = BE_POSITION*/, FRONT_FACE facing/* = FF_OUTSIDE*/, const POINT3& center/* = POINT3::ZERO*/,
			const Vector3& axisX/* = Vector3::UNIT_X*/, const Vector3& axisY/* = Vector3::UNIT_Y*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}
			assert(!axisX.isZero());
			assert(!axisY.isZero());

			Vector3 X = axisX.getNormalizedVector();
			Vector3 Y = axisY;

			Vector3 Z = axisX.crossProduct(axisY);
			Z.normalize();
			Y = Z.crossProduct(X);
			Y.normalize();

			bool hasUV = (element&BE_UV) != 0;
			bool hasNormal = (element&BE_NORMAL) != 0;

			HVDECL hDecl = RenderUtility::getVertexDeclaration(element);

			//vertices do not share because of different UV/normal
			const size_t FACE_COUNT = 6;
			size_t VertexCount = 8*3;
			size_t IndexCount = FACE_COUNT*2*3;

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			HVBUFFER vertexBuffer = manager.createVertexBuffer(NULL, hDecl->getVertexSize(0),VertexCount,IGraphicsBuffer::GBU_DEFAULT );
			HIBUFFER indexBuffer = manager.createIndexBuffer(NULL, IIndexBuffer::IT_16BIT,IndexCount,IGraphicsBuffer::GBU_DEFAULT );

			fp32*	vertexData = (fp32*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);
			uint16*	indexData = (uint16*)indexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);
			/*
			   y
			   |
			   |
			   |     C(center)
			   |     |
			   +-----|-------x
			  /      |
			 /       C(center XZ)
			/
			z
			*/

			Vector3 Front[4] =
			{
				center - X*(sx*0.5f) - Y*(sy*0.5f)	+ Z*(sz*0.5f),
				center + X*(sx*0.5f) - Y*(sy*0.5f)	+ Z*(sz*0.5f),
				center + X*(sx*0.5f) + Y*(sy*0.5f)	+ Z*(sz*0.5f),
				center - X*(sx*0.5f) + Y*(sy*0.5f)	+ Z*(sz*0.5f)
			};

			Vector3 Back[4] = 
			{
				Front[3] - Z*sz,
				Front[2] - Z*sz,
				Front[1] - Z*sz,
				Front[0] - Z*sz,
			};

			Vector3 Right[4] =
			{
				center + Z*(sz*0.5f) - Y*(sy*0.5f)	+ X*(sx*0.5f),
				center - Z*(sz*0.5f) - Y*(sy*0.5f)	+ X*(sx*0.5f),
				center - Z*(sz*0.5f) + Y*(sy*0.5f)	+ X*(sx*0.5f),
				center + Z*(sz*0.5f) + Y*(sy*0.5f)	+ X*(sx*0.5f)
			};
			Vector3 Left[4] =
			{
				Right[3] - X*sx,
				Right[2] - X*sx,
				Right[1] - X*sx,
				Right[0] - X*sx,	
			};
			Vector3 Top[4] =
			{
				center - X*(sx*0.5f) + Z*(sz*0.5f) + Y*(sy*0.5f),
				center + X*(sx*0.5f) + Z*(sz*0.5f) + Y*(sy*0.5f),
				center + X*(sx*0.5f) - Z*(sz*0.5f) + Y*(sy*0.5f),
				center - X*(sx*0.5f) - Z*(sz*0.5f) + Y*(sy*0.5f),
			};
			Vector3 Bottom[4] =
			{
				Top[3] - Y*sy,
				Top[2] - Y*sy,
				Top[1] - Y*sy,
				Top[0] - Y*sy,
			};
			Vector3 positive_normals[FACE_COUNT] = {Z,-Z,X,-X,Y,-Y};
			Vector3 negative_normals[FACE_COUNT] = {-Z, Z, -X, X, -Y, Y};
			Vector3* normals = facing == FF_OUTSIDE ? positive_normals : negative_normals;

			//uv top - down
			Vector2 postiveUV[4] = { Vector2(0,1), Vector2(1,1), Vector2(1,0), Vector2(0,0) };
			Vector2 negativeUV[4] = { Vector2(1,1), Vector2(0,1), Vector2(0,0), Vector2(1,0) };
			Vector2* UV = facing == FF_OUTSIDE ? postiveUV : negativeUV;

			const Vector3* Faces[FACE_COUNT] = {Front, Back, Right, Left, Top, Bottom};

			const size_t POINT_COUNT = 4;

			uint16 indexBase = 0;
			
			size_t indexCounting = 0;
			size_t vertexCounting = 0;

			for(size_t i = 0; i < FACE_COUNT; ++i)
			{
				const Vector3* Face = Faces[i];
				const Vector3& normal = normals[i];

				for(size_t j = 0; j < POINT_COUNT; ++j)
				{
					*(vertexData++) = Face[j].x;
					*(vertexData++) = Face[j].y;
					*(vertexData++) = Face[j].z;

					if( hasUV )
					{
						//separated UV. TODO: use cube UV (uvw)?
						*(vertexData++) = UV[j].x;
						*(vertexData++) = UV[j].y;
					}
					if( hasNormal )
					{
						*(vertexData++) = normal.x;
						*(vertexData++) = normal.y;
						*(vertexData++) = normal.z;
					}
					++vertexCounting;
				}

				//indices
				//2 triangle with 2*3 indices
				if( facing == FF_OUTSIDE )
				{
					*(indexData++) = indexBase;
					*(indexData++) = indexBase + 1u;
					*(indexData++) = indexBase + 2u;

					*(indexData++) = indexBase;
					*(indexData++) = indexBase + 2u;
					*(indexData++) = indexBase + 3u;
				}
				else
				{
					*(indexData++) = indexBase;
					*(indexData++) = indexBase + 2u;
					*(indexData++) = indexBase + 1u;

					*(indexData++) = indexBase;
					*(indexData++) = indexBase + 3u;
					*(indexData++) = indexBase + 2u;
				}
				indexCounting += 6;

				indexBase += POINT_COUNT;
			}

			assert( vertexCounting == VertexCount);
			assert( indexCounting == IndexCount);

			vertexBuffer->unlock();
			indexBuffer->unlock();

			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			HIBUFFER ibuffer;

			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);
			outGeom.mManager.cloneIndexBuffer(ibuffer, *indexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mGeometry.reset();
			outGeom.mVertexSource->setSource(0, vbuffer);
			outGeom.mIndexBuffer = ibuffer;

			outGeom.mGeometry.mVertexDecl = hDecl;
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			outGeom.mGeometry.useIndexBuffer(true);
			outGeom.mGeometry.mIndexCount = (uint32)IndexCount;
			outGeom.mGeometry.mIndexBuffer = ibuffer;
			outGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	createArrow(GEOMETRY& outArrowGeom, GEOMETRY& outTailGeom, scalar length, scalar headLength, scalar headRadius, size_t headDensity,
			const POINT3& center/* = POINT3::ZERO*/, const Vector3& axis/* = Vector3::UNIT_X*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}

			if( headDensity < 3)
			{
				assert(false);
				return false;
			}
			assert( !axis.isZero() );

			HVDECL hDecl = getVertexDeclaration(BE_POSITION);
			size_t VertexCount = headDensity + 3;
			size_t IndexCount = (headDensity * 2)*3;

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			//shared vertex buffer
			HVBUFFER vertexBuffer = manager.createVertexBuffer(
				NULL,
				hDecl->getVertexSize(0),
				VertexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			Vector3* points = (Vector3*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);

			Vector3 dir = axis.getNormalizedVector();

			//tail: single line
			size_t vertexCountng = 0;
			points[vertexCountng++] = center;
			Vector3 headCenter = center + dir*length;
			points[vertexCountng++] = headCenter;	//this point is shared with head

			//arrow head
			const scalar step = Math::PI*2/headDensity;
			Quaternion quat(dir, step);

			//get any perpendicular vector
			Vector3 p = dir.getPerpendicularVector();
			for(size_t i = 0; i < headDensity; ++i)
			{
				points[ vertexCountng++ ] = headCenter + p*headRadius;
				p = p * quat;
			}
			points[ vertexCountng++ ] = headCenter + dir*(headLength);
			assert(vertexCountng == VertexCount);
			vertexBuffer->unlock();

			//tail line do not use index.

			//head index
			HIBUFFER indexBuffer = manager.createIndexBuffer(
				NULL,
				IndexBufferHelper::calcIndexType(VertexCount),
				IndexCount, 
				IGraphicsBuffer::GBU_DEFAULT);

			IndexBufferHelper indexData(indexBuffer);

			//head bottom plane
			for(size_t i = 1;i < headDensity; i++)
			{
				*(indexData++) = 1;
				*(indexData++) = (uint32)i + 2;
				*(indexData++) = (uint32)i + 1;
			}
			*(indexData++) = 1;
			*(indexData++) = 2;
			*(indexData++) = (uint32)headDensity+1;

			//head outer cone face
			for(size_t i = 1;i < headDensity; i++)
			{
				*(indexData++) = (uint32)headDensity + 2;
				*(indexData++) = (uint32)i + 1;
				*(indexData++) = (uint32)i + 2;
			}
			*(indexData++) = (uint32)headDensity + 2;
			*(indexData++) = (uint32)headDensity + 1;
			*(indexData++) = 2;
			assert(indexData.getCursor() == IndexCount);
			vertexBuffer->unlock();
			indexBuffer->unlock();

			//arrow geometry
			if( outArrowGeom.mVertexSource == NULL )
				outArrowGeom.mVertexSource = IVertexSource::create();
			else
				outArrowGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			HIBUFFER ibuffer;
			outArrowGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outArrowGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);
			outArrowGeom.mManager.cloneIndexBuffer(ibuffer, *indexBuffer, outArrowGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outArrowGeom.mGeometry.reset();
			outArrowGeom.mVertexSource->setSource(0, vbuffer);
			outArrowGeom.mIndexBuffer = ibuffer;

			outArrowGeom.mGeometry.mVertexDecl = hDecl;
			outArrowGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			outArrowGeom.mGeometry.mIndexBuffer = ibuffer;
			outArrowGeom.mGeometry.useIndexBuffer(true);
			outArrowGeom.mGeometry.mIndexCount = (uint32)IndexCount;
			//skip the tail line end point
			outArrowGeom.mGeometry.mVertexStart = 0;
			outArrowGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outArrowGeom.mGeometry.mVertexSource = outArrowGeom.mVertexSource;

			//tail geometry
			if( outTailGeom.mVertexSource == NULL )
				outTailGeom.mVertexSource = IVertexSource::create();
			else
				outTailGeom.mVertexSource->clearAllSource();

			outTailGeom.mGeometry.reset();
			outTailGeom.mVertexSource->setSource(0, vbuffer);

			outTailGeom.mGeometry.mVertexDecl = hDecl;
			outTailGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_LINE_LIST;
			outTailGeom.mGeometry.useIndexBuffer(false);
			outTailGeom.mGeometry.mIndexCount = 0;
			outTailGeom.mGeometry.mIndexBuffer = NULL;
			outTailGeom.mGeometry.mVertexStart = 0;
			outTailGeom.mGeometry.mVertexCount = 2;
			outTailGeom.mGeometry.mVertexSource = outTailGeom.mVertexSource;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	createRing(GEOMETRY& outGeom, scalar radius, size_t density,
			const POINT3& center/* = POINT3::ZERO*/, const Vector3& axis/* = Vector3::UNIT_X*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}
			if( density < 3 )
			{
				assert(false);
				return false;
			}
			assert( !axis.isZero() );

			HVDECL hDecl = getVertexDeclaration(BE_POSITION);
			size_t VertexCount = density;

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			//shared vertex buffer
			HVBUFFER vertexBuffer = manager.createVertexBuffer(
				NULL,
				hDecl->getVertexSize(0),
				VertexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			Vector3* points = (Vector3*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);

			Vector3 dir = axis.getNormalizedVector();
			const scalar step = Math::PI*2/density;
			Quaternion quat(dir, step);

			//get any perpendicular vector
			Vector3 p = dir.getPerpendicularVector();
			for(size_t i = 0; i < density; ++i)
			{
				points[ i ] = center + p*radius;
				p = p * quat;
			}
			vertexBuffer->unlock();

			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mGeometry.reset();
			outGeom.mVertexSource->setSource(0, vbuffer);

			outGeom.mGeometry.mVertexDecl = hDecl;
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_LINE_STRIP;
			outGeom.mGeometry.useIndexBuffer(false);
			outGeom.mGeometry.mIndexCount = 0;
			outGeom.mGeometry.mIndexBuffer = NULL;
			outGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	BLADE_GRAPHICS_API createCircle(GEOMETRY& outGeom, scalar radius, size_t density,
			BUFFER_ELEMENT element/* = BE_POSITION*/, FRONT_FACE facing/* = FF_OUTSIDE*/,
			const POINT3& center/* = POINT3::ZERO*/, const Vector3& axis/* = Vector3::UNIT_X*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}

			if( density < 3 )
			{
				assert(false);
				return false;
			}
			assert( !axis.isZero() );

			bool hasUV = (element&BE_UV) != 0;
			bool hasNormal = (element&BE_NORMAL) != 0;

			HVDECL hDecl = getVertexDeclaration(element);
			size_t VertexCount = density + 1;

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			//shared vertex buffer
			HVBUFFER vertexBuffer = manager.createVertexBuffer(
				NULL,
				hDecl->getVertexSize(0),
				VertexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			scalar* points = (scalar*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);

			Vector3 dir = axis.getNormalizedVector();
			const scalar step = Math::PI*2/density;
			Quaternion quat(dir, step);
			Vector3 normal = facing == FF_OUTSIDE ? dir : -dir;
			Vector3 uvCenter(0.5, 0.5, 0.5);

			//center point
			*points++ = center.x;
			*points++ = center.y;
			*points++ = center.z;
			if( hasNormal )
			{
				*points++ = normal.x;
				*points++ = normal.y;
				*points++ = normal.z;
			}
			if( hasUV )
			{
				*points++ = uvCenter.x;
				*points++ = uvCenter.y;
			}
			Quaternion axisToZ;
			Quaternion::generateRotation(axisToZ, axis, Vector3::UNIT_Z);

			//get any perpendicular vector
			Vector3 p = dir.getPerpendicularVector();
			for(size_t i = 0; i < density; ++i)
			{
				Vector3 cp = center + p*radius;
				*points++ = cp.x;
				*points++ = cp.y;
				*points++ = cp.z;
				if( hasNormal )
				{
					*points++ = normal.x;
					*points++ = normal.y;
					*points++ = normal.z;
				}
				if( hasUV )
				{
					assert(false);
					Vector3 pXY = p * axisToZ;
					pXY *= 0.5f;		//abs(u),abs(v) less than 0.5 because center is (0.5,0.5)
					pXY += uvCenter;	//apply center
					*points++ = pXY.x;
					*points++ = pXY.y;
				}
				p *= quat;
			}
			vertexBuffer->unlock();

			//
			size_t IndexCount = density*3;
			HIBUFFER indexBuffer = manager.createIndexBuffer(
				NULL,
				IndexBufferHelper::calcIndexType(VertexCount),
				IndexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			IndexBufferHelper indexData(indexBuffer);
			for(size_t i = 0; i < density-1; ++i)
			{
				if( facing == FF_OUTSIDE )
				{
					*(indexData++) = (uint32)0;
					*(indexData++) = (uint32)i+1;
					*(indexData++) = (uint32)i+2;
				}
				else
				{
					*(indexData++) = (uint32)0;
					*(indexData++) = (uint32)i+2;
					*(indexData++) = (uint32)i+1;
				}
			}

			if( facing == FF_OUTSIDE )
			{
				*(indexData++) = (uint32)0;
				*(indexData++) = (uint32)density;
				*(indexData++) = (uint32)1;
			}
			else
			{
				*(indexData++) = (uint32)0;
				*(indexData++) = (uint32)1;
				*(indexData++) = (uint32)density;
			}

			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			HIBUFFER ibuffer;
			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);
			outGeom.mManager.cloneIndexBuffer(ibuffer, *indexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mGeometry.reset();
			outGeom.mVertexSource->setSource(0, vbuffer);
			outGeom.mIndexBuffer = ibuffer;

			outGeom.mGeometry.mVertexDecl = hDecl;
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;

			outGeom.mGeometry.useIndexBuffer(true);
			outGeom.mGeometry.mIndexCount = (uint32)IndexCount;
			outGeom.mGeometry.mIndexBuffer = ibuffer;
			outGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		bool	createPlane(GEOMETRY& outGeom, scalar size0, scalar size1, 
			BUFFER_ELEMENT element/* = BE_POSITION*/, FRONT_FACE facing/* = FF_OUTSIDE*/,
			const POINT3& corner/* = POINT3::ZERO*/, const Vector3& side0/* = Vector3::UNIT_X*/, const Vector3& side1/* = Vector3::UNIT_Y*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}
			//TODO: use index?
			HVDECL hDecl = getVertexDeclaration(element);

			bool hasUV = (element&BE_UV) != 0;
			bool hasNormal = (element&BE_NORMAL) != 0;

			const size_t VertexCount = 6;

			Vector3 points[VertexCount] =
			{
				corner,		points[0]+side0*size0,		points[1] + side1*size1,
				corner,		points[2],					corner + side1*size1
			};
			Vector2 UV[VertexCount] = 
			{
				Vector2(0,0), Vector2(1,0), Vector2(1,1),
				Vector2(0,0), Vector2(1,1), Vector2(0,1)
			};
			Vector3 normal = side0.crossProduct(side1);

			if( facing == FF_INSIDE )
			{
				std::swap( points[1], points[2] );
				std::swap( points[4], points[5] );
				std::swap( UV[1], UV[2] );
				std::swap( UV[4], UV[5] );
				normal = -normal;
			}

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			//shared vertex buffer
			HVBUFFER vertexBuffer = manager.createVertexBuffer(
				NULL,
				hDecl->getVertexSize(0),
				VertexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			scalar* destPoints = (scalar*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);
			for(size_t i = 0; i < VertexCount; ++i)
			{
				*destPoints++ = points[i].x;
				*destPoints++ = points[i].y;
				*destPoints++ = points[i].z;
				if( hasNormal )
				{
					*destPoints++ = normal.x;
					*destPoints++ = normal.y;
					*destPoints++ = normal.z;
				}
				if( hasUV )
				{
					*destPoints++ = UV[i].x;
					*destPoints++ = UV[i].y;
				}
			}
			
			vertexBuffer->unlock();

			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mGeometry.reset();
			outGeom.mVertexSource->setSource(0, vbuffer);

			outGeom.mGeometry.mVertexDecl = hDecl;
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			outGeom.mGeometry.useIndexBuffer(false);
			outGeom.mGeometry.mIndexCount = 0;
			outGeom.mGeometry.mIndexBuffer = NULL;
			outGeom.mGeometry.mVertexCount = VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	BLADE_GRAPHICS_API	createCylinder(GEOMETRY& outGeom, scalar radius, scalar length, size_t density,
			BUFFER_ELEMENT element/* = BE_POSITION*/, FRONT_FACE facing/* = FF_OUTSIDE*/,
			const POINT3& center/* = POINT3::ZERO*/, const Vector3& axis/* = Vector3::UNIT_X*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}

			if( density < 3 )
			{
				assert(false);
				return false;
			}
			assert( !axis.isZero() );

			//TODO: use index?
			HVDECL hDecl = getVertexDeclaration(element);

			bool hasUV = (element&BE_UV) != 0;
			bool hasNormal = (element&BE_NORMAL) != 0;

			size_t VertexCount = density*2;
			size_t TriangleCount = density*2;

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			//shared vertex buffer
			HVBUFFER vertexBuffer = manager.createVertexBuffer(
				NULL,
				hDecl->getVertexSize(0),
				VertexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			scalar* points = (scalar*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);
			Vector3 dir = axis.getNormalizedVector();
			const scalar step = Math::PI*2/density;
			Quaternion quat(dir, step);

			Vector3 topCenter = center + dir*(length*0.5f);
			Vector3 bottomCenter = center - dir*(length*0.5f);

			scalar u = 0.0f;
			scalar uStep = 1.0f/density;
			if( facing == FF_INSIDE )
			{
				u = 1.0f;
				uStep = -uStep;
			}

			//get any perpendicular vector
			Vector3 p = dir.getPerpendicularVector();
			for(size_t i = 0; i < density; ++i)
			{
				Vector3 topPoint = topCenter + p*radius;
				Vector3 bottomPoint = bottomCenter + p*radius;
				*(points++) = topPoint.x;
				*(points++) = topPoint.y;
				*(points++) = topPoint.z;

				Vector3 normal;
				if( hasNormal )
				{
					if( facing == FF_OUTSIDE )
						normal = p;
					else
						normal = -p;

					*(points++) = normal.x;
					*(points++) = normal.y;
					*(points++) = normal.z;
				}

				if( hasUV )
				{
					*(points++) = u;
					*(points++) = 0.0f;
				}

				*(points++) = bottomPoint.x;
				*(points++) = bottomPoint.y;
				*(points++) = bottomPoint.z;

				if( hasNormal )
				{
					*(points++) = normal.x;
					*(points++) = normal.y;
					*(points++) = normal.z;
				}

				if( hasUV )
				{
					*(points++) = u;
					*(points++) = 1.0f;
				}

				u += uStep;
				//note: quat is counter clockwise rotation
				//so p is rotating CCW
				p *= quat;
			}
			vertexBuffer->unlock();

			size_t IndexCount = TriangleCount*3;
			HIBUFFER indexBuffer = manager.createIndexBuffer(NULL,
				IndexBufferHelper::calcIndexType( VertexCount ), TriangleCount*3, IGraphicsBuffer::GBU_DEFAULT);

			IndexBufferHelper indexData(indexBuffer);
			for(size_t i = 0; i < density*2 - 3; i+=2)
			{
				//look from outside:
				//top		top-next
				//+-----------+
				//|           |
				//|           |
				//|           |
				//+-----------+
				//			bottom-next
				if( facing == FF_OUTSIDE )
				{
					*(indexData++) = (uint32)i;		//top
					*(indexData++) = (uint32)i+1;	//bottom
					*(indexData++) = (uint32)i+2;	//top next

					*(indexData++) = (uint32)i+1;	//bottom
					*(indexData++) = (uint32)i+3;	//bottom next
					*(indexData++) = (uint32)i+2;	//top next
				}
				else
				{
					*(indexData++) = (uint32)i;		//top
					*(indexData++) = (uint32)i+2;	//top next
					*(indexData++) = (uint32)i+1;	//bottom

					*(indexData++) = (uint32)i+1;	//bottom
					*(indexData++) = (uint32)i+2;	//top next
					*(indexData++) = (uint32)i+3;	//bottom next
				}
			}

			//closing up
			if( facing == FF_OUTSIDE )
			{
				*(indexData++) = (uint32)density*2-2;	//top
				*(indexData++) = (uint32)density*2-1;	//bottom
				*(indexData++) = (uint32)0;				//top next

				*(indexData++) = (uint32)density*2-1;	//bottom
				*(indexData++) = (uint32)1;				//bottom next
				*(indexData++) = (uint32)0;				//top next
			}
			else
			{
				*(indexData++) = (uint32)density*2-2;	//top
				*(indexData++) = (uint32)0;				//top next
				*(indexData++) = (uint32)density*2-1;	//bottom
				

				*(indexData++) = (uint32)density*2-1;	//bottom
				*(indexData++) = (uint32)0;				//top next
				*(indexData++) = (uint32)1;				//bottom next
			}
			indexBuffer->unlock();

			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();

			HVBUFFER vbuffer;
			HIBUFFER ibuffer;
			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);
			outGeom.mManager.cloneIndexBuffer(ibuffer, *indexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mGeometry.reset();
			outGeom.mVertexSource->setSource(0, vbuffer);
			outGeom.mIndexBuffer = ibuffer;

			outGeom.mGeometry.mVertexDecl = hDecl;
			//triangle strip has face winding problem, if one is CCW, then next is CW
			//so use triangle list
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			outGeom.mGeometry.useIndexBuffer(true);
			outGeom.mGeometry.mIndexCount = (uint32)IndexCount;
			outGeom.mGeometry.mIndexBuffer = ibuffer;
			outGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool	createCone(GEOMETRY& outGeom, scalar radius, scalar length, size_t density,
			BUFFER_ELEMENT element/* = BE_POSITION*/, FRONT_FACE facing/* = FF_OUTSIDE*/,
			const POINT3& apex/* = POINT3::ZERO*/, const Vector3& axis/* = Vector3::NEGATIVE_UNIT_Z*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
			{
				assert(false);
				return false;
			}

			if( density < 3 )
			{
				assert(false);
				return false;
			}
			Vector3 dir = axis.getNormalizedVector();
			assert( !dir.isZero() );
			POINT3 center = apex + dir * length;

			bool hasUV = (element&BE_UV) != 0;
			bool hasNormal = (element&BE_NORMAL) != 0;

			HVDECL hDecl = getVertexDeclaration(element);
			size_t VertexCount = density + 2;

			IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

			//shared vertex buffer
			HVBUFFER vertexBuffer = manager.createVertexBuffer(
				NULL,
				hDecl->getVertexSize(0),
				VertexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			scalar* points = (scalar*)vertexBuffer->lock(IGraphicsBuffer::GBLF_NORMAL);

			const scalar step = Math::PI*2/density;
			Quaternion quat(dir, step);
			Vector3 normal = facing == FF_OUTSIDE ? dir : -dir;
			Vector3 uvCenter(0.5, 0.5, 0.5);

			//center point
			*points++ = center.x;
			*points++ = center.y;
			*points++ = center.z;
			if( hasNormal )
			{
				*points++ = normal.x;
				*points++ = normal.y;
				*points++ = normal.z;
			}
			if( hasUV )
			{
				*points++ = uvCenter.x;
				*points++ = uvCenter.y;
			}

			//apex point
			*points++ = apex.x;
			*points++ = apex.y;
			*points++ = apex.z;
			if( hasNormal )
			{
				*points++ = -normal.x;
				*points++ = -normal.y;
				*points++ = -normal.z;
			}
			if( hasUV )
			{
				*points++ = uvCenter.x;
				*points++ = uvCenter.y;
			}

			Quaternion axisToZ;
			Quaternion::generateRotation(axisToZ, axis, Vector3::UNIT_Z);

			//get any perpendicular vector
			Vector3 p = dir.getPerpendicularVector();
			for(size_t i = 0; i < density; ++i)
			{
				Vector3 cp = center + p*radius;
				*points++ = cp.x;
				*points++ = cp.y;
				*points++ = cp.z;
				if( hasNormal )
				{
					*points++ = normal.x;
					*points++ = normal.y;
					*points++ = normal.z;
				}
				if( hasUV )
				{
					assert(false);
					Vector3 pXY = p * axisToZ;
					pXY *= 0.5f;		//abs(u),abs(v) less than 0.5 because center is (0.5,0.5)
					pXY += uvCenter;	//apply center
					*points++ = pXY.x;
					*points++ = pXY.y;
				}
				p *= quat;
			}
			vertexBuffer->unlock();

			//
			size_t IndexCount = density*6;
			HIBUFFER indexBuffer = manager.createIndexBuffer(NULL,
				IndexBufferHelper::calcIndexType(VertexCount),
				IndexCount,
				IGraphicsBuffer::GBU_DEFAULT);

			IndexBufferHelper indexData(indexBuffer);
			for(size_t i = 0; i < density-1; ++i)
			{
				if( facing == FF_OUTSIDE )
				{
					*(indexData++) = (uint32)0;
					*(indexData++) = (uint32)i+2;
					*(indexData++) = (uint32)i+3;

					*(indexData++) = (uint32)1;
					*(indexData++) = (uint32)i+3;
					*(indexData++) = (uint32)i+2;
				}
				else
				{
					*(indexData++) = (uint32)0;
					*(indexData++) = (uint32)i+3;
					*(indexData++) = (uint32)i+2;

					*(indexData++) = (uint32)1;
					*(indexData++) = (uint32)i+2;
					*(indexData++) = (uint32)i+3;
				}
			}
			if( facing == FF_OUTSIDE )
			{
				*(indexData++) = (uint32)0;
				*(indexData++) = (uint32)density+1;
				*(indexData++) = (uint32)2;

				*(indexData++) = (uint32)1;
				*(indexData++) = (uint32)2;
				*(indexData++) = (uint32)density+1;
			}
			else
			{
				*(indexData++) = (uint32)0;
				*(indexData++) = (uint32)2;
				*(indexData++) = (uint32)density+1;

				*(indexData++) = (uint32)1;
				*(indexData++) = (uint32)density+1;
				*(indexData++) = (uint32)2;
			}
			indexBuffer->unlock();

			if( outGeom.mVertexSource == NULL )
				outGeom.mVertexSource = IVertexSource::create();
			else
				outGeom.mVertexSource->clearAllSource();
			outGeom.mGeometry.reset();

			HVBUFFER vbuffer;
			HIBUFFER ibuffer;
			outGeom.mManager.cloneVertexBuffer(vbuffer, *vertexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);
			outGeom.mManager.cloneIndexBuffer(ibuffer, *indexBuffer, outGeom.cpuRead ? IGraphicsBuffer::GBU_CPU_READ : IGraphicsBuffer::GBU_STATIC);

			outGeom.mVertexSource->setSource(0, vbuffer);
			outGeom.mIndexBuffer = ibuffer;

			outGeom.mGeometry.mVertexDecl = hDecl;
			outGeom.mGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;

			outGeom.mGeometry.useIndexBuffer(true);
			outGeom.mGeometry.mIndexCount = (uint32)IndexCount;
			outGeom.mGeometry.mIndexBuffer = ibuffer;
			outGeom.mGeometry.mVertexCount = (uint32)VertexCount;
			outGeom.mGeometry.mVertexSource = outGeom.mVertexSource;
			return true;
		}


		//////////////////////////////////////////////////////////////////////////
		const GraphicsGeometry& getUnitQuad(FRONT_FACE facing/* = FF_OUTSIDE*/)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
				BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot create resource without graphics context."));

			if( facing != FF_OUTSIDE && facing != FF_INSIDE )
				BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid facing enum."));

			ScopedLock lock(SharedLock);
			if(QuadDecl == NULL)
				QuadDecl = RenderUtility::getVertexDeclaration(RenderUtility::BE_POSITION);
			if( QuadSource[facing] == NULL )
			{
				IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);
				
				HVBUFFER quad = manager.createVertexBuffer(NULL, sizeof(Vector3), 6, IGraphicsBuffer::GBU_DEFAULT);
				Vector3* pts = (Vector3*)quad->lock(IGraphicsBuffer::GBLF_NORMAL);
				if (facing == FF_OUTSIDE)
				{
					pts[0] = Vector3(-1, -1, 0);
					pts[1] = Vector3(1, -1, 0);
					pts[2] = Vector3(1, 1, 0);
					pts[3] = Vector3(-1, -1, 0);
					pts[4] = Vector3(1, 1, 0);
					pts[5] = Vector3(-1, 1, 0);
				}
				else
				{
					pts[0] = Vector3(-1, -1, 0);
					pts[2] = Vector3(1, -1, 0);
					pts[1] = Vector3(1, 1, 0);
					pts[3] = Vector3(-1, -1, 0);
					pts[5] = Vector3(1, 1, 0);
					pts[4] = Vector3(-1, 1, 0);
				}

				quad->unlock();

				HVBUFFER vbuffer;
				IGraphicsResourceManager::getSingleton().cloneVertexBuffer(vbuffer, *quad, IGraphicsBuffer::GBU_STATIC);

				QuadSource[facing] = IVertexSource::create();
				QuadSource[facing]->setSource(0, vbuffer);
				QuadGeometry[facing].useIndexBuffer(false);
				QuadGeometry[facing].mIndexCount = 0;
				QuadGeometry[facing].mVertexSource = QuadSource[facing];
				QuadGeometry[facing].mVertexStart = 0;
				QuadGeometry[facing].mVertexCount = 6;
				QuadGeometry[facing].mVertexDecl = QuadDecl;
				QuadGeometry[facing].mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			}
			return QuadGeometry[facing];
		}

		//////////////////////////////////////////////////////////////////////////
		const GraphicsGeometry& getFullScreenTirangle()
		{
			ScopedLock lock(SharedLock);
			if (ScreenTriangleDecl == NULL)
				ScreenTriangleDecl = RenderUtility::getVertexDeclaration(RenderUtility::BE_POSITION);
			if (ScreenTriangleSource == NULL)
			{
				IGraphicsResourceManager& manager = IGraphicsResourceManager::getOtherSingleton(IGraphicsResourceManager::TYPE_TEMPORARY);

				HVBUFFER triangle = manager.createVertexBuffer(NULL, sizeof(Vector3), 3, IGraphicsBuffer::GBU_DEFAULT);
				Vector3* pts = (Vector3*)triangle->lock(IGraphicsBuffer::GBLF_NORMAL);
				//make it symmetrical so that normal can be normalized in vertex shader.
				//pts[0] = Vector3(-2.366f, -1.366f, 0);
				//pts[1] = Vector3(2.366f, -1.366f, 0);
				//pts[2] = Vector3(0, 2.732f, 0);
				pts[0] = Vector3(-1, -1, 0);
				pts[1] = Vector3(3, -1, 0);
				pts[2] = Vector3(-1, 3, 0);
				triangle->unlock();

				HVBUFFER vbuffer;
				IGraphicsResourceManager::getSingleton().cloneVertexBuffer(vbuffer, *triangle, IGraphicsBuffer::GBU_STATIC);

				ScreenTriangleSource = IVertexSource::create();
				ScreenTriangleSource->setSource(0, vbuffer);
				ScreenTriangleGeometry.useIndexBuffer(false);
				ScreenTriangleGeometry.mIndexCount = 0;
				ScreenTriangleGeometry.mVertexSource = ScreenTriangleSource;
				ScreenTriangleGeometry.mVertexStart = 0;
				ScreenTriangleGeometry.mVertexCount = 3;
				ScreenTriangleGeometry.mVertexDecl = ScreenTriangleDecl;
				ScreenTriangleGeometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;
			}
			return ScreenTriangleGeometry;
		}

		//////////////////////////////////////////////////////////////////////////
		static int internalCreateArrow(EAxis axis)
		{
			if(!BLADE_TS_CHECK_GRAPHICS_CONTEXT())
				BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("cannot create resource without graphics context."));

			if( axis != GA_X && axis != GA_Y && axis != GA_Z 
				&& axis != GA_NEG_X && axis != GA_NEG_Y && axis != GA_NEG_Z  )
				BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid axis parameter."));
			int index = 0;
			int a = axis;
			while( a != 1 )
			{
				a = a >> 1; 
				++index;
			};
			assert(index >= 0 && index < ARROW_AXIS_COUNT);
			ScopedLock lock(SharedLock);
			if( ArrowHeadSource[index] == NULL || ArrowHeadIndex[index] == NULL || ArrowTailSource[index] == NULL)
			{
				Vector3 axes[] = 
				{
					Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z,
					Vector3::NEGATIVE_UNIT_X, Vector3::NEGATIVE_UNIT_Y, Vector3::NEGATIVE_UNIT_Z,
				};
				HIBUFFER dummy;
				GEOMETRY head(ArrowHeadGeometry[index], ArrowHeadIndex[index], ArrowHeadSource[index]);
				GEOMETRY tail(ArrowTailGeometry[index], dummy, ArrowTailSource[index]);
				RenderUtility::createArrow(head, tail, 0.5f, 0.5f, 0.05f, 36, POINT3::ZERO, axes[index]);
			}
			return index;
		}

		//////////////////////////////////////////////////////////////////////////
		const GraphicsGeometry& getUnitArrowHead(EAxis axis)
		{
			int index = RenderUtility::internalCreateArrow(axis);
			return ArrowHeadGeometry[index];
		}

		//////////////////////////////////////////////////////////////////////////
		const GraphicsGeometry& getUnitArrowTail(EAxis axis)
		{
			int index = RenderUtility::internalCreateArrow(axis);
			return ArrowTailGeometry[index];
		}
		
		//////////////////////////////////////////////////////////////////////////
		void drawQuad(IRenderDevice* device, Material* material, size_t pass/* = 0*/, size_t shader/* = 0*/)
		{	
			Pass* matMass = material->getActiveTechnique()->getPass(pass);
			const HSHADEROPTION& shaderOpt = matMass->getShaderOption(shader);
			device->setShader(shaderOpt->getProgram());

			IRenderSchemeManager& rsm = IRenderSchemeManager::getSingleton();
			rsm.applyRenderProperty(device, *matMass->getRenderProperty(), GFM_DEFAULT);
			rsm.applyGlobalShaderSetting(device, shaderOpt);
			//rsm.applyInstanceShaderSetting(device, *shaderOpt);

			const GraphicsGeometry& geom = RenderUtility::getUnitQuad();
			device->renderGeometry(geom);
		}

	}//namespace RenderUtility

	
}//namespace Blade