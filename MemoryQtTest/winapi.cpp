#include "winapi.h"
#include <QCoreApplication>
#include <QDir>
WinAPI::WinAPI(QObject *parent)
    : QObject{parent}
{
    //1,初始化WinIO

    Debug("WinAPI 构造初始化加载WinIO");
    this->Load_WinIO();
    if(InitializeWinIo())
    {

        Debug("二次初始化成功!");
    }

}

WinAPI::~WinAPI()
{
    Debug("WinAPI 析构函数");
    if(ShutdownWinIo != nullptr)  // 确保指针已经初始化成功
    {
        RemoveWinIoDriver();
        ShutdownWinIo();
    }

}

bool WinAPI::Load_WinIO()
{
    //加载动态链接库，并通过GetProcAddress方法调用取函数空间地址
    // @ 传入所需的动态链接库
    // 返回m_hDllFile将作为GetProcAddress去调用WinIO的函数地址使用
    int Fail = 0;
   // RealBom特供WinIO  ，读取内存 无需 测试模式情况下
#ifdef Q_OS_WIN64
    // 64 位 Windows 版本
//    m_hDllFile = LoadLibrary(L"Realbom64drv.dll");
    m_hDllFile = LoadLibrary(L"GENIO64.dll");
#else
    // 32 位 Windows 版本
    m_hDllFile = LoadLibrary(L"Realbom32drv.dll");
#endif


    if(m_hDllFile == nullptr)
    {
        Debug("1,Load Realbom64drv失败,请检查锐宝 WinIO 文件是否与exe same location!!! ");
        Fail ++;
    }
    else
    {
        // 对函数指针初始化OEMLib_InitializeWinIo -> InitializeWinIo

        InitializeWinIo=(LP_InitializeWinIo)::GetProcAddress(m_hDllFile,"InitializeWinIo");
        if(nullptr == InitializeWinIo)
        {
            Debug( "InitializeWinIo 加载函数指针失败");
            Fail ++;
        }
        //qDebug()<< "后:"<< OEMLib_InitializeWinIo;
        // 函数指针初始化  OEMLib_ShutdownWinIo ->  ShutdownWinIo
        ShutdownWinIo=(LP_ShutdownWinIo)::GetProcAddress(m_hDllFile,"ShutdownWinIo");
        if(nullptr == ShutdownWinIo)
        {
            Debug("ShutdownWinIo 加载函数指针失败");
            Fail ++;
        }
        InstallWinIoDriver=(LP_InstallWinIoDriver)::GetProcAddress(m_hDllFile,"InstallWinIoDriver");
        if(nullptr==InstallWinIoDriver)
        {
            qDebug()<< "InstallWinIoDriver 加载函数指针失败";
            Fail ++;
        }
        RemoveWinIoDriver=(LP_RemoveWinIoDriver)::GetProcAddress(m_hDllFile,"RemoveWinIoDriver");
        if(nullptr==RemoveWinIoDriver)
        {
            qDebug()<< "RemoveWinIoDriver 加载函数指针失败";
            Fail ++;
        }
        // 函数指针初始化 OEMLib_GetPhysLong -> GetPhysLong
        GetPhysLong=(LP_GetPhysLong)::GetProcAddress(m_hDllFile,"GetPhysLong");
        if(nullptr == GetPhysLong)
        {
            Debug( "GetPhysLong 加载函数指针失败");
            Fail ++;
        }
        // 函数        OEMLib_SetPhysLong  -> SetPhysLong
        SetPhysLong = (LP_SetPhysLong)::GetProcAddress(m_hDllFile,"SetPhysLong");
        if(nullptr == SetPhysLong)
        {
            Debug( "SetPhysLong 加载函数指针失败");
            Fail ++;
        }
        SetPortVal=(LP_SetPortVal)::GetProcAddress(m_hDllFile,"SetPortVal");
        if(nullptr == SetPortVal)
        {
            Debug("SetPortVal 加载函数指针失败");
            Fail ++;
        }
        // 初始化操作
        GetPortVal = (LP_GetPortVal)::GetProcAddress(m_hDllFile, "GetPortVal");
        if( nullptr== GetPortVal)
        {
            Debug("GetPortVal 加载函数指针失败");
            Fail ++;
        }
        MapPhysToLin=(LP_MapPhysToLin)::GetProcAddress(m_hDllFile,"MapPhysToLin");
        if(nullptr==MapPhysToLin)
        {
            Debug( "MapPhysToLin 加载函数指针失败");
            Fail ++;
        }
        UnmapPhysicalMemory = (LP_UnmapPhysicalMemory)::GetProcAddress(m_hDllFile,"UnmapPhysicalMemory");
        if(nullptr == UnmapPhysicalMemory)
        {
            Debug( "UnmapPhysicalMemory 加载函数指针失败");
            Fail ++;
        }
        OpenSioDecode = (LP_OpenSioDecode)::GetProcAddress(m_hDllFile,"OpenSioDecode");
        if(nullptr == OpenSioDecode)
        {
            Debug( "OpenSioDecode 加载函数指针失败");
            Fail ++;
        }
        CloseSioDecode = (LP_CloseSioDecode)::GetProcAddress(m_hDllFile,"CloseSioDecode");
        if(nullptr == CloseSioDecode)
        {
            Debug( "CloseSioDecode 加载函数指针失败");
            Fail ++;
        }


        // 对WinIO进行初始化
        bool status = InitializeWinIo();
        if(status)
        {

            Debug("1，WinIO初始化成功");
        }
        else
        {
            Debug("WinIO初始化失败,失败原因请检查如下:");
            Fail ++;
        };
    }
    if(Fail == 0)
    {
        return true;
    }
    else
    {
        return false;
    }


}

