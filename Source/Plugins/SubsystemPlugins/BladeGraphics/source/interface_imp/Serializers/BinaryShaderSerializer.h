/********************************************************************
	created:	2013/05/19
	filename: 	BinaryShaderSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BinaryShaderSerializer_h__
#define __Blade_BinaryShaderSerializer_h__
#include <interface/public/ISerializer.h>
#include <ShaderResource.h>
#include <utility/StringList.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class BinaryShaderSerializer : public ISerializer , public TempAllocatable
	{
	public:
		BinaryShaderSerializer();
		~BinaryShaderSerializer();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* res,const HSTREAM& stream, const ParamList& params);

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
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource);


	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*@brief prepare content for saving */
		virtual bool	getContentBuffer(SubShader* subShader, Buffer& buffer);

		/*@brief loading intermediate from content buffer */
		virtual HSHADERINTMEDIA	loadContentBuffer(const TString& name, const void* data, size_t bytes,
			const TString& profile, const TString entrypoint[SHADER_TYPE_COUNT]);

		/*
		@describe
		@param
		@return
		*/
		bool			createShaderInterMediate(const HSTREAM& stream);

	protected:
		IOBuffer		mBuffer;
		typedef TempVector<HSHADERINTMEDIA> InterMediaList;
		typedef TempVector<TString>			SubShaderNameList;

		typedef TStringMap<TString>	SemanticMap;
		typedef TempVector<SemanticMap> SubShaderSemantics;
		typedef TStringMap<EShaderVariableUsage> CustomSemanticMap;
		typedef TempVector<CustomSemanticMap> SubShaderCustomSemantics;


		struct ShaderProfile
		{
			SubShaderNameList		mShaderNames;
			InterMediaList			mShaderInterMedia;
			//note: different sub shaders may have the same variable with different semantics (by using #define)
			SubShaderSemantics		mSemantics[SHADER_TYPE_COUNT];
			SubShaderCustomSemantics mCustomSemantics[SHADER_TYPE_COUNT];
		};

		typedef TempTStringMap<ShaderProfile>	ProfileList;
		ProfileList	mProfiles;
		bool		mError;
	};//class BinaryShaderSerializer
	
}//namespace Blade

#endif //__Blade_BinaryShaderSerializer_h__