/********************************************************************
	created:	2013/03/23
	filename: 	IModelResource.h
	author:		Crazii
	purpose:	interface exposed for importer/exporter
*********************************************************************/
#ifndef __Blade_IModelResource_h__
#define __Blade_IModelResource_h__
#include <utility/Version.h>
#include <utility/StringList.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/ITexture.h>
#include <interface/public/graphics/Color.h>
#include <interface/public/graphics/RenderProperty.h>
#include <interface/public/graphics/IVertexDeclaration.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <interface/public/IResource.h>
#include <math/Half3.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Quaternion.h>
#include <math/AxisAlignedBox.h>
#include <BladeModel.h>
#include <interface/IModelConfigManager.h>

namespace Blade
{
	class IVertexBuffer;
	class IIndexBuffer;

	//fwd decl
	class TextureState;
	typedef Handle<TextureState> HTEXTURESTATE;

	class IModelResource : public IResource
	{
	public:
		static const BLADE_MODEL_API Version MODEL_LATEST_SERIALIZER_VERSION;

	public:
		enum ENormalType
		{
			NT_NONE,
			NT_NORMAL_VECTOR,
			NT_TANGENT_FRAME,
		};

		//channel usage
		typedef enum EMapChannel
		{
			//note: the order should not be changed, this order is used in the shader
			MC_DIFFUSE,
			MC_NORMAL,			//normal map
			MC_SPECULAR_LEVEL,	//specular level which controls shininess
			MC_SPECULAR_GLOSS,	//specular gloss which controls range(power)
			MC_SPECULAR,		//specular color
			MC_GLOW,			//emissive/self illumination

			MC_COUNT,
			MC_BEGIN = MC_DIFFUSE,
		}CHANNEL;

		//channel names: the sampler uniform name used in shader
		static const BLADE_MODEL_API TString MAP_CHANNEL_NAMES[MC_COUNT];

		static const EVertexElementType POSITION_FMT = VET_HALF4;
		static const EVertexElementType UV_FMT = VET_HALF2;
		static const EVertexElementType NORMAL_FMT = VET_UBYTE4N;

		typedef struct SPosition
		{
			fp16 x,y,z,w;
		}POSITION;
		static_assert(sizeof(POSITION) == 8, "size error");

		typedef struct STextureCoordinates
		{
#if 1
			fp16 u,v;
			inline scalar getU() const { return u.getFloat(); }
			inline scalar getV() const { return v.getFloat(); }
#else
			fp32 u, v;
			inline scalar getU() const { return u; }
			inline scalar getV() const { return v; }
#endif
		}UV;

		//used to store compressed tangent frame
		typedef union UQuaternionU8x4
		{
			struct
			{
				uint8	x,y,z,w;
			};
			uint32	quat;
			Vector4::PACKED packed;

			inline Vector3 getNormal() const
			{
				Vector4 v = Vector4::unpack(packed);
				return Vector3::UNIT_Z * reinterpret_cast<const Quaternion&>(v);
			}
		}TANGENT_FRAME;

		//used to store compressed normal data
		typedef union UNormalU8x4
		{
			struct
			{
				uint8	x,y,z,w;
			};
			uint32	normal;
			Vector4::PACKED packed;

			inline Vector3 getNormal() const
			{
				return Vector4::unpack(packed);
			}
		}NORMAL;

		static_assert(sizeof(TANGENT_FRAME) == sizeof(NORMAL), "size/type mismatch.");	//TODO: use unique type for both

		static const int MAX_BONE_COUNT = 4;
		//bone assignment: bone index & bone weight
		typedef union UBoneWeight
		{
			uint32	value;
			uint8	weight[MAX_BONE_COUNT]; //VET_UBYTE4N
		}BONE_WEIGHT;

		typedef union UBoneIndice
		{
			uint32	value;
			uint8	index[MAX_BONE_COUNT];	//VET_UBYTE4 0-255
		}BONE_INDICE;

		//model material
		//share it in material instance to save memory
		typedef struct SMaterialInfo
		{
			Color		ambient;		//deprecated in engine, but maybe needed by others?
			Color		diffuse;
			Color		specular;
			Color		emissive;
			TStringList	textures;
			int8		channelMap[IModelResource::MC_COUNT];	//channel to index map
			bool		hasTransparency;

			SMaterialInfo()
			{
				for(int i = 0; i < IModelResource::MC_COUNT; ++i)
					channelMap[i] = -1;
				hasTransparency = false;
			}
			
			//runtime data only (NO serialization)
			//use full path for texture names, used on runtime to load imported model data with external texture linkage.
			//if this flag is set, serializer will write the full absolute path of the external texture.
			bool		textureFullPath;
		}MATERIAL_INFO;

		typedef struct SMeshMaterial
		{
			HRENDERPROPERTY	color;
			HTEXTURESTATE	texture;
			bool			hasTransparency;
		}MATERIAL;
		static const size_t MAX_ANIMATION_FILE_BASE = 192;

		typedef struct SBoneBounding : public ModelAllocatable<SBoneBounding>
		{
			Half3 min;
			Half3 max;
		}BONE_BOUNDING;

		typedef struct SMeshData
		{
			TString		mName;		///mesh name
			AABB		mAAB;		///AAB bounding
			scalar		mRadius;	///sphere bounding
			TStringList	mActiveBones;	///all bones used, including parent
			uint32*		mBones;		///bones directly used for animation, indices to mActiveBones
									///note: this data is duplicated when calling addSubMesh(),
									///so as input parameter this pointer can be deleted or be temporary
			BONE_BOUNDING*	mBoneBoundings;	///created if model has skeleton & has multiple sub meshes
											///in size of bone count, this data will be duplicated when calling addSubMesh
			uint32		mBoneCount;	///count of bones directly used for animation

			//soft/hardware graphics buffer
			HVBUFFER	mPosition;
			HVBUFFER	mNormalBuffer;	///either normal vectors, or tangent frame in quaternions
			HVBUFFER	mBoneIndices;	///bone indices
			HVBUFFER	mBoneWeights;	///bone weights
			HVBUFFER	mTexcoord[ITexture::MAX_TEXCOORD_COUNT];
			HIBUFFER	mIndices;
			HVERTEXSOURCE mVertexSource;
			GraphicsGeometry mGeometry;
			uint8		mTexcoordCount;	///currently only one set supported in shader
			uint8		mMaterialID;	///material reference id
			uint8		mPreTransformed : 1;///runtime flag: vertex position pre transformed to world space
		}MESHDATA;

	public:
		BLADE_MODEL_API virtual ~IModelResource()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		setNormalType(ENormalType type) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ENormalType	getNormalType() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getSubMeshCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const MESHDATA&	getSubMeshData(index_t index) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual index_t		findSubMeshData(const TString& name) const = 0;

		/**
		@describe 
		@param 
		@return the index of new added sub mesh
		*/
		virtual index_t		addSubMesh(const MESHDATA& meshData) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t		getMaterialCount() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual index_t		addMaterial(const MATERIAL_INFO& materialinfo) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		setSkeletonRefernce(const TString& skeletonName, bool useFullPath = false) = 0;

		/************************************************************************/
		/*  run-time function (NOT available on exporting/importing)                                                                    */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual bool		setSkeleton(const HRESOURCE& skeletonResource) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const MATERIAL&			getMaterial(index_t index) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const AABB&	getModelAABB() const = 0;

	};//class IModelResource
	
}//namespace Blade

#endif //  __Blade_IModelResource_h__