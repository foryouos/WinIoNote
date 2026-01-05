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
  // 添加自定义函数 IT8786 系列的
  WINIO_API bool _stdcall IoWrite8(UINT8 Register, UINT8 Value);
  WINIO_API UINT8 _stdcall IoRead8(UINT8 Register);

  // 使 ITE  使能
  WINIO_API bool _stdcall OpenSioITEDecode();
  WINIO_API bool _stdcall CloseSioITEDecode();
  // 操作SUPERUI 单个寄存器的 单个Bit值
  // 参数：读取Port 读取的 bit位
  // 返回值：返回bit位是 0还是 1
  WINIO_API int _stdcall IoRead8_Single_Bit(WORD port, int bit);
  // 参数 Port 写入Bit 写入值0/1
  WINIO_API bool _stdcall IoWrite8_Single_Bit(WORD addr, int bit, int bit_value);
  // 访问SumBus函数封装于此
  WINIO_API bool _stdcall SmbusWriteByte(WORD slav_address, WORD offset_address, WORD write_data);
  WINIO_API bool _stdcall SmbusReadByte(WORD slav_address, WORD offset_address, WORD* read_data);
}

extern HANDLE hDriver;
extern bool IsWinIoInitialized;
extern bool g_Is64BitOS;

bool _stdcall StartWinIoDriver();
bool _stdcall StopWinIoDriver();

#endif
