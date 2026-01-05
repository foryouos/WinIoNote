// ---------------------------------------------------- //
//                      WinIo v3.0                      //
//				 Direct Hardware Access Under Windows	//
//           Copyright 1998-2010 Yariv Kaplan           //
//               http://www.internals.com 
//				对SuperIO访问的相关函数                //
// ---------------------------------------------------- //

#include <windows.h>
#include <winioctl.h>
#include <conio.h>
#include "port32.h"
#include "./WinIoNoteSys/OpenLibSys.h"

#include "winio.h"


bool _stdcall GetPortVal(WORD wPortAddr, PDWORD pdwPortVal, BYTE bSize)
{
	if (!IsWinIoInitialized)
	{
		return false;
	}

#ifdef _WIN64
	tagPortStruct PortStruct;
	DWORD dwBytesReturned;

	PortStruct.bSize = bSize;
	PortStruct.wPortAddr = wPortAddr;
	// 与驱动程序通信   控制代码 IOCTL_WINIO_READPORT
	return DeviceIoControl(hDriver,  // WinIO设备句柄
		IOCTL_WINIO_READPORT, // 调用的控制码
		&PortStruct,		  // 输入缓冲区 输入参数 驱动层会解析参数
		sizeof(PortStruct),  // 输入缓冲区大小
		pdwPortVal,         // 输出缓冲区 通过指针的形式 传出参数
		sizeof(DWORD),     // 输出缓冲区大小
		&dwBytesReturned,   // 返回的字节数
		NULL);


#elif _WIN32
	// If this is a 64 bit OS, we must use the driver to access I/O ports even if the application is 32 bit
	if (g_Is64BitOS)
	{
		tagPortStruct PortStruct;
		DWORD dwBytesReturned;

		PortStruct.bSize = bSize;
		PortStruct.wPortAddr = wPortAddr;

		return DeviceIoControl(hDriver, IOCTL_WINIO_READPORT, &PortStruct, sizeof(PortStruct),
			pdwPortVal, sizeof(DWORD), &dwBytesReturned, NULL);
	}
	else
	{
		switch (bSize)
		{
		case 1:
			// _inp 32位的IO访问函数，  。 从 Visual Studio 2015 开始，CRT 中不再提供这些函数。
			*pdwPortVal = _inp(wPortAddr);

			break;

		case 2:

			*pdwPortVal = _inpw(wPortAddr);

			break;

		case 4:

			*pdwPortVal = _inpd(wPortAddr);

			break;
		}
	}
#endif

	return true;
}


bool _stdcall SetPortVal(WORD wPortAddr, DWORD dwPortVal, BYTE bSize)
{
	if (!IsWinIoInitialized)
	{
		return false;
	}

#ifdef _WIN64
	tagPortStruct PortStruct;
	DWORD dwBytesReturned;

	PortStruct.bSize = bSize;
	PortStruct.dwPortVal = dwPortVal;
	PortStruct.wPortAddr = wPortAddr;
	
	return DeviceIoControl(hDriver, IOCTL_WINIO_WRITEPORT, &PortStruct, sizeof(PortStruct),
		NULL, 0, &dwBytesReturned, NULL);
#elif _WIN32
	// If this is a 64 bit OS, we must use the driver to access I/O ports even if the application is 32 bit
	if (g_Is64BitOS)
	{
		tagPortStruct PortStruct;
		DWORD dwBytesReturned;

		PortStruct.bSize = bSize;
		PortStruct.dwPortVal = dwPortVal;
		PortStruct.wPortAddr = wPortAddr;

		return DeviceIoControl(hDriver, IOCTL_WINIO_WRITEPORT, &PortStruct, sizeof(PortStruct),
			NULL, 0, &dwBytesReturned, NULL);
	}
	else
	{
		switch (bSize)
		{
		case 1:

			_outp(wPortAddr, dwPortVal);

			break;

		case 2:

			_outpw(wPortAddr, (WORD)dwPortVal);

			break;

		case 4:

			_outpd(wPortAddr, dwPortVal);

			break;
		}
	}
#endif

	return true;
}
