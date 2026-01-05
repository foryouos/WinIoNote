#ifndef WINAPI_H
#define WINAPI_H

#include <QObject>
#include <windows.h>
#include <winioctl.h>
// 定义内存映射结构
#include "winio_nt.h"
#include <QLibrary>
#include <QDebug>
// 函数定义
typedef bool( _stdcall* LP_InitializeWinIo)();
typedef bool( _stdcall* LP_ShutdownWinIo)();
typedef bool( _stdcall* LP_InstallWinIoDriver)(PSTR pszWinIoDriverPath,bool IsDemandLoaded);
typedef bool (_stdcall* LP_RemoveWinIoDriver)();
typedef bool( _stdcall* LP_GetPortVal)(WORD wPortAddr, PDWORD pdwPortVal, BYTE bSize);
typedef bool( _stdcall* LP_SetPortVal)(WORD wPortAddr, DWORD dwPortVal, BYTE bSize);
typedef bool( _stdcall* LP_GetPhysLong)(PBYTE pbPhysAddr, PDWORD pdwPhysVal);
typedef bool( _stdcall* LP_SetPhysLong)(PBYTE pbPhysAddr, DWORD dwPhysVal);
typedef PBYTE(_stdcall* LP_MapPhysToLin)(tagPhysStruct &PhysStruct);
typedef bool (_stdcall* LP_UnmapPhysicalMemory)(tagPhysStruct &PhysStruct);

typedef bool (_stdcall* LP_OpenSioDecode)();
typedef bool (_stdcall* LP_CloseSioDecode)();



class WinAPI : public QObject
{
    Q_OBJECT
public:
    explicit WinAPI(QObject *parent = nullptr);
    ~ WinAPI() override;
    // 共有供外接访问的 API 接口
public slots:
    // 1，加载动态链接库
    bool Load_WinIO();
    // 2, 函数调用端口 以BIOS源代码同步
    // 2.1 读取SuperIO 数据
    BYTE IoRead8(WORD  port); //参数： 端口地址 ，端口值  BYTE 8位
    WORD  IoRead16(WORD  port); //参数： 端口地址 ，端口值  WORD 16位
    DWORD  IoRead32(WORD  port); //参数： 端口地址 ，端口值  DWORD 32 wei
    // 2,2 向SuperIO中写入数据
    bool IoWrite8(WORD port, DWORD  val);
    bool IoWrite16(WORD port, DWORD  val);
    bool IoWrite32(WORD port, DWORD  val);

    // 2.3 读取内存中的数据
    DWORD GetMemory(PBYTE pbPhysAddr);  //获取内存中的值
     // 2.4 向内存中写入数据
    bool SetMemory(PBYTE pbPhysAddr, DWORD dwPhysVal); // 向内存中写入数据
    // 2.5 向内存空间映射地址
    PBYTE MapMemoryToLin(tagPhysStruct &PhysStruct);
    bool UnMapMemoryToLin(tagPhysStruct &PhysStruct);



    // 私有接口
private slots:
    inline void Debug(QString Content)
    {
        qDebug()<<Content;
        emit Send_Log(Content + "\n");
    }
signals:

    void Send_Log(QString log);
private:
    //
    HMODULE m_hDllFile  =  nullptr;  // 初始化WinIO 句柄
    // 对函数指针进行重定义。
    LP_InitializeWinIo InitializeWinIo =  nullptr;
    LP_ShutdownWinIo ShutdownWinIo  =  nullptr;
    LP_GetPhysLong GetPhysLong  =  nullptr;
    LP_SetPhysLong SetPhysLong  =  nullptr;
    LP_SetPortVal SetPortVal  =  nullptr;
    LP_GetPortVal GetPortVal  =  nullptr;
    LP_MapPhysToLin MapPhysToLin  =  nullptr;
    LP_UnmapPhysicalMemory UnmapPhysicalMemory  =  nullptr;
    LP_InstallWinIoDriver InstallWinIoDriver = nullptr;
    LP_RemoveWinIoDriver RemoveWinIoDriver = nullptr;

    LP_OpenSioDecode OpenSioDecode = nullptr;
    LP_CloseSioDecode CloseSioDecode = nullptr;


};

#endif // WINAPI_H
