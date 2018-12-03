/********************************************************************
	created:	2014/12/24
	filename: 	ShaderIntermediate.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderIntermedia_h__
#define __Blade_ShaderIntermedia_h__
#include <interface/public/graphics/IShader.h>
#include <interface/public/graphics/ShaderInfoTable.h>
#include <utility/BladeContainer.h>
#include <utility/IOBuffer.h>

namespace Blade
{
	class ShaderIntermediate : public IShaderIntermediate
	{
	public:
		ShaderIntermediate(const TString& name);
		~ShaderIntermediate();

		/************************************************************************/
		/* IShaderIntermediate interface                                                                     */
		/************************************************************************/
		/** @brief get the source file string of the shader */
		virtual const TString&		getName() const	{return mName;}

		/** @brief  */
		virtual const TString&		getProfile(EShaderType type) const;

		/** @brief  */
		virtual const TString&		getEntry(EShaderType type) const;

		/** @brief  */
		virtual const void*	getData() const		{return mBuffer.getData();}

		/** @brief  */
		virtual size_t		getDataSize() const	{return mBuffer.getSize();}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void				setProfile(EShaderType type, const TString& profile);

		/** @brief  */
		void				setEntry(EShaderType type, const TString& entry);

		/** @brief  */
		TempBuffer&			getBuffer() const{return mBuffer;}

		/** @brief  */
		inline void			setBuffer(const void* data, size_t bytes)
		{
			mBuffer.clear();
			mBuffer.reserve(bytes);
			mBuffer.setSize(bytes);
			std::memcpy(mBuffer.getData(), data, bytes);
		}

	protected:
		TString			mName;
		TString			mProfile[SHADER_TYPE_COUNT];
		TString			mEntry[SHADER_TYPE_COUNT];
		mutable TempBuffer	mBuffer;
	};

}//namespace Blade

#endif//__Blade_ShaderIntermedia_h__