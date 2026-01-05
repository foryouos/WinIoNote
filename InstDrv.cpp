// ---------------------------------------------------- //
//                      WinIo v3.0                      //
//				 Direct Hardware Access Under Windows	//
//           Copyright 1998-2010 Yariv Kaplan           //
//               http://www.internals.com 
//            主要作用为安装移除启用暂停WinIo驱动        //
// ---------------------------------------------------- //

#include <windows.h>
#include "winio.h"
//安装内核驱动程序
bool _stdcall InstallWinIoDriver(PWSTR pszWinIoDriverPath, bool IsDemandLoaded)
{
	SC_HANDLE hSCManager; //存储服务控制管理器的句柄
	SC_HANDLE hService;   //存储创建的服务的句柄

	// Remove any previous instance of the driver
	RemoveWinIoDriver();
	//打开服务控制管理器，用于后续对服务的创建与管理
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	//SERVICE_DEMAND_START：表示该驱动程序在需求时加载。
	//SERVICE_SYSTEM_START：表示该驱动程序在系统启动时自动加载。
	if (hSCManager)
	{
		// Install the driver 创建新服务，(驱动程序)
		hService = CreateService(hSCManager, //服务控制管理器句柄
			L"WinIoNote", //服务的名称
			L"WinIoNote", //服务的显示名称
			SERVICE_ALL_ACCESS,//对服务的所有访问权限
			SERVICE_KERNEL_DRIVER, // 指定服务类型为内核驱动程序
			(IsDemandLoaded == true) ? SERVICE_DEMAND_START : SERVICE_SYSTEM_START,//确认启动类型
			SERVICE_ERROR_NORMAL,//服务启动失败时的错误处理
			pszWinIoDriverPath,//驱动程序文件的完整路径。 在加载时加载
			NULL,
			NULL,
			NULL,
			NULL,
			NULL);

		CloseServiceHandle(hSCManager);//关闭打开的服务控制管理器句柄，以释放系统资源。

		if (hService == NULL)
			return false;
	}
	else
		return false;

	CloseServiceHandle(hService);

	return true;
}

// 停止驱动服务
bool _stdcall RemoveWinIoDriver()
{
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	LPQUERY_SERVICE_CONFIG pServiceConfig;
	DWORD dwBytesNeeded;
	DWORD cbBufSize;
	bool bResult;
	// 停止WinIO驱动
	StopWinIoDriver();
	//打开服务控制管理器，
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!hSCManager)
	{
		return false;
	}

	hService = OpenService(hSCManager, L"WinIoNote", SERVICE_ALL_ACCESS);
	CloseServiceHandle(hSCManager);

	if (!hService)
	{
		return false;
	}

	bResult = QueryServiceConfig(hService, NULL, 0, &dwBytesNeeded);
	//处理不足的缓冲区
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		cbBufSize = dwBytesNeeded;
		pServiceConfig = (LPQUERY_SERVICE_CONFIG)malloc(cbBufSize);
		bResult = QueryServiceConfig(hService, pServiceConfig, cbBufSize, &dwBytesNeeded);
		//检测服务是否可删除
		if (!bResult)
		{
			free(pServiceConfig);
			CloseServiceHandle(hService);
			return bResult;
		}
		// If service is set to load automatically, don't delete it!
		if (pServiceConfig->dwStartType == SERVICE_DEMAND_START)
		{
			// 删除服务
			bResult = DeleteService(hService);
		}
	}

	CloseServiceHandle(hService);

	return bResult;
}


bool _stdcall StartWinIoDriver()
{
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	bool bResult;

	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCManager)
	{
		//打开服务
		hService = OpenService(hSCManager, L"WinIoNote", SERVICE_ALL_ACCESS);

		CloseServiceHandle(hSCManager);

		if (hService)
		{
			bResult = StartService(hService, 0, NULL) || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;

			CloseServiceHandle(hService);
		}
		else
			return false;
	}
	else
		return false;

	return bResult;
}


bool _stdcall StopWinIoDriver()
{
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	SERVICE_STATUS ServiceStatus;
	bool bResult;

	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCManager)
	{
		hService = OpenService(hSCManager, L"WinIoNote", SERVICE_ALL_ACCESS);

		CloseServiceHandle(hSCManager);

		if (hService)
		{
			//暂停服务
			bResult = ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);

			CloseServiceHandle(hService);
		}
		else
			return false;
	}
	else
		return false;

	return bResult;
}