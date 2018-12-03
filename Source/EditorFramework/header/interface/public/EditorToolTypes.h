/********************************************************************
	created:	2011/05/07
	filename: 	EditorToolTypes.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorToolTypes_h__
#define __Blade_EditorToolTypes_h__
#include <interface/public/graphics/IImage.h>
#include <interface/public/ui/UITypes.h>
#include <BladeEditor.h>
#include <interface/public/IEditorTool.h>
#include <interface/public/graphics/IGraphicsEffect.h>
#include <interface/IConfig.h>

namespace Blade
{
	class Ray;
	class IBrushDecalEffect;

	class EditorToolTypes
	{
	public:
		///editor tool will be executed immediately on selected(clicked)
		static const BLADE_EDITOR_API TString IMMEDIATE_EDIT_TYPE;

		///immediately enter status on selected and will exit status ( like radio or check button)
		static const BLADE_EDITOR_API TString IMMEDIATE_STATUS_EDIT_TYPE;

		///editor tool will be selected and may switch to other tools
		static const BLADE_EDITOR_API TString COMMON_EDIT_TYPE;

		///brush type
		static const BLADE_EDITOR_API TString BRUSH_EDIT_TYPE;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class BLADE_EDITOR_API EditorBaseTool : public IEditorTool, public Bindable
	{
	public:
		EditorBaseTool(const TString& name,IconIndex icon,HOTKEY hotkey);

		virtual ~EditorBaseTool();

		/**
		@describe tool name
		@param 
		@return 
		*/
		virtual const TString&		getName() const		{return mName;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IconIndex			getToolIcon() const	{return mIconIndex;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HOTKEY				getHotkey() const	{return mHotkey;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getToolConfig() const	{return mToolConfig;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isEnable() const		{return mEndabled;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				setEnable(bool enabled);

	protected:

		/** @brief enable/disable notification */
		virtual void				onEnable(bool bEnabled)	{	BLADE_UNREFERENCED(bEnabled);	}
		
		/** @brief  */
		virtual void				onEditableRemoved(IEditable* editable) { BLADE_UNREFERENCED(editable); }

		/** @brief helper functions to pick object */
		static IEditable*			pickEditable(scalar x, scalar y, IEditorFile* file);
		/** @brief  */
		static bool					pickPosition(scalar x, scalar y, AppFlag flag, IEditorFile* file, Ray& outRay, scalar& outDist);
		/** @brief  */
		static POINT3				pickPosition(scalar x, scalar y, AppFlag flag, float fallbackDist, IEditorFile* file);

		TString		mName;
		IconIndex	mIconIndex;
		HOTKEY		mHotkey;
		bool		mEndabled;
		HCONFIG		mToolConfig;

	private:
		/** @brief  */
		void onEditableUnload(const Event& data);
	};//class EditorBaseTool


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class BLADE_EDITOR_API EditorCommandTool : public EditorBaseTool
	{
	public:
		EditorCommandTool(const TString& name,IconIndex icon,HOTKEY hotkey)
			:EditorBaseTool(name,icon,hotkey)
		{}

		virtual ~EditorCommandTool() {}

		/**
		@describe 
		@param 
		@return 
		@see Blade::EditorToolTypes
		*/
		virtual const TString&		getType() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				setSelected(bool select);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isSelected() const	{return false;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(file);

			return false;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(file);

			return false;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(file);

			return false;
		}

		/**
		@describe
		@param [in] x,y : normalized mouse position
		@return 
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(deltaMove);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(deltaTime);
			BLADE_UNREFERENCED(file);

			return false;
		}

		/**
		@describe this method will be called if tool type is immediate type
		@param 
		@return 
		*/
		virtual bool				doCommand() = 0;

	};//class EditorCommandTool


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class BLADE_EDITOR_API EditorStatusCommandTool : public EditorBaseTool
	{
	public:
		EditorStatusCommandTool(const TString& name,IconIndex icon,HOTKEY hotkey)
			:EditorBaseTool(name,icon,hotkey)
			,mSelected(false)
		{}

		virtual ~EditorStatusCommandTool() {}

		/**
		@describe 
		@param 
		@return 
		@see Blade::EditorToolTypes
		*/
		virtual const TString&		getType() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				setSelected(bool select);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isSelected() const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(file);

			return false;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftDown(scalar x, scalar y, uint32 keyModifier, IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(file);

			return false;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(file);
			return false;
		}

		/**
		@describe
		@param [in] x,y : normalized mouse position
		@return 
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file)
		{
			BLADE_UNREFERENCED(x);
			BLADE_UNREFERENCED(y);
			BLADE_UNREFERENCED(deltaMove);
			BLADE_UNREFERENCED(keyModifier);
			BLADE_UNREFERENCED(deltaTime);
			BLADE_UNREFERENCED(file);
			return false;
		}

		/**
		@describe this method will be called if tool type is immediate type
		@param 
		@return 
		*/
		virtual bool				doCommand() = 0;

	protected:
		bool			mSelected;

	};//class EditorStatusCommandTool



	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class BLADE_EDITOR_API EditorCommonTool : public EditorBaseTool
	{
	public:
		EditorCommonTool(const TString& name,IconIndex icon,HOTKEY hotkey)
			:EditorBaseTool(name,icon,hotkey)
			,mSelected(false)
		{}

		virtual ~EditorCommonTool()				{}

		/*
		@describe 
		@param 
		@return 
		@see Blade::EditorToolTypes
		*/
		virtual const TString&		getType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				setSelected(bool select);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				isSelected() const;

	protected:

		/*
		@describe called when selected or de-selected
		@param 
		@return 
		*/
		virtual void				onSelection()	{}

		bool		mSelected;
	};//class EditorCommonTool



	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class BLADE_EDITOR_API EditorBrushTool : public EditorBaseTool
	{
	public:
		class ConfigString
		{
		public:
			static const BLADE_EDITOR_API TString CONFIG_NAME;
			static const BLADE_EDITOR_API TString BRUSH_SIZE;
			static const BLADE_EDITOR_API TString BRUSH_INTENSITY;
			static const BLADE_EDITOR_API TString BRUSH_SHAPE;
			static const BLADE_EDITOR_API TString BRUSH_COLOR;
			static const BLADE_EDITOR_API TString BRUSH_MATERIAL_NAME;
		};

		class BLADE_EDITOR_API BrushData
		{
		public:
			BrushData();
			~BrushData();

			/** @brief  */
			bool			loadBrushImage(const TString& path,size_t targetSize,size_t intensity);

			/** @brief  */
			bool			changeBrushData(size_t targetSize,size_t intensity);

			/** @brief  */
			const int32*	getBrushData() const	{return mData;}

			/** @brief  */
			size_t			getBrushSize() const	{return mSize;}

			/** @brief  */
			size_t			getIntensity() const	{return mIntensity;}

		protected:

			/** @brief  */
			void		releaseData();

			int32*	mData;
			HIMAGE	mImage;
			TString	mPath;
			size_t	mSize;
			size_t	mIntensity;
		};

	public:
		EditorBrushTool(const TString& name,IconIndex icon,HOTKEY hotkey,const TString& defaultBrushImage = TString::EMPTY,uint32 brushSize = 1,uint32 brushIntensity = 1);
		virtual ~EditorBrushTool();

		/************************************************************************/
		/* IEditorTool interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		@see Blade::EditorToolTypes
		*/
		virtual const TString&		getType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				setSelected(bool select);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				isSelected() const;

		/*
		@describe file switch notification, this usually happens when open/create a file,\n
		or user click the sub window tabs of the editor to switch to another sub window
		@param 
		@return 
		@note DO NOT keep any reference of the params 'prevFile' nor 'currentFile'
		you should delete any related data on prevFile when switch happens,
		because currently, there's no 'file close' notification yet.
		*/
		virtual void				onFileSwitch(const IEditorFile* prevFile,const IEditorFile* currentFile);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		size_t						getBrushSize() const;

		/** @brief  */
		size_t						getIntensity() const;

		/** @brief  */
		const Color&				getBrushColor() const;

		/** @brief  */
		const TString&				getBrushImage() const;

	protected:

		/** @brief  */
		virtual void				onEnable(bool bEnabled);

		/*
		@describe called when selected or de-selected
		@param 
		@return 
		*/
		virtual void				onSelection();

		//data change event trough UI/loading

		/** @brief  */
		virtual void				setBrushSize(void* data);

		/** @brief  */
		virtual void				setBrushIntensity(void* data);

		/** @brief  */
		virtual void				setBrushImage(void* data);

		/** @brief  */
		virtual void				setBrushColor(void* data);

		/** @brief calling when brush data changes. (image, intensity, size, etc.) */
		virtual void				onBrushDataChanged() {}

		/** @brief  */
		IBrushDecalEffect*			getBrushDecalEffect() const;

		uint32		mBrushSize;
		uint32		mBrushHeight;
		uint32		mIntensity;
		Color		mBrushColor;
		TString		mBrushImage;
		BrushData	mBrushData;
		HGRAPHICSEFFECT mBrushDecal;
		size_t		mBrushSizeMultiple;
		bool		mSelected;
	};//class EditorBrushTool
	

}//namespace Blade




#endif // __Blade_EditorToolTypes_h__