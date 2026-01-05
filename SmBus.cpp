// ---------------------------------------------------- //
//				对SMBus进行访问的相关函数                //
// ---------------------------------------------------- //
#include <windows.h>
#include "winio.h"
WORD Sumbus_base = 0xf000;
bool _stdcall SmbusWriteByte(WORD slav_address, WORD offset_address, WORD write_data)
{
    int Timer_Num = 0;
    WORD tempvalue; // 存储读到的值
    // 初始化超时时间位 0
    unsigned long   timeout = 0;
    // 使用outb方法将write_date的值写入到I/O端口地址0xf005
    /*
        端口IO
        * port I/O 地址，此地址位虚拟地址
        * data 位写入数据
    */
    IoWrite8(Sumbus_base + 0x05, write_data);
    // 将0x1f的值写入指定的I/O端口地址0xf000
    // 清零
    IoWrite8(Sumbus_base, 0x1f);

    while (1)
    {
        // inb读取指定寄存器的值 0xf000
        
        //tempvalue = io->inb(Sumbus_base);
        tempvalue = IoRead8(Sumbus_base);
        // 暂停10 毫秒
        Sleep(10 / 1000); 
       
        // 检查对应的值最低为是否为 0
        if ((tempvalue & 0x1) == 0)
        {
            break;
        }
        // 超市时间
        timeout++;
        // 10微妙一次，相当于超过一秒，访问超时
        if (timeout > 100)
        {
            Timer_Num++;
            //            perror("Smbus timeout!");

            break;
        }
    }
    // 将slav_address基地址写到0xf004
    IoWrite8(Sumbus_base + 0x04, slav_address);
    // 将偏移量写道 0xf003
    IoWrite8(Sumbus_base + 0x03, offset_address);
    // 将0x48 写入到 0xf002
    IoWrite8(Sumbus_base + 0x02, 0x48);
    // 将超时进行初始化
    timeout = 0;

    while (1)
    {
        // 读取0xf000的数据
        tempvalue = IoRead8(Sumbus_base);
        Sleep(10 / 1000);
        // 判断最低为是否为0
        if ((tempvalue & 0x1) == 0)
        {
            break;
        }
        timeout++;
        // 超时算上10微妙就为1秒
        if (timeout > 100)
        {
            Timer_Num++;
            //            perror("Smbus timeout!");

            break;
        }
    }
    // 如果tempvalue为零 则写入成功
    if ((tempvalue & 0x1C) != 0)
    {
        Timer_Num++;
        return false;
    }
	return true;
}

bool _stdcall SmbusReadByte(WORD slav_address, WORD offset_address, WORD* read_data)
{
    int Timer_Num = 0;
    // 记录读取的值
    WORD tempvalue;
    // 设置超时时间
    unsigned long   timeout = 0;
    // 将0x1f写入0xf000
    IoWrite8(Sumbus_base, 0x1f);

    while (1)
    {
        // 判断是否写入
        tempvalue = IoRead8(Sumbus_base);
        Sleep(10 / 1000);
        // 判断最低为是否为0
        if ((tempvalue & 0x1) == 0)
        {
            break;
        }
        timeout++;
        //
        if (timeout > 100)
        {
            Timer_Num++;
            //            perror("Smbus timeout!");
            break;
        }
    }
    // 将基地址+1存入寄存器IO 0xf004
    // +1 为读.
    IoWrite8(Sumbus_base + 0x04, (slav_address + 1));
    // 将偏移量 写入 0xf003
    IoWrite8(Sumbus_base + 0x03, offset_address);
    // 将0x48 写入 0x002
    IoWrite8(Sumbus_base + 0x02, 0x48);

    timeout = 0;

    while (1)
    {
        tempvalue = IoRead8(Sumbus_base);
        Sleep(10 / 1000);
        if ((tempvalue & 0x1) == 0)
        {
            break;
        }
        timeout++;
        if (timeout > 100)
        {
            Timer_Num++;
            //            perror("Smbus timeout!");
            break;
        }
    }
    if ((tempvalue & 0x1C) != 0)
    {
        Timer_Num++;
    }
    else
    {
        *read_data = IoRead8(Sumbus_base + 0x05);
    }
    if (Timer_Num != 0)
    {
        return false;
    }
    return true;
}
