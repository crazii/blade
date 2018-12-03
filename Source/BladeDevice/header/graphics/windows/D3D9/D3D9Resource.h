/********************************************************************
	created:	2010/04/14
	filename: 	D3D9Resource.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9Resource_h__
#define __Blade_D3D9Resource_h__
#include <BladePlatform.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>
#include <interface/InterfaceSingleton.h>


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class ID3D9Resource
	{
	public:
		virtual ~ID3D9Resource()	{}
		/** @brief  */
		virtual void release() = 0;
		/** @brief  */
		virtual void rebuild(IDirect3DDevice9* device)	= 0;

	};//class ID3DResource

	class ID3D9TextureResource
	{
	public:
		virtual IDirect3DBaseTexture9* getTexture() = 0;
	};

	class ID3D9LostResourceManager : public InterfaceSingleton<ID3D9LostResourceManager>
	{
	public:
		virtual ~ID3D9LostResourceManager()	{}

		/** @brief  */
		virtual void	registerDefaultResource(ID3D9Resource* resource) = 0;

		/** @brief  */
		virtual void	unRegisterDefaultResource(ID3D9Resource* resource) = 0;

		/** @brief */
		virtual void	releaseDefaultResource() = 0;

		/** @brief  */
		virtual void	rebuildDefaultResource() = 0;

		/** @brief  */
		virtual void	setD3D9Device(IDirect3DDevice9* device, IDirect3D9* d3d9, UINT adapter) = 0;

	};//class D3D9LostResourceManager


	template <typename T>
	class D3D9ResourceTraits
	{
	private:
		D3D9ResourceTraits();
		~D3D9ResourceTraits();
	public:
		typedef void* DESC_TYPE;

		/** @brief  */
		static void getD3D9Desc(T* resouce,DESC_TYPE* desc)	{assert(false);}

		static bool isInDefaultPool(const DESC_TYPE& desc){assert(false);return false;}
	};

	template<>
	class D3D9ResourceTraits<IDirect3DSurface9>
	{
	public:
		typedef D3DSURFACE_DESC DESC_TYPE;

		static void getD3D9Desc(IDirect3DSurface9* resource,D3DSURFACE_DESC* desc)
		{
			assert( resource != NULL );
			resource->GetDesc(desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			return desc.Pool == D3DPOOL_DEFAULT;
		}
	};

	template<>
	class D3D9ResourceTraits<IDirect3DTexture9>
	{
	public:
		typedef D3DSURFACE_DESC DESC_TYPE;

		static void getD3D9Desc(IDirect3DTexture9* resource,D3DSURFACE_DESC* desc)
		{
			assert( resource != NULL );
			resource->GetLevelDesc(0,desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			return desc.Pool == D3DPOOL_DEFAULT;
		}
	};

	template<>
	class D3D9ResourceTraits<IDirect3DVolumeTexture9>
	{
	public:
		typedef D3DVOLUME_DESC DESC_TYPE;

		static void getD3D9Desc(IDirect3DVolumeTexture9* resource,DESC_TYPE* desc)
		{
			assert( resource != NULL );
			resource->GetLevelDesc(0,desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			return desc.Pool == D3DPOOL_DEFAULT;
		}

	};

	template<>
	class D3D9ResourceTraits<IDirect3DCubeTexture9>
	{
	public:
		typedef D3DSURFACE_DESC DESC_TYPE;

		static void getD3D9Desc(IDirect3DCubeTexture9* resource,DESC_TYPE* desc)
		{
			assert( resource != NULL );
			resource->GetLevelDesc(0,desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			return desc.Pool == D3DPOOL_DEFAULT;
		}

	};

	template<>
	class D3D9ResourceTraits<IDirect3DIndexBuffer9>
	{
	public:
		typedef D3DINDEXBUFFER_DESC DESC_TYPE;

		static void getD3D9Desc(IDirect3DIndexBuffer9* resource,D3DINDEXBUFFER_DESC* desc)
		{
			assert( resource != NULL );
			resource->GetDesc(desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			return desc.Pool == D3DPOOL_DEFAULT;
		}

	};


	template<>
	class D3D9ResourceTraits<IDirect3DVertexBuffer9>
	{
	public:
		typedef D3DVERTEXBUFFER_DESC DESC_TYPE;

		static void getD3D9Desc(IDirect3DVertexBuffer9* resource,D3DVERTEXBUFFER_DESC* desc)
		{
			assert( resource != NULL );
			resource->GetDesc(desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			return desc.Pool == D3DPOOL_DEFAULT;
		}

	};


	template<>
	class D3D9ResourceTraits<IDirect3DSwapChain9>
	{
	public:
		typedef D3DPRESENT_PARAMETERS DESC_TYPE;

		static void getD3D9Desc(IDirect3DSwapChain9* resource,D3DPRESENT_PARAMETERS* desc)
		{
			assert( resource != NULL );
			resource->GetPresentParameters(desc);
		}

		static bool isInDefaultPool(const DESC_TYPE& desc)
		{
			BLADE_UNREFERENCED(desc);

			return true;
		}
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/


	//////////////////////////////////////////////////////////////////////////
	template < typename T >
	class D3D9Resource : public ID3D9Resource
	{
	public:
		//typedef IDirect3DResource9 ElemType;
		//now using IUnknown because swap chain is not a resource,well..
		typedef IUnknown ElemType;

		typedef typename D3D9ResourceTraits<T>::DESC_TYPE DESC_TYPE;

		//////////////////////////////////////////////////////////////////////////
		D3D9Resource(T* resource)
			:mResource(NULL)
		{
			assert( resource );
			std::memset(&mDX9Desc, 0, sizeof(mDX9Desc) );

			mResource = static_cast<ElemType*>(resource);
			D3D9ResourceTraits<T>::getD3D9Desc(resource,&mDX9Desc);

			if( this->isInDefaultPool() )
				ID3D9LostResourceManager::getSingleton().registerDefaultResource(this);			
		}

		//////////////////////////////////////////////////////////////////////////
		virtual ~D3D9Resource()
		{
			if( this->isInDefaultPool() )
			{
				ID3D9LostResourceManager::getSingleton().unRegisterDefaultResource(this);
			}
			this->release();
		}

		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		T*					getD3D9Resource() const
		{
			return static_cast<T*>(mResource);
		}

		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		void				setD3D9Resource(T* res)
		{
			this->release();
			this->onSet(res);

			bool oldPooled = this->isInDefaultPool();
			mResource = static_cast<ElemType*>(res);
			if(mResource != NULL )
				D3D9ResourceTraits<T>::getD3D9Desc(res,&mDX9Desc);	

			if(oldPooled != this->isInDefaultPool() )
			{
				if(oldPooled)
					ID3D9LostResourceManager::getSingleton().unRegisterDefaultResource(this);
				else
					ID3D9LostResourceManager::getSingleton().registerDefaultResource(this);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		bool isInDefaultPool() const
		{
			return D3D9ResourceTraits<T>::isInDefaultPool( this->getD3D9Desc() );
		}

		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		const DESC_TYPE&	getD3D9Desc() const
		{
			return mDX9Desc;
		}

		//////////////////////////////////////////////////////////////////////////
		/* @brief */
		void				setD3D9Desc(const DESC_TYPE& desc)
		{
			mDX9Desc = desc;
		}

		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		virtual void		release()
		{
			if( mResource != NULL )
			{
				ULONG count = mResource->Release();
				BLADE_UNREFERENCED(count);
				mResource = NULL;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		/** @brief  */
		virtual void		rebuild(IDirect3DDevice9* device)
		{
			if( this->isInDefaultPool() )
				this->rebuildImpl(device);
		}

	protected:
		/** @brief  */
		/* @remark it is the sub class's duty to maintain the resource (re-create it still in default pool)  */
		virtual void		rebuildImpl(IDirect3DDevice9* device) = 0;

		/** @brief  */
		virtual	void		onSet(T* source) {	BLADE_UNREFERENCED(source);	};

	private:
		ElemType*			mResource;

		DESC_TYPE			mDX9Desc;

	};//class D3D9Resource

}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif //__Blade_D3D9Resource_h__