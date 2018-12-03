/********************************************************************
	created:	2010/07/06
	filename: 	IShader.h
	author:		Crazii
	purpose:	shader program interface, which contains VS & FS
*********************************************************************/
#ifndef __Blade_IShader_h__
#define __Blade_IShader_h__
#include <Handle.h>
#include <utility/Variant.h>

namespace Blade
{
	class ShaderInfoTable;
	class Buffer;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	enum EShaderConstantType
	{
		SCT_UNDEFINED = 0xFF,
		//| 7 | 6 | 5 | 4         | 3 | 2 | 1 | 0|
		//|type       | reserved  |  row  | col  |

		SCTF_FLOAT	= 0x20,
		SCTF_INT	= 0x40,
		SCTF_UINT	= 0x60,
		SCTF_BOOL	= 0x80,
		SCTF_SAMPLER= 0xA0,
		SCTM_TYPE	= 0xE0,

		SCTM_ROWCOUNT	= 0x0C,
		SCTM_COLCOUNT	= 0x03,

		SCT_SAMPLER = SCTF_SAMPLER,
		
		SCT_FLOAT1	= SCTF_FLOAT,
		SCT_FLOAT2,
		SCT_FLOAT3,
		SCT_FLOAT4,
		
		///note: matrix types are for GL/GLES API, or floatNxN for D3D
		SCTF_MATRIX2ROW = SCTF_FLOAT|(1<<2),
		SCTF_MATRIX3ROW = SCTF_FLOAT|(2<<2),
		SCTF_MATRIX4ROW = SCTF_FLOAT|(3<<2),

		SCT_MATRIX22 = SCTF_MATRIX2ROW|0x01,
		SCT_MATRIX23 = SCTF_MATRIX2ROW|0x02,
		SCT_MATRIX24 = SCTF_MATRIX2ROW|0x03,
		SCT_MATRIX32 = SCTF_MATRIX3ROW|0x01,
		SCT_MATRIX33 = SCTF_MATRIX3ROW|0x02,
		SCT_MATRIX34 = SCTF_MATRIX3ROW|0x03,
		SCT_MATRIX42 = SCTF_MATRIX4ROW|0x01,
		SCT_MATRIX43 = SCTF_MATRIX4ROW|0x02,
		SCT_MATRIX44 = SCTF_MATRIX4ROW|0x03,

		SCT_INT1 = SCTF_INT,
		SCT_INT2,
		SCT_INT3,
		SCT_INT4,

		SCT_UINT1 = SCTF_UINT,
		SCT_UINT2,
		SCT_UINT3,
		SCT_UINT4,

		SCT_BOOL1 = SCTF_BOOL,
		SCT_BOOL2,
		SCT_BOOL3,
		SCT_BOOL4,
	};

	enum EShaderVariableUsage
	{
		SVU_UNDEFINED = 0xFF,
		SVU_GLOBAL	= 0x00,
		///per-shader variables, differs between shader
		///all materials/shader-instances with the same shader share one same variable
		SVU_SHADER,	
		///per-pass variables, differs between passes/shader-instances. 
		///it is not frequently used, because each pass usually have its own different shader, and SVU_SHADER will work out fine
		///it is only useful when different passes shares the same shader, and need different variable data for the shader
		SVU_PASS,
		///per-instance variables, like WORLD_MATRIX, differs between renderables/material instances
		SVU_INSTANCE,

		SVU_COUNT,
		SVU_BEGIN = SVU_GLOBAL,

		SVU_TYPE_MASK		= 0x07,
		SVU_CUSTOM_FLAG		= 0x08,

		SVU_GLOBAL_CUSTOM = SVU_GLOBAL | SVU_CUSTOM_FLAG,
		SVU_SHADER_CUSTOM = SVU_SHADER | SVU_CUSTOM_FLAG,
		SVU_PASS_CUSTOM = SVU_PASS | SVU_CUSTOM_FLAG,
		SVU_INSTANCE_CUSTOM = SVU_INSTANCE | SVU_CUSTOM_FLAG,
	};