BYTE WinAPI::IoRead8(WORD Port)
{
    DWORD val = 0x0;
    if(GetPortVal(Port,&val,1)) // Can be 1 (BYTE), 2 (WORD) or 4 (DWORD).
    {
        Debug("十六进制:" + QString::number(val,16) + "  二进制:" + QString::number(val,2) );
    }
    else
    {
        Debug("读取 0x " + QString::number(Port,16)+ "数据失败");
    };
    return val;

}


WORD WinAPI::IoRead16(WORD Port)
{
    DWORD val = 0x0;
    if(GetPortVal(Port,&val,2)) // Can be 1 (BYTE), 2 (WORD) or 4 (DWORD).
    {
        Debug("十六进制:" + QString::number(val,16) + "  二进制:" + QString::number(val,2));
    }
    else
    {
        Debug("读取 0x" + QString::number(Port,16) + "数据失败");
    };
    return val;
}

DWORD WinAPI::IoRead32(WORD Port)
{
    DWORD val = 0x0;
    if(GetPortVal(Port,&val,4)) // Can be 1 (BYTE), 2 (WORD) or 4 (DWORD).
    {
        Debug("十六进制:" + QString::number(val,16) + "  二进制:" + QString::number(val,2) ) ;
    }
    else
    {
        Debug( "读取 0x" + QString::number(Port,16) + "数据失败" );
    };
    return val;
}

bool WinAPI::IoWrite8(WORD port, DWORD val)
{
    if(SetPortVal(port,val,1))
    {
        Debug( "向:" + QString::number(port,16) + "中写入:" + QString::number(val,16) + "成功");
        return true;
    }
    else
    {
        Debug( "向:" + QString::number(port,16) + "中写入:" + QString::number(val,16) + "失败");
        return false;
    }
}

bool WinAPI::IoWrite16(WORD port, DWORD val)
{
    if(SetPortVal(port,val,2))
    {
        Debug( "向:" + QString::number(port,16) + "中写入:" + QString::number(val,16) + "成功");
        return true;
    }
    else
    {
        Debug( "向:" + QString::number(port,16) + "中写入:" + QString::number(val,16) + "失败");
        return false;
    }
}

bool WinAPI::IoWrite32(WORD port, DWORD val)
{
    if(SetPortVal(port,val,4))
    {
        Debug( "向:" + QString::number(port,16) + "中写入:" + QString::number(val,16) + "成功");
        return true;
    }
    else
    {
        Debug( "向:" + QString::number(port,16) + "中写入:" + QString::number(val,16) + "失败");
        return false;
    }
}

DWORD WinAPI::GetMemory(PBYTE pbPhysAddr)
{
    DWORD val = 0x0;
    if(GetPhysLong)
    {
        if(GetPhysLong(pbPhysAddr,&val))
        {
            Debug( QString("GetMemory 数据读取 Successful " ));
        }
        else
        {
            Debug( QString( "GetMemory 数据读取   Failed ") );
        };
    }

    return val;
}

bool WinAPI::SetMemory(PBYTE pbPhysAddr, DWORD dwPhysVal)
{

    if(SetPhysLong(pbPhysAddr,dwPhysVal))
    {
        Debug("SetMemory 写入 成功  Successful");
        return true;
    }
    else
    {
        Debug("SetMemory 写入 失败  Failed");
        return false;
    }
}

PBYTE WinAPI::MapMemoryToLin(tagPhysStruct &PhysStruct)
{
    return MapPhysToLin(PhysStruct);
}

bool WinAPI::UnMapMemoryToLin(tagPhysStruct &PhysStruct)
{
    return UnmapPhysicalMemory(PhysStruct);
}
