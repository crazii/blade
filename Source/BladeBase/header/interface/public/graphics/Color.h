/********************************************************************
	created:	2010/04/21
	filename: 	Color.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Color_h__
#define __Blade_Color_h__

namespace Blade
{

	class Color
	{
	public:
		///byte order RGBA
		enum EChannel
		{
			R = 0,
			G = 1,
			B = 2,
			A = 3,
		};

		typedef union UColor
		{
			uint8 component[4];
			uint32 color;

			inline UColor()						{}
			inline UColor(uint32 v) :color(v)	{}
			inline UColor(uint8 c0, uint8 c1, uint8 c2, uint8 c3)	{component[0] = c0; component[1] = c1; component[2] = c2; component[3] = c3;}
			inline operator uint32() const		{return color;}
		}COLOR;

		///byte order RGBA
		typedef union UColorRGBA
		{
			struct
			{
				uint8 r;
				uint8 g;
				uint8 b;
				uint8 a;
			};
			uint8 component[4];
			uint32 color;
			inline UColorRGBA()						{}
			inline UColorRGBA(uint32 v) :color(v)	{}
			inline UColorRGBA(uint8 _r, uint8 _g, uint8 _b, uint8 _a)	{r = _r; g = _g; b = _b; a = _a;}
		}RGBA;

		///byte order RGB
		typedef union UColorRGB
		{
			struct
			{
				uint8 r;
				uint8 g;
				uint8 b;
			};
			uint8 component[3];
			inline UColorRGB()								{}
			inline UColorRGB(UColorRGBA rgba)				{r = rgba.r; g = rgba.g; b = rgba.b;}
			inline UColorRGB(uint8 _r, uint8 _g, uint8 _b)	{r = _r; g = _g; b = _b;}
		}RGB;

		inline Color()
			:r(0.0f)
			,g(0.0f)
			,b(0.0f)
			,a(1.0f)
		{

		}

		inline Color(fp32 red,fp32 green,fp32 blue,fp32 alpha)
			:r(red)
			,g(green)
			,b(blue)
			,a(alpha)
		{

		}

		inline Color(fp32 red,fp32 green,fp32 blue)
			:r(red)
			,g(green)
			,b(blue)
			,a(1.0f)
		{

		}

		/** @brief  */
		inline Color(UColorRGBA rgba)
		{
			this->setComponent8U(rgba.r, rgba.g, rgba.b, rgba.a);
		}

		/** @brief  */
		inline operator RGBA() const
		{
			RGBA ret;
			this->getComponent8U(ret.r, ret.g, ret.b, ret.a);
			return ret;
		}

		/** @brief  */
		inline fp32		operator[](size_t index) const
		{
			assert( index < 4 );
			return v[index];
		}

		/** @brief  */
		inline fp32&	operator[](size_t index)
		{
			assert( index < 4 );
			return v[index];
		}

		/** @brief  */
		inline const fp32*	getData() const
		{
			return v;
		}

		/** @brief  */
		inline fp32*		getData()
		{
			return v;
		}

		/** @brief  */
		inline Color		operator+(const Color& rhs) const
		{
			Color ret;
			ret.r = this->r + rhs.r;
			ret.g = this->g + rhs.g;
			ret.b = this->b + rhs.b;
			ret.a = this->a + rhs.a;
			return ret;
		}

		/** @brief  */
		inline Color		operator-(const Color& rhs) const
		{
			Color ret;
			ret.r = this->r - rhs.r;
			ret.g = this->g - rhs.g;
			ret.b = this->b - rhs.b;
			ret.a = this->a - rhs.a;
			return ret;
		}

		/** @brief  */
		inline Color		operator*(const Color& rhs) const
		{
			Color ret;
			ret.r = this->r * rhs.r;
			ret.g = this->g * rhs.g;
			ret.b = this->b * rhs.b;
			ret.a = this->a * rhs.a;
			return ret;
		}

		/** @brief  */
		inline Color		operator/(const Color& rhs) const
		{
			Color ret;
			ret.r = this->r / rhs.r;
			ret.g = this->g / rhs.g;
			ret.b = this->b / rhs.b;
			ret.a = this->a / rhs.a;
			return ret;
		}

		/** @brief  */
		inline Color		operator*(fp32 rhs) const
		{
			Color ret;
			ret.r = this->r * rhs;
			ret.g = this->g * rhs;
			ret.b = this->b * rhs;
			ret.a = this->a * rhs;
			return ret;
		}

		/** @brief  */
		inline Color		operator/(fp32 rhs) const
		{
			Color ret;
			ret.r = this->r / rhs;
			ret.g = this->g / rhs;
			ret.b = this->b / rhs;
			ret.a = this->a / rhs;
			return ret;
		}

		/** @brief  */
		inline Color		operator+(fp32 rhs) const
		{
			Color ret;
			ret.r = this->r + rhs;
			ret.g = this->g + rhs;
			ret.b = this->b + rhs;
			ret.a = this->a + rhs;
			return ret;
		}

		/** @brief  */
		inline Color		operator-(fp32 rhs) const
		{
			Color ret;
			ret.r = this->r - rhs;
			ret.g = this->g - rhs;
			ret.b = this->b - rhs;
			ret.a = this->a - rhs;
			return ret;
		}

		/** @brief  */
		inline friend Color	operator*(fp32 lhs,const Color& rhs)
		{
			Color ret;
			ret.r = rhs.r * lhs;
			ret.g = rhs.g * lhs;
			ret.b = rhs.b * lhs;
			ret.a = rhs.a * lhs;
			return ret;
		}

		/** @brief  */
		inline Color&		operator+=(const Color& rhs)
		{
			this->r += rhs.r;
			this->g += rhs.g;
			this->b += rhs.b;
			this->a += rhs.a;
			return *this;
		}

		/** @brief  */
		inline Color&		operator-=(const Color& rhs)
		{
			this->r -= rhs.r;
			this->g -= rhs.g;
			this->b -= rhs.b;
			this->a -= rhs.a;
			return *this;
		}

		/** @brief  */
		inline Color&		operator*=(fp32 rhs)
		{
			this->r *= rhs;
			this->g *= rhs;
			this->b *= rhs;
			this->a *= rhs;
			return *this;
		}

		/** @brief  */
		inline Color&		operator/=(fp32 rhs)
		{
			this->r /= rhs;
			this->g /= rhs;
			this->b /= rhs;
			this->a /= rhs;
			return *this;
		}

		/** @brief  */
		inline Color&		operator+=(fp32 rhs)
		{
			this->r += rhs;
			this->g += rhs;
			this->b += rhs;
			this->a += rhs;
			return *this;
		}

		/** @brief  */
		inline Color&		operator-=(fp32 rhs)
		{
			this->r -= rhs;
			this->g -= rhs;
			this->b -= rhs;
			this->a -= rhs;
			return *this;
		}

		/** @brief  */
		inline void			setComponent8U(uint8 r8, uint8 g8, uint8 b8, uint8 a8 = 0xFF)
		{
			float baseConversion = 1.0f/255.0f;
			r = r8*baseConversion;
			g = g8*baseConversion;
			b = b8*baseConversion;
			a = a8*baseConversion;
		}

		/** @brief  */
		inline void			setComponent32F(fp32 r32f, fp32 g32f, fp32 b32f, fp32 a32f = 1.0f)
		{
			r = r32f;
			g = g32f;
			b = b32f;
			a = a32f;
		}

		/** @brief  */
		inline void			getComponent8U(uint8& r8, uint8& g8, uint8& b8, uint8& a8) const
		{
			float baseConversion = 255.0f;
			r8 = uint8(r*baseConversion);
			g8 = uint8(g*baseConversion);
			b8 = uint8(b*baseConversion);
			a8 = uint8(a*baseConversion);
		}

		/** @brief  */
		inline void			getComponent32F(fp32& r32f, fp32& g32f, fp32& b32f, fp32& a32f) const
		{
			r32f = r;
			g32f = g;
			b32f = b;
			a32f = a;
		}

		/** @brief  */
		bool				operator==(const Color& rhs) const
		{
			return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
		}

		/** @brief  */
		bool				operator!=(const Color& rhs) const
		{
			return !(*this == rhs);
		}

		/** @brief  */
		BLADE_BASE_API void				saturate();

		/** @brief  */
		BLADE_BASE_API Color			getSaturateColor() const;

		/** @brief  */
		BLADE_BASE_API COLOR			getRGBA() const;

		/** @brief  */
		BLADE_BASE_API void				setRGBA(COLOR rgba);

		/** @brief  */
		BLADE_BASE_API COLOR			getARGB() const;

		/** @brief  */
		BLADE_BASE_API void				setARGB(COLOR argb);

		/** @brief  */
		BLADE_BASE_API COLOR			getBGRA() const;

		/** @brief  */
		BLADE_BASE_API void				setBGRA(COLOR bgra);

		/** @brief  */
		BLADE_BASE_API COLOR			getABGR() const;

		/** @brief  */
		BLADE_BASE_API void				setABGR(COLOR abgr);

		//add class keyword to avoid weird symbol conflicts
		BLADE_BASE_API static const class Blade::Color WHITE;
		BLADE_BASE_API static const class Blade::Color BLACK;
		BLADE_BASE_API static const class Blade::Color GRAY;
		BLADE_BASE_API static const class Blade::Color RED;
		BLADE_BASE_API static const class Blade::Color GREEN;
		BLADE_BASE_API static const class Blade::Color BLUE;
		BLADE_BASE_API static const class Blade::Color BLACK_ALPHA;

		BLADE_BASE_API static COLOR	ARGB2RGBA(COLOR argb);
		BLADE_BASE_API static COLOR	ABGR2RGBA(COLOR abgr);
		BLADE_BASE_API static COLOR	BGRA2RGBA(COLOR bgra);

		BLADE_BASE_API static COLOR	RGBA2ARGB(COLOR rgba);
		BLADE_BASE_API static COLOR	ABGR2ARGB(COLOR abgr);
		BLADE_BASE_API static COLOR	BGRA2ARGB(COLOR bgra);

		BLADE_BASE_API static COLOR	RGBA2ABGR(COLOR rgba);
		BLADE_BASE_API static COLOR	ARGB2ABGR(COLOR argb);
		BLADE_BASE_API static COLOR	BGRA2ABGR(COLOR bgra);

		BLADE_BASE_API static COLOR	RGBA2BGRA(COLOR rgba);
		BLADE_BASE_API static COLOR	ARGB2BGRA(COLOR argb);
		BLADE_BASE_API static COLOR	ABGR2BGRA(COLOR abgr);

		union
		{
			struct
			{
				fp32	r,g,b,a;
			};
			fp32		v[4];
		};
		
	};//class Color
	
}//namespace Blade


#endif //__Blade_Color_h__