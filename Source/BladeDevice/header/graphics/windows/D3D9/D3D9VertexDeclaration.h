/********************************************************************
	created:	2010/04/22
	filename: 	D3D9VertexDeclaration.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#ifndef __Blade_D3D9VertexDeclaration_h__
#define __Blade_D3D9VertexDeclaration_h__
#include <graphics/windows/D3D9/D3D9Header.h>
#include <graphics/VertexDeclaration.h>


namespace Blade
{
	class D3D9VertexDeclaration : public VertexDeclaration , public Allocatable
	{
	public:
		D3D9VertexDeclaration();
		~D3D9VertexDeclaration();

		/************************************************************************/
		/* IVertexDeclaration interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual HVDECL					clone() const;

		/************************************************************************/
		/* VertexDeclaration interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void		invalidate()	{this->releaseD3D9Declaration();}

		/************************************************************************/
		/* custom interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		IDirect3DVertexDeclaration9*	getD3D9Declaration(IDirect3DDevice9* device) const;

		/*
		@describe 
		@param 
		@return 
		*/
		void							releaseD3D9Declaration();

	protected:

		mutable IDirect3DVertexDeclaration9*	mD3D9VertexDecl;
	};//class D3D9VertexDeclaration
	
}//namespace Blade


#endif //__Blade_D3D9VertexDeclaration_h__

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS