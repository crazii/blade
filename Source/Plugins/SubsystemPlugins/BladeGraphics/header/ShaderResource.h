/********************************************************************
	created:	2010/09/02
	filename: 	ShaderResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderResource_h__
#define __Blade_ShaderResource_h__
#include <GraphicsResource.h>
#include <utility/StringList.h>
#include <interface/public/graphics/IShader.h>
#include <AutoShaderVariable.h>
#include <ShaderVariableMap.h>

namespace Blade
{
	namespace Impl
	{
		class SubShaderVaraibleDataImpl;
		class SubShaderCustomVaraibleDataImpl;
		class SubShaderSemanticDataImpl;
	}//namespace Impl

	class IRenderDevice;
	class TextureState;
	struct TextureStateInstance;
	class SamplerState;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_GRAPHICS_API SubShader : public NonAssignable, public NonCopyable, public Allocatable
	{
	public:
		SubShader();
		virtual ~SubShader();

		/** @brief  */
		inline const HSHADER&		getShader() const	{return mShader;}

		/** @brief  */
		inline const TString&		getName() const		{return mName;}

		/*
		@describe
		@param
		@return
		*/
		void				updateBultInVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const;

		/*
		@describe
		@param
		@return
		*/
		void				updateCustomVariable(EShaderVariableUsage usage, IRenderDevice* renderer) const;

		/*
		@describe create & link custom pass variable
		@param
		@return
		*/
		void				createAndLinkCustomPassVariable(PassVariableMap& variableData);

		/**
		@describe create & link custom instance variable
		@param
		@return
		*/
		void				createAndLinkCustomInstanceVariable(InstanceVariableMap& variableData);

		/** @brief  */
		size_t				getInstanceVariableCount();

		/** @brief  */
		inline bool			setShaderVariable(const TString& variableName, EShaderConstantType ctype, size_t count, const void* pData)
		{
			return mCustomShaderVariables.writeVariable(variableName, ctype, count, pData);
		}

		/** @brief shader source. usually EMPTY at runtime */
		inline const String&getSourceText() const				{return mTextContent;}
		
		/** @brief  */
		inline void			setSourceText(const String& text)	{mTextContent = text;}

		/** @brief  */
		inline const ShaderVariableMap& getShaderVariables() const { return mCustomShaderVariables; }
	protected:
		/*
		@describe link shader variables defined in shader header [semantics] section
		@param
		@return
		*/
		bool				linkShaderVariable(EShaderType type, const TString& name,const TString& semantic);

		/*
		@describe
		@param
		@return
		*/
		bool				linkCustomShaderVariable(EShaderType type, const TString& name, EShaderConstantType ctype, size_t count,
			EShaderVariableUsage usage);

		/** @brief link variable & add record semantic map info */
		void				linkInternalShaderVariable(EShaderType type, const TString& name, const TString& semantic);
		/** @brief link custom variable & add record semantic map info */
		void				linkInternalCustomVariable(EShaderType type, const TString& name, EShaderConstantType ctype, size_t count,
			EShaderVariableUsage usage);

		/** @brief  */
		void				setShader(const HSHADER& shader);
		/** @brief  */
		void				setName(const TString& name);

		HSHADER				mShader;
		TString				mName;
		//original text content (without #include parsing)
		//this could be EMPTY for shaders in binary format
		String				mTextContent;
		size_t				mInstanceVariableCount;
		ShaderVariableMap	mCustomShaderVariables;

		friend class ShaderResourceSerializer;
		friend class BinaryShaderSerializer;
		friend class HybridShaderSerializer;
		friend class ShaderInstance;
	private:

		/*
		@describe 
		@param 
		@return 
		*/
		void				updateShaderVariable(const LazyPimpl<Impl::SubShaderVaraibleDataImpl>& data, IRenderDevice* renderer) const;
		void				updateCustomShaderVariable(const LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>& data, IRenderDevice* renderer) const;
		void				updateCustomInstanceShaderVariable(const LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>& data, IRenderDevice* renderer) const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool				hasVariable(const LazyPimpl<Impl::SubShaderVaraibleDataImpl>& data, const IShader::CONSTANT_INFO* info) const;
		bool				hasVariable(const LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>& data, const IShader::CONSTANT_INFO* info) const;

		LazyPimpl<Impl::SubShaderVaraibleDataImpl>			mBuiltInData[SVU_COUNT];
		LazyPimpl<Impl::SubShaderCustomVaraibleDataImpl>	mCustomData[SVU_COUNT];
		LazyPimpl<Impl::SubShaderSemanticDataImpl>			mSemantics[SHADER_TYPE_COUNT];

	};//class SubShader

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_GRAPHICS_API ShaderResource : public GraphicsResource , public Allocatable
	{
	public:
		//default shaders & serializer
		static const TString SHADER_RESOURCE_TYPE;
		//binary shader
		static const TString BINARY_SHADER_SERIALIZER;
		//source code serializer
		static const TString TEXT_SHADER_SERIALIZER;
		//hybrid: source code with constant semantics table
		static const TString HYBRID_SHADER_SERIALIZER;
	public:
		ShaderResource();
		~ShaderResource();
		/** @brief  */
		inline size_t		getProfileCount() const
		{
			return mProfiles.size();
		}

		/** @brief  */
		inline const TString&getProfileName(index_t index) const
		{
			return index < mProfiles.size() ? mProfiles[index] : TString::EMPTY;
		}

		/** @brief  */
		inline size_t		getSubShaderCount(const TString& profieName)	const
		{
			index_t index = mProfiles.find(profieName);
			if( index == INVALID_INDEX )
			{
				assert(false);
				return 0;
			}
			return mSubShaderCount[index];
		}

		/** @brief  */
		inline SubShader*	getSubShaders(const TString& profile) const
		{
			index_t profileIndex = mProfiles.find(profile);
			if( profileIndex == INVALID_INDEX )
			{
				assert(false);
				return NULL;
			}
			return mSubShaders[profileIndex];
		}

		/** @brief  */
		inline SubShader*	getSubShader(const TString& profile, index_t index) const
		{
			index_t profileIndex = mProfiles.find(profile);
			if( profileIndex == INVALID_INDEX )
			{
				assert(false);
				return NULL;
			}
			return index < mSubShaderCount[profileIndex] ? &mSubShaders[profileIndex][index] : NULL;
		}

		/*
		@describe
		@param
		@return
		*/
		SubShader*			findShader(const TString& profile, const TString& name);

	protected:
		TStringList	mProfiles;
		SubShader**	mSubShaders;
		size_t*		mSubShaderCount;	//count of each profile

		friend class ShaderResourceSerializer;
		friend class BinaryShaderSerializer;
	};//class ShaderResource

	typedef Handle<ShaderResource> HSHADERRESOURCE;

	
}//namespace Blade


#endif //__Blade_ShaderResource_h__