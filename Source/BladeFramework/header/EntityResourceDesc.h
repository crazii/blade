/********************************************************************
	created:	2013/04/12
	filename: 	EntityResourceDesc.h
	author:		Crazii
	
	purpose:	intermediate desc for loading entity/elements. Don't hold any copy of this type.
*********************************************************************/
#ifndef __Blade_EntityResourceDesc_h__
#define __Blade_EntityResourceDesc_h__
#include <BladeFramework.h>
#include <BladeBase.h>
#include <utility/String.h>
#include <Pimpl.h>

namespace Blade
{
	namespace Impl
	{
		class EntityResourceDescImpl;
	}//namespace Impl

	class BLADE_FRAMEWORK_API EntityResourceDesc : public NonAllocatable, public NonCopyable
	{
	public:
		EntityResourceDesc();
		EntityResourceDesc(const TString& resourcePath);
		~EntityResourceDesc();

		EntityResourceDesc& operator=(const EntityResourceDesc& rhs);

		/** @brief return the previous entity resource path */
		void			setPath(const TString& path);

		/** @brief  */
		const TString&	getPath() const;

		/*
		note: the entity resource may contain linkage to element resource,
		if entity resource is empty(could be on reload), then the element resource is loaded by this information
		otherwise the loader will choose this info when param 'force' is true, 
		or choose info in the entity resource when 'force' is false
		*/
		bool	addElementResource(const TString& elementName, const TString& elementReosurcePath, bool force = true);

		/** @brief return false if element with the 'elementName' does not exist */
		bool	getElementResource(const TString& elementName, TString& outPath, bool& outForce) const;

		/** @brief enumeration */
		size_t	getElementResourceCount() const;

		/** @brief  */
		bool	getElementResourceInfo(index_t index, TString& outElementName, TString& outPath, bool& outForce) const;

		/** @brief  */
		const TString* getElementName(index_t index) const;

	private:
		TString		mResourcePath;
	private:
		Pimpl<Impl::EntityResourceDescImpl>	mImpl;
	};//class ResourceDesc
	
}//namespace Blade

#endif//__Blade_EntityResourceDesc_h__