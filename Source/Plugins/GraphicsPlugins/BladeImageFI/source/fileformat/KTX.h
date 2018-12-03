/********************************************************************
	created:	2015/01/28
	filename: 	KTX.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_KTX_h__
#define __Blade_KTX_h__
#include <interface/public/graphics/PixelFormat.h>
#include <utility/IOBuffer.h>

#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }
#define KTX_ENDIAN_REF      (0x04030201)
#define KTX_ENDIAN_REF_REV  (0x01020304)
#define KTX_HEADER_SIZE		(64)

#define KTX_BLOCK_MARK BLADE_FCC('B','L','B','M')
#define KTX_GL_UNPACK_ALIGNMENT 4
#define KTX_GL_UNPACK_ALIGNMENT_MASK (KTX_GL_UNPACK_ALIGNMENT-1)
#define KTX_GL_UNPACK_ALIGNMENT_PITCH(_bytes) (((_bytes) + KTX_GL_UNPACK_ALIGNMENT-1)&(~KTX_GL_UNPACK_ALIGNMENT_MASK))
#define KTX_GL_UNPACK_ALIGNMENT_PADDING(_bytes) ((KTX_GL_UNPACK_ALIGNMENT - ((_bytes)&KTX_GL_UNPACK_ALIGNMENT_MASK) )%KTX_GL_UNPACK_ALIGNMENT)

#define KTX_OPENGL		0
#define KTX_OPENGL_ES	1

#ifndef SUPPORT_LEGACY_FORMAT_CONVERSION
#if KTX_OPENGL
#define SUPPORT_LEGACY_FORMAT_CONVERSION 1
#elif KTX_OPENGL_ES
/* ES1, ES2 & ES3 support the legacy formats */
#define SUPPORT_LEGACY_FORMAT_CONVERSION 0
#endif
#endif

/*
 * These defines are needed to compile the KTX library. When
 * these things are not available in the GL header in use at
 * compile time, the library provides its own support, handles
 * the expected run-time errors or just needs the token value.
 */
#ifndef GL_LUMINANCE
	#define GL_ALPHA						0x1906
	#define GL_LUMINANCE					0x1909
	#define GL_LUMINANCE_ALPHA				0x190A
#endif

#ifndef GL_INTENSITY
	#define GL_INTENSITY					0x8049
#endif

#if SUPPORT_LEGACY_FORMAT_CONVERSION
/* For loading legacy KTX files. */
	#ifndef GL_LUMINANCE4
		#define GL_ALPHA4						0x803B
		#define GL_ALPHA8						0x803C
		#define GL_ALPHA12						0x803D
		#define GL_ALPHA16						0x803E
		#define GL_LUMINANCE4					0x803F
		#define GL_LUMINANCE8					0x8040
		#define GL_LUMINANCE12					0x8041
		#define GL_LUMINANCE16					0x8042
		#define GL_LUMINANCE4_ALPHA4			0x8043
		#define GL_LUMINANCE6_ALPHA2			0x8044
		#define GL_LUMINANCE8_ALPHA8			0x8045
		#define GL_LUMINANCE12_ALPHA4			0x8046
		#define GL_LUMINANCE12_ALPHA12			0x8047
		#define GL_LUMINANCE16_ALPHA16			0x8048
	#endif
	#ifndef GL_INTENSITY4
		#define GL_INTENSITY4					0x804A
		#define GL_INTENSITY8					0x804B
		#define GL_INTENSITY12					0x804C
		#define GL_INTENSITY16					0x804D
	#endif
	#ifndef GL_SLUMINANCE
		#define GL_SLUMINANCE_ALPHA				0x8C44
		#define GL_SLUMINANCE8_ALPHA8			0x8C45
		#define GL_SLUMINANCE					0x8C46
		#define GL_SLUMINANCE8					0x8C47
	#endif
#endif /* SUPPORT_LEGACY_FORMAT_CONVERSION */

#define GL_TEXTURE_1D                   0x0DE0
#define GL_TEXTURE_2D                   0x0DE1
#define GL_TEXTURE_3D                   0x806F
#define GL_TEXTURE_CUBE_MAP             0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X  0x8515
/* from GL_EXT_texture_array */
#define GL_TEXTURE_1D_ARRAY_EXT         0x8C18
#define GL_TEXTURE_2D_ARRAY_EXT         0x8C1A
#define GL_GENERATE_MIPMAP              0x8191

/* For writer.c */
#if !defined(GL_BGR)
#define GL_BGR							0x80E0
#define GL_BGRA							0x80E1
#endif
#if !defined(GL_RED_INTEGER)
#define GL_RED_INTEGER					0x8D94
#define GL_RGB_INTEGER					0x8D98
#define GL_RGBA_INTEGER					0x8D99
#endif
#if !defined(GL_GREEN_INTEGER)
#define GL_GREEN_INTEGER				0x8D95
#define GL_BLUE_INTEGER					0x8D96
#define GL_ALPHA_INTEGER				0x8D97
#endif
#if !defined (GL_BGR_INTEGER)
#define GL_BGR_INTEGER					0x8D9A
#define GL_BGRA_INTEGER					0x8D9B
#endif
#if !defined(GL_INT)
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#endif
#if !defined(GL_HALF_FLOAT)
typedef unsigned short GLhalf;
#define GL_HALF_FLOAT					0x140B
#endif
#if !defined(GL_UNSIGNED_BYTE_3_3_2)
#define GL_UNSIGNED_BYTE_3_3_2			0x8032
#define GL_UNSIGNED_INT_8_8_8_8			0x8035
#define GL_UNSIGNED_INT_10_10_10_2		0x8036
#endif
#if !defined(GL_UNSIGNED_BYTE_2_3_3_REV)
#define GL_UNSIGNED_BYTE_2_3_3_REV		0x8362
#define GL_UNSIGNED_SHORT_5_6_5			0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV		0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV	0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV	0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV		0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV	0x8368
#endif
#if !defined(GL_UNSIGNED_INT_24_8)
#define GL_DEPTH_STENCIL				0x84F9
#define GL_UNSIGNED_INT_24_8			0x84FA
#endif
#if !defined(GL_UNSIGNED_INT_5_9_9_9_REV)
#define GL_UNSIGNED_INT_5_9_9_9_REV		0x8C3E
#endif
#if !defined(GL_UNSIGNED_INT_10F_11F_11F_REV)
#define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#endif
#if !defined (GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV	0x8DAD
#endif

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES				0x8D64
#endif

#ifndef GL_COMPRESSED_R11_EAC
#define GL_COMPRESSED_R11_EAC                            0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
#define GL_COMPRESSED_RG11_EAC                           0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
#define GL_COMPRESSED_RGB8_ETC2                          0x9274
#define GL_COMPRESSED_SRGB8_ETC2                         0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC				 0x9279
#endif
#ifndef GL_R16_SNORM
#define GL_R16_SNORM					0x8F98
#define GL_RG16_SNORM					0x8F99
#endif
#ifndef GL_RED
#define GL_RED							0x1903
#define GL_GREEN						0x1904
#define GL_BLUE							0x1905
#define GL_RG							0x8227
#define GL_RG_INTEGER					0x8228
#endif
#ifndef GL_R16
#define GL_R16							0x822A
#define GL_RG16							0x822C
#endif
#ifndef GL_RGB8
#define GL_RGB8                         0x8051
#define GL_RGBA8                        0x8058
#endif
#ifndef GL_SRGB8
#define GL_SRGB8						0x8C41
#define GL_SRGB8_ALPHA8					0x8C43
#endif

#ifndef GL_MAJOR_VERSION
#define GL_MAJOR_VERSION                0x821B
#define GL_MINOR_VERSION                0x821C
#endif

#ifndef GL_CONTEXT_PROFILE_MASK
#define GL_CONTEXT_PROFILE_MASK				 0x9126
#define GL_CONTEXT_CORE_PROFILE_BIT			 0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

#define GL_RG                                            0x8227
#define GL_RG_INTEGER                                    0x8228
#define GL_R8                                            0x8229
#define GL_RG8                                           0x822B
#define GL_R16F                                          0x822D
#define GL_R32F                                          0x822E
#define GL_RG16F                                         0x822F
#define GL_RG32F                                         0x8230
#define GL_R8I                                           0x8231
#define GL_R8UI                                          0x8232
#define GL_R16I                                          0x8233
#define GL_R16UI                                         0x8234
#define GL_R32I                                          0x8235
#define GL_R32UI                                         0x8236
#define GL_RG8I                                          0x8237
#define GL_RG8UI                                         0x8238
#define GL_RG16I                                         0x8239
#define GL_RG16UI                                        0x823A
#define GL_RG32I                                         0x823B
#define GL_RG32UI                                        0x823C

