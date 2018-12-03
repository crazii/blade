/********************************************************************
	created:	2011/05/22
	filename: 	EditorElement.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorElement_h__
#define __Blade_EditorElement_h__
#include <BladeEditor.h>
#include <ElementBase.h>
#include <utility/Variant.h>
#include <interface/public/graphics/Color.h>
#include <interface/public/graphics/GraphicsDefs.h>
#include <interface/public/graphics/IGraphicsEffect.h>
#include <interface/public/geometry/GeomTypes.h>

namespace Blade
{
	class IEditable;
	class IParaState;
	class IGeometry;

	class BLADE_EDITOR_API EditorElement : public ElementBase , public Allocatable
	{
	public:
		static const TString EDITOR_ELEMENT_TYPE;
		static const TString EDITOR_ELEMENT_NAME;
	public:
		EditorElement(const TString& type = EDITOR_ELEMENT_TYPE, const TString& iconHUD = TString::EMPTY);
		virtual ~EditorElement();
		
		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data in main synchronous state */
		virtual void	postProcess(const ProgressNotifier& notifier);
		/** @brief editor element will be loaded last, postProcess called at last */
		virtual int16	getPriority() const { return P_LOWEST; }

		/************************************************************************/
		/* IElement interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	initParallelStates() { return mParallelStates.size(); }

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	onParallelStateChange(const IParaState& /*data*/)	{}
		/** @brief  */
		virtual void	onEntityChange()	{/*this->showHUD(this->getEntity() != NULL);*/}

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/** @brief  */
		void			initialize(IScene* scene);
		/** @brief  */
		void			setEditable(IEditable* editable);

		/** @brief  */
		IEditable*		getEditable() const		{return mEditable;}

		/** @brief  */
		bool			showBounding(bool enable,Color color = Color::WHITE);

		/** @brief  */
		bool			showGizmoHelper(bool visible, EGizmoType type = GT_MOVE, GEOM_SPACE space = GS_WORLD);

		/** @brief  */
		bool			showHUD(bool visible);

		/** @brief  */
		inline HGRAPHICSEFFECT	getGizmoEffect() const	{return mGizmoEffect;}

		/** @brief  */
		inline HGRAPHICSEFFECT	getHUDEffect() const	{return mHUDEffect;}

		/** @brief  */
		inline void		setHUDIcon(const TString& icon)	{mHUDIcon = icon;}
		
		/** @brief  */
		IGeometry*		getGeomtry() const;

		/** @brief  */
		Variant			getElementConfig(const TString& elementType, const TString& configName);

		/** @brief  */
		bool			setElementConfig(const TString& elementType, const TString& configName, const Variant& val);

	protected:
		/** @brief  */
		virtual void	onInitialize()	{}

		IEditable*		mEditable;
		TString			mHUDIcon;
		HGRAPHICSEFFECT	mHUDEffect;
		HGRAPHICSEFFECT	mGizmoEffect;
	private:
		//serialization data
		TString			mEditableClass;
		friend bool		Blade::initializeEditorFramework();
	};//class EditorElement
	
	extern template class BLADE_EDITOR_API Factory<EditorElement>;
	typedef Factory<EditorElement> EditorElementFactory;

}//namespace Blade



#endif // __Blade_EditorElement_h__