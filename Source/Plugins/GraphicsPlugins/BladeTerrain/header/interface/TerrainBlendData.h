/********************************************************************
	created:	2011/10/22
	filename: 	TerrainBlendData.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainBlendData_h__
#define __Blade_TerrainBlendData_h__
#include <BladeTerrain.h>
#include <interface/ITerrainConfigManager.h>
#include <Pimpl.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class TerrainBlendData
	{
	public:
		inline TerrainBlendData()
			:mX(0),mZ(0),mSizeX(0),mSizeZ(0),mNeedUpdateTexture(false)				{}

		inline TerrainBlendData(size_t x,size_t z,size_t cx,size_t cz,const TString& texture,bool updateTexture)
			:mX(x),mZ(z),mSizeX(cx),mSizeZ(cz),mTexture(texture),mNeedUpdateTexture(updateTexture)
		{}

		inline ~TerrainBlendData()							{}

		/** @brief  */
		inline size_t	getX() const	{return mX;}

		/** @brief  */
		inline size_t	getZ() const	{return mZ;}

		/** @brief  */
		inline size_t	getSizeX() const	{return mSizeX;}

		/** @brief  */
		inline size_t	getSizeZ() const	{return mSizeZ;}

		/** @brief  */
		inline const TString&	getTexture() const	{return mTexture;}

		/** @brief  */
		inline bool		needUpdateTexture() const	{return mNeedUpdateTexture;}

	protected:
		size_t	mX;
		size_t	mZ;
		size_t	mSizeX;
		size_t	mSizeZ;
		TString	mTexture;
		bool	mNeedUpdateTexture;
	};//class TerrainBlendData

	//////////////////////////////////////////////////////////////////////////
	class SingleBlockBlendData : public TerrainBlendData
	{
	public:
		SingleBlockBlendData()
			:mBlockIndex(0,0)		{}

		SingleBlockBlendData(size_t x,size_t z,size_t cx,size_t cz,const TString& texture,bool updateTexture,BLOCK_INDEX blockIndex)
			:TerrainBlendData(x,z,cx,cz,texture,updateTexture),mBlockIndex(blockIndex)	{}

		/** @brief  */
		inline BLOCK_INDEX	getBlockIndex() const		{return mBlockIndex;}

	protected:
		BLOCK_INDEX		mBlockIndex;
	};//class SingleBlockBlendData

	class TerrainBlockBlendData : public Allocatable, public NonCopyable
	{
	public:
		typedef TList<SingleBlockBlendData, TempAllocator<SingleBlockBlendData> > BlockBlendDataList;
	public:
		inline TerrainBlockBlendData()
		{
			size_t blocks = ITerrainConfigManager::getInterface().getTerrainInfo().mBlocksPerTileSide;
			mList.reserve( blocks*blocks );
		}
		inline ~TerrainBlockBlendData()
		{

		}

		/** @brief  */
		size_t		getCount() const
		{
			return mList.size();
		}

		/** @brief
		   @remark index ranges [0, getCount() )
		*/
		const SingleBlockBlendData&	getData(index_t index) const
		{
			return mList.at(index);
		}

		/** @brief  */
		void		add(const SingleBlockBlendData& singleData)
		{
			return mList.push_back(singleData);
		}

	private:
		BlockBlendDataList mList;
	};
	

}//namespace Blade


#endif // __Blade_TerrainBlendData_h__