#ifndef WINIO_H
#define WINIO_H

 #include "winio_nt.h"
#include <windows.h>

#ifndef WINIO_DLL
#define WINIO_API _declspec(dllimport)
#else
#define WINIO_API 
#endif

extern "C"
{
  WINIO_API bool _stdcall InitializeWinIo();
  WINIO_API void _stdcall ShutdownWinIo();
  WINIO_API PBYTE _stdcall MapPhysToLin(tagPhysStruct &PhysStruct);
  WINIO_API bool _stdcall UnmapPhysicalMemory(tagPhysStruct &PhysStruct);
  WINIO_API bool _stdcall GetPhysLong(PBYTE pbPhysAddr, PDWORD pdwPhysVal);
  WINIO_API bool _stdcall SetPhysLong(PBYTE pbPhysAddr, DWORD dwPhysVal);
  WINIO_API bool _stdcall GetPortVal(WORD wPortAddr, PDWORD pdwPortVal, BYTE bSize);
  WINIO_API bool _stdcall SetPortVal(WORD wPortAddr, DWORD dwPortVal, BYTE bSize);
  WINIO_API bool _stdcall InstallWinIoDriver(PWSTR pszWinIoDriverPath, bool IsDemandLoaded = false);
  WINIO_API bool _stdcall RemoveWinIoDriver();
  //// 添加自定义函数 IT8786 系列的
  WINIO_API bool _stdcall IoWrite8(UINT8 Register, UINT8 Value);
  WINIO_API UINT8 _stdcall IoRead8(UINT8 Register);

  //// 是IT8786 使能
  WINIO_API bool _stdcall OpenSioDecode();
  WINIO_API bool _stdcall CloseSioDecode();
  WINIO_API bool _stdcall WriteIoRegister(UINT8 Register, UINT8 Value);
  WINIO_API UINT8 _stdcall ReadIoRegister(UINT8 Register);
  WINIO_API bool _stdcall ReWriteIoRegister(UINT8 Register, UINT8 andMask, UINT8 orMask);


}

extern HANDLE hDriver;
extern bool IsWinIoInitialized;
extern bool g_Is64BitOS;

bool _stdcall StartWinIoDriver();
bool _stdcall StopWinIoDriver();

#endif
