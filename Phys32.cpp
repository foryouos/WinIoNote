// ---------------------------------------------------- //
//                      WinIo v3.0                      //
//				 Direct Hardware Access Under Windows	//
//           Copyright 1998-2010 Yariv Kaplan           //
//               http://www.internals.com 
//				对物理内存访问的相关函数               //
// ---------------------------------------------------- //

#include <windows.h>
#include <winioctl.h>
#include "phys32.h"
#include "./WinIoNoteSys/OpenLibSys.h"

#include "winio.h"


PBYTE _stdcall MapPhysToLin(tagPhysStruct &PhysStruct)
{
	PBYTE pbLinAddr = NULL;
	DWORD dwBytesReturned;

	if (!IsWinIoInitialized)
		return false;
	// 通Port32处的调用驱动的，使用IOCTL_WINIO_MAPPHYSTOLIN 控制代码 传入传出参数
	if (!DeviceIoControl(hDriver, IOCTL_WINIO_MAPPHYSTOLIN, &PhysStruct,
		sizeof(tagPhysStruct), &PhysStruct, sizeof(tagPhysStruct),
		&dwBytesReturned, NULL))
	{
		return NULL;
	}

	return (PBYTE)PhysStruct.pvPhysMemLin;
}


bool _stdcall UnmapPhysicalMemory(tagPhysStruct &PhysStruct)
{
	DWORD dwBytesReturned;

	if (!IsWinIoInitialized)
	{
		return false;
	}

	if (!DeviceIoControl(hDriver, IOCTL_WINIO_UNMAPPHYSADDR, &PhysStruct,
		sizeof(tagPhysStruct), NULL, 0, &dwBytesReturned, NULL))
	{
		return false;
	}

	return true;
}

// Support functions 通过调用 MapPhysToLin方法 来进行实现
bool _stdcall GetPhysLong(PBYTE pbPhysAddr, PDWORD pdwPhysVal)
{
	PDWORD pdwLinAddr;
	tagPhysStruct PhysStruct;

	if (!IsWinIoInitialized)
		return false;

	if (g_Is64BitOS)
	{
		PhysStruct.pvPhysAddress = (DWORD64)pbPhysAddr;
	}
	else
	{
		// Avoid sign extension issues
		PhysStruct.pvPhysAddress = (DWORD64)(DWORD32)pbPhysAddr;
	}

	PhysStruct.dwPhysMemSizeInBytes = 4;

	pdwLinAddr = (PDWORD)MapPhysToLin(PhysStruct);

	if (pdwLinAddr == NULL)
		return false;

	*pdwPhysVal = *pdwLinAddr;

	UnmapPhysicalMemory(PhysStruct);

	return true;
}


bool _stdcall SetPhysLong(PBYTE pbPhysAddr, DWORD dwPhysVal)
{
	PDWORD pdwLinAddr;
	tagPhysStruct PhysStruct;

	if (!IsWinIoInitialized)
		return false;

	if (g_Is64BitOS)
	{
		PhysStruct.pvPhysAddress = (DWORD64)pbPhysAddr;
	}
	else
	{
		// Avoid sign extension issues
		PhysStruct.pvPhysAddress = (DWORD64)(DWORD32)pbPhysAddr;
	}

	PhysStruct.dwPhysMemSizeInBytes = 4;

	pdwLinAddr = (PDWORD)MapPhysToLin(PhysStruct);

	if (pdwLinAddr == NULL)
		return false;

	*pdwLinAddr = dwPhysVal;

	UnmapPhysicalMemory(PhysStruct);

	return true;
}
