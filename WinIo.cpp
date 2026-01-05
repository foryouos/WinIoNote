// ---------------------------------------------------- //
//                      WinIo v3.0                      //
//				 Direct Hardware Access Under Windows	//
//           Copyright 1998-2010 Yariv Kaplan           //
//               http://www.internals.com 
//				初始化与 加载WinIO驱动的相关函数实现      //
// ---------------------------------------------------- //

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winioctl.h>
#include "phys32.h"
// 加载驱动头文件
#include "./WinIoNoteSys/OpenLibSys.h"
#include "winio.h"


HANDLE hDriver = INVALID_HANDLE_VALUE;
bool IsWinIoInitialized = false;
wchar_t szWinIoDriverPath[32768];
bool g_Is64BitOS;


typedef UINT (WINAPI* GETSYSTEMWOW64DIRECTORY)(LPTSTR, UINT);

BOOL Is64BitOS()
{
#ifdef _WIN64
	return TRUE;
#else
	GETSYSTEMWOW64DIRECTORY getSystemWow64Directory;
	HMODULE hKernel32;
	TCHAR Wow64Directory[32767];

	hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
	if (hKernel32 == NULL)
	{
		//
		// This shouldn't happen, but if we can't get 
		// kernel32's module handle then assume we are 
		// on x86. We won't ever install 32-bit drivers
		// on 64-bit machines, we just want to catch it 
		// up front to give users a better error message.
		//
		return FALSE;
	}

	getSystemWow64Directory = (GETSYSTEMWOW64DIRECTORY)GetProcAddress(hKernel32, "GetSystemWow64DirectoryW");

	if (getSystemWow64Directory == NULL)
	{
		//
		// This most likely means we are running 
		// on Windows 2000, which didn't have this API 
		// and didn't have a 64-bit counterpart.
		//
		return FALSE;
	}

	if ((getSystemWow64Directory(Wow64Directory, _countof(Wow64Directory)) == 0) &&
		(GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)) {
			return FALSE;
	}

	//
	// GetSystemWow64Directory succeeded 
	// so we are on a 64-bit OS.
	//
	return TRUE;
#endif
}


bool GetDriverPath()
{
	PWSTR pszSlash;

	//if (!GetModuleFileName(GetModuleHandle(NULL), szWinIoDriverPath, sizeof(szWinIoDriverPath)))
		
	if (!GetModuleFileName(GetModuleHandle(NULL), szWinIoDriverPath, sizeof(szWinIoDriverPath) / sizeof(szWinIoDriverPath[0])))
		return false;
	pszSlash = wcsrchr(szWinIoDriverPath, '\\');

	if (pszSlash)
		pszSlash[1] = 0;
	else
		return false;
	//定义的驱动文件的名称，在此处写时了，也就意味着，sys驱动文件必须为如下名称
	// 如果需要请更改如下 sys文件名称，并修改加载时的同级文件名称
	if (g_Is64BitOS)
		wcscat(szWinIoDriverPath, L"WinIoNote64.sys");
	else
		wcscat(szWinIoDriverPath, L"WinIoNote64.sys");

	return true;
}


bool __stdcall InitializeWinIo()
{
	bool bResult;
	DWORD dwBytesReturned;

	g_Is64BitOS = Is64BitOS();

	hDriver = CreateFile(L"\\\\.\\WinIoNote",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	// If the driver is not running, install it

	if (hDriver == INVALID_HANDLE_VALUE)
	{		
		// 获取驱动文件路径
		GetDriverPath();
		// 安装RealBomIO驱动服务
		bResult = InstallWinIoDriver(szWinIoDriverPath, true);

		if (!bResult)
			return false;
		// 开始RealBomIO驱动
		bResult = StartWinIoDriver();

		if (!bResult)
			return false;
		// 打开设备驱动程序
		hDriver = CreateFile(L"\\\\.\\WinIoNote", //驱动程序路径
			GENERIC_READ | GENERIC_WRITE, // 访问权限 ：读写
			FILE_SHARE_READ | FILE_SHARE_WRITE, //共享读写权限运行其它进程打开该设备共享权限 
			NULL,
			OPEN_EXISTING, //如果设备已存在则打开它
			FILE_ATTRIBUTE_NORMAL, //文件属性正常，没有特殊属性
			NULL);

		if (hDriver == INVALID_HANDLE_VALUE)
			return false;
	}

	// Enable I/O port access for this process if running on a 32 bit OS

	if (!g_Is64BitOS) //如果为32位系统
	{
		//启用直接 I/O 端口访问的操作
		//dwBytesReturned 接受操作后返回的字节数的指针
		if (!DeviceIoControl(hDriver, IOCTL_WINIO_ENABLEDIRECTIO, NULL,
			0, NULL, 0, &dwBytesReturned, NULL))
		{
			return false;
		}
	}

	IsWinIoInitialized = true;

	return true;
}


void _stdcall ShutdownWinIo()
{
	DWORD dwBytesReturned;

	if (hDriver != INVALID_HANDLE_VALUE)
	{
		// Disable I/O port access if running on a 32 bit OS

		if (!g_Is64BitOS)
		{
			DeviceIoControl(hDriver, IOCTL_WINIO_DISABLEDIRECTIO, NULL,
				0, NULL, 0, &dwBytesReturned, NULL);
		}

		CloseHandle(hDriver);

	}
	//移除WinIO驱动
	RemoveWinIoDriver();

	IsWinIoInitialized = false;
}