#define GL_COLOR                                         0x1800
#define GL_DEPTH                                         0x1801
#define GL_STENCIL                                       0x1802
#define GL_RED                                           0x1903
#define GL_RGB8                                          0x8051
#define GL_RGBA8                                         0x8058
#define GL_RGB10_A2                                      0x8059

#define GL_RGBA4                                         0x8056
#define GL_RGB5_A1                                       0x8057
#define GL_RGB565                                        0x8D62
#define GL_DEPTH_COMPONENT16                             0x81A5
#define GL_STENCIL_INDEX8                                0x8D48

#define GL_DEPTH_COMPONENT                               0x1902
#define GL_ALPHA                                         0x1906
#define GL_RGB                                           0x1907
#define GL_RGBA                                          0x1908
#define GL_LUMINANCE                                     0x1909
#define GL_LUMINANCE_ALPHA                               0x190A

#define GL_UNSIGNED_SHORT_4_4_4_4                        0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1                        0x8034
#define GL_UNSIGNED_SHORT_5_6_5                          0x8363

#define GL_RGBA32UI                                      0x8D70
#define GL_RGB32UI                                       0x8D71
#define GL_RGBA16UI                                      0x8D76
#define GL_RGB16UI                                       0x8D77
#define GL_RGBA8UI                                       0x8D7C
#define GL_RGB8UI                                        0x8D7D
#define GL_RGBA32I                                       0x8D82
#define GL_RGB32I                                        0x8D83
#define GL_RGBA16I                                       0x8D88
#define GL_RGB16I                                        0x8D89
#define GL_RGBA8I                                        0x8D8E
#define GL_RGB8I                                         0x8D8F
#define GL_RED_INTEGER                                   0x8D94
#define GL_RGB_INTEGER                                   0x8D98
#define GL_RGBA_INTEGER                                  0x8D99

#define GL_RGBA32F                                       0x8814
#define GL_RGB32F                                        0x8815
#define GL_RGBA16F                                       0x881A
#define GL_RGB16F                                        0x881B

#define GL_BYTE                                          0x1400
#define GL_UNSIGNED_BYTE                                 0x1401
#define GL_SHORT                                         0x1402
#define GL_UNSIGNED_SHORT                                0x1403
#define GL_INT                                           0x1404
#define GL_UNSIGNED_INT                                  0x1405
#define GL_FLOAT                                         0x1406
#define GL_FIXED                                         0x140C

#define GL_DEPTH_STENCIL_ATTACHMENT                      0x821A
#define GL_DEPTH_STENCIL                                 0x84F9
#define GL_UNSIGNED_INT_24_8                             0x84FA
#define GL_DEPTH24_STENCIL8                              0x88F0

#define GL_DEPTH_COMPONENT32F                            0x8CAC
#define GL_DEPTH32F_STENCIL8                             0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV                0x8DAD


namespace Blade
{
	/* OpenGL ES 2.0 */

	typedef void             GLvoid;
	typedef char             GLchar;
	typedef unsigned int     GLenum;
	typedef unsigned char    GLboolean;
	typedef unsigned int     GLbitfield;
	typedef int8             GLbyte;
	typedef short            GLshort;
	typedef int              GLint;
	typedef int              GLsizei;
	typedef uint8            GLubyte;
	typedef unsigned short   GLushort;
	typedef unsigned int     GLuint;
	typedef float            GLfloat;
	typedef float            GLclampf;
	typedef int32            GLfixed;
	typedef intptr_t         GLintptr;
	typedef size_t           GLsizeiptr;

	/* OpenGL ES 3.0 */

	typedef unsigned short   GLhalf;
	typedef int64            GLint64;
	typedef uint64           GLuint64;

	namespace KTXUtil
	{

		typedef struct KTX_header_t {
			uint8  identifier[12];
			uint32 endianness;
			uint32 glType;
			uint32 glTypeSize;
			uint32 glFormat;
			uint32 glInternalFormat;
			uint32 glBaseInternalFormat;
			uint32 pixelWidth;
			uint32 pixelHeight;
			uint32 pixelDepth;
			uint32 numberOfArrayElements;
			uint32 numberOfFaces;
			uint32 numberOfMipmapLevels;
			uint32 bytesOfKeyValueData;
		} KTX_header;
		static_assert(sizeof(KTX_header) == KTX_HEADER_SIZE, "size/alignment error");


