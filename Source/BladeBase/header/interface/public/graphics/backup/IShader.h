/********************************************************************
	created:	2010/07/06
	filename: 	IShader.h
	author:		Crazii
	purpose:	shader program interface, which contains VS & FS
*********************************************************************/
#ifndef __Blade_IShader_h__
#define __Blade_IShader_h__
#include <Handle.h>
#include <utility/String.h>
#include <utility/ParamList.h>

namespace Blade
{
	class ShaderInfoTable;
	class Buffer;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	enum EShaderConstantType
	{
		SCT_UNDEFINED = 0x0000FFFF,

		SCTF_FLOAT	= 0x00010000,
		SCTF_INT	= 0x00020000,
		SCTF_BOOL	= 0x00040000,
		SCTM_TYPE	= 0xFFFF0000,

		SCTM_ROWCOUNT	= 0x00F0,
		SCTM_COLCOUNT	= 0x000F,
		SCTF_ARRAY		= 0x0005,
		
		SCT_FLOAT1	= SCTF_FLOAT|0x0011,
		SCT_FLOAT2,
		SCT_FLOAT3,
		SCT_FLOAT4,
		SCT_FLOAT_ARRAY,	//array count > 4
		
		SCTF_MATRIX2ROW = SCTF_FLOAT|0x0020,
		SCTF_MATRIX3ROW = SCTF_FLOAT|0x0030,
		SCTF_MATRIX4ROW = SCTF_FLOAT|0x0040,

		SCT_MATRIX22 = SCTF_MATRIX2ROW|0x0002,
		SCT_MATRIX23 = SCTF_MATRIX2ROW|0x0003,
		SCT_MATRIX24 = SCTF_MATRIX2ROW|0x0004,
		SCT_MATRIX32 = SCTF_MATRIX3ROW|0x0002,
		SCT_MATRIX33 = SCTF_MATRIX3ROW|0x0003,
		SCT_MATRIX34 = SCTF_MATRIX3ROW|0x0004,
		SCT_MATRIX42 = SCTF_MATRIX4ROW|0x0002,
		SCT_MATRIX43 = SCTF_MATRIX4ROW|0x0003,
		SCT_MATRIX44 = SCTF_MATRIX4ROW|0x0004,

		SCT_INT1 = SCTF_INT|0x0011,
		SCT_INT2,
		SCT_INT3,
		SCT_INT4,
		SCT_INT_ARRAY,

		SCT_BOOL1 = SCTF_BOOL|0x11,
		SCT_BOOL2,
		SCT_BOOL3,
		SCT_BOOL4,
		SCT_BOOL_ARRAY,
	};

	enum EShaderVariableUsage
	{
		SVU_GLOBAL	= 0x00,
		SVU_SHADER,	//differs between shader(all materials/shader-instances with the same shader share one variable)
		SVU_PASS,	//differs between passes(shader-instances or materials)
		SVU_INSTANCE, //differs between renderables(actually material instances)

		SVU_COUNT,
		SVU_BEGIN = SVU_GLOBAL,

		SVU_TYPE_MASK		= 0x0F,
		SVU_CUSTOM_FLAG		= 0x10,

		SVU_GLOBAL_CUSTOM = SVU_GLOBAL | SVU_CUSTOM_FLAG,
		SVU_SHADER_CUSTOM = SVU_SHADER | SVU_CUSTOM_FLAG,
		SVU_PASS_CUSTOM = SVU_PASS | SVU_CUSTOM_FLAG,
		SVU_INSTANCE_CUSTOM = SVU_INSTANCE | SVU_CUSTOM_FLAG,
	};

	typedef enum EShaderType
	{
		SHADER_VERTEX = 0,
		SHADER_FRAGMENT,
		SHADER_GEOMETRY,

		SHADER_TYPE_COUNT,
		SHADER_TYPE_BEGIN =  SHADER_VERTEX,
		SHADER_UNDEFINED = -1,
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef class BLADE_BASE_API ShaderConstantType
	{
	public:
		inline ShaderConstantType() 
			:mType(SCT_UNDEFINED)	{}

		inline ShaderConstantType(EShaderConstantType type_enum)
			:mType(type_enum)	{}

		/* @brief  */
		inline operator EShaderConstantType() const			{return mType;}

		/* @brief  */
		EShaderConstantType	getAtomType() const				{return EShaderConstantType(mType&SCTM_TYPE);}

		/* @brief  */
		inline size_t	isArray() const						{return (mType&SCTF_ARRAY) == SCTF_ARRAY;}

		/* @brief  */
		inline size_t	getColCount() const					{return (mType&SCTM_COLCOUNT); }

		/* @brief  */
		inline size_t	getRowCount() const					{return (mType&SCTM_ROWCOUNT); }

		/* @brief  */
		inline bool		isCompatible(ShaderConstantType type) const
		{
			return this->getAtomType() == type.getAtomType()  && !type.isArray()
				&& this->getColCount() == type.getColCount() && this->getRowCount() >= type.getRowCount();
		}

		/* @brief get the real element size in bytes  */
		//@see IGraphicsResourceManager::getShaderConstantSize
		static size_t			getConstantSize(EShaderConstantType type);

	protected:
		EShaderConstantType mType;
	}SHADER_CONSTANT_TYPE;//class ShaderConstantType

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API IShader
	{
	public:
		typedef struct SShaderConstantInfo
		{
			TString				mName;
			ShaderConstantType	mType;
			index_t				mIndex;
			size_t				mTotalBytes;
			size_t				mElementSize;
			size_t				mElementCount;
			size_t				mAlignedCount;
		}CONSTANT_INFO;

		typedef ShaderInfoTable INFO_TABLE;

	public:
		virtual ~IShader()	{}

		/*
		@describe
		@param
		@return
		*/
		virtual const INFO_TABLE& getConstantInfo(EShaderType type) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual const TString& getProfileString(EShaderType type) const = 0;

		/*
		@describe
		@param
		@return
		*/
		virtual void			getCustomOptions(EShaderType type, TParamList& option) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			getData(EShaderType type, Buffer& data) const = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReady() const = 0;

	};//class IShader

	extern template class BLADE_BASE_API Handle<IShader>;
	typedef Handle<IShader> HSHADER;


	/************************************************************************/
	/* inter media for shader compiling result                                                                     */
	/************************************************************************/
	//this is temporary and should not be stored/held anywhere except data loading
	class BLADE_BASE_API IShaderIntermedia : public TempAllocatable
	{
	public:
		virtual ~IShaderIntermedia()	{}

		/* @brief get the source file string of the shader */
		virtual const TString&		getName() const = 0;

		/* @brief safe id through different shader framework, such as HLSL & CG, etc */
		virtual const TString&		getShaderID() const = 0;

		/* @brief  */
		virtual IShader::INFO_TABLE&getConstantTable(EShaderType type) = 0;

		/* @brief  */
		virtual const TString&		getProfile(EShaderType type) const = 0;

		/* @brief  */
		virtual void				addSemanticMap(EShaderType type, const TString& variableName, const TString& semantic) = 0;

		/* @brief  */
		virtual size_t				getSemanticMapCount(EShaderType type) const = 0;

		/* @brief  */
		virtual bool				getSemnaticMap(EShaderType type, index_t index, TString& outVariable,TString& outSemantic) const = 0;

		/* @brief  */
		virtual void				addCustomSemantic(EShaderType type, const TString& variableName, EShaderVariableUsage usage) = 0;

		/* @brief  */
		virtual size_t				getCustomSemanticCount(EShaderType type) const = 0;

		/* @brief  */
		virtual bool				getCustomSemantic(EShaderType type, index_t index, TString& outVariable, EShaderVariableUsage& outUsage) = 0;

		/* @brief  */
		virtual const void*			getMediaData(EShaderType type) const = 0;

		/* @brief  */
		virtual size_t				getMediaDataSize(EShaderType type) const = 0;
	};

	extern template class BLADE_BASE_API Handle<IShaderIntermedia>;
	typedef Handle<IShaderIntermedia> HSHADERINTMEDIA;

	
}//namespace Blade


#endif //__Blade_IShader_h__