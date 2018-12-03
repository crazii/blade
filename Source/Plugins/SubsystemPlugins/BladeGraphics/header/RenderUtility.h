/********************************************************************
	created:	2013/09/22
	filename: 	RenderUtility.h
	author:		Crazii
	purpose:	render utility functions
*********************************************************************/
#ifndef __Blade_RenderUtility_h__
#define __Blade_RenderUtility_h__
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <BladeGraphics.h>
#include <interface/public/graphics/GraphicsDefs.h>
#include <math/Vector3.h>
#include <math/BladeMath.h>

namespace Blade
{
	class GraphicsGeometry;
	class IRenderDevice;
	class Material;

	namespace RenderUtility
	{
		//shared function to create general geometry
		typedef enum EBufferElemnt
		{
			BE_POSITION = 0x00,		//position 0
			BE_UV		= 0x01,		//texture coordinates set 0
			BE_NORMAL	= 0x02,		//normal 0
		}BUFFER_ELEMENT;

		typedef enum EVertexWinding
		{
			FF_INSIDE	= 0,		//default front face(CCW) is inside (face is CCW winding if look from inside)
			FF_OUTSIDE	= 1,		//default front face(CCW) is outside (face is CCW if look from outside)
		}FRONT_FACE;

		//get shared vertex declaration
		//same value of parameter 'element' will return same object
		BLADE_GRAPHICS_API HVDECL	getVertexDeclaration(int element);

		//release all vertex declarations (terminate routine)
		BLADE_GRAPHICS_API void	releaseResources();

		//output geometry param
		typedef struct SOutputGeomtry : public NonCopyable, public NonAssignable
		{
			GraphicsGeometry&	mGeometry;
			HIBUFFER&			mIndexBuffer;
			HVERTEXSOURCE&		mVertexSource;
			IGraphicsResourceManager& mManager;
			bool				cpuRead;

			SOutputGeomtry(GraphicsGeometry& Geom, HIBUFFER& index, HVERTEXSOURCE& vertexStreams, bool read = false, IGraphicsResourceManager& manager = IGraphicsResourceManager::getSingleton())
				:mGeometry(Geom)
				,mIndexBuffer(index)
				,mVertexSource(vertexStreams)
				,mManager(manager)	
				,cpuRead(read)
			{}
		}GEOMETRY;

		/*
		@describe create sphere vertex buffer
		@param [in] density the face/triangle density of the sphere, higher density generates more sub faces on the sphere
		@return 
		@note: all element are in the same stream
		!important!: index buffer & vertex source in the output geometry data should be deleted manually by caller
		*/
		bool	BLADE_GRAPHICS_API createSphere(GEOMETRY& outGeom, scalar radius, size_t density, 
			int element = BE_POSITION, FRONT_FACE facing = FF_OUTSIDE,
			scalar latitude = Math::PI, scalar longitude = Math::TWICE_PI, const POINT3& center = POINT3::ZERO);

		inline bool	createUnitSphere(GEOMETRY& outGeom, size_t density, BUFFER_ELEMENT element = BE_POSITION)
		{
			return createSphere(outGeom, 1.0f, density, element);
		}

		/*
		@describe create vertex buffer for axis aligned box
		@param [in] density the face/triangle density of the sphere, higher density generates more sub faces on the sphere
		@return 
		@note :all element are in the same stream
		@remarks axisZ = axisX cross axisY, if axisX & axisY are not orthogonal, axisY will be altered axisY = axisZ cross axisX
		*/
		bool	BLADE_GRAPHICS_API createAABox(GEOMETRY& outGeom, scalar sx, scalar sy, scalar sz, 
			int element = BE_POSITION, FRONT_FACE facing = FF_OUTSIDE, const POINT3& center = POINT3::ZERO,
			const Vector3& axisX = Vector3::UNIT_X, const Vector3& axisY = Vector3::UNIT_Y);

		inline bool	createUnitAABox(GEOMETRY& outGeom, BUFFER_ELEMENT element = BE_POSITION)
		{
			return createAABox(outGeom, 1,1,1,element);
		}

		/*
		@describe create vertex buffer for an arrow
		@return 
		@note: 2 primitive set: arrow head as triangle list, arrow tail as line
				1 stream type : position (normal and texture coordinates are not supported)

						 |\
						 |   \
						 |      \
		----------------+         >     -----------
						 |      /
						 |   /				radius
						 |/				-----------

		|   length  	 |headLength|
		*/
		bool	BLADE_GRAPHICS_API	createArrow(GEOMETRY& outArrowGeom, GEOMETRY& outTailGeom, scalar length, scalar headLength, scalar headRadius, size_t headDensity,
			const POINT3& center = POINT3::ZERO, const Vector3& axis = Vector3::UNIT_X);

		/*
		@describe create vertex buffer for a ring using line strips
		@param [in] axis the axis which the ring is around
		@return 
		@note lines are in strips and has no normal/UV/vertex color
		*/
		bool	BLADE_GRAPHICS_API createRing(GEOMETRY& outGeom, scalar radius, size_t density,
			const POINT3& center = POINT3::ZERO, const Vector3& axis = Vector3::UNIT_X);

		/*
		@describe create a solid circle
		@param [in] facing FF_OUTSIDE to make the circle face as the axis direction, otherwise face to the -axis
		@return 
		*/
		bool	BLADE_GRAPHICS_API createCircle(GEOMETRY& outGeom, scalar radius, size_t density,
			BUFFER_ELEMENT element = BE_POSITION, FRONT_FACE facing = FF_OUTSIDE,
			const POINT3& center = POINT3::ZERO, const Vector3& axis = Vector3::UNIT_X);

		/*
		@describe 
		@param [in] facing - FF_OUTSIDE to specify facing direction to side0 cross side1 , otherwise facing direction is side1 cross side0
		@return 
		*/
		bool	BLADE_GRAPHICS_API	createPlane(GEOMETRY& outGeom, scalar size0, scalar size1, 
			BUFFER_ELEMENT element = BE_POSITION, FRONT_FACE facing = FF_OUTSIDE,
			const POINT3& corner = POINT3::ZERO, const Vector3& side0 = Vector3::UNIT_X, const Vector3& side1 = Vector3::UNIT_Y);

		/*
		@describe axis aligned cylinder
		@param 
		@return 
		*/
		bool	BLADE_GRAPHICS_API	createCylinder(GEOMETRY& outGeom, scalar radius, scalar length, size_t density,
			BUFFER_ELEMENT element = BE_POSITION, FRONT_FACE facing = FF_OUTSIDE,
			const POINT3& center = POINT3::ZERO, const Vector3& axis = Vector3::UNIT_X);

		/**
		@describe axis aligned cone
		@param
		@return
		*/
		bool	BLADE_GRAPHICS_API createCone(GEOMETRY& outGeom, scalar radius, scalar length, size_t density,
			BUFFER_ELEMENT element = BE_POSITION, FRONT_FACE facing = FF_OUTSIDE,
			const POINT3& apex = POINT3::ZERO, const Vector3& axis = Vector3::NEGATIVE_UNIT_Z);

		/**
		@describe shared resources: create quad in projected space (-1,-1)~(1,1)
		@param facing: quad facing. FF_OUTSIDE - face out to the viewer
		@return
		*/
		BLADE_GRAPHICS_API const GraphicsGeometry& getUnitQuad(FRONT_FACE facing = FF_OUTSIDE);

		/**
		  @describe optimized single triangle for full screen rendering
		  @param
		  @return
		*/
		BLADE_GRAPHICS_API const GraphicsGeometry& getFullScreenTirangle();

		/**
		@describe shared resources: create axis aligned unit arrow
		@param axis GA_X, GA_Y, GA_Z, GA_NEG_X, GA_NEG_Y, GA_NEG_Z
		@return
		*/
		BLADE_GRAPHICS_API const GraphicsGeometry& getUnitArrowHead(EAxis axis);
		BLADE_GRAPHICS_API const GraphicsGeometry& getUnitArrowTail(EAxis axis);


		/**
		@describe draw a full screen quad with specified material
		@param
		@return
		*/
		BLADE_GRAPHICS_API void drawQuad(IRenderDevice* device, Material* material, size_t pass = 0, size_t shader = 0);

	}//namespace RenderUtility
	
}//namespace Blade

#endif //  __Blade_RenderUtility_h__