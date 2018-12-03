/********************************************************************
	created:	2010/05/06
	filename: 	DefaultMaterial.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultMaterial_h__
#define __Blade_DefaultMaterial_h__
#include <Material.h>

namespace Blade
{

	class DefaultMaterial : public Material , public Allocatable
	{
	public:
		DefaultMaterial();
		DefaultMaterial(const TString& name);
		~DefaultMaterial();

		DefaultMaterial(const DefaultMaterial& src,const TString& name,int method);

		/************************************************************************/
		/* Material interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual HMATERIAL	clone(const TString& name,int method = MS_ALL) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void		removeAllTechniques()
		{
			return Material::removeAllTechniques();
		}

		/** @brief  */
		inline bool	isCloned() {return mCloned;}

		using Material::setTextureUniforms;
		
	protected:
		//hack:whether this material is a cloned copy of another. to skip resource unloading
		//TODO: better ways: i.e. add IResourceManager::addResource() for registsering cloned/manual resource
		bool	mCloned;
	private:
		friend class MaterialSerializer;
	};//class DefaultMaterial

	
}//namespace Blade


#endif //__Blade_DefaultMaterial_h__