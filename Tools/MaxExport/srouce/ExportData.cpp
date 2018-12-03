/********************************************************************
	created:	2015/09/07
	filename: 	ExportData.cpp
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <ExportData.h>

namespace Blade
{

	/************************************************************************/
	/* IModelCollector interface                                                                     */
	/************************************************************************/


	/************************************************************************/
	/* IAnimationCollector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			MaxModelCollector::getBoneTransform(BoneDQ& transform, index_t boneIndex, scalar time)
	{
		if( boneIndex >= mBoneList.size() )
		{
			assert(false);
			return false;
		}

		IGameNode* node = mBoneList[boneIndex];
		GMatrix tm = node->GetWorldTM( SecToTicks(time) );

		Matrix3 m3 = tm.ExtractMatrix3();
		m3.NoScale();
		tm = m3;

		//get transform relative to parent
		IGameNode* parent = node->GetNodeParent();
		if( parent != NULL )
		{
			GMatrix ptm = parent->GetWorldTM( SecToTicks(time) );
			m3 = ptm.ExtractMatrix3(); 
			m3.NoScale();
			ptm = m3;
			tm = tm*ptm.Inverse();
		}

		transform.set( reinterpret_cast<Matrix44&>(tm) );
		transform.normalize();
		return true;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	MaxModelCollector::exportMaterial(IGameMaterial* material, Blade::IModelResource::MATERIAL_INFO& target)
	{
		target.ambient = Blade::Color::WHITE;
		target.diffuse = Blade::Color::WHITE;
		target.specular = Blade::Color::WHITE;
		target.emissive = Blade::Color::BLACK;
		if( material == NULL || material->GetMaxMaterial() == NULL)
			return;

		//HOWTO: alpha clip. check the bitmap is alpha channel?
		//ID_OP == opacity map
		target.hasTransparency = material->GetMaxMaterial()->GetSubTexmap(ID_OP) != NULL;
		target.textureFullPath = false;

		//export material
		Point4 val4;
		Point3 val3;
		PropType pt;

		IGameProperty* p = material->GetAmbientData();
		if(p != NULL)
		{
			pt = p->GetType();
			if (pt == IGAME_POINT3_PROP && p->GetPropertyValue(val3) )
				target.ambient = Blade::Color(val3.x,val3.y,val3.z,1.0f);
			if (pt == IGAME_POINT4_PROP && p->GetPropertyValue(val4))
				target.ambient = Blade::Color(val4.x,val4.y,val4.z,val4.w);
		}

		p = material->GetDiffuseData();
		if(p != NULL)
		{
			pt = p->GetType();
			if (pt == IGAME_POINT3_PROP && p->GetPropertyValue(val3))
				target.diffuse = Blade::Color(val3.x,val3.y,val3.z,1.0f);
			if (pt == IGAME_POINT4_PROP && p->GetPropertyValue(val4))
				target.diffuse = Blade::Color(val4.x,val4.y,val4.z,val4.w);
		}

		p = material->GetSpecularData();
		if (p != NULL)
		{
			pt = p->GetType();
			if (pt == IGAME_POINT3_PROP && p->GetPropertyValue(val3))
				target.specular = Blade::Color(val3.x,val3.y,val3.z,1.0f);
			if (pt == IGAME_POINT4_PROP && p->GetPropertyValue(val4))
				target.specular = Blade::Color(val4.x,val4.y,val4.z,val4.w);
		}

		p = material->GetEmissiveData();
		if (p != NULL) {
			pt = p->GetType();

			if (pt == IGAME_POINT3_PROP && p->GetPropertyValue(val3))
				target.emissive = Blade::Color(val3.x,val3.y,val3.z,1.0f);
			if (pt == IGAME_POINT4_PROP && p->GetPropertyValue(val4))
				target.emissive = Blade::Color(val4.x,val4.y,val4.z,val4.w);
		}

		int numTexMaps = material->GetNumberOfTextureMaps();
		StdMat* stdmat = dynamic_cast<StdMat*>( material->GetMaxMaterial() );

		//sort by channel
		typedef TempMap<IModelResource::CHANNEL, TString> ChannelMap;
		ChannelMap chMap;

		for (int texMapIdx = 0; texMapIdx < numTexMaps; texMapIdx++)
		{
			IGameTextureMap* map = material->GetIGameTextureMap(texMapIdx);
			int mapType = map->GetStdMapSlot();
			const tchar* file = map->GetBitmapFileName();

			if(stdmat == NULL || !stdmat->MapEnabled(mapType) )
				continue;

			bool mapValid = (map != NULL && map->IsEntitySupported());

			//find the first valid sub map if any. TODO: what if multiple sub maps exist?
			if( !mapValid && map != NULL )
			{
				Texmap* maxMap = map->GetMaxTexmap();
				int n = maxMap->NumSubTexmaps();
				for(int j = 0; j < n; ++j)
				{
					Texmap* subMap = maxMap->GetSubTexmap(j);
					if(subMap != NULL && subMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0) && maxMap->SubTexmapOn(j) )
					{
						file = ((BitmapTex *)subMap)->GetMapName();
						mapValid = true;
						break;
					}
				}
			}

			if( !mapValid )
				continue;

			IModelResource::CHANNEL ch;
			switch(mapType)
			{
			case ID_DI:
				ch = IModelResource::MC_DIFFUSE;
				break;
			case ID_SP:
				ch = IModelResource::MC_SPECULAR;
				break;
			case ID_SH:	//gloss
				ch = IModelResource::MC_SPECULAR_GLOSS;
				break;
			case ID_SS:	//specular level
				ch = IModelResource::MC_SPECULAR_LEVEL;
				break;
			case ID_SI:	//self illumination
				ch = IModelResource::MC_GLOW;
				break;
			case ID_BU:
				ch = IModelResource::MC_NORMAL;
				break;
			default:
				//others not supported
				if( mapType == ID_OP )
					target.hasTransparency = true;
				continue;
			}

			chMap[ch] = TString( file );
		}//

		for(ChannelMap::iterator i = chMap.begin(); i != chMap.end(); ++i)
		{
			target.channelMap[ i->first ] = (int8)target.textures.size();
			target.textures.push_back( i->second );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MaxModelCollector::addCamera(IGameCamera* camera)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MaxModelCollector::getInitTransform(BoneDQ& transform, IGameNode* node)
	{
		if( node != NULL )
		{
			GMatrix tm = node->GetWorldTM(0);

			//note: GMatrix's coordinate is correct but when extracting into max Quat, it becomes incorrect,
			//as extraction from a Matrix3 in max coordinate
			//Quat maxq = tm.Rotation();
			//Matrix3 testm = tm.ExtractMatrix3();
			//testm.NoScale();
			//Quat test = testm;

			//drop scale
			Matrix3 m3 = tm.ExtractMatrix3();
			m3.NoScale();
			tm = m3;

			transform.set( reinterpret_cast<Matrix44&>(tm) );
			transform.normalize();
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		MaxModelCollector::initBoneData()
	{
		if( mBoneData.size() != 0)
		{
			assert(false);
			return false;
		}
		
		mBoneData.clear();
		mBoneData.resize(mBoneList.size());

		BoneDQ transform;
		for(size_t i = 0; i < mBoneList.size(); ++i)
		{
			IGameNode* node = mBoneList[i];
			BONE_DATA& bone = mBoneData[i];
			//transform = BoneDQ::IDENTITY;

			index_t parentIndex = INVALID_INDEX;
			IGameNode* parent = node->GetNodeParent();
			if( parent != NULL )
			{
				BoneList::iterator iter = std::find(mBoneList.begin(), mBoneList.end(), parent );
				if( iter != mBoneList.end() )
					parentIndex = iter - mBoneList.begin();
				else
					assert(false);
			}
			//0.0 => starting T pose time
			this->getInitTransform(transform, node);

			bone.mParent = (uint32)parentIndex;
			bone.mIndex = (uint32)i;
			bone.mInitialPose = transform;
		}// for each bone in list

		return true;
	}
	
}//namespace Blade