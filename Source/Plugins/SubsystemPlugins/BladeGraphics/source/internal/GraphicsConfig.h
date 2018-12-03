/********************************************************************
	created:	2018/05/21
	filename: 	GraphicsConfig.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsConfig_h__
#define __Blade_GraphicsConfig_h__

#if BLADE_DEBUG
#define MULTITHREAD_CULLING 0
#else
#define MULTITHREAD_CULLING 1
#endif

#define SHADOW_CULLING_OPTIMIZE 0

#endif//__Blade_GraphicsConfig_h__