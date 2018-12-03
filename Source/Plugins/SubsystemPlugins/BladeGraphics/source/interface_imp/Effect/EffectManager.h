/********************************************************************
	created:	2011/04/02
	filename: 	EffectManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EffectManager_h__
#define __Blade_EffectManager_h__
#include <interface/IEffectManager.h>
#include <utility/BladeContainer.h>
#include <BladeGraphics.h>
#include <Material.h>
#include "EffectRenderType.h"
#include "../GraphicsScene.h"

namespace Blade
{

	class EffectManager : public IEffectManager, public IGraphicsUpdatable, public Allocatable
	{
	public:
		EffectManager(GraphicsScene* scene);
		~EffectManager();

		/************************************************************************/
		/* IGraphicsEffectManger interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsScene*	getScene() const	{return mScene;}

		/*
		@describe add effect to manage it's life time (auto delete).when time up, auto-detach from element if attached
		@param [in] lifetime : the effect's life circle. 
		<= 0 is persistent
		> 0 is a real life circle
		@return NULL if failed
		*/
		virtual bool	addEffect(const HGRAPHICSEFFECT& hEffect,scalar lifeTime);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeEffect(const HGRAPHICSEFFECT& hEffect);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HGRAPHICSEFFECT	createEffect(const TString& type);

		/**
		@describe
		@param [in] dynamicUpdate indicates this effect will be frequently updated through updateCustomEffect()
		@return NULL if add failed
		@return NULL if failed
		*/
		virtual HGRAPHICSEFFECT  createCustomEffect(const Vector3* vertexdata, size_t vertexCount, bool dynamicUpdate, bool independent,
			Material* material, const HTEXTURESTATE& textureState,
			const AABB* worldAABB = NULL, const uint32* indexdata = NULL, size_t indexCount = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			updateCustomEffect(const HGRAPHICSEFFECT& hCustomEffect, const Vector3* vertexdata, size_t vertexCount,
			const AABB* worldAABB = NULL, const uint32* indexdata = NULL, size_t indexCount = 0);

		/*
		@describe deprecated
		@param 
		@return 
		*/
		virtual bool	setCustomShaderVariable(const TString& materialName, const TString& variableName,
			SHADER_CONSTANT_TYPE type,size_t count,const void* pData);

		/************************************************************************/
		/* IEffectManager interface                                                                     */
		/************************************************************************/

		/************************************************************************/
		/* IGraphicsUpdatable interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void update(SSTATE state);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		static EffectRenderType&	getEffectRenderType();

		/** @brief  */
		void deactivateEffect(const HGRAPHICSEFFECT& hEffect);

	protected:
		static EffectRenderType	msRenderType;
		typedef struct STimeEffect
		{
			scalar mLifeTime;
			scalar mTiming;
			HGRAPHICSEFFECT mEffect;

			STimeEffect(){}
			STimeEffect(const HGRAPHICSEFFECT& hEffect,scalar lifetime)
				:mLifeTime(lifetime),mTiming(0),mEffect(hEffect)	{}
			bool operator==(const STimeEffect& effect) const		{return mEffect == effect.mEffect;}
		}EFFECT_INFO;

		typedef List<EFFECT_INFO>			TimingEffectList;
		typedef Set<HGRAPHICSEFFECT>		PersitentEffectList;
		typedef Set<HGRAPHICSEFFECT>		InternalEffectList;
		typedef Set<IGraphicsEffect*>		AllEffectLIst;

		GraphicsScene*			mScene;

		Lock					mLifeTimeEffectLock;
		Lock					mEffectLock;
		TimingEffectList		mLifeTimeEffect;
		PersitentEffectList		mEffectList;
	};
	

}//namespace Blade



#endif // __Blade_EffectManager_h__