	typedef class ShaderVariableUsage
	{
#if BLADE_DEBUG
		typedef EShaderVariableUsage Type;
#else
		typedef uint8 Type;
#endif
	public:
		ShaderVariableUsage() :mUsage((Type)SVU_UNDEFINED) {}
		ShaderVariableUsage(EShaderVariableUsage usage) :mUsage((Type)usage)	{}
		/** @brief  */
		inline operator EShaderVariableUsage () const			{return EShaderVariableUsage(mUsage);}
		/** @brief  */
		inline bool operator==(EShaderVariableUsage usage) const	{return mUsage == usage;}
		inline bool operator!=(EShaderVariableUsage usage) const	{return mUsage != usage;}
		/** @brief  */
		inline bool	operator<(EShaderVariableUsage usage) const	{return mUsage < usage;}
	protected:
		Type mUsage;
	}SHADER_VARIABLE_USAGE;

	enum EShaderType
	{
		SHADER_UNDEFINED = 0xFF,
		SHADER_VERTEX = 0,
		SHADER_FRAGMENT,
		SHADER_GEOMETRY,

		SHADER_ANY,			//for GLSL program
		SHADER_TYPE_COUNT,
		SHADER_TYPE_BEGIN =  SHADER_VERTEX,
	};

	typedef class ShaderType
	{
#if BLADE_DEBUG
		typedef EShaderType Type;
#else
		typedef uint8 Type;
#endif
	public:
		ShaderType() :mType((Type)SHADER_UNDEFINED)		{}
		ShaderType(EShaderType type) :mType((Type)type)	{}
		/** @brief  */
		inline operator EShaderType () const			{return EShaderType(mType);}
		/** @brief  */
		inline bool operator==(EShaderType type) const	{return mType == type;}
		inline bool operator!=(EShaderType type) const	{return mType != type;}
		/** @brief  */
		inline bool	operator<(EShaderType type) const	{return mType < type;}
	protected:
		Type mType;
	}SHADER_TYPE;


