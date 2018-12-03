/********************************************************************
	created:	2017/02/24
	filename: 	ITerrainInterface.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ITerrainInterface_h__
#define __Blade_ITerrainInterface_h__
#include <interface/Interface.h>
#include "TerrainLayer.h"
#include "TerrainBlendData.h"

namespace Blade
{
	class ITerrainInterface : public Interface
	{
	public:

		/** @brief  */
		virtual void	initializeTerrain(size_t x, size_t z, const TString& resourcePath) = 0;

		/** @brief  */
		virtual bool	isModified() const = 0;

		/** @brief  */
		virtual pint16	getHeightBuffer() const = 0;

		/** @brief  */
		virtual puint8	getNormalBuffer() const = 0;

		/** @brief  */
		virtual puint8	getBlendBuffer() const = 0;

		/** @brief  */
		virtual TERRAIN_LAYER* getLayerBuffer() const = 0;

		/** @brief  */
		virtual const TStringList& getTextureList() const = 0;

		/** @brief  */
		virtual void	updateNormalBuffer() = 0;

		/** @brief  */
		virtual void	updateHeight(size_t x, size_t z, size_t sizeX, size_t sizeZ) = 0;

		/** @brief  */
		virtual void	updateBlockBlend(const TerrainBlockBlendData& blockBlend) = 0;
	};

	typedef InterfaceID<ITerrainInterface> IID_TERRAIN;
	
}//namespace Blade



#endif // __Blade_ITerrainInterface_h__