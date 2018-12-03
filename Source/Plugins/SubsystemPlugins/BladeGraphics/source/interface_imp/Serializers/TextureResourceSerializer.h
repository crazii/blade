/********************************************************************
	created:	2010/05/23
	filename: 	TextureResourceSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TextureResourceSerializer_h__
#define __Blade_TextureResourceSerializer_h__
#include <interface/public/graphics/ITexture.h>
#include <utility/IOBuffer.h>
#include <interface/public/ISerializer.h>

#include <interface/public/graphics/IImage.h>

namespace Blade
{
	class TextureResource;

	class TextureResourceSerializer : public ISerializer ,public TempAllocatable
	{
	public:
		TextureResourceSerializer(ETextureType type);
		~TextureResourceSerializer();


		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* res,const HSTREAM& stream, const ParamList& /*params*/);

		/*
		@describe load resource in main thread,if success,return true \n
		and then the resource manager will not load it again in loading thread.
		i.e. when load a texture that already exist,the serializer just check ,and return true \n
		if the texture not exist(not loaded yet) ,then return false
		@param
		@return
		*/
		//virtual bool	preLoadResource(const TString& path,IResource* res);

		/*
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* res,const HSTREAM& stream);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* res,ParamList& params);

		/*
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& /*params*/);

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource);

	protected:
		ETextureType	mTextureType;
		HIMAGE			mImage;
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Texture1DSerializer : public TextureResourceSerializer
	{
	public:
		Texture1DSerializer();

	};//class Texture1DResourceSerializer


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Texture2DSerializer : public TextureResourceSerializer
	{
	public:
		Texture2DSerializer();

	};//class Texture2DResourceSerializer

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class Texture3DSerializer : public TextureResourceSerializer
	{
	public:
		Texture3DSerializer();

	};//class Texture3DResourceSerializer

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TextureCubeSerializer : public TextureResourceSerializer
	{
	public:
		TextureCubeSerializer();

	};//class Texture3DResourceSerializer
	
	
}//namespace Blade


#endif //__Blade_TextureResourceSerializer_h__