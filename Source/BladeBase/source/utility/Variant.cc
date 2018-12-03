/********************************************************************
	created:	2010/06/04
	filename: 	Variant.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)		//while(true)
#endif

namespace Blade
{
	template <typename T>
	struct VariantTypeTraits
	{
		static const int TYPE = VI_UNDEFINED;
	};
#define VARIANT_TYPE_TRAITS(_T, _Enum) template<> struct VariantTypeTraits<_T> { static const int TYPE = _Enum; }

	VARIANT_TYPE_TRAITS(Vector4, VI_V4);
	VARIANT_TYPE_TRAITS(Vector3, VI_V3);
	VARIANT_TYPE_TRAITS(Vector2, VI_V2);
	VARIANT_TYPE_TRAITS(Quaternion, VI_QUAT);
	VARIANT_TYPE_TRAITS(Box3, VI_AAB);
	VARIANT_TYPE_TRAITS(Color, VI_COLOR);
	VARIANT_TYPE_TRAITS(fp32, VI_FLOAT);
	VARIANT_TYPE_TRAITS(fp64, VI_FLOAT);
	VARIANT_TYPE_TRAITS(uint64, VI_UINT);
	VARIANT_TYPE_TRAITS(int64, VI_INT);
	VARIANT_TYPE_TRAITS(uint32, VI_UINT);
	VARIANT_TYPE_TRAITS(int32, VI_INT);
	VARIANT_TYPE_TRAITS(uint16, VI_UINT);
	VARIANT_TYPE_TRAITS(int16, VI_INT);
	VARIANT_TYPE_TRAITS(uint8, VI_UINT);
	VARIANT_TYPE_TRAITS(int8, VI_INT);
	VARIANT_TYPE_TRAITS(bool, VI_BOOL);
	VARIANT_TYPE_TRAITS(String, VI_STRING);
	VARIANT_TYPE_TRAITS(WString, VI_WSTRING);
	VARIANT_TYPE_TRAITS(void*, VI_POINTER);

#define TYPE_CHECK(_type)	do{ if( this->getType() != _type )	{BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("type mismatch.") );}  }while(false)
#define SIZE_CHECK(_T)	do{ if( this->getSize() != sizeof(_T))	{BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("size mismatch.") );}  }while(false)
#define TYPESIZE_CHECK(_T) do{ if( this->getType() != (size_t)VariantTypeTraits<_T>::TYPE || this->getSize() != sizeof(_T)) {BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("type or size mismatch.") );}  }while(false)
#define TYPESIZE_CHECKSET(_T) do{\
	if( this->getType() == VI_UNDEFINED )\
		mInfo.reset( (EVariantInfo)VariantTypeTraits<_T>::TYPE, sizeof(_T));\
	else if( this->getType() != (size_t)VariantTypeTraits<_T>::TYPE || this->getSize() != sizeof(_T)) {\
		BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("type or size mismatch.") );}\
	}while(false)

	const Blade::Variant Blade::Variant::EMPTY;

	//////////////////////////////////////////////////////////////////////////
	Variant::Variant()
	{
		mInfo = VI_UNDEFINED;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::~Variant()
	{
		this->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	TString		Variant::toTString() const
	{
		size_t type = this->getType();
		size_t size = this->getSize();
		switch( type )
		{
		case VI_M33:
			{
				TStringStream stream;
				stream.format(TEXT("(%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f)"),
					u_f33[0],u_f33[1],u_f33[2],u_f33[3],u_f33[4],u_f33[5],u_f33[6],u_f33[7],u_f33[8]
				);
				return stream.str();
			}
		case VI_M44:
			{
				TStringStream stream;
				stream.format(TEXT("(%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f)"),
					u_f44[0],u_f44[1],u_f44[2],u_f44[3],u_f44[4],u_f44[5],u_f44[6],u_f44[7],
					u_f44[8],u_f44[9],u_f44[10],u_f44[11],u_f44[12],u_f44[13],u_f44[14],u_f44[15]
				);
				return stream.str();
			}
		case VI_V4:
		case VI_QUAT:
		case VI_COLOR:
			{
				TStringStream stream;
				stream.format(TEXT("(%0.3f,%0.3f,%0.3f,%0.3f)"),u_f4[0],u_f4[1],u_f4[2],u_f4[3]);
				return stream.str();
			}
		case VI_V3:
			{
				TStringStream stream;
				stream.format(TEXT("(%0.3f,%0.3f,%0.3f)"),u_f4[0],u_f4[1],u_f4[2] );
				return stream.str();
			}
		case VI_V2:
			{
				TStringStream stream;
				stream.format(TEXT("(%0.3f,%0.3f)"),u_f4[0],u_f4[1]);
				return stream.str();
			}
		case VI_AAB:
			{
				TStringStream stream;
				stream.format(TEXT("(%0.3f,%0.3f,%0.3f),(%0.3f,%0.3f,%0.3f):%d"), 
					u_f44[0], u_f44[1], u_f44[2], u_f44[3], u_f44[4], u_f44[5], reinterpret_cast<const int&>(u_f44[6]));
				return stream.str();
			}
			break;
		case VI_INT:
			if( size == 1)
				return TStringHelper::fromInt32( u_i8 );
			else if( size == 2 )
				return TStringHelper::fromInt32( u_i16 );
			else if( size == 4)
				return TStringHelper::fromInt32( u_i32 );
			else
			{
				assert(size == 8);
				return TStringHelper::fromInt64( u_i64 );
			}
		case VI_UINT:
			if( size == 1)
				return TStringHelper::fromUInt32( u_ui8 );
			else if( size == 2 )
				return TStringHelper::fromUInt32( u_ui16 );
			else if( size == 4)
				return TStringHelper::fromUInt32( u_ui32 );
			else
			{
				assert(size == 8);
				return TStringHelper::fromUInt64( u_ui64 );
			}
		case VI_FLOAT:
			{
				if( size == 4 )
					return TStringHelper::fromF32(u_f1);
				if( size == 8 )
					return TStringHelper::fromF64(u_d1);
			}
		case VI_BOOL:
			return TStringHelper::fromBool(u_bool);
		case VI_STRING:
#if BLADE_UNICODE
			return StringConverter::StringToTString( this->operator const String&() );
#else
			return this->operator const String&();
#endif
		case VI_WSTRING:
#if BLADE_UNICODE
			return this->operator const WString&();
#else
			return StringConverter::WStringToTString( this->operator const WString&() );
#endif
		case VI_POINTER:
			return TStringHelper::fromPointer(u_pdata);
		}
		assert(false);
		return TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Variant::fromTString(const TString& strVal)
	{
		size_t type = this->getType();
		size_t size = this->getSize();
		switch( type )
		{
		case VI_M33:
			{
				TStringStream stream;
				int n = stream.scanFormat(TEXT("(%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f)"),
					u_f33[0],u_f33[1],u_f33[2],u_f33[3],u_f33[4],u_f33[5],u_f33[6],u_f33[7],u_f33[8] );
				assert( n == 9 );
				BLADE_UNREFERENCED(n);
			}
			break;
		case VI_M44:
			{
				TStringStream stream;
				int n = stream.scanFormat(TEXT("(%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f)"),
					u_f44[0],u_f44[1],u_f44[2],u_f44[3],u_f44[4],u_f44[5],u_f44[6],u_f44[7],
					u_f44[8],u_f44[9],u_f44[10],u_f44[11],u_f44[12],u_f44[13],u_f44[14],u_f44[15]
				);
				assert( n == 16 );
				BLADE_UNREFERENCED(n);
			}
			break;
		case VI_V4:
		case VI_QUAT:
		case VI_COLOR:
			{
				TStringStream stream;
				stream.str( strVal );
#if BLADE_DOUBLE_PRESITION
				//for scanf: %lf is double, %f is float
				//for printf: %lf,%f are all promoted to doubles
				//it's weird.
				int n = stream.scanFormat( TEXT("(%lf,%lf,%lf,%lf)"),&u_f4[0],&u_f4[1],&u_f4[2],&u_f4[3] );
#else
				int n = stream.scanFormat( TEXT("(%f,%f,%f,%f)"),&u_f4[0],&u_f4[1],&u_f4[2],&u_f4[3] );
#endif
				assert( n == 4 || (n == 3 && type == VI_COLOR) );
				BLADE_UNREFERENCED(n);
			}
			break;
		case VI_V3:
			{
				TStringStream stream;
				stream.str( strVal );
#if BLADE_DOUBLE_PRESITION
				int n = stream.scanFormat( TEXT("(%lf,%lf,%lf)"),&u_f4[0],&u_f4[1],&u_f4[2] );
#else
				int n = stream.scanFormat( TEXT("(%f,%f,%f)"),&u_f4[0],&u_f4[1],&u_f4[2] );
#endif
				assert( n == 3 );
				BLADE_UNREFERENCED(n);
			}
			break;
		case VI_V2:
			{
				TStringStream stream;
				stream.str( strVal );
#if BLADE_DOUBLE_PRESITION
				int n = stream.scanFormat( TEXT("(%lf,%lf)"),&u_f4[0],&u_f4[1] );
#else
				int n = stream.scanFormat( TEXT("(%f,%f)"),&u_f4[0],&u_f4[1] );
#endif
				assert( n == 2 );
				BLADE_UNREFERENCED(n);
			}
			break;
		case VI_AAB:
			{
				TStringStream stream;
				stream.str(strVal);
#if BLADE_DOUBLE_PRESITION
				int n = stream.scanFormat(TEXT("(%lf,%lf,%lf),(%lf,%lf,%lf):%d"),
					&u_f44[0], &u_f44[1], &u_f44[2], &u_f44[3], &u_f44[4], &u_f44[5], (int*)&u_f44[6]);
#else
				int n = stream.scanFormat(TEXT("(%f,%f,%f),(%f,%f,%f):%d"), 
					&u_f44[0], &u_f44[1], &u_f44[2], &u_f44[3], &u_f44[4], &u_f44[5], (int*)&u_f44[6]);
#endif
				assert(n == 7);
				BLADE_UNREFERENCED(n);
			}
			break;
		case VI_INT:
			{
				if( size == 1)
					u_i8 = (int8)TStringHelper::toInt32(strVal);
				else if( size == 2 )
					u_i16 = (int16)TStringHelper::toInt32(strVal);
				else if( size == 4)
					u_i32 = (int32)TStringHelper::toInt32(strVal);
				else
				{
					assert(size == 8);
					u_i64 = TStringHelper::toInt64(strVal);
				}
			}
			break;
		case VI_UINT:
			{
				if( size == 1)
					u_ui8 = (uint8)TStringHelper::toUInt32(strVal);
				else if( size == 2 )
					u_ui16 = (uint16)TStringHelper::toUInt32(strVal);
				else if( size == 4)
					u_ui32 = (uint32)TStringHelper::toUInt32(strVal);
				else
				{
					assert(size == 8);
					u_ui64 = TStringHelper::toUInt64(strVal);
				}
			}
			break;
		case VI_FLOAT:
			{
				if( size == 4 )
					u_f1 = TStringHelper::toF32(strVal);
				if( size == 8 )
					u_d1 = TStringHelper::toF64(strVal);
			}
			break;
		case VI_BOOL:
			u_bool = TStringHelper::toBool(strVal);
			break;
		case VI_STRING:
#if BLADE_UNICODE
			this->operator String&() = StringConverter::TStringToString(strVal);
#else
			this->operator String&() = strVal;
#endif
			break;
		case VI_WSTRING:
#if BLADE_UNICODE
			this->operator WString&() = strVal;
#else
			this->operator WString&() = StringConverter::TStringToWString(strVal);
#endif
			break;
		case VI_POINTER:
				u_pdata = TStringHelper::toPointer(strVal);
			break;
		default:
			assert(false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Variant& var)
	{
		this->clear();
		if( &var != this )
		{
			mInfo = var.getInfo();
			if( var.getType() == VI_STRING )
				this->reset( (const String&)var );
			else if( var.getType() == VI_WSTRING )
				this->reset( (const WString&)var );
			else
				std::memcpy(u_max,var.u_max, VI_SIZE_MAX);
		}
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Vector4& v4)
	{
		this->clear();
		mInfo.reset(VI_V4, VI_BYTE_V4);
		TYPESIZE_CHECK(Vector4);
		std::memcpy(u_f4, &v4, sizeof(v4));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Matrix33& m33)
	{
		this->clear();
		mInfo.reset(VI_M33, VI_BYTE_M33);
		TYPESIZE_CHECK(Matrix33);
		std::memcpy(u_f33, &m33, sizeof(m33));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Matrix44& m44)
	{
		this->clear();
		mInfo.reset(VI_M44, VI_BYTE_M44);
		TYPESIZE_CHECK(Matrix33);
		std::memcpy(u_f44, &m44, sizeof(m44));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Quaternion& quat)
	{
		this->clear();
		mInfo.reset(VI_QUAT, VI_BYTE_QUAT);
		TYPESIZE_CHECK(Quaternion);
		std::memcpy(u_f4, &quat, sizeof(Quaternion) );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Color& color)
	{
		this->clear();
		mInfo.reset(VI_COLOR, VI_BYTE_COLOR);
		TYPESIZE_CHECK(Color);
		std::memcpy(u_f4,&color,sizeof(color));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Vector3& v3)
	{
		this->clear();
		mInfo.reset(VI_V3, VI_BYTE_V3);
		TYPESIZE_CHECK(Vector3);
		std::memcpy(u_f3,&v3,sizeof(v3));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Vector2& v2)
	{
		this->clear();
		mInfo.reset(VI_V2, VI_BYTE_V2);
		TYPESIZE_CHECK(Vector2);
		std::memcpy(u_f2,&v2,sizeof(v2));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const Box3& box3)
	{
		this->clear();
		mInfo.reset(VI_AAB, VI_BYTE_AAB);
		TYPESIZE_CHECK(Box3);
		std::memcpy(u_f44, &box3, sizeof(box3));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(fp32 f32)
	{
		this->clear();
		mInfo.reset(VI_FLOAT, VI_BYTE_4);
		u_f1 = f32;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(fp64 f64)
	{
		this->clear();
		mInfo.reset(VI_FLOAT, VI_BYTE_8);
		u_d1 = f64;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(uint64 ui64)
	{
		this->clear();
		mInfo.reset(VI_UINT, VI_BYTE_8);
		u_ui64 = ui64;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(int64 i64)
	{
		this->clear();
		mInfo.reset(VI_INT, VI_BYTE_8);
		u_i64 = i64;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(uint32 ui32)
	{
		this->clear();
		mInfo.reset(VI_UINT, VI_BYTE_4);
		u_ui32 = ui32;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(int32 i32)
	{
		this->clear();
		mInfo.reset(VI_INT, VI_BYTE_4);
		u_i32 = i32;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(uint16 ui16)
	{
		this->clear();
		mInfo.reset(VI_UINT, VI_BYTE_2);
		u_ui16 = ui16;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(int16 i16)
	{
		this->clear();
		mInfo.reset(VI_INT, VI_BYTE_2);
		u_i16 = i16;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(uint8 ui8)
	{
		this->clear();
		mInfo.reset(VI_UINT, VI_BYTE_1);
		u_ui8 = ui8;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(int8 i8)
	{
		this->clear();
		mInfo.reset(VI_UINT, VI_BYTE_1);
		u_i8 = i8;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(bool bln)
	{
		this->clear();
		mInfo.reset(VI_BOOL, sizeof(bool) );
		u_bool = bln;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const String& str)
	{
		this->clear();
		mInfo.reset(VI_STRING, sizeof(String) );
		String* ustr = new (u_sob) String();
		*ustr = str;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(const WString& str)
	{
		this->clear();
		mInfo.reset(VI_WSTRING, sizeof(WString) );
		WString* ustr = new (u_sob) WString();
		*ustr = str;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::reset(void* pdata)
	{
		this->clear();
		mInfo.reset(VI_POINTER, VI_BYTE_POINTER);
		u_pdata = pdata;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Variant& rhs)
	{
		//if( this->getType() != VI_UNDEFINED )
		//	TYPE_CHECK( rhs.getType() );
		if( this != &rhs)
			this->reset(rhs);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Vector4& v4)
	{
		TYPESIZE_CHECKSET(Vector4);
		std::memcpy(u_f4,&v4,sizeof(v4));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Quaternion& quat)
	{
		TYPESIZE_CHECKSET(Quaternion);
		std::memcpy(u_f4,&quat,sizeof(quat));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Color& color)
	{
		TYPESIZE_CHECKSET(Color);
		std::memcpy(u_f4,&color,sizeof(color));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Vector3& v3)
	{
		TYPESIZE_CHECKSET(Vector3);
		std::memcpy(u_f3,&v3,sizeof(v3));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Vector2& v2)
	{
		TYPESIZE_CHECKSET(Vector2);
		std::memcpy(u_f2,&v2,sizeof(v2));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Matrix33& m33)
	{
		TYPESIZE_CHECKSET(Matrix33);
		std::memcpy(u_f33, &m33, sizeof(m33));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Matrix44& m44)
	{
		TYPESIZE_CHECKSET(Matrix44);
		std::memcpy(u_f44, &m44, sizeof(m44));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const Box3& aab)
	{
		TYPESIZE_CHECKSET(Box3);
		std::memcpy(u_f44, &aab, sizeof(aab));
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(fp32 fp)
	{
		TYPESIZE_CHECKSET(fp32);
		u_f1 = fp;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(fp64 fp)
	{
		TYPESIZE_CHECKSET(fp64);
		u_d1 = fp;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(uint64 ui64)
	{
		TYPESIZE_CHECKSET(uint64);
		u_ui64 = ui64;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(int64 i64)
	{
		TYPESIZE_CHECKSET(uint64);
		u_i64 = i64;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(uint32 ui32)
	{
		TYPESIZE_CHECKSET(uint32);
		u_ui32 = ui32;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(int32 i32)
	{
		TYPESIZE_CHECKSET(int32);
		u_i32 = i32;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(uint16 ui16)
	{
		TYPESIZE_CHECKSET(uint16);
		u_ui16 = ui16;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(int16 i16)
	{
		TYPESIZE_CHECKSET(uint16);
		u_i16 = i16;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(uint8 ui8)
	{
		TYPESIZE_CHECKSET(uint8);
		u_ui8 = ui8;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(int8 i8)
	{
		TYPESIZE_CHECKSET(int8);
		u_i8 = i8;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(bool bln)
	{
		TYPESIZE_CHECKSET(bool);
		u_bool = bln;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const String& str)
	{
		if (this->getType() == VI_UNDEFINED)
			new (u_sob) WString();
		TYPESIZE_CHECKSET(String);
		String* ustr = (String*)u_sob;
		*ustr = str;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(const WString& str)
	{
		if (this->getType() == VI_UNDEFINED)
			new (u_sob) WString();
		TYPESIZE_CHECKSET(WString);
		WString* ustr = (WString*)u_sob;
		*ustr = str;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant&		Variant::operator=(void* pdata)
	{
		TYPESIZE_CHECKSET(void*);
		u_pdata = pdata;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Variant& rhs) const
	{
		size_t type = this->getType();
		if( type != rhs.getType() )
			return false;
		size_t size = this->getSize();

		if( size != rhs.getSize() )
			return false;

		if( type == VI_STRING )
			return *this == (const String&)rhs;
		else if( type == VI_WSTRING )
			return *this == (const WString&)rhs;
		else
			return ::memcmp(u_max, rhs.u_max, size) == 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Vector4& v4) const
	{
		TYPESIZE_CHECK(Vector4);
		return reinterpret_cast<const Vector4&>(u_f4) == v4;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Quaternion& quat) const
	{
		TYPESIZE_CHECK(Quaternion);
		return reinterpret_cast<const Quaternion&>(u_f4) == quat;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Color& color) const
	{
		TYPESIZE_CHECK(Color);
		return reinterpret_cast<const Color&>(u_f4) == color;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Vector3& v3) const
	{
		TYPESIZE_CHECK(Vector3);
		return reinterpret_cast<const Vector3&>(u_f3) == v3;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Vector2& v2) const
	{
		TYPESIZE_CHECK(Vector2);
		return reinterpret_cast<const Vector2&>(u_f2) == v2;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const Box3& aab) const
	{
		TYPESIZE_CHECK(Box3);
		return reinterpret_cast<const Box3&>(u_f44) == aab;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(fp32 f32) const
	{
		TYPESIZE_CHECK(fp32);
		return u_f1 == f32;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(fp64 f64) const
	{
		TYPESIZE_CHECK(fp64);
		return u_d1 == f64;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(uint64 ui64) const
	{
		TYPESIZE_CHECK(uint64);
		return u_ui64 == ui64;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(int64 i64) const
	{
		TYPESIZE_CHECK(int64);
		return u_i64 == i64;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(uint32 ui32) const
	{
		TYPESIZE_CHECK(uint32);
		return u_ui32 == ui32;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(int32 i32) const
	{
		TYPESIZE_CHECK(int32);
		return u_i32 == i32;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(uint16 ui16) const
	{
		TYPESIZE_CHECK(uint16);
		return u_ui16 == ui16;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(int16 i16) const
	{
		TYPESIZE_CHECK(int16);
		return u_i16 == i16;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(uint8 ui8) const
	{
		TYPESIZE_CHECK(uint8);
		return u_ui8 == ui8;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(int8 i8) const
	{
		TYPESIZE_CHECK(int8);
		return u_i8 == i8;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(bool bln) const
	{
		TYPESIZE_CHECK(bool);
		return u_bool == bln;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const String& str) const
	{
		TYPESIZE_CHECK(String);
		const String* ustr = (const String*)u_sob;
		return *(ustr) == str;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const WString& str) const
	{
		TYPESIZE_CHECK(WString);
		const WString* ustr = (const WString*)u_sob;
		return *(ustr) == str;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Variant::operator==(const void* pdata) const
	{
		TYPESIZE_CHECK(void*);
		return u_pdata == pdata;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Vector4&() const
	{
		TYPESIZE_CHECK(Vector4);
		return reinterpret_cast<const Vector4&>(u_f4);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		Vector4&()
	{
		TYPESIZE_CHECK(Vector4);
		return reinterpret_cast<Vector4&>(u_f4);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Quaternion&() const
	{
		TYPESIZE_CHECK(Quaternion);
		return reinterpret_cast<const Quaternion&>(u_f4);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		Quaternion&()
	{
		TYPESIZE_CHECK(Quaternion);
		return reinterpret_cast<Quaternion&>(u_f4);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Color&() const
	{
		TYPESIZE_CHECK(Color);
		return reinterpret_cast<const Color&>(u_f4);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		Color&()
	{
		TYPESIZE_CHECK(Color);
		return reinterpret_cast<Color&>(u_f4);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Vector3&() const
	{
		TYPESIZE_CHECK(Vector3);
		return reinterpret_cast<const Vector3&>(u_f3);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		Vector3&()
	{
		TYPESIZE_CHECK(Vector3);
		return reinterpret_cast<Vector3&>(u_f3);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Vector2&() const
	{
		TYPESIZE_CHECK(Vector2);
		return reinterpret_cast<const Vector2&>(u_f2);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		Vector2&()
	{
		TYPESIZE_CHECK(Vector2);
		return reinterpret_cast<Vector2&>(u_f2);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Matrix33&() const
	{
		TYPESIZE_CHECK(Matrix33);
		return reinterpret_cast<const Matrix33&>(u_f33);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator Matrix33&()
	{
		TYPESIZE_CHECK(Matrix33);
		return reinterpret_cast<Matrix33&>(u_f33);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Matrix44&() const
	{
		TYPESIZE_CHECK(Matrix44);
		return reinterpret_cast<const Matrix44&>(u_f44);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator Matrix44&()
	{
		TYPESIZE_CHECK(Matrix44);
		return reinterpret_cast<Matrix44&>(u_f44);
	}


	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const Box3&() const
	{
		TYPESIZE_CHECK(Box3);
		return reinterpret_cast<const Box3&>(u_f44);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator Box3&()
	{
		TYPESIZE_CHECK(Box3);
		return reinterpret_cast<Box3&>(u_f44);
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const scalar&() const
	{
		TYPESIZE_CHECK(scalar);
		return u_f1;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		scalar&()
	{
		TYPESIZE_CHECK(scalar);
		return u_f1;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const uint64() const
	{
		TYPESIZE_CHECK(uint64);
		return u_ui64;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		uint64&()
	{
		TYPESIZE_CHECK(uint64);
		return u_ui64;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const int64&() const
	{
		TYPESIZE_CHECK(int64);
		return u_i64;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		int64&()
	{
		TYPESIZE_CHECK(int64);
		return u_i64;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const uint32&() const
	{
		TYPESIZE_CHECK(uint32);
		return u_ui32;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		uint32&()
	{
		TYPESIZE_CHECK(uint32);
		return u_ui32;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const int32&() const
	{
		TYPESIZE_CHECK(int32);
		return u_i32;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		int32&()
	{
		TYPESIZE_CHECK(int32);
		return u_i32;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const uint16&() const
	{
		TYPESIZE_CHECK(uint16);
		return u_ui16;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		uint16&()
	{
		TYPESIZE_CHECK(uint16);
		return u_ui16;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const int16&() const
	{
		TYPESIZE_CHECK(int16);
		return u_i16;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		int16&()
	{
		TYPESIZE_CHECK(int16);
		return u_i16;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const uint8&() const
	{
		TYPESIZE_CHECK(uint8);
		return u_ui8;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		uint8&()
	{
		TYPESIZE_CHECK(uint8);
		return u_ui8;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const int8&() const
	{
		TYPESIZE_CHECK(int8);
		return u_i8;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		int8&()
	{
		TYPESIZE_CHECK(int8);
		return u_i8;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const bool&() const
	{
		TYPESIZE_CHECK(bool);
		return u_bool;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		bool&()
	{
		TYPESIZE_CHECK(bool);
		return u_bool;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const String&() const
	{
		TYPE_CHECK(VI_STRING);
		const String* ustr = (const String*)u_sob;
		return *ustr;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		String&()
	{
		TYPE_CHECK(VI_STRING);
		String* ustr = (String*)u_sob;
		return *ustr;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		const WString&() const
	{
		TYPE_CHECK(VI_WSTRING);
		const WString* ustr = (const WString*)u_sob;
		return *ustr;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		WString&()
	{
		TYPE_CHECK(VI_WSTRING);
		WString* ustr = (WString*)u_sob;
		return *ustr;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		void* const &() const
	{
		TYPESIZE_CHECK(void*);
		return u_pdata;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant::operator		void*&()
	{
		TYPESIZE_CHECK(void*);
		return u_pdata;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Variant::clear()
	{
		if( this->getType() == VI_STRING )
		{
			String* ustr = (String*)u_sob;
			ustr->~String();
		}
		else if( this->getType() == VI_WSTRING )
		{
			WString* ustr = (WString*)u_sob;
			ustr->~WString();
		}
		mInfo = VI_UNDEFINED;
		std::memset(u_max, 0, VI_SIZE_MAX);
	}

	
}//namespace Blade