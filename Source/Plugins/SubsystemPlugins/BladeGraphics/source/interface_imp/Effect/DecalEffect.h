/********************************************************************
	created:	2012/12/30
	filename: 	DecalEffect.h
	author:		Crazii
	
	purpose:	geometry decal implementation
*********************************************************************/
#ifndef __Blade_DecalEffect_h__
#define __Blade_DecalEffect_h__
#include <utility/BladeContainer.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/IGraphicsUpdater.h>
#include <interface/IMaterialManager.h>
#include <interface/IEffectManager.h>
#include <Material.h>
#include "CustomEffect.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class DecalTriangleList : public SpaceQuery::ITriangleBuffer, public TempAllocatable
	{
	public:
		typedef TempVector<POINT3> VertexList;
		VertexList	mVertices;

		DecalTriangleList()
		{
			mVertices.reserve(64);
		}

		virtual ~DecalTriangleList()
		{

		}

		/** @brief  */
		virtual size_t		getVertexCount() const	{return mVertices.size();}

		/** @brief  */
		virtual const POINT3*getVertexData() const	{return mVertices.size() >0 ? &mVertices[0] : NULL;}

		/** @brief  */
		virtual size_t		getVertexSize() const	{return sizeof(POINT3);}

		/** @brief  */
		virtual void	add(const POINT3& p0,const POINT3& p1,const POINT3& p2)
		{
			mVertices.push_back(p0);
			mVertices.push_back(p1);
			mVertices.push_back(p2);
		}
	};

	class DecalEffect : public IDecalEffect, public IGraphicsUpdatable
	{
	public:
		static const TString DEFAULT_MATERIAL;

		DecalEffect() :mMaterial(NULL)	{mMaterialName = DEFAULT_MATERIAL;mUpdateStateMask.raiseBitAtIndex(SS_PRE_RENDER);}
		~DecalEffect()	{ this->onDetach(); }
		/************************************************************************/
		/* IGraphicsEffect interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	onAttach()
		{
			return true;
		}
		/** @brief  */
		virtual bool	onDetach()
		{
			IEffectManager* manager = static_cast<IEffectManager*>( this->getManager() );
			if( mInternalEffect != NULL && manager != NULL && mDynamic)
				manager->removeEffect( mInternalEffect );

			mInternalEffect.clear();

			if( mDynamic )
			{
				IRenderScene* scene = static_cast<IRenderScene*>(this->getManager()->getScene());
				scene->getUpdater()->addForUpdate(this);
			}
			return true;
		}
		/** @brief  */
		virtual bool	isReady() const
		{
			return mMaterial != NULL && mMaterial->isLoaded();
		}

		/************************************************************************/
		/* IDecalEffect interface                                                                     */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		virtual bool	setDecalImages(const TStringParam& textures, const TStringParam& samplers)
		{
			if(mMaterial == NULL || textures.size() == 0)
				return false;

			IEffectManager* manager = static_cast<IEffectManager*>( this->getManager() );
			mDecalImages.clear();
			mDecalImages = manager->createTextureState(textures, samplers);
			mDecalImages->load();
			return true;
		}

		/** @brief  */
		virtual bool initializeImpl(const TStringParam& textures, const TStringParam& samplers)
		{
			if( mMaterial != NULL )
				return false;

			mMaterial = IMaterialManager::getSingleton().getMaterial( mMaterialName );
			if( mMaterial == NULL )
				return false;

			this->setDecalImages(textures, samplers);

			IRenderScene* scene = static_cast<IRenderScene*>(this->getManager()->getScene());
			if( mDynamic )
				scene->getUpdater()->addForUpdate(this);
			else
				scene->getUpdater()->addForUpdateOnce(this);
			return true;
		}

		/** @brief  */
		virtual bool setMaterialImpl(const TString& materialName)
		{
			if( mMaterial == NULL )
				return false;

			Material* material = IMaterialManager::getSingleton().getMaterial( materialName );
			if( mMaterial == NULL )
				return false;

			mMaterial = material;
			return true;
		}

		/************************************************************************/
		/* IGraphicsUpdatable interface                                                                    */
		/************************************************************************/
		virtual void update(SSTATE)
		{
			assert( mDecalImages != NULL );

			IRenderScene* scene = static_cast<IRenderScene*>(this->getManager()->getScene());

			AABB aab = this->getAABB();
			DecalTriangleList* triangleList = BLADE_NEW DecalTriangleList();
			AABBQuery decalVolume(aab, mAppFlag, triangleList);
			bool result = scene->queryElements(decalVolume);

			if( result && triangleList->getVertexCount() != 0 && mDynamic)
			{
				if( mInternalEffect != NULL)
				{
					const MaterialInstance* material = static_cast<CustomEffect*>(mInternalEffect)->getMaterial();
					ICamera* camera = material->getTextureProjectionCamera(0, true);
					camera->setOrthoMode(mSize,mSize);
					Vector3 eye(mPosition.x , mHeight, mPosition.z);
					Matrix44 viewMatrix;
					//the view matrix is looking down (0,-1,0), so do not use UNIT_Y as up vector
					Matrix44::generateViewMatrix(viewMatrix,eye, mPosition, Vector3::UNIT_Z);
					camera->setViewMatrix(viewMatrix, true);
				}

				IEffectManager* manager = static_cast<IEffectManager*>(this->getManager());

				//TODO: use decal volume instead of Infinite AABB when quad tree space is ready
				const AABB* paab = &aab;
				if (mInternalEffect == NULL)
				{
					mInternalEffect = manager->createCustomEffect(triangleList->getVertexData(), triangleList->getVertexCount(), mDynamic, false, mMaterial, mDecalImages, paab);
					//manager->addEffect(mInternalEffect, 0); //optional
				}
				else
					manager->updateCustomEffect(mInternalEffect, triangleList->getVertexData(), triangleList->getVertexCount());
			}
			BLADE_DELETE triangleList;
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline Material* getMaterial() const	{return mMaterial;}
	protected:
		Material*		mMaterial;
		HTEXTURESTATE	mDecalImages;
		HGRAPHICSEFFECT	mInternalEffect;
	};
	
}//namespace Blade

#endif//__Blade_DecalEffect_h__