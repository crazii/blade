/********************************************************************
	created:	2011/05/07
	filename: 	EditorToolTypes.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <interface/public/EditorToolTypes.h>
#include <interface/IEditorFramework.h>

#include <interface/IResourceManager.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <ConfigTypes.h>
#include <databinding/DataSourceWriter.h>
#include <interface/IEventManager.h>
#include <interface/EditorEvents.h>
#include <BladeEditor_blang.h>


namespace Blade
{
	const TString EditorToolTypes::IMMEDIATE_EDIT_TYPE = BTString("ImmediateType");
	const TString EditorToolTypes::IMMEDIATE_STATUS_EDIT_TYPE = BTString("ImmediateMarkableType");
	const TString EditorToolTypes::COMMON_EDIT_TYPE = BTString("CommonType");
	const TString EditorToolTypes::BRUSH_EDIT_TYPE = BTString("BrushType");


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	EditorBaseTool::EditorBaseTool(const TString& name,IconIndex icon,HOTKEY hotkey)
		:mName(name)
		,mIconIndex(icon)
		,mHotkey(hotkey)
		,mEndabled(true)
	{
		mToolConfig.bind( BLADE_NEW ConfigGroup(name) );
		IEventManager::getSingleton().addEventHandler(EditableUnLoadEvent::NAME, EventDelegate(this, &EditorBaseTool::onEditableUnload));
	}

	//////////////////////////////////////////////////////////////////////////
	EditorBaseTool::~EditorBaseTool()
	{
		IEventManager::getSingleton().removeEventHandlers(EditableUnLoadEvent::NAME, this);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorBaseTool::setEnable(bool enabled)	
	{
		if( mEndabled != enabled )
		{
			this->onEnable(enabled);
			mEndabled = enabled;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditable*			EditorBaseTool::pickEditable(scalar x, scalar y, IEditorFile* file)
	{
		if( x < 0.0f || x > 1.0f || y < 0.0f || y > 1.0f )
			return NULL;
		if( file == NULL )
			return NULL;

		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		if( view == NULL || scene == NULL )
			return NULL;

		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if( camera == NULL )
			return NULL;

		ScreenQuery screenQuery(Box2(x,y,x,y), camera, view);
		IElement* elem = scene->queryNearestElement(screenQuery);
		if(elem != NULL)
		{
			IEntity* entity = elem->getEntity();
			ElementListParam list;
			EditorElement* edem = static_cast<EditorElement*>(entity->getElementBySystemType(EditorElement::EDITOR_ELEMENT_TYPE));
			if(edem != NULL)
				return edem->getEditable();
		}

		Ray ray = camera->getSpaceRayfromViewPoint(x,y);
		RaycastQuery rcq(ray);
		rcq.setPrecise(true);
		elem = scene->queryNearestElement(rcq);
		if(elem != NULL)
		{
			IEntity* entity = elem->getEntity();
			ElementListParam list;
			EditorElement* edem = static_cast<EditorElement*>(entity->getElementBySystemType(EditorElement::EDITOR_ELEMENT_TYPE));
			if(edem)
				return edem->getEditable();
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorBaseTool::pickPosition(scalar x, scalar y, AppFlag flag, IEditorFile* file, Ray& outRay, scalar& outDist)
	{
		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if (view == NULL || scene == NULL || camera == NULL)
		{
			assert(false);
			return false;
		}

		outRay = camera->getSpaceRayfromViewPoint(x, y);

		RaycastQuery rcq(outRay, FLT_MAX, PS_POSITIVE, flag);
		rcq.setPrecise(true);

		IElement* elem = scene->queryNearestElement(rcq);
		if (elem == NULL)
			return false;

		outDist = rcq[0].distance;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	POINT3				EditorBaseTool::pickPosition(scalar x, scalar y, AppFlag flag, float fallbackDist, IEditorFile* file)
	{
		IGraphicsView* view = file->getView();
		IGraphicsScene* scene = file->getGraphicsScene();
		IGraphicsCamera* camera = file->getDefaultCamera()->getCamera();
		if (view == NULL || scene == NULL || camera == NULL)
		{
			assert(false);
			return POINT3::ZERO;
		}

		scalar distance = fallbackDist;

		Ray ray = camera->getSpaceRayfromViewPoint(x, y);
		RaycastQuery rcq(ray, FLT_MAX, PS_POSITIVE, flag);
		rcq.setPrecise(true);
		IElement* elem = scene->queryNearestElement(rcq);
		if (elem != NULL)
			distance = rcq[0].distance;

		return rcq.getRay().getRayPoint(distance);
	}

	//////////////////////////////////////////////////////////////////////////
	void EditorBaseTool::onEditableUnload(const Event& data)
	{
		assert(data.mName == EditableUnLoadEvent::NAME);
		const EditableUnLoadEvent& evt = static_cast<const EditableUnLoadEvent&>(data);
		this->onEditableRemoved(evt.editable);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	const TString&		EditorCommandTool::getType() const
	{
		return EditorToolTypes::IMMEDIATE_EDIT_TYPE;
	}
	//////////////////////////////////////////////////////////////////////////
	bool				EditorCommandTool::setSelected(bool select)
	{
		if( select )
			this->doCommand();
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	const TString&		EditorStatusCommandTool::getType() const
	{
		return EditorToolTypes::IMMEDIATE_STATUS_EDIT_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorStatusCommandTool::setSelected(bool select)
	{
		if( select )
			this->doCommand();

		mSelected = select;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorStatusCommandTool::isSelected() const
	{
		return mSelected;
	}



	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	const TString&		EditorCommonTool::getType() const
	{
		return EditorToolTypes::COMMON_EDIT_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorCommonTool::setSelected(bool select)
	{
		if( mSelected !=select )
		{
			mSelected = select;
			this->onSelection();
		}
		
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorCommonTool::isSelected() const
	{
		return mSelected;
	}



	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	const TString EditorBrushTool::ConfigString::CONFIG_NAME = BXLang(BLANG_BRUSH_CONFIG);
	const TString EditorBrushTool::ConfigString::BRUSH_SIZE = BXLang(BLANG_BRUSH_SIZE);
	const TString EditorBrushTool::ConfigString::BRUSH_INTENSITY = BXLang(BLANG_BRUSH_INTENSITY);
	const TString EditorBrushTool::ConfigString::BRUSH_SHAPE = BXLang(BLANG_BRUSH_SHAPE);
	const TString EditorBrushTool::ConfigString::BRUSH_COLOR = BXLang(BLANG_BRUSH_COLOR);
	const TString EditorBrushTool::ConfigString::BRUSH_MATERIAL_NAME = BTString("editor_brush");


	//////////////////////////////////////////////////////////////////////////
	EditorBrushTool::BrushData::BrushData()
		:mData(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	EditorBrushTool::BrushData::~BrushData()
	{
		this->releaseData();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EditorBrushTool::BrushData::loadBrushImage(const TString& path, size_t targetSize, size_t intensity)
	{
		if( mImage != NULL && mData != NULL && mSize == targetSize && mIntensity == intensity && mPath == path )
			return false;

		this->releaseData();

		HSTREAM stream = IResourceManager::getSingleton().loadStream(path);
		mImage = IImageManager::getSingleton().loadDecompressedImage( stream, IP_DEFAULT, PF_ARGB, 1);

		if( mImage == NULL )
			return false;
		HIMAGE image = IImageManager::getSingleton().scaleImage( mImage, SIZE2I( (int)targetSize, (int)targetSize), IP_TEMPORARY, IImage::SF_BOX);

		if( image == NULL )
			return false;

		mData = BLADE_RES_ALLOCT(int32, targetSize*targetSize);
		if(mData == NULL )
			return false;

		int32* data = mData;
		int channel = -1;
		for(size_t i = 0; i < targetSize; ++i)
		{
			for(size_t j = 0; j < targetSize; ++j)
			{
				Color::RGBA color;
				bool ret = image->getPixelColor(i, j, color);
				assert(ret);
				BLADE_UNREFERENCED(ret);

				//find valid channel
				if( channel == -1 )
				{
					for(int k = 0; k < 3; ++k)
					{
						if( color.component[k] != 0)
						{
							channel = k;
							break;
						}
					}
				}

				if( channel != -1 )
					*(data+j) = (int32)( color.component[channel]*(intensity) );
				else
					*(data+j) = (int32)( color.r*(intensity) );	//any channel
			}
			data += targetSize;
		}

		mSize = targetSize;
		mIntensity = intensity;
		mPath = path;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		EditorBrushTool::BrushData::changeBrushData(size_t targetSize, size_t intensity)
	{
		if (mImage != NULL && mSize == targetSize && mIntensity == intensity)
			return false;

		this->releaseData();

		if( mImage == NULL )
			return false;

		HIMAGE image = IImageManager::getSingleton().scaleImage(mImage, SIZE2I( (int)targetSize, (int)targetSize), IP_TEMPORARY, IImage::SF_BOX);

		if( image == NULL )
			return false;

		mData = BLADE_RES_ALLOCT(int32, targetSize*targetSize);
		if(mData == NULL )
			return false;

		int32* data = mData;
		int channel = -1;
		for(size_t i = 0; i < targetSize; ++i)
		{
			for(size_t j = 0; j < targetSize; ++j)
			{
				Color::RGBA color;
				bool ret = image->getPixelColor(i,j,color);
				assert(ret);
				BLADE_UNREFERENCED(ret);

				//find valid channel
				if (channel == -1)
				{
					for (int k = 0; k < 3; ++k)
					{
						if (color.component[k] != 0)
						{
							channel = k;
							break;
						}
					}
				}

				if (channel != -1)
					*(data + j) = (int32)(color.component[channel] * (intensity));
				else
					*(data + j) = (int32)(color.r*(intensity));	//any channel
			}
			data += targetSize;
		}

		mSize = targetSize;
		mIntensity = intensity;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		EditorBrushTool::BrushData::releaseData()
	{
		if( mData != NULL )
		{
			BLADE_RES_FREE(mData);
			mData = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	EditorBrushTool::EditorBrushTool(const TString& name,IconIndex icon,HOTKEY hotkey,
		const TString& defaultBrushImage/* = TString::EMPTY*/,uint32 brushSize/* = 1*/,uint32 brushIntensity/* = 1*/)
		:EditorBaseTool(name,icon,hotkey)
		,mBrushSize(brushSize)
		,mBrushHeight(400)
		,mIntensity(brushIntensity)
		,mBrushColor(Color::RED)
		,mBrushSizeMultiple(1)
		,mSelected(false)
	{
		//add default config version
		DataSourceWriter<EditorBrushTool> writer(mToolConfig);

		writer.beginVersion( Version(0,1) );
		writer << ConfigString::BRUSH_SIZE  << &EditorBrushTool::setBrushSize << ConfigAtom::Range(2u, 128u, 1u) << &EditorBrushTool::mBrushSize;
		writer << ConfigString::BRUSH_INTENSITY << ConfigAtom::Range(1u, 128u, 1u) << &EditorBrushTool::setBrushIntensity << &EditorBrushTool::mIntensity;
		
		const TString& editorResPath = IEditorFramework::EDITOR_RESOURCE_PATH;
		TString brushImage;
		if( defaultBrushImage == TString::EMPTY )
			brushImage = BTString("/brushes/fade_circle.png");
		else
			brushImage = defaultBrushImage;
		mBrushImage = editorResPath + brushImage;

		writer << ConfigString::BRUSH_SHAPE << CONFIG_UIHINT(CUIH_IMAGE,IEditorFramework::MEDIA_TYPE_BRUSH) 
			<< &EditorBrushTool::setBrushImage << &EditorBrushTool::mBrushImage;

		writer << ConfigString::BRUSH_COLOR << &EditorBrushTool::setBrushColor << &EditorBrushTool::mBrushColor;
		writer.endVersion();

		mToolConfig->bindTarget(this);
	}

	//////////////////////////////////////////////////////////////////////////
	EditorBrushTool::~EditorBrushTool()
	{
		
	}

	/************************************************************************/
	/* IEditorTool interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&		EditorBrushTool::getType() const
	{
		return EditorToolTypes::BRUSH_EDIT_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorBrushTool::setSelected(bool select)
	{
		if( mSelected !=select )
		{
			mSelected = select;
			this->onSelection();
			IBrushDecalEffect* effect = this->getBrushDecalEffect();
			if( effect != NULL )
				effect->setVisible(mSelected);	
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				EditorBrushTool::isSelected() const
	{
		return mSelected;
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::onFileSwitch(const IEditorFile* /*prevFile*/,const IEditorFile* currentFile)
	{
		//BLADE_LW_PROFILING_FUNCTION();
		mBrushDecal.clear();

		if( this->isEnable() && currentFile != NULL && currentFile->getGraphicsScene() != NULL)
		{
			mBrushDecal = currentFile->getGraphicsScene()->getEffectManager()->createBrushDecal((scalar)mBrushSize, (scalar)mBrushHeight, mBrushImage);
			IBrushDecalEffect* effect = this->getBrushDecalEffect();
			effect->setVisible(mSelected);
			effect->setColor(mBrushColor);
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t				EditorBrushTool::getBrushSize() const
	{
		return mBrushSize;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				EditorBrushTool::getIntensity() const
	{
		return mIntensity;
	}

	//////////////////////////////////////////////////////////////////////////
	const Color&		EditorBrushTool::getBrushColor() const
	{
		return mBrushColor;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		EditorBrushTool::getBrushImage() const
	{
		return mBrushImage;
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::onEnable(bool bEnabled)
	{
		if(bEnabled)
		{
			if (mBrushData.loadBrushImage(mBrushImage, mBrushSize*mBrushSizeMultiple, mIntensity))
				this->onBrushDataChanged();

			IBrushDecalEffect* effect = this->getBrushDecalEffect();
			if(effect != NULL)
				effect->setVisible(mSelected);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::onSelection()
	{
		if( this->isSelected() )
		{
			const Color& cr = mToolConfig->getSubConfigByName( ConfigString::BRUSH_COLOR )->getBinaryValue();
			IBrushDecalEffect* effect = this->getBrushDecalEffect();
			if( effect != NULL )
			{
				effect->setColor(cr);
				effect->setSize((scalar)mBrushSize-1);
			}

			if (mBrushData.loadBrushImage(mBrushImage, mBrushSize*mBrushSizeMultiple, mIntensity))
				this->onBrushDataChanged();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::setBrushSize(void* data)
	{
		assert(data == &mBrushSize);
		BLADE_UNREFERENCED(data);
		if( mBrushData.changeBrushData(mBrushSize*mBrushSizeMultiple, mIntensity) )
			this->onBrushDataChanged();

		IBrushDecalEffect* effect = this->getBrushDecalEffect();
		if( effect != NULL )
			effect->setSize((scalar)mBrushSize-1);	//count border in: i.e 1x1 cell has 2x2 data, in each corner
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::setBrushIntensity(void* data)
	{
		assert(data == &mIntensity);
		BLADE_UNREFERENCED(data);
		if( mBrushData.changeBrushData(mBrushSize*mBrushSizeMultiple, mIntensity) )
			this->onBrushDataChanged();
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::setBrushImage(void* data)
	{
		assert( data == &mBrushImage );
		BLADE_UNREFERENCED(data);

		if( mBrushData.loadBrushImage(mBrushImage,mBrushSize*mBrushSizeMultiple,mIntensity) )
			this->onBrushDataChanged();

		IBrushDecalEffect* effect = this->getBrushDecalEffect();
		if( effect != NULL )
			effect->setDecalImage(mBrushImage);
	}

	//////////////////////////////////////////////////////////////////////////
	void				EditorBrushTool::setBrushColor(void* data)
	{
		assert(data == &mBrushColor);
		BLADE_UNREFERENCED(data);
		IBrushDecalEffect* effect = this->getBrushDecalEffect();
		if( effect != NULL )
			effect->setColor(mBrushColor);
	}

	//////////////////////////////////////////////////////////////////////////
	IBrushDecalEffect*			EditorBrushTool::getBrushDecalEffect() const
	{
		return static_cast<IBrushDecalEffect*>(mBrushDecal);
	}


}//namespace Blade