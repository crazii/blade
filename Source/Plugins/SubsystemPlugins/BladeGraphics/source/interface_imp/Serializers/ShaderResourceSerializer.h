/********************************************************************
	created:	2010/09/02
	filename: 	ShaderResourceSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderResourceSerializer_h__
#define __Blade_ShaderResourceSerializer_h__
#include <interface/public/ISerializer.h>
#include <ShaderResource.h>
#include <utility/IOBuffer.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class ShaderResourceSerializer : public ISerializer, public TempAllocatable
	{
	public:
		ShaderResourceSerializer();
		~ShaderResourceSerializer();

		/************************************************************************/
		/* ISerializer interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* res, const HSTREAM& stream, const ParamList& params);

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
		/*
		@describe
		@param
		@return
		*/
		bool			createShaderInterMediate(const HSTREAM& stream, const ParamList& complieParams);

		/*
		@describe 
		@param 
		@return 
		*/
		void			parsingShaderInfomation(const HSTREAM& stream,TString& outVHeader,bool root);

		/**
		@describe add variable without semantic. treat them as custom shader variable (as if having "_SHADER_" senmatic).
		@param
		@return
		*/
		void			addDefaultVariables(SubShader* subShader);

	protected:
		IOBuffer		mBuffer;

		typedef TempVector<HSHADERINTMEDIA>		InterMediaList;
		typedef TempTStringMap<TString>			SubShaderDefinitionList;

		struct ShaderProfile
		{
			TempVector<TString>	subShaderNames;
			TempVector<String>	subShaderContent;
			InterMediaList		subIntermedia;
		};
		typedef TempTStringMap<ShaderProfile>	ProfileList;

		ProfileList			mShaderProfiles;
		bool				mError;

		typedef TempVector<TString>	SemanticList;
		typedef TStringMap<SemanticList>	SemanticMap;
		//note: one variable should have one valid semantic, but it may have multiple semantic by conditional compiling (#define)
		//so we record all semantics for variable, link them to compiled shader to filter out invalid ones
		SemanticMap				mSemantics;
		typedef TStringMap<EShaderVariableUsage> CustomSemanticMap;
		CustomSemanticMap		mCustomSemantics;

		/** @brief  */
		static EShaderVariableUsage	getUsage(const TString& usageDef);

		typedef StaticTStringMap<EShaderVariableUsage> StringToUsageMap;
		typedef StaticTStringSet	ValidProfileList;
		static StringToUsageMap msStringToUsage;
		static ValidProfileList	msValidProfiles;
		static StaticLock		msInitLock;
	};//class ShaderResourceSerializer
	
}//namespace Blade

#endif //__Blade_ShaderResourceSerializer_h__