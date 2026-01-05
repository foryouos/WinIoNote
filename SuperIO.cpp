// ---------------------------------------------------- //
//				对SuperIO相关的定制化函数                //
// ---------------------------------------------------- //

#include <windows.h>
#include "winio.h"


UINT8  IT8728F_CONFIG_INDEX = 0x2e;
UINT8  IT8728F_CONFIG_DATA = 0x2f;
bool _stdcall IoWrite8(UINT8 Register, UINT8 Value)
{
    SetPortVal((WORD)Register, (WORD)Value, 1);
    return true;
}
UINT8 _stdcall IoRead8(UINT8 Register)
{
    DWORD portValue = 0; // 用于存储读取的值
    GetPortVal(Register, &portValue, 1);
    return (UINT8)(portValue & 0xFF); // 只返回低8位
}

bool _stdcall OpenSioITEDecode()
{
    IoWrite8(IT8728F_CONFIG_INDEX, 0x87);
    IoWrite8(IT8728F_CONFIG_INDEX, 0x01);
    IoWrite8(IT8728F_CONFIG_INDEX, 0x55);
    IoWrite8(IT8728F_CONFIG_INDEX, 0x55);
    return true;
}
bool _stdcall CloseSioITEDecode()
{
    IoWrite8(IT8728F_CONFIG_INDEX, 0x02);
    IoWrite8(IT8728F_CONFIG_DATA, 0x02);
    return true;
}

int _stdcall IoRead8_Single_Bit(WORD port, int bit)
{
    BYTE Value = 0x0;
    DWORD val = 0x0;
    if (GetPortVal(port, &val, 1)) // Can be 1 (BYTE), 2 (WORD) or 4 (DWORD).
    {
        // 获取 val 的第 bit 位的值
        if (bit < 0 || bit >= 32) // 确保 bit 在合法范围内
        {
            return -1; // 返回错误代码
        }
        // 使用位操作获取对应的 bit 位
        int bitValue = (val >> bit) & 1; // 右移 bit 位并与 1 进行与操作
        return bitValue; // 返回对应 bit 位的值（0或1）
    }
    else
    {
        return -1;
    }

}

bool _stdcall IoWrite8_Single_Bit(WORD addr, int bit, int bit_value)
{
    BYTE Curr_Value = IoRead8(addr);
    // 根据条件与或操作来更新对应的值
    if (bit_value == 1)
    {
        // 将第bit位设置为1
        Curr_Value |= (1 << bit);
    }
    else
    {
        // 将第bit位设置为0
        Curr_Value &= ~(1 << bit);
    }
    // 将更新后的值 写入
    if (SetPortVal(addr, Curr_Value, 1))
    {
        return true;
    }
    else
    {
        return false;
    }
    return true;
}
