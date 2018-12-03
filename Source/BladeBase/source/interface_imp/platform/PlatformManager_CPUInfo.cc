/********************************************************************
	created:	2013/12/25
	filename: 	PlatformInfo.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	undef TEXT
#	include <Windows.h>
#endif
#include "PlatformManager.h"

using namespace Blade;

#if BLADE_PROCESSOR == BLADE_PROCESSOR_X86 || BLADE_PROCESSOR == BLADE_PROCESSOR_IA64

	union UCPUInfo
	{
		int CPUinfo[4];
		struct
		{
			int _EAX;
			int _EBX;
			int _ECX;
			int _EDX;
		}reg;
	};//struct SCPUInfo

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		include <intrin.h>
#		pragma warning(push)
#		pragma warning(disable: 4035)  // no return value

	//////////////////////////////////////////////////////////////////////////
	//check if the CPU has a CPUID instruction by test flag register's #ID flag (bit 21)
	static int isCPUIDsupported(void)
	{
#		if defined(_M_X64) || defined(_M_IA64)
		return true;
#		else
		// If we can modify flag register bit 21, the cpu is supports CPUID instruction
		__asm
		{
			// Read EFLAG
			pushfd
				pop     eax
				mov     ecx, eax

				// Modify bit 21
				xor     eax, 0x200000
				push    eax
				popfd

				// Read back EFLAG
				pushfd
				pop     eax

				// Restore EFLAG
				push    ecx
				popfd

				// Check bit 21 modifiable
				xor     eax, ecx
				neg     eax
				sbb     eax, eax

				//return : EAX
		}
#		endif
	}

	//////////////////////////////////////////////////////////////////////////
	//call the CPUID instruction
	static uint queryCPUID(int query, UCPUInfo& uinfo)
	{
		__cpuid(uinfo.CPUinfo, query);
		return (uint)uinfo.reg._EAX;
	}

	//////////////////////////////////////////////////////////////////////////
	//get the processors count
	static size_t getNumOfProcessors()
	{
		UCPUInfo uinfo;
		queryCPUID(1,uinfo);
		return size_t((uinfo.reg._EBX >> 16)&0x7F);
	}

	//////////////////////////////////////////////////////////////////////////
	//read simd support and CPU brand string
	static void readCPUInfomation(CPUINFO& outInfo)
	{
		outInfo.mProcessorCount = getNumOfProcessors();
		outInfo.mHardwareFP = true;					//x86 has FPU (ignore old odd 8086/286)
		CPUINFO::SIMDIS& simd = outInfo.mSIMD;
		TString& cpuTString = outInfo.mCPUString;

		if( isCPUIDsupported() )
		{
			UCPUInfo uinfo;
			//get the CPU id SIMD
			uint result = queryCPUID(1,uinfo);
			assert(result != 0);

			const int maskSSE41 = 1 << 19;
			const int maskSSE42 = 1 << 20;
			const int maskSSE3 = 1;
			if( uinfo.CPUinfo[2] & maskSSE42 )
				simd = CPUINFO::SIMDS_SSE42;
			else if( uinfo.CPUinfo[2] & maskSSE41 )
				simd = CPUINFO::SIMDS_SSE41;
			else
				//SSE3 feature bit is at data index 2 ,bit 0
				if( uinfo.CPUinfo[2] & maskSSE3 )
					simd = CPUINFO::SIMDS_SSE3;
				else
				{
					const int maskMMX = 1 << 23;
					const int maskSSE = 1 << 25;
					const int maskSSE2 = 1 << 26;

					//data index 3
					if( uinfo.CPUinfo[3] & maskSSE2 )
						simd = CPUINFO::SIMDS_SSE2;
					else if( uinfo.CPUinfo[3] & maskSSE )
						simd = CPUINFO::SIMDS_SSE;
					else if( uinfo.CPUinfo[3] & maskMMX )
						simd = CPUINFO::SIMDS_MMX;
					else
						simd = CPUINFO::SIMDS_NONE;
				}

				//////////////////////////////////////////////////////////////////////////
				//second read the CPU brand string
				result = queryCPUID(0,uinfo);
				assert(result != 0);

				uint nExIds;
				char CPUString[0x20];
				char CPUBrandString[0x40];

				std::memset(CPUString, 0, sizeof(CPUString));
				std::memset(CPUBrandString, 0, sizeof(CPUBrandString));

				*((int*)CPUString) = uinfo.reg._EBX;
				*((int*)(CPUString+4)) = uinfo.reg._EDX;
				*((int*)(CPUString+8)) = uinfo.reg._ECX;

				StringConcat cpubrand( CPUString );

				// Calling CPUID with 0x80000000
				// gets the number of valid extended IDs.
				nExIds = queryCPUID(0x80000000, uinfo);

				for (uint i=0x80000000; i<= nExIds; ++i)
				{
					queryCPUID((int)i, uinfo);

					// Interpret CPU brand string and cache information.
					if  (i == 0x80000002)
					{
						std::memcpy(CPUBrandString + 0, &uinfo.reg._EAX, sizeof(uinfo.reg._EAX));
						std::memcpy(CPUBrandString + 4, &uinfo.reg._EBX, sizeof(uinfo.reg._EBX));
						std::memcpy(CPUBrandString + 8, &uinfo.reg._ECX, sizeof(uinfo.reg._ECX));
						std::memcpy(CPUBrandString + 12, &uinfo.reg._EDX, sizeof(uinfo.reg._EDX));
					}
					else if  (i == 0x80000003)
					{
						std::memcpy(CPUBrandString + 16 + 0, &uinfo.reg._EAX, sizeof(uinfo.reg._EAX));
						std::memcpy(CPUBrandString + 16 + 4, &uinfo.reg._EBX, sizeof(uinfo.reg._EBX));
						std::memcpy(CPUBrandString + 16 + 8, &uinfo.reg._ECX, sizeof(uinfo.reg._ECX));
						std::memcpy(CPUBrandString + 16 + 12, &uinfo.reg._EDX, sizeof(uinfo.reg._EDX));
					}
					else if  (i == 0x80000004)
					{
						std::memcpy(CPUBrandString + 32 + 0, &uinfo.reg._EAX, sizeof(uinfo.reg._EAX));
						std::memcpy(CPUBrandString + 32 + 4, &uinfo.reg._EBX, sizeof(uinfo.reg._EBX));
						std::memcpy(CPUBrandString + 32 + 8, &uinfo.reg._ECX, sizeof(uinfo.reg._ECX));
						std::memcpy(CPUBrandString + 32 + 12, &uinfo.reg._EDX, sizeof(uinfo.reg._EDX));
					}
				}
				cpuTString = StringConverter::StringToTString( cpubrand + "-" + StringHelper::trim(BString(CPUBrandString)) );
		}
		else
		{
			simd = CPUINFO::SIMDS_NONE;
			cpuTString = BTString("Unknown");
		}
	}

#	pragma warning(pop)

#else

#error "not implemented"

#endif

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#	include "android/ACpuFeatures.h"

static void readCPUInfomation(CPUINFO& outInfo)
{
		outInfo.mProcessorCount = (size_t)android_getCpuCount();
		uint64_t feature = android_getCpuFeatures();

		if( (feature&ANDROID_CPU_ARM_FEATURE_NEON) )
			outInfo.mSIMD = CPUINFO::SIMDS_NEON;
		else if( (feature&ANDROID_CPU_X86_FEATURE_SSSE3) )
			outInfo.mSIMD = CPUINFO::SIMDS_SSE3;
		else
			outInfo.mSIMD = CPUINFO::SIMDS_NONE;

#	if BLADE_PROCESSOR == BLADE_PROCESSOR_X86

		outInfo.mHardwareFP = true;

#	elif BLADE_PROCESSOR == BLADE_PROCESSOR_ARM

	if( (feature&(ANDROID_CPU_ARM_FEATURE_NEON
		|ANDROID_CPU_ARM_FEATURE_VFPv3|ANDROID_CPU_ARM_FEATURE_VFPv2
		|ANDROID_CPU_ARM_FEATURE_VFP_D32|ANDROID_CPU_ARM_FEATURE_VFP_FP16)) )
	{
		outInfo.mHardwareFP = true;
	}

	outInfo.mCPUString = android_getCpuString();

#	else
#	error not implemented.
#	endif
	}

#endif


static size_t getCPUCacheLineSize();

#if BLADE_PLATFORM == BLADE_PLATFORM_MAC || BLADE_PLATFORM == BLADE_PLATFORM_IOS

#include <sys/sysctl.h>
static size_t getCPUCacheLineSize()
{
	size_t line_size = 0;
	size_t sizeof_line_size = sizeof(line_size);
	sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
	return line_size;
}

#elif BLADE_IS_WINDOWS_CLASS_SYSTEM

static size_t getCPUCacheLineSize()
{
	size_t line_size = 0;
	DWORD buffer_size = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = 0;

	::GetLogicalProcessorInformation(0, &buffer_size);
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)BLADE_TMP_ALLOC(buffer_size);
	::GetLogicalProcessorInformation(&buffer[0], &buffer_size);

	for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
		if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
			line_size = buffer[i].Cache.LineSize;
			break;
		}
	}

	BLADE_TMP_FREE(buffer);
	return line_size;
}

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID || BLADE_PLATFORM == BLADE_PLATFORM_LINUX

#include <stdio.h>
static size_t getCPUCacheLineSize() 
{
	FILE* p = 0;
	p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
	unsigned int i = 16;
	if(p != NULL ) {
		fscanf(p, "%d", &i);
		fclose(p);
	}
	return i;
}

#else
#	error un supported platform.
#endif



namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	void		PlatformManager::initCPUInfo(CPUINFO& outCpuInfo)
	{
		readCPUInfomation(outCpuInfo);
		outCpuInfo.mCacheLineSize = getCPUCacheLineSize();
	}
	
}//namespace Blade