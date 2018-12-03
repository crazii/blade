/********************************************************************
	created:	2015/09/07
	filename: 	ModelBuilder.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelBuilder_h__
#define __Blade_ModelBuilder_h__
#include <interface/IModelBuilder.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BoneInfo
	{
	public:
		float	weight;
		int		index;	//temp index by IGameSkin, not final/real index
		inline BoneInfo() :weight(0),index(-1)		{}
		//large weight comes first
		inline bool operator<(const BoneInfo& rhs) const	{return weight > rhs.weight;}
	};
	typedef TempVector<BoneInfo> BoneInfoCache;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class SubmeshBoneSet
	{
	public:
		typedef TempMap<TString, uint32>	BoneIdMap;
		typedef TempVector<TString>			BoneList;

		/** @brief  */
		void	reserve(size_t n)
		{
			mBones.reserve(n);
		}

		/** @brief  */
		size_t	size() const
		{
			assert( mBones.size() == mBoneMap.size() );
			return mBones.size();
		}

		/** @brief  */
		inline const TString*	getBoneNames()
		{
			return mBones.size() > 0 ? &mBones[0] : NULL;
		}

		/** @brief  */
		uint32	addBone(const TString& boneName)
		{
			BoneIdMap::iterator i = mBoneMap.find( boneName );
			if( i == mBoneMap.end() )
			{
				mBones.push_back(boneName);
				uint32 index = (uint32)mBones.size()-1;
				mBoneMap[boneName] = index;
				return index;
			}
			else
				return i->second;
		}
		BoneIdMap	mBoneMap;	//original bone index (in skeleton) to sub mesh index map
		BoneList	mBones;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ModelIndex
	{
	public:
		ModelIndex() :mIndex(INVALID_INDEX)	{mIndices.reserve(8);}
		inline bool	isValid() const				{return mIndex != INVALID_INDEX;}
		inline operator size_t() const			{return mIndex;}
		inline void operator=(size_t val)
		{
			if( mIndex == INVALID_INDEX ) 
			{
				assert( mIndices.size() == 0);
				mIndices.push_back(val);
			}
			else
			{
				assert( mIndices.size() != 0 && mIndices[0] == mIndex );
				mIndices[0] = val;
			}
			mIndex = val; 
		}

		inline void add(size_t val)				{assert( mIndices.size() != 0 && mIndices[0] == mIndex && mIndex != INVALID_INDEX); mIndices.push_back(val);}
		size_t mIndex;
		typedef TempVector<size_t> MultiIndex;
		MultiIndex mIndices;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//uv mirror info
	enum EUVMirror
	{
		UVM_UNDECIDED = 0,
		UVM_UNMIRRORED = 1,
		UVM_MIRRORED = 2,
		UVM_COUNT
	};
	class ModelVertex
	{
	public:
		ModelVertex(const Vector3& v)
		{
			position = v;
		}


		Vector3			position;
		Vector2			uv[ITexture::MAX_TEXCOORD_COUNT];
		uint32			bones[IModelResource::MAX_BONE_COUNT];
		scalar			boneWeights[IModelResource::MAX_BONE_COUNT];
		//there's no normals because vertex should have unique normal
		const Vector3*	normal;	//shared normal
		uint32			index;
		uint32			normalChannel;
		Vector3			tangent;
		Vector3			bitangent;
		EUVMirror		mirror;

		//sort/split vertex by position & uv, split normal
		inline bool operator<(const ModelVertex& rhs) const
		{
			const ModelVertex& lhs = *this;
			return (lhs.normal < rhs.normal)
				|| (lhs.normal == rhs.normal && lhs.position.x < rhs.position.x)
				|| (lhs.normal == rhs.normal && lhs.position.x == rhs.position.x && lhs.position.y < rhs.position.y)
				|| (lhs.normal == rhs.normal && lhs.position.x == rhs.position.x && lhs.position.y == rhs.position.y && lhs.position.z < rhs.position.z)

				|| (lhs.normal == rhs.normal && lhs.position.x == rhs.position.x && lhs.position.y == rhs.position.y && lhs.position.z == rhs.position.z
				&& lhs.uv[lhs.normalChannel].x < rhs.uv[rhs.normalChannel].x)

				|| (lhs.normal == rhs.normal && lhs.position.x == rhs.position.x && lhs.position.y == rhs.position.y && lhs.position.z == rhs.position.z
				&& lhs.uv[lhs.normalChannel].x == rhs.uv[rhs.normalChannel].x && lhs.uv[lhs.normalChannel].y < rhs.uv[rhs.normalChannel].y);
		}
	};
	typedef TempMap<ModelVertex, ModelIndex> MirrorIndexMap;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	struct VertexNormal
	{

		VertexNormal()
		{
			mOriginalNormal = mNormal = Vector3::ZERO;
			mVertexID = INVALID_INDEX;
		}
		VertexNormal(const Vector3& v, index_t vertexID)
		{
			mNormal = mOriginalNormal = v;
			mOriginalNormal.normalize();
			mVertexID = vertexID;
		}
		VertexNormal& operator+=(const Vector3& rhs)
		{
			mNormal += rhs;
			return *this;
		}

		Vector3 mNormal;
		Vector3 mOriginalNormal;
		index_t mVertexID;
	};

	//normals are not related to UV, merge/split vertex normal by position only
	//vertices with the same position will have the same weighted normal
	//note: normals should be stored globally, sub meshes with the same vertex should have the same normal
	struct FnVertexNormalLess
	{
		bool operator()(const ModelVertex& lhs, const ModelVertex& rhs) const
		{
			return (lhs.position.x < rhs.position.x)
				|| (lhs.position.x == rhs.position.x && lhs.position.y < rhs.position.y)
				|| (lhs.position.x == rhs.position.x && lhs.position.y == rhs.position.y && lhs.position.z < rhs.position.z);
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class VertexNormalList
	{
		typedef List<VertexNormal> NormalList;
	public:
		//add new normal, return whether normal merged to existing
		const Vector3* addNormal(const Vector3& inputNormal, scalar weight, index_t vertexID, bool& merged)
		{
			merged = false;
			const Vector3* dest = NULL;

			Vector3 weightedNormal = inputNormal;

			if (weight > Math::HIGH_EPSILON)
			{
				weightedNormal *= weight;

				for (NormalList::iterator i = mNormals.begin(); i != mNormals.end(); ++i)
				{
					VertexNormal& n = *i;
					const scalar mergeThreshold = 0.5;	//cos 60
														//const scalar splitThreshold = 0.0871557;	//cos 85
					const scalar splitThreshold = 0.087156; //cos 85

															//merge normal for all vertices if angles < mergeAngle
															//split vertices for same vertex if angle > splitAngle (different normal address will make it split)
					scalar fCos = n.mOriginalNormal.dotProduct(weightedNormal.getNormalizedVector());
					scalar fCosMerged = n.mNormal.getNormalizedVector().dotProduct(weightedNormal.getNormalizedVector());
					if (fCos > mergeThreshold || (fCosMerged > mergeThreshold)
						|| (n.mVertexID == vertexID && (fCos > splitThreshold || fCosMerged > splitThreshold)))
					{
						n += weightedNormal;
						dest = &n.mNormal;
						merged = true;
						break;
					}
				}
			}

			if( dest == NULL )
			{
				assert(merged == false);
				mNormals.push_back(VertexNormal(weightedNormal, vertexID) );
				dest = &mNormals.back().mNormal;
			}
			else
				assert(merged == true);
			return dest;
		}

	protected:
		NormalList			mNormals;
	};
	typedef TempMap<ModelVertex, VertexNormalList, FnVertexNormalLess> VertexNormalMap;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	struct FnVertexIndexLess
	{
	public:
		//vertex index identical by UV
		//same vertex position with different uv,normal will be duplicated
		bool operator()(const ModelVertex& lhs, const ModelVertex& rhs) const
		{
			return lhs.index < rhs.index
				|| (lhs.index == rhs.index && lhs.normal < rhs.normal)
				|| (lhs.index == rhs.index && lhs.normal == rhs.normal  && lhs.uv[lhs.normalChannel].x < rhs.uv[rhs.normalChannel].x)
				|| (lhs.index == rhs.index && lhs.normal == rhs.normal  && lhs.uv[lhs.normalChannel].x == rhs.uv[rhs.normalChannel].x && lhs.uv[lhs.normalChannel].y < rhs.uv[rhs.normalChannel].y);
		}
	};
	typedef TempMap<ModelVertex, ModelIndex, FnVertexIndexLess> VertexIndexMap;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ModelTriangle
	{
	public:
		Vector3		normal;		//normalized normal
		Vector3		tangent;	//weighted tangent
		Vector3		bitangent;	//weighted bitangent
		ModelIndex	originalIndex[3];
		scalar		weight;
		EUVMirror	mirror;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	typedef TempVector<Vector3>			NormalList;					//shared normals
	typedef TempVector<ModelVertex>		VertexList;					//vertices
	typedef TempVector<uint32>			IndexList;					//triangles
	typedef TempVector<ModelTriangle>	TriangleList;
	typedef TempVector<AABB>	BoneBoudning;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class SubMeshData
	{
	public:
		TString			name;
		TriangleList	triangles;
		VertexList		vertices;
		IndexList		indices;
		SubmeshBoneSet	activeBoneSet;
		SubmeshBoneSet	usedBoneSet;
		BoneBoudning	boneBoundings;
		////separate 3ds-max mesh triangles into 2 group according to reflection
		VertexIndexMap	indexMap;
		MirrorIndexMap	mirrorIndexMap[UVM_COUNT];
		uint8			uvCount;	//number of uv sets
		bool			hasSkin;

		SubMeshData()
		{
			uvCount = 0;
			hasSkin = false;
		}
	};
	typedef TempVector<SubMeshData>	MeshList;	//per material mesh list
	typedef TempVector<MeshList>	MeshBuildData;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ModelBuilder : public IModelBuilder, public TempAllocatable
	{
	public:
		ModelBuilder()
			:mNormalType(IModelResource::NT_TANGENT_FRAME)
			,mTSLeftHanded(true)
		{

		}
		~ModelBuilder()	{}

		/*
		@describe 
		@param [in] TSLeftHanded: tangent space handedness
		@param [in] animFrequency: animation sampling frequency
		@return
		*/
		virtual bool	initialize(IModelResource::ENormalType normalType, bool TSLeftHanded)
		{
			mNormalType = normalType;
			mTSLeftHanded = TSLeftHanded;
			return true;
		}

		/*
		@describe collect meshes & materials used by mesh
		@param
		@return
		*/
		virtual bool	collectMesh(IModelCollector* collector, ProgressNotifier& callback);

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	buildMesh(IModelResource* target);

	protected:

		/** @brief  */
		void			buildSubmesh(SubMeshData& subMesh, index_t materialID, IModelResource* target);

		typedef TempVector<IModelResource::MATERIAL_INFO>	MaterialList;
		typedef TempSet<TString> BoneNameSet;

		MaterialList	mMaterials;
		MeshBuildData	mMeshes;
		VertexNormalMap	mNormalList;

		IModelResource::ENormalType	mNormalType;
		bool			mTSLeftHanded;
	};//class ModelBuilder
	
}//namespace Blade

#endif // __Blade_ModelBuilder_h__


