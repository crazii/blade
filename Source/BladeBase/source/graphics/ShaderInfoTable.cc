/********************************************************************
	created:	2010/08/30
	filename: 	ShaderInfoTable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/ShaderInfoTable.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	size_t			ShaderConstantType::getConstantSize(EShaderConstantType type)
	{
		switch(type)
		{
		case SCT_FLOAT1:
		case SCT_INT1:
		case SCT_UINT1:
		case SCT_BOOL1:
		case SCT_SAMPLER:
			return 1*4;
		case SCT_FLOAT2:
		case SCT_INT2:
		case SCT_UINT2:
		case SCT_BOOL2:
			return 2*4;
		case SCT_FLOAT3:
		case SCT_INT3:
		case SCT_UINT3:
		case SCT_BOOL3:
			return 3*4;
		case SCT_FLOAT4:
		case SCT_INT4:
		case SCT_UINT4:
		case SCT_BOOL4:
		case SCT_MATRIX22:
			return 4*4;
		case SCT_MATRIX23:
		case SCT_MATRIX32:
			return 6*4;
		case SCT_MATRIX42:
		case SCT_MATRIX24:
			return 8*4;		// 2 float4s
		case SCT_MATRIX33:
			return 9*4;
		case SCT_MATRIX34:
		case SCT_MATRIX43:
			return 12*4; // 3 float4s
		case SCT_MATRIX44:
			return 16*4; // 4 float4s
		default:
			{
				ShaderConstantType t = type;
				return t.getRowCount()*t.getColCount()*4;
			}
		};
	}

	//////////////////////////////////////////////////////////////////////////
	const TString ShaderConstantType::getConstanName(EShaderConstantType etype, size_t count)
	{
		ShaderConstantType type = etype;
		const tchar* _type;

		switch (type.getAtomType())
		{
		case SCT_FLOAT1:
			_type = TEXT("float");
			break;
		case SCT_INT1:
			_type = TEXT("int");
			break;
		case SCT_UINT1:
			_type = TEXT("uint");
		case SCT_BOOL1:
			_type = TEXT("bool");
			break;
		case SCT_SAMPLER:
			_type = TEXT("sampler");
		default:
			assert(false);
			_type = TEXT("unknown");
			break;
		}

		const tchar* row;

		switch (type.getRowCount())
		{
		case 1:
			row = TEXT("");
			break;
		case 2:
			row = TEXT("2x");
			break;
		case 3:
			row = TEXT("3x");
			break;
		case 4:
			row = TEXT("4x");
			break;
		default:
			assert(false);
			row = TEXT("");
			break;
		}

		const tchar* col;
		switch (type.getColCount())
		{
		case 1:
			col = TEXT("");
			break;
		case 2:
			col = TEXT("2");
			break;
		case 3:
			col = TEXT("3");
			break;
		case 4:
			col = TEXT("4");
			break;
		default:
			assert(false);
			col = TEXT("");
			break;
		}

		TString _array;
		if(count > 1)
			_array = BTString("[") + TStringHelper::fromUInt(count) + BTString("]");

		return TStringConcat(_type) + row + col + _array;
	}

	namespace Impl
	{
		class ShaderInfoImpl : public Allocatable
		{
		public:
			ShaderInfoImpl()	{}
			~ShaderInfoImpl()	{}

		protected:
			typedef Set<IShader::CONSTANT_INFO>					InfoMap;
			typedef TStringMap<const IShader::CONSTANT_INFO*>	NameMap;

			InfoMap	mConstantMap;
			NameMap	mNamedConstantMap[SHADER_TYPE_COUNT];
		public:

			/** @brief  */
			bool			addConstantInfo(const IShader::CONSTANT_INFO& info)
			{
				if( info.mShaderType < SHADER_TYPE_BEGIN || info.mShaderType >= SHADER_TYPE_COUNT )
				{
					assert(false);
					return false;
				}

				std::pair<InfoMap::iterator,bool> result = mConstantMap.insert( info );
				if( !result.second )
				{
					//assert(false);
					return false;
				}

				std::pair<NameMap::iterator,bool> name_result = mNamedConstantMap[info.mShaderType].insert( std::make_pair(info.mName, &(*result.first)) );
				if( !name_result.second )
				{
					mConstantMap.erase(result.first);
					return false;
				}
				return true;
			}

			/** @brief  */
			const IShader::CONSTANT_INFO*	getInfo(EShaderType shaderType, index_t index, SHADER_CONSTANT_TYPE type) const
			{
				IShader::CONSTANT_INFO key;
				key.mShaderType = shaderType;
				key.mIndex = (uint16)index;
				key.mType = type;

				InfoMap::const_iterator i = mConstantMap.find(key);

				//try 'any' type
				if( i == mConstantMap.end() && shaderType != SHADER_ANY )
				{
					key.mShaderType = SHADER_ANY;
					i = mConstantMap.find(key);
				}

				if( i == mConstantMap.end() )
					return NULL;
				else
					return &(*i);
			}

			/** @brief  */
			const IShader::CONSTANT_INFO*	getInfo(EShaderType shaderType, const TString& name) const
			{
				if( shaderType < SHADER_TYPE_BEGIN || shaderType >= SHADER_TYPE_COUNT )
				{
					assert(false);
					return NULL;
				}

				NameMap::const_iterator i = mNamedConstantMap[shaderType].find(name);

				//try 'any' type
				if(i == mNamedConstantMap[shaderType].end() && shaderType != SHADER_ANY)
				{
					shaderType = SHADER_ANY;
					i = mNamedConstantMap[shaderType].find(name);
				}

				if( i == mNamedConstantMap[shaderType].end() )
					return NULL;
				else
					return i->second;
			}

			/** @brief  */
			size_t			size() const
			{
				return mConstantMap.size();
			}

			/** @brief  */
			const IShader::CONSTANT_INFO*	getAt(index_t index) const
			{
				if( index > mConstantMap.size() )
					return NULL;

				InfoMap::const_iterator iter = mConstantMap.begin();
				std::advance(iter, index);
				return &(*iter);
			}

		};//class ShaderConstantInfoData
		
	}//namespace Impl


	//////////////////////////////////////////////////////////////////////////
	ShaderInfoTable::ShaderInfoTable()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ShaderInfoTable::~ShaderInfoTable()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	const IShader::CONSTANT_INFO*	ShaderInfoTable::getConstantInfo(EShaderType shaderType, index_t index, SHADER_CONSTANT_TYPE type) const
	{
		return mInfoData != NULL ? mInfoData->getInfo(shaderType, index, type) : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const IShader::CONSTANT_INFO*	ShaderInfoTable::getConstantInfoByName(EShaderType type, const TString& name) const
	{
		return mInfoData != NULL ? mInfoData->getInfo(type, name) : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					ShaderInfoTable::addConstantInfo(const IShader::CONSTANT_INFO& cinfo)
	{
		return mInfoData->addConstantInfo(cinfo);
	}

	//////////////////////////////////////////////////////////////////////////
	size_t					ShaderInfoTable::getCount() const
	{
		return mInfoData != NULL ? mInfoData->size() : 0;
	}

	//////////////////////////////////////////////////////////////////////////
	const IShader::CONSTANT_INFO*	ShaderInfoTable::getAt(index_t index) const
	{
		return mInfoData != NULL ? mInfoData->getAt(index) : NULL;
	}
	
}//namespace Blade