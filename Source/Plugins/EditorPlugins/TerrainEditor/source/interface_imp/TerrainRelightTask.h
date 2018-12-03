/********************************************************************
	created:	2011/09/06
	filename: 	TerrainRelightTask.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainRelightTask_h__
#define __Blade_TerrainRelightTask_h__
#include <TaskBase.h>
#include "TerrainEditable.h"
#include <utility/BladeContainer.h>

namespace Blade
{
	class TerrainRelightTool;

	class TerrainRelightTask : public TaskBase
	{
	public:
		TerrainRelightTask(TerrainRelightTool* toolBinding)
			:TaskBase(BTString("TerrainRelightTask"), TP_BACKGROUND)
			,mEditables(BLADE_TEMPCONT_INIT)
			,mToolBinding(toolBinding)
		{

		}

		~TerrainRelightTask()	{}

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe get task main device type on which it depends,such as CPU or GPU or HDD,etc.
		@param 
		@return 
		@remark this is useful for parallel optimizing \n
		for example,a physics task maybe on CPU,or on other device like GPU (PhysX)
		*/
		virtual const ITask::Type&	getType() const;

		/*
		@describe run the ITask
		@param 
		@return 
		*/
		virtual void			run();


		/*
		@describe run the ITask
		@param 
		@return 
		*/
		virtual void			update()	{}

		/*
		@describe run the ITask
		@param 
		@return 
		*/
		virtual void			onAccepted()	{}

		/*
		@describe
		@param
		@return
		*/
		virtual void			onRemoved();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		void	addTerrainForUpdate(TerrainEditable* te, const pint16 height, puint8 normal);


		/*
		@describe 
		@param 
		@return 
		*/
		void	setupRawVertexData(const Vector3& position,const Vector3& scale,const int16* height,Vector3* vertexData,size_t size,bool fullyUpdate);

	protected:

		/** @brief vertex normal */
		void	upateTerrainNormalVector(uint8* NormalBuffer, const Vector3* vertexData, size_t size, const Vector3** neighborVertex);

		/** @brief  */
		void	updateTerrainNormalMap(uint8* NormalBuffer, const Vector3* vertexData, size_t size);

		struct NormalData
		{
			TerrainEditable*	editable;
			int16 const* 		height;
			puint8				normal;
		};

		typedef TempVector<NormalData>	TerrainEditableList;

		TerrainEditableList	mEditables;

		TerrainRelightTool* mToolBinding;
	};//class TerrainRelightTask

	

}//namespace Blade



#endif // __Blade_TerrainRelightTask_h__