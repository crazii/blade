/********************************************************************
	created:	2010/04/25
	filename: 	GraphicsElement.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsElement_h__
#define __Blade_GraphicsElement_h__
#include <Factory.h>

#include <ElementBase.h>
#include <parallel/ParaStateDef.h>
#include <BladeGraphics.h>
#include <interface/public/graphics/GraphicsDefs.h>
#include <interface/IRenderScene.h>
#include <interface/public/graphics/IGraphicsInterface.h>
#include <GraphicsCommand.h>

namespace Blade
{
	class Event;
	class ISpace;
	class ISpaceContent;

	namespace Impl
	{
		class EffectListImpl;
		class CommandListImpl;
	}//namespace Impl

	 /*
	 Async behavior/spec of GraphicsElement:
	 1. a graphic element should be read only on TS_ASYNC_RUN (async run state), so that any other tasks/elements from other subsystems can query the element [shared read].
	 2. a graphics element can only add/remove its contents/effects in TS_ASYNC_UPDATE (async update state), only at this time its space is writable.
	 3. if graphics buffers (vertex/index buffer) are written/updated through PUBLIC INTERFACE (accessed by other tasks),
		they should delay it on TS_ASYNC_UPDATE. on TS_ASYNC_RUN state, those buffers are used for rendering.
	 4. if contents are added/removed through PUBLIC INTERFACE, then operations should be queued/delayed to TS_ASYNC_UPDATE state.
	 5. use GraphicsCommand/GraphicsElementCommand to queue/delay the operation.
	 */
	class BLADE_GRAPHICS_API GraphicsElement : public IGraphicsInterface, public ElementBase, public NonCopyable, public NonAssignable
	{
	public:
		GraphicsElement(const TString& type);
		~GraphicsElement();

		/************************************************************************/
		/* IGraphicsInterface interface                                                                     */
		/************************************************************************/
	protected:
		/** @copydoc IGraphicsInterface::addEffect  */
		virtual bool	addEffect(const HGRAPHICSEFFECT& effect)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return effect != NULL && this->addCommand(GraphicsElementCommand::make(effect, &GraphicsElement::addGraphicsEffect));
		}

		/** @copydoc IGraphicsInterface::removeEffect  */
		virtual bool	removeEffect(const HGRAPHICSEFFECT& effect)
		{
			BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();
			return effect != NULL && this->addCommand(GraphicsElementCommand::make(effect, &GraphicsElement::removeGraphicsEffect));
		}

		/** @copydoc IGraphicsInterface::getEffect  */
		virtual const HGRAPHICSEFFECT& getEffect(const TString& type) const;

		/** @brief  */
		virtual IGraphicsScene* getGraphicsScene() const { BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS(); return this->getRenderScene(); }

		/** @brief  */
		virtual void setVisible(bool visible);

		/** @brief  */
		virtual void enablePicking(bool enable);

		/** @brief  */
		virtual const Vector3&	getStaticPosition() const;
		/** @brief  */
		virtual const Quaternion&	getStaticRotation() const;
		/** @brief  */
		virtual const Vector3&	getStaticScale() const;
		/** @brief  */
		virtual const AABB& getStaticLocalBounds() const;

		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void	postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t	initParallelStates();

		/** @brief  */
		virtual void	onParallelStateChange(const IParaState& data);

		/** @brief  */
		virtual Interface*		getInterface(InterfaceName type)
		{
			CHECK_RETURN_INTERFACE(IGraphicsInterface, type, this);
			return NULL;
		}

		/************************************************************************/
		/* ElementBase interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void onResourceUnload();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
	public:
		/** @copydoc IGraphicsInterface::addEffect  */
		bool	addGraphicsEffect(const HGRAPHICSEFFECT& effect);

		/** @copydoc IGraphicsInterface::removeEffect  */
		bool	removeGraphicsEffect(const HGRAPHICSEFFECT& effect);

		/**
		@describe
		@param
		@return
		*/
		virtual bool		showBoundingVolume(bool show, const Color& color = Color::WHITE);

		/**
		@describe used for some effects, i.e. HUD
		@param
		@return
		*/
		virtual POINT3		getPositionHint() const { return mPositionData; }

		/*
		@describe
		@param
		@return
		*/
		void				initialize(IRenderScene* scene,ISpace* InitSpace);

		/*
		@describe
		@param
		@return
		*/
		ISpaceContent*		getContent() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const Vector3&		getPosition() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const Vector3&		getScale() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const Quaternion&	getRotation() const;

		/*
		@describe
		@param
		@return
		*/
		void				setPosition(const Vector3& pos);

		inline void			setPosition(scalar x,scalar y,scalar z)
		{
			this->setPosition( Vector3(x,y,z) );
		}

		/*
		@describe
		@param
		@return
		*/
		void				setRotation(const Quaternion& rotation );

		/*
		@describe
		@param
		@return
		*/
		void				setScale(const Vector3& vscale);

		/**
		@describe dispatch transform changes to all effects bound to this element
		@param
		@return
		*/
		bool				dispatchPositionChanges(const Vector3& pos);
		bool				dispatchRotationChanges(const Quaternion& rotation);
		bool				dispatchScaleChanges(const Vector3& scale);
		bool				dispatchLocalBoundsChanges(const AABB& aab);

		/**
		@describe 
		@param 
		@return 
		*/
		void	notifyContentChange();

		/** @brief data binding helper */
		void	onConfigChange(void* data);

		inline void			setScale(scalar x,scalar y,scalar z)
		{
			this->setScale( Vector3(x,y,z) );
		}

		/** @brief  */
		inline IRenderScene*	getRenderScene() const
		{
			return static_cast<IRenderScene*>( this->getScene() );
		}

		/** @brief  */
		inline ISpace*		getSpace() const
		{
			return mSpace;
		}

		/** @brief  */
		inline ParaStateQueue* getStateQueue() const
		{
			IRenderScene* scene = this->getRenderScene();
			return scene != NULL ? scene->getStateQueue() : NULL;
		}

		/** @brief  */
		inline IGraphicsUpdater* getUpdater() const
		{
			IRenderScene* scene = this->getRenderScene();
			return scene != NULL ? scene->getUpdater() : NULL;
		}


		/************************************************************************/
		/* data binding helpers                                                                     */
		/************************************************************************/
		/** @brief  */
		template<typename T /*= GraphicsElement*/> //default template arguments may not be used in function templates without c++11
		inline static Vector3 T::* getPositioAddr()
		{
			return static_cast<Vector3 T::*> (&GraphicsElement::mPositionData);
		}

		/** @brief  */
		template<typename T /*= GraphicsElement*/>
		inline static Quaternion T::* getRotationAddr()
		{
			return static_cast<Quaternion T::*> (&GraphicsElement::mRotationData);
		}

		/** @brief  */
		template<typename T /*= GraphicsElement*/>
		inline static Vector3 T::* getScaleAddr()
		{
			return static_cast<Vector3 T::*> (&GraphicsElement::mScaleData);
		}

		/** @brief  */
		template<typename T /*= GraphicsElement*/>
		inline static Box3 T::* getBoundingAddr()
		{
			return static_cast<Box3 T::*> ((Box3 GraphicsElement::*)&GraphicsElement::mAABData);
		}

		//data binding

		/** @brief  */
		const uint32&	getSpacePartitionMask(index_t) const;
		/** @brief  */
		bool			setSpacePartitionMask(index_t, const uint32& mask);

		/** @brief  */
		const bool&	getDynamic(index_t) const;
		/** @brief  */
		virtual bool setDynamic(index_t, const bool& _dynamic);
		/** @brief  */
		bool isDynamic() const { return getDynamic(0); }

	protected:
		/** @brief  */
		void activateContent();

		/** @brief  */
		void deactivateContent();

		/** @brief whether content is active. */
		bool isContentActive() const
		{
			return mContentInSpace;
		}

		/** @brief  */
		void clearEffects();

		/** @brief  */
		bool addCommand(GraphicsElementCommand* cmd);

	protected:
		/*
		@describe
		@param
		@return
		*/
		virtual void		onInitialize() = 0;

		ISpaceContent*		mContent;

		//raw data for data-binding
		Vector3				mPositionData;
		Quaternion			mRotationData;
		Vector3				mScaleData;
		AABB				mAABData;	//local AAB

		//parallel data using raw data
		ParaVec3			mPosition;
		ParaQuat			mRotation;
		ParaVec3			mScale;
		ParaAAB				mBounding;
	private:
		ISpace*				mSpace;
		mutable uint32		mPartitionMask;	//content space mask kept when element resource unloaded
		mutable bool		mDynamic;
		bool				mContentInSpace;
		bool				mUnloaded;		//resource unloaded for at least once
		LazyPimpl<Impl::EffectListImpl>		mEffectList;
		LazyPimpl<Impl::CommandListImpl>	mCommandList;

		/** @brief  */
		bool finishCommand(GraphicsElementCommand* cmd);
		friend class GraphicsElementCommand;
	};//class GraphicsElement


	extern template class BLADE_GRAPHICS_API	Factory<GraphicsElement>;
	typedef Factory<GraphicsElement>	GraphicsElementFactory;

}//namespace Blade



#endif //__Blade_GraphicsElement_h__