	typedef struct SShaderConstantBool
	{
		int mBool;

		SShaderConstantBool() :mBool(false)			{}
		SShaderConstantBool(bool b)	:mBool((int)b)	{}
		SShaderConstantBool&	operator=(bool b)	{mBool = b; return *this;}
		bool	operator==(bool b) const	{return (mBool && b) || (!mBool && !b);}
		operator bool() const				{return true && mBool;}
		bool	operator!() const			{return !mBool;}
	}SC_BOOL;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef class BLADE_BASE_API ShaderConstantType
	{
#if BLADE_DEBUG
		typedef EShaderConstantType Type;
#else
		typedef uint8 Type;
#endif
	public:
		inline ShaderConstantType() 
			:mType(SCT_UNDEFINED)		{}

		inline ShaderConstantType(EShaderConstantType type_enum)
			:mType( (Type)type_enum)	{}

		/** @brief  */
		inline operator EShaderConstantType() const			{return EShaderConstantType(mType);}

		/** @brief get the atom flag: SCTF_* */
		EShaderConstantType	getAtomType() const				{return EShaderConstantType(mType&SCTM_TYPE);}

		/** @brief  */
		inline size_t	getColCount() const					{return size_t(mType&SCTM_COLCOUNT)+1u;}

		/** @brief  */
		inline size_t	getRowCount() const					{return size_t(mType&SCTM_ROWCOUNT)+1u;}

		/** @brief  */
		inline bool		isCompatible(ShaderConstantType type) const
		{
			return (this->getAtomType() == type.getAtomType())  && 
				( (this->getColCount() == type.getColCount() && this->getRowCount() >= type.getRowCount())
				|| (this->getColCount() > type.getColCount() && this->getRowCount() == type.getRowCount() && this->getRowCount() == 1)
					);
		}

		/** @brief get the real element size in bytes  */
		/// see @IGraphicsResourceManager::getShaderConstantSize
		static size_t	getConstantSize(EShaderConstantType type);

		/** @brief  */
		static const TString getConstanName(EShaderConstantType type, size_t coun);

	protected:
		Type mType;
	}SHADER_CONSTANT_TYPE;//class ShaderConstantType

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API IShader
	{
	public:
		///constant info
		typedef struct SConstantInfo
		{
			TString		mName;
			size_t		mBytes;			///total bytes
			uint16		mIndex;			///register / location
			uint16		mElementSize;	///size of each element
			uint16		mRegisterCount;	///register count for D3D( 64K max to SM5.0)
			uint16		mElementCount;	///element count for GL/GLES
			SHADER_CONSTANT_TYPE	mType;	///EShaderConstantType
			SHADER_TYPE	mShaderType;	///EShaderType

			inline bool operator<(const SConstantInfo& rhs) const
			{
				return mShaderType < rhs.mShaderType
					|| (mShaderType == rhs.mShaderType && mType.getAtomType() < rhs.mType.getAtomType())
					|| (mShaderType == rhs.mShaderType && mType.getAtomType() == rhs.mType.getAtomType() && mIndex < rhs.mIndex)
					|| (mShaderType == rhs.mShaderType && mType.getAtomType() == rhs.mType.getAtomType() && mIndex == rhs.mIndex && FnTStringFastLess::compare(mName, rhs.mName) );
			}
		}CONSTANT_INFO;

		///constant key info for runtime draw call
		typedef struct SConstantKeyInfo
		{
			uint16	mIndex;		///register / location
			uint16	mMaxBytes;	//max valid bytes
			SHADER_CONSTANT_TYPE	mType;	///EShaderConstantType
			SHADER_TYPE	mShaderType;	///EShaderType
#if BLADE_DEBUG
			TString	mName;
#endif


			SConstantKeyInfo() { mType = SCT_UNDEFINED; }
			SConstantKeyInfo(const CONSTANT_INFO& info)
			{
				mIndex = (uint16)info.mIndex;
				mType = info.mType;
				mShaderType = info.mShaderType;
				mMaxBytes = (uint16)info.mBytes;
#if BLADE_DEBUG
				mName = info.mName;
#endif
			}

			inline bool operator<(const SConstantKeyInfo& rhs) const
			{
				return mShaderType < rhs.mShaderType
					|| (mShaderType == rhs.mShaderType && mType.getAtomType() < rhs.mType.getAtomType())
					|| (mShaderType == rhs.mShaderType && mType.getAtomType() == rhs.mType.getAtomType() && mIndex < rhs.mIndex);
			}

			inline void operator=(const CONSTANT_INFO& info)
			{
				mIndex = (uint16)info.mIndex;
				mType = info.mType;
				mShaderType = info.mShaderType;
				mMaxBytes = (uint16)info.mBytes;
#if BLADE_DEBUG
				mName = info.mName;
#endif
			}
		}CONSTANT_KEY;

		typedef ShaderInfoTable INFO_TABLE;

	public:
		virtual ~IShader()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual const INFO_TABLE& getConstantTable() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const TString& getProfileString(EShaderType type) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const TString&	getEntry(EShaderType type) const = 0;


		/**
		@describe
		@param
		@return
		*/
		virtual void			getCustomOptions(EShaderType type, ParamList& option) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			getData(Buffer& data) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReady() const = 0;

	};//class IShader

	typedef Handle<IShader> HSHADER;


	/************************************************************************/
	/* inter media for shader compiling result                                                                     */
	/************************************************************************/
	///this is temporary and should not be stored/held anywhere except data loading
	class BLADE_BASE_API IShaderIntermediate : public TempAllocatable
	{
	public:
		virtual ~IShaderIntermediate()	{}

		/** @brief get the source file string of the shader */
		virtual const TString&		getName() const = 0;

		/** @brief safe id through different shader framework, such as HLSL & CG, etc */
		virtual const TString&		getShaderID() const = 0;

		/** @brief  */
		virtual const TString&		getProfile(EShaderType type) const = 0;

		/** @brief  */
		virtual const TString&		getEntry(EShaderType type) const = 0;

		/** @brief  */
		virtual const void*	getData() const	= 0;

		/** @brief  */
		virtual size_t		getDataSize() const	= 0;
	};
	typedef Handle<IShaderIntermediate> HSHADERINTMEDIA;

	
}//namespace Blade


#endif //__Blade_IShader_h__