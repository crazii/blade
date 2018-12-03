/********************************************************************
	created:	2010/04/24
	filename: 	ICamera.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ICamera_h__
#define __Blade_ICamera_h__
#include <Handle.h>
#include <BladeGraphics.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/ISpaceData.h>

namespace Blade
{
	class IRenderGroup;
	class IRenderQueue;
	class RenderType;
	class Frustum;

	class ICamera : public IGraphicsCamera
	{
	public:
		typedef enum EContentVisibility
		{
			CV_NULL = 0,
			CV_PARTIAL,
			CV_ALL,
		}VISIBILITY;

	public:
		typedef enum ECopyFlag
		{
			CF_DEFAULT	= 0x01,	//copy projection params: projection type, near/far, aspect, FOV/w,h
			CF_GEOMETRY = 0x02,	//copy view params: position & rotation
			CF_SPACE	= 0x04,	//copy space data added by addCameraData()
			CF_CULLING	= 0x08,	//copy culling data(frustum) and culling result (visible objects)
		}COPY_FLAG;
	public:
		virtual ~ICamera()		{}

		/*
		@describe prepare render queue for rendering,\n
		this action may first obtain visible objects(space contents) in scene(space), \n
		and added the contents' renderables into the queue
		@param
		@return may not the same as input param
		*/
		virtual	IRenderQueue*	getRenderQueue(const RenderType* renderType) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addCameraData(ISpaceData* data) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeCameraData(const TString& DataName) = 0;

		/** @brief  */
		inline bool				removeCameraData(ISpaceData* data)
		{
			if( data != NULL )
				return this->removeCameraData( data->getSpaceTypeName() );
			return false;
		}

		/*
		@describe manually update view & projection matrix
		@param 
		@return 
		@note getViewMatrix/getProjectionMatrix method will auto update the corresponding matrix
		*/
		virtual void			updateMatrix() = 0;

		/*
		@describe update the visible objects
		@param 
		@return 
		*/
		virtual void			update() = 0;

		/*
		@describe frustum is only valid after update()
		@param
		@return
		*/
		virtual const Frustum*	getFrustum() const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual ICamera*		clone(int flag = CF_DEFAULT) const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			copyFrom(const ICamera& src, int flag = CF_DEFAULT) = 0;

	};//class ICamera
	typedef Handle<ICamera>		HCAMERA;

	
}//namespace Blade


#endif //__Blade_ICamera_h__