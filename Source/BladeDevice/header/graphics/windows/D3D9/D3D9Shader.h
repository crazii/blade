/********************************************************************
	created:	2014/12/24
	filename: 	D3D9Shader.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9Shader_h__
#define __Blade_D3D9Shader_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/Shader.h>
#include <graphics/windows/D3D9/D3D9Header.h>

namespace Blade
{
	class D3DShaderIntermediate;

	class D3D9Shader : public Shader, public Allocatable
	{
	public:
		D3D9Shader(IDirect3DVertexShader9* pVS, IDirect3DPixelShader9* pPS, D3DShaderIntermediate* intermedia);
		~D3D9Shader();

		/************************************************************************/
		/* IShader interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			getCustomOptions(EShaderType type, ParamList& option);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			getData(Buffer& data) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReady() const	{return mVertexShader != NULL && mPixelShader != NULL;}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		IDirect3DVertexShader9*	getVertexShader() const	{return mVertexShader;}
		/** @brief  */
		IDirect3DPixelShader9*	getPixelShader() const	{return mPixelShader;}

		/** @brief  */
		static bool	readShaderBinary(const void* intermediate, size_t interBytes,
			void*& vsData, size_t& vsBytes, void*& psData, size_t& psBytes);

		/** @brief  */
		static bool writeShaderBinary(Buffer& buffer, const void* vsData, size_t vsBytes, const void* psData, size_t psBytes);

	protected:
		IDirect3DVertexShader9*	mVertexShader;
		IDirect3DPixelShader9*	mPixelShader;
		DWORD					mCompilingFlags[SHADER_TYPE_COUNT];
	};

}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#endif //__Blade_D3D9Shader_h__