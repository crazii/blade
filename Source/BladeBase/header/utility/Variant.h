/********************************************************************
	created:	2010/06/04
	filename: 	Variant.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Variant_h__
#define __Blade_Variant_h__
#include "String.h"
#include "TList.h"

namespace Blade
{
	class Vector2;
	class Vector3;
	class Vector4;
	class Matrix44;
	class Matrix33;
	class Quaternion;
	class Color;
	class Box3;

	enum EVariantInfo
	{
		//type flags
		VI_UNDEFINED  = 0,
		VI_FLOAT,
		VI_UINT,
		VI_INT,
		VI_BOOL,
		VI_POINTER,
		VI_WSTRING,
		VI_STRING,	//non fixed size for string, end with a string terminator,\n
					//and use stable utf8 for unicode/none-unicode build
		VI_COLOR,
		VI_QUAT,	//quaternion
		VI_V2,		//Vector2
		VI_V3,		//
		VI_V4,		//
		VI_M33,		//matrix 33
		VI_M44,		//matrix 44
		VI_AAB,
		//more types here

		VI_COLLECTION = 0x80000000,	//for collection (container) types, only keep a type info without real data
		VI_READONLY	 = 0x40000000,	//usually not used. used on special/customized scenario. 

		VI_TYPE_MASK = 0x000000FF,
		VI_SIZE_MASK = 0x0FFFFF00,
		VI_SIZE_SHIFT = 8,

		VI_BYTE_1	= 0x0100,
		VI_BYTE_2	= 0x0200,
		VI_BYTE_4	= 0x0400,
		VI_BYTE_8	= 0x0800,
		VI_BYTE_12	= 0x0C00,
		VI_BYTE_16	= 0x1000,
		VI_BYTE_24	= 0x1800,
		VI_BYTE_32	= 0x2000,
		VI_BYTE_36	= 0x2400,
		VI_BYTE_48	= 0x3000,
		VI_BYTE_64	= 0x4000,

#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
		VI_BYTE_POINTER	= VI_BYTE_4,
#else
		VI_BYTE_POINTER	= VI_BYTE_8,
#endif

#if BLADE_DOUBLE_PRESITION
		VI_BYTE_SCALAR	= VI_BYTE_8,
		VI_BYTE_V2		= VI_BYTE_16,
		VI_BYTE_V3		= VI_BYTE_24,
		VI_BYTE_V4		= VI_BYTE_32,
		VI_BYTE_QUAT	= VI_BYTE_32,
		VI_BYTE_COLOR	= VI_BYTE_32,
		VI_BYTE_M33		= 72 << VI_SIZE_SHIFT,
		VI_BYTE_M44		= 128 << VI_SIZE_SHIFT,
		VI_BYTE_AAB		= VI_BYTE_V3*2 + VI_BYTE_4,

		VI_SIZE_MAX		= 128,	//64 byte for double precision matrix44 is not enough
#else
		VI_BYTE_SCALAR	= VI_BYTE_8,
		VI_BYTE_V2		= VI_BYTE_8,
		VI_BYTE_V3		= VI_BYTE_12,
		VI_BYTE_V4		= VI_BYTE_16,
		VI_BYTE_QUAT	= VI_BYTE_16,
		VI_BYTE_COLOR	= VI_BYTE_16,
		VI_BYTE_M33		= VI_BYTE_36,
		VI_BYTE_M44		= VI_BYTE_64,
		VI_BYTE_AAB		= VI_BYTE_V3 * 2 + VI_BYTE_4,

		VI_SIZE_MAX		= 64,
#endif

#if BLADE_UNICODE
		VI_TSTRING		= VI_WSTRING,
#else
		VI_TSTRING		= VI_STRING,
#endif
	};

	/** @brief  */
	typedef class VariantInformation
	{
	public:
		VariantInformation() :mInfo(VI_UNDEFINED)					{}
		inline VariantInformation(EVariantInfo info):mInfo(info)	{}
		inline VariantInformation(EVariantInfo type, EVariantInfo bytesInfo){this->reset(type, bytesInfo);}
		inline VariantInformation(EVariantInfo type, size_t bytes){this->reset(type, bytes);}
		inline bool					isValid() const					{ return mInfo != VI_UNDEFINED; }
		inline size_t				getSizeBytes() const			{return (mInfo&VI_SIZE_MASK) >> VI_SIZE_SHIFT;}
		inline size_t				getType() const					{return mInfo&VI_TYPE_MASK;}
		inline void operator=(EVariantInfo einfo)					{mInfo = (size_t)einfo;}
		inline bool operator==(EVariantInfo einfo) const			{return mInfo == (size_t)einfo;}
		inline void					reset(EVariantInfo type, size_t bytes)	{mInfo = type|(bytes << VI_SIZE_SHIFT); }
		inline void					reset(EVariantInfo type, EVariantInfo bytesInfo)	{mInfo = size_t(type|bytesInfo); }
		inline void					setCollection() { mInfo |= VI_COLLECTION; }
		inline bool					isCollection() const { return (mInfo&VI_COLLECTION) != 0; }
		inline void					setReadOnly(bool readOnly) { if (readOnly) mInfo |= VI_READONLY; else mInfo &= ~VI_READONLY; }
		inline bool					isReadOnly() const { return (mInfo&VI_READONLY) != 0; }
	protected:
		size_t	mInfo;
	}VARIANT_INFO;

	class BLADE_BASE_API Variant : public Allocatable
	{
	private:
		//avoid implicit bool conversion
		template<typename T, typename V>
		Variant(V T::*) { assert(false); }
		template<typename T>
		Variant(T*) { assert(false); }
	public:
		Variant();
		Variant(const Variant& var)			{this->reset(var);}
		Variant(const Vector4& v4)			{this->reset(v4);}
		Variant(const Quaternion& quat)		{this->reset(quat);}
		Variant(const Color& color)			{this->reset(color);}
		Variant(const Vector3& v3)			{this->reset(v3);}
		Variant(const Vector2& v2)			{this->reset(v2);}
		Variant(const Matrix33& m33)		{ this->reset(m33); }
		Variant(const Matrix44& m44)		{ this->reset(m44); }
		Variant(const Box3& aab)			{this->reset(aab);}
		Variant(const fp32 f32)				{this->reset(f32);}
		Variant(const fp64 f64)				{this->reset(f64);}
		Variant(const uint64& ui64)			{this->reset(ui64);}
		Variant(const int64& i64)			{this->reset(i64);}
		Variant(const uint32& ui32)			{this->reset(ui32);}
		Variant(const int32& i32)			{this->reset(i32);}
		Variant(const uint16& ui16)			{this->reset(ui16);}
		Variant(const int16& i16)			{this->reset(i16);}
		Variant(const uint8& ui8)			{this->reset(ui8);}
		Variant(const int8& i8)				{this->reset(i8);}
		Variant(const bool& bln)			{this->reset(bln);}
		Variant(const String& str)			{this->reset(str);}
		Variant(const WString& wstr)		{this->reset(wstr);}
		explicit Variant(void* const& data)	{this->reset(data);}

		~Variant();

		/**
		@describe get the string value converted from actual type
		@param
		@return
		*/
		TString			toTString() const;

		/**
		@describe set the value of actual type by string
		@param
		@return
		*/
		void			fromTString(const TString& strVal);

		/**
		@describe reset type & value
		@param
		@return
		*/
		Variant&		reset(const Variant& var);
		/** @brief  */
		Variant&		reset(const Vector4& v4);
		/** @brief  */
		Variant&		reset(const Matrix33& m33);
		/** @brief  */
		Variant&		reset(const Matrix44& m44);
		/** @brief  */
		Variant&		reset(const Quaternion& quat);
		/** @brief  */
		Variant&		reset(const Color& color);
		/** @brief  */
		Variant&		reset(const Vector3& v3);
		/** @brief  */
		Variant&		reset(const Vector2& v2);
		/** @brief  */
		Variant&		reset(const Box3& box3);
		/** @brief  */
		Variant&		reset(fp32 f32);
		/** @brief  */
		Variant&		reset(fp64 f64);
		/** @brief  */
		Variant&		reset(uint64 ui64);
		/** @brief  */
		Variant&		reset(int64 i64);
		/** @brief  */
		Variant&		reset(uint32 ui32);
		/** @brief  */
		Variant&		reset(int32 i32);
		/** @brief  */
		Variant&		reset(uint16 ui16);
		/** @brief  */
		Variant&		reset(int16 i16);
		/** @brief  */
		Variant&		reset(uint8 ui8);
		/** @brief  */
		Variant&		reset(int8 i8);
		/** @brief  */
		Variant&		reset(bool bln);
		/** @brief  */
		Variant&		reset(const String& str);
		/** @brief  */
		Variant&		reset(const WString& wstr);
		/** @brief  */
		Variant&		reset(void* pdata);
		/** @brief reset to un-initialized(invalid) value. */
		Variant&		reset() { this->clear(); return *this; }

		/** @brief  */
		inline Variant&	setCollection()			{ this->reset((uint32)0); mInfo.setCollection(); return *this; }
		inline bool		isCollection() const	{ return mInfo.isCollection(); }
		inline Variant&	setReadOnly(bool readOnly) { mInfo.setReadOnly(readOnly); return *this; }
		inline bool		isReadOnly() const		{ return mInfo.isReadOnly(); }
		inline bool		isValid() const			{ return mInfo.isValid(); }

		/** @brief  */
		Variant&		operator=(const Variant& rhs);
		/** @brief  */
		Variant&		operator=(const Vector4& v4);
		/** @brief  */
		Variant&		operator=(const Quaternion& quat);
		/** @brief  */
		Variant&		operator=(const Color& color);
		/** @brief  */
		Variant&		operator=(const Vector3& v3);
		/** @brief  */
		Variant&		operator=(const Vector2& v2);
		/** @brief  */
		Variant&		operator=(const Matrix33& m33);
		/** @brief  */
		Variant&		operator=(const Matrix44& m44);
		/** @brief  */
		Variant&		operator=(const Box3& aab);
		/** @brief  */
		Variant&		operator=(fp32 fp);
		/** @brief  */
		Variant&		operator=(fp64 fp);
		/** @brief  */
		Variant&		operator=(uint64 ui64);
		/** @brief  */
		Variant&		operator=(int64 i64);
		/** @brief  */
		Variant&		operator=(uint32 ui32);
		/** @brief  */
		Variant&		operator=(int32 i32);
		/** @brief  */
		Variant&		operator=(uint16 ui16);
		/** @brief  */
		Variant&		operator=(int16 i16);
		/** @brief  */
		Variant&		operator=(uint8 ui8);
		/** @brief  */
		Variant&		operator=(int8 i8);
		/** @brief  */
		Variant&		operator=(bool bln);
		/** @brief  */
		Variant&		operator=(const String& str);
		/** @brief  */
		Variant&		operator=(const WString& str);
		/** @brief  */
		Variant&		operator=(void* pdata);

		/** @brief  */
		bool			operator==(const Variant& rhs) const;
		/** @brief  */
		bool			operator==(const Vector4& v4) const;
		/** @brief  */
		bool			operator==(const Quaternion& quat) const;
		/** @brief  */
		bool			operator==(const Color& color) const;
		/** @brief  */
		bool			operator==(const Vector3& v3) const;
		/** @brief  */
		bool			operator==(const Vector2& v2) const;
		/** @brief  */
		bool			operator==(const Box3& aab) const;
		/** @brief  */
		bool			operator==(fp32 f32) const;
		/** @brief  */
		bool			operator==(fp64 f64) const;
		/** @brief  */
		bool			operator==(uint64 ui64) const;
		/** @brief  */
		bool			operator==(int64 i64) const;
		/** @brief  */
		bool			operator==(uint32 ui32) const;
		/** @brief  */
		bool			operator==(int32 i32) const;
		/** @brief  */
		bool			operator==(uint16 ui16) const;
		/** @brief  */
		bool			operator==(int16 i16) const;
		/** @brief  */
		bool			operator==(uint8 ui8) const;
		/** @brief  */
		bool			operator==(int8 i8) const;
		/** @brief  */
		bool			operator==(bool bln) const;
		/** @brief  */
		bool			operator==(const String& str) const;
		/** @brief  */
		bool			operator==(const WString& str) const;
		/** @brief  */
		bool			operator==(const void* pdata) const;


		/** @brief  */
		inline bool		operator!=(const Variant& rhs) const		{return !(*this == rhs);}
		/** @brief  */
		inline bool		operator!=(const Vector4& v4) const			{return !(*this == v4);}
		/** @brief  */
		inline bool		operator!=(const Quaternion& quat) const	{return !(*this == quat);}
		/** @brief  */
		inline bool		operator!=(const Color& color) const		{return !(*this == color);}
		/** @brief  */
		inline bool		operator!=(const Vector3& v3) const			{return !(*this == v3);}
		/** @brief  */
		inline bool		operator!=(const Vector2& v2) const			{return !(*this == v2);}
		/** @brief  */
		inline bool		operator!=(const Box3& aab) const			{ return !(*this == aab); }
		/** @brief  */
		inline bool		operator!=(fp32 f32) const					{return !(*this == f32);}
		/** @brief  */
		inline bool		operator!=(fp64 f64) const					{return !(*this == f64);}
		/** @brief  */
		inline bool		operator!=(uint64 ui64) const				{return !(*this == ui64);}
		/** @brief  */
		inline bool		operator!=(int64 i64) const					{return !(*this == i64);}
		/** @brief  */
		inline bool		operator!=(uint32 ui32) const				{return !(*this == ui32);}
		/** @brief  */
		inline bool		operator!=(int32 i32) const					{return !(*this == i32);}
		/** @brief  */
		inline bool		operator!=(uint16 ui16) const				{return !(*this == ui16);}
		/** @brief  */
		inline bool		operator!=(int16 i16) const					{return !(*this == i16);}
		/** @brief  */
		inline bool		operator!=(uint8 ui8) const					{return !(*this == ui8);}
		/** @brief  */
		inline bool		operator!=(int8 i8) const					{return !(*this == i8);}
		/** @brief  */
		inline bool		operator!=(bool bln) const					{return !(*this == bln);}
		/** @brief  */
		inline bool		operator!=(const String& str) const			{return !(*this == str);}
		/** @brief  */
		inline bool		operator!=(const WString& wstr) const		{return !(*this == wstr);}
		/** @brief  */
		inline bool		operator!=(const void* pdata) const			{return !(*this == pdata);}


		/** @brief  */
		operator		const Vector4&() const;
		/** @brief  */
		operator		Vector4&();
		/** @brief  */
		operator		const Quaternion&() const;
		/** @brief  */
		operator		Quaternion&();
		/** @brief  */
		operator		const Color&() const;
		/** @brief  */
		operator		Color&();
		/** @brief  */
		operator		const Vector3&() const;
		/** @brief  */
		operator		Vector3&();
		/** @brief  */
		operator		const Vector2&() const;
		/** @brief  */
		operator		Vector2&();
		/** @brief  */
		operator		const Matrix33&() const;
		/** @brief  */
		operator Matrix33&();
		/** @brief  */
		operator		const Matrix44&() const;
		/** @brief  */
		operator Matrix44&();
		/** @brief  */
		operator		const Box3&() const;
		/** @brief  */
		operator Box3&();
		/** @brief  */
		operator		const scalar& () const;
		/** @brief  */
		operator		scalar&();
		/** @brief  */
		operator		const uint64() const;
		/** @brief  */
		operator		uint64&();
		/** @brief  */
		operator		const int64&() const;
		/** @brief  */
		operator		int64&();
		/** @brief  */
		operator		const uint32&() const;
		/** @brief  */
		operator		uint32&();
		/** @brief  */
		operator		const int32&() const;
		/** @brief  */
		operator		int32&();
		/** @brief  */
		operator		const uint16&() const;
		/** @brief  */
		operator		uint16&();
		/** @brief  */
		operator		const int16&() const;
		/** @brief  */
		operator		int16&();
		/** @brief  */
		operator		const uint8&() const;
		/** @brief  */
		operator		uint8&();
		/** @brief  */
		operator		const int8&() const;
		/** @brief  */
		operator		int8&();
		/** @brief  */
		operator		const bool&() const;
		/** @brief  */
		operator		bool&();
		/** @brief  */
		operator		const String&() const;
		/** @brief  */
		operator		String&();
		/** @brief  */
		operator		const WString&() const;
		/** @brief  */
		operator		WString&();
		/** @brief  */
		operator		void* const&() const;
		/** @brief  */
		operator		void*&();

		/** @brief get type info & size in bytes */
		inline VARIANT_INFO		getInfo() const		{return mInfo;}
		/** @brief get Variant type */
		size_t					getType() const		{return mInfo.getType();}
		/** @brief get size in bytes  */
		size_t					getSize() const		{return mInfo.getSizeBytes();}

		/** @brief get data without type info */
		inline const void*		getBinaryData() const
		{
			return &u_max;
		}

		/** @brief get data without type info */
		inline void*			getBinaryData()
		{
			return &u_max;
		}

		static const Variant	EMPTY;

	protected:
		union
		{
			scalar	u_f4[4];
			scalar	u_f3[3];
			scalar	u_f2[2];
			scalar	u_f33[9];
			scalar	u_f44[16];

			fp32	u_f1;
			fp64	u_d1;
			uint64	u_ui64;
			int64	u_i64;
			uint32	u_ui32;
			int32	u_i32;
			uint16	u_ui16;
			int16	u_i16;
			uint8	u_ui8;
			int8	u_i8;
			bool	u_bool;
			char	u_sob[sizeof(TString)];
			void*	u_pdata;
			char	u_max[VI_SIZE_MAX];
		};
		VARIANT_INFO	mInfo;

	private:
			void	clear();
	};//class Variant

	typedef NamedType<Variant> NamedVariant;
	typedef TNamedList<Variant, TempAllocator<NamedVariant>, TempAllocatable> ParamList;

}//namespace Blade


#endif //__Blade_Variant_h__