		typedef enum KTX_error_code_t {
			KTX_SUCCESS = 0,		 /*!< Operation was successful. */
			KTX_FILE_OPEN_FAILED,	 /*!< The target file could not be opened. */
			KTX_FILE_WRITE_ERROR,    /*!< An error occurred while writing to the file. */
			KTX_GL_ERROR,            /*!< GL operations resulted in an error. */
			KTX_INVALID_OPERATION,   /*!< The operation is not allowed in the current state. */
			KTX_INVALID_VALUE,	     /*!< A parameter value was not valid */
			KTX_NOT_FOUND,			 /*!< Requested key was not found */
			KTX_OUT_OF_MEMORY,       /*!< Not enough memory to complete the operation. */
			KTX_UNEXPECTED_END_OF_FILE, /*!< The file did not contain enough data */
			KTX_UNKNOWN_FILE_FORMAT, /*!< The file not a KTX file */
			KTX_UNSUPPORTEDEXTUREYPE, /*!< The KTX file specifies an unsupported texture type. */
		} KTX_error_code;

		/**
		 * @brief structure used to pass information about the texture to ktxWriteKTX
		 */
		typedef struct KTXexture_info_t
		{
			/**
			 * @brief The type of the image data.
			 *
			 * Values are the same as in the @p type parameter of
			 * glTexImage*D. Must be 0 for compressed images.
			 */
			uint32 glType;
			/**
			 * @brief The data type size to be used in case of endianness
			 *        conversion.
			 *
			 * This value is used in the event conversion is required when the
			 * KTX file is loaded. It should be the size in bytes corresponding
			 * to glType. Must be 1 for compressed images.
			 */
			uint32 glTypeSize;
			/**
			 * @brief The format of the image(s).
			 *
			 * Values are the same as in the format parameter
			 * of glTexImage*D. Must be 0 for compressed images.
			 */
			uint32 glFormat;
			/** @brief The internalformat of the image(s).
			 *
			 * Values are the same as for the internalformat parameter of
			 * glTexImage*2D. Note: it will not be used when a KTX file
			 * containing an uncompressed texture is loaded into OpenGL ES.
			 */
			uint32 glInternalFormat;
			/** @brief The base internalformat of the image(s)
			 *
			 * For non-compressed textures, should be the same as glFormat.
			 * For compressed textures specifies the base internal, e.g.
			 * GL_RGB, GL_RGBA.
			 */                        
			uint32 glBaseInternalFormat;
			/** @brief Width of the image for texture level 0, in pixels. */
			uint32 pixelWidth;
			/** @brief Height of the texture image for level 0, in pixels.
			 *
			 * Must be 0 for 1D textures.
			 */
 			uint32 pixelHeight;
			/** @brief Depth of the texture image for level 0, in pixels.
			 *
			 * Must be 0 for 1D, 2D and cube textures.
			 */
			uint32 pixelDepth;
			/** @brief The number of array elements.
			 *
			 * Must be 0 if not an array texture.
			 */
			uint32 numberOfArrayElements;
			/** @brief The number of cubemap faces.
			 *
			 * Must be 6 for cubemaps and cubemap arrays, 1 otherwise. Cubemap
			 * faces must be provided in the order: +X, -X, +Y, -Y, +Z, -Z.
			 */
			uint32 numberOfFaces;
			/** @brief The number of mipmap levels.
			 *
			 * 1 for non-mipmapped texture. 0 indicates that a full mipmap pyramid should
			 * be generated from level 0 at load time (this is usually not allowed for
			 * compressed formats). Mipmaps must be provided in order from largest size to
			 * smallest size. The first mipmap level is always level 0.
			 */
			uint32 numberOfMipmapLevels;
		} KTXexture_info;

		/** @brief  */
		void swapEndian16(uint16* pData16, int count);

		/** @brief  */
		void swapEndian32(uint32* pData32, int count);

		/** @brief  */
		PixelFormat		getPixelFormat(const KTX_header& header);

		/** @brief  */
		bool			setPixelFormat(PixelFormat format, KTX_header& header);

		/** @brief  */
		KTX_error_code	loadTextureKTX(IOBuffer& buffer, KTX_header& header, IOBuffer& outputContent);

		/** @brief  */
		KTX_error_code	saveTextureKTX(IOBuffer& buffer, const KTX_header& header, const uint8* content, size_t contenSize);

	}//namespace KTXUtil
	
}//namespace Blade


#endif // __Blade_KTX_h__