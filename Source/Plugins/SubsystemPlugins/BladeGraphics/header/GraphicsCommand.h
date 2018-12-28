/********************************************************************
	created:	2017/12/05
	filename: 	GraphicsCommand.h
	author:		Crazii
	purpose:	delayed command. some request issued in TS_ASYNC_RUN state from other subsystem may be not probably handled instantly,
				they are cached as commands and executed in TS_ASYNC_UPDATE state.

				i.e. add/remove space contents cannot be done in TS_ASYNC_RUN mode, because in TS_ASYNC_RUN spaces provide
				space queries and are read only to all subsystems

				Actions that modifies graphics buffer need to be cached as commands too, because graphics buffers are used in draw calls
				in TS_ASYNC_RUN state, they should not be modified at that time.
*********************************************************************/
#ifndef __Blade_GraphicsCommand_h__
#define __Blade_GraphicsCommand_h__
#include <utility/BiRef.h>
#include <interface/IGraphicsUpdater.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	class GraphicsCommand : public IGraphicsUpdatable, public BiRef, public TempAllocatable
	{
	public:
		GraphicsCommand(bool ignoreSlot = false)
			:mIgonreSlot(ignoreSlot)
		{
			mUpdateStateMask.raiseBitAtIndex(SS_ASYNC_UPDATE);
		}
		virtual ~GraphicsCommand() {}

		/** @brief  */
		virtual void update(SSTATE)
		{
			if(mIgonreSlot || this->getSlot() != NULL)	//slot destroyed, execution canceled
				this->execute();

			BLADE_DELETE this;
		}

	protected:
		/** @brief  */
		BiRef* getSlot() const { return this->getRef(); }

		/** @brief  */
		virtual void execute() = 0;

		bool	mIgonreSlot;
	};

	//////////////////////////////////////////////////////////////////////////
	class GraphicsCommandSlot : public BiRef, public TempAllocatable
	{
	public:
		GraphicsCommandSlot(GraphicsCommand* cmd) :BiRef(cmd) {}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	class GraphicsElement;
	class GraphicsElementCommand : public GraphicsCommand
	{
	protected:
		/** @brief  */
		inline GraphicsElement*	getTarget() const;

		/** @brief  */
		BLADE_GRAPHICS_API virtual void update(SSTATE);

	public:
		template<typename T, typename V>
		static inline GraphicsElementCommand* make(V v, bool (T::*pfnset)(V v));

		template<typename T, typename V>
		static inline GraphicsElementCommand* make(const V& v, bool (T::*pfnset)(const V& v));
	};

	namespace Impl
	{
		/** @brief helper */
		template<typename T, typename V>
		class TGraphicsElementCommand : public GraphicsElementCommand
		{
		public:
			typedef bool (T::*FNSET)(V v);

			TGraphicsElementCommand(const V& v, FNSET fnset)
				:set(fnset), val(v)
			{}

		protected:
			/** @brief  */
			virtual void execute()
			{
				T* element = static_cast<T*>(this->getTarget());
				(element->*set)(val);
			}
			FNSET set;
			V val;
		};

	}//namespace Impl

	template<typename T, typename V>
	static inline GraphicsElementCommand* GraphicsElementCommand::make(V v, bool (T::*pfnset)(V v))
	{
		return BLADE_NEW Impl::TGraphicsElementCommand<T, V>(v, pfnset);
	}
	template<typename T, typename V>
	static inline GraphicsElementCommand* GraphicsElementCommand::make(const V& v, bool (T::*pfnset)(const V& v))
	{
		return BLADE_NEW Impl::TGraphicsElementCommand<T, const V&>(v, pfnset);
	}

	//////////////////////////////////////////////////////////////////////////
	class GraphicsElementCommandSlot : public GraphicsCommandSlot
	{
	public:
		GraphicsElementCommandSlot(GraphicsElement* element, GraphicsCommand* cmd) :GraphicsCommandSlot(cmd),mElement(element) {}
		GraphicsElement* mElement;
	};

	//////////////////////////////////////////////////////////////////////////
	inline GraphicsElement*	GraphicsElementCommand::getTarget() const
	{
		return static_cast<GraphicsElementCommandSlot*>(this->getSlot())->mElement;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ISpaceContent;
	class ISpace;
	//////////////////////////////////////////////////////////////////////////
	class AddContentToSpaceCommand : public GraphicsCommand
	{
	public:
		AddContentToSpaceCommand(ISpaceContent* content, ISpace* space, bool ignoreSlot)
			:GraphicsCommand(ignoreSlot)
			,mContent(content)
			,mSpace(space)
		{

		}

		/** @brief  */
		virtual void execute();

	protected:
		ISpaceContent* mContent;
		ISpace* mSpace;
	};

	//////////////////////////////////////////////////////////////////////////
	class RemoveContentFromSpaceCommand : public GraphicsCommand
	{
	public:
		RemoveContentFromSpaceCommand(ISpaceContent* content, ISpace* space, bool ignoreSlot)
			:GraphicsCommand(ignoreSlot)
			,mContent(content)
			,mSpace(space)
		{

		}

		/** @brief  */
		virtual void execute();

	protected:
		ISpaceContent* mContent;
		ISpace* mSpace;
	};

}//namespace Blade

#endif//__Blade_GraphicsCommand_h__