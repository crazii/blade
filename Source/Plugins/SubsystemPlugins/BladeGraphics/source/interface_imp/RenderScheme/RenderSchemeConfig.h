/********************************************************************
	created:	2017/07/12
	filename: 	RenderSchemeConfig.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderSchemeConfig_h__
#define __Blade_RenderSchemeConfig_h__
#include <utility/Bindable.h>
#include <Singleton.h>
#include <interface/IConfig.h>
#include <GraphicsShaderShared.inl>

namespace Blade
{
	class RenderSchemeConfig : public Bindable, public Singleton<RenderSchemeConfig>
	{
	public:
		RenderSchemeConfig() {}
		~RenderSchemeConfig() {}

		/** @brief  */
		void initConfig();

		/** @brief  */
		const HCONFIG&	getOperationConfig(const TString& typeName) const;

		/** @brief  */
		const HCONFIG&	getIFXConfig(const TString& typeName) const;

	protected:
		/** @brief  */
		void onOperationConfigChanged(void* data);
		/** @brief  */
		void onIFXConfigChanged(void* data);

		uint32 mPadding;	//used for delegate to work
	};
	
}//namespace Blade

#endif//__Blade_RenderSchemeConfig_h__