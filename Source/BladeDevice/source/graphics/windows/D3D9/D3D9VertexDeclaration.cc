/********************************************************************
	created:	2010/04/22
	filename: 	D3D9VertexDeclaration.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9TypeConverter.h>
#include <graphics/windows/D3D9/D3D9VertexDeclaration.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	D3D9VertexDeclaration::D3D9VertexDeclaration()
		:mD3D9VertexDecl(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	D3D9VertexDeclaration::~D3D9VertexDeclaration()
	{
		this->releaseD3D9Declaration();
	}

	/************************************************************************/
	/* IVertexDeclaration interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HVDECL					D3D9VertexDeclaration::clone() const
	{
		HVDECL decl(BLADE_NEW D3D9VertexDeclaration());
		for( VertexElementList::const_iterator iter = mElementList.begin(); iter != mElementList.end(); ++iter )
		{
			const VertexElement& elem = *iter;
			decl->addElement( elem.getSource(),elem.getOffset(),elem.getType(),elem.getUsage(),elem.getIndex() );
		}
		return decl;
	}

	/************************************************************************/
	/* custom interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IDirect3DVertexDeclaration9*	D3D9VertexDeclaration::getD3D9Declaration(IDirect3DDevice9* device) const
	{
		if( mD3D9VertexDecl != NULL )
			return mD3D9VertexDecl;

		TempBuffer tempbuffer;
		tempbuffer.reserve( (mElementList.size() + 2)*sizeof(D3DVERTEXELEMENT9)+4 );

		D3DVERTEXELEMENT9* d3d9elems = (D3DVERTEXELEMENT9*)tempbuffer.getData();

		int i = 0;
		uint16 Source = 0;
		uint16 lasRealSource = uint16(-1);
		
		for(VertexElementList::const_iterator iter = mElementList.begin(); iter != mElementList.end(); ++iter,++i)
		{
			const VertexElement& elem = *iter;
			//close gaps
			if( lasRealSource != elem.getSource() )
			{
				if( lasRealSource != uint16(-1) )
					++Source;
				lasRealSource = elem.getSource();
			}
		
			d3d9elems[i].Method = D3DDECLMETHOD_DEFAULT;
			d3d9elems[i].Offset = static_cast<WORD>(elem.getOffset());
			d3d9elems[i].Stream = Source;
			d3d9elems[i].Type	= (BYTE)(D3DDECLTYPE)D3DVertexDeclarationTypeConverter(elem.getType());
			d3d9elems[i].Usage	= (BYTE)(D3DDECLUSAGE)D3DVertexDeclarationUsageConverter(elem.getUsage());

			d3d9elems[i].UsageIndex = static_cast<BYTE>(elem.getIndex());
		}

		//end of elements
		d3d9elems[i].Stream = 0xFF;
		d3d9elems[i].Offset = 0;
		d3d9elems[i].Type = D3DDECLTYPE_UNUSED;
		d3d9elems[i].Method = 0;
		d3d9elems[i].Usage = 0;
		d3d9elems[i].UsageIndex = 0;

		HRESULT result = device->CreateVertexDeclaration(d3d9elems, &mD3D9VertexDecl);

		if( FAILED(result) )
			BLADE_EXCEPT(EXC_UNKNOWN,BTString("cannot create D3D9 vertex declaration:") + DXGetErrorString(result) );
		return mD3D9VertexDecl;
	}

	//////////////////////////////////////////////////////////////////////////
	void							D3D9VertexDeclaration::releaseD3D9Declaration()
	{
		if(mD3D9VertexDecl != NULL)
		{
			ULONG count = mD3D9VertexDecl->Release();
			assert(count == 0);
			BLADE_UNREFERENCED(count);
			mD3D9VertexDecl = NULL;
		}
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS