#include "memory.h"
#include "ui_memory.h"
#define  COM1_EN_RS422          0xFD6D0990      // gpp_d1
Memory::Memory(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Memory)
{
    ui->setupUi(this);
    setWindowTitle("锐宝WinIO测试工具Demo");
    /* #define addr 0x70
        #define data_value 0x71
        #define year 0x09
        #define month 0x08
        #define day 0x07
        #define hour 0x04
        #define minute 0x02
        #define mile 0x00
     *
     * */
    WinIO = new WinAPI;
    connect(WinIO,&WinAPI::Send_Log,this,&Memory::Deal_Log);
    // Test Super IO 访问  以读取CMOS 时间 为例
    if(WinIO->IoWrite8(0x2E,0x87))
    {
        qDebug()<<"Super IO  成功写入87";
        this->Deal_Log("Super IO  成功写入87");
    }
    else
    {
        qDebug()<<"Super IO 失败写入87";
        this->Deal_Log("Super IO 失败写入87");
    };
    WinIO->IoWrite8(0x2E,0x01);
    WinIO->IoWrite8(0x2E,0x55);
    WinIO->IoWrite8(0x2E,0x55);
    QString Time;
    // 年
    WinIO->IoWrite8(0x70,0x09);
    Time = QString("%1").arg(WinIO->IoRead8(0x71),2,16,QChar('0')) + " 年 ";
    WinIO->IoWrite8(0x70,0x08);
    Time += (QString("%1").arg(WinIO->IoRead8(0x71),2,16,QChar('0')) + " 月 ");
    WinIO->IoWrite8(0x70,0x07);
    Time += (QString("%1").arg(WinIO->IoRead8(0x71),2,16,QChar('0')) +" 日 ");
    WinIO->IoWrite8(0x70,0x04);
    Time += (QString("%1").arg(WinIO->IoRead8(0x71),2,16,QChar('0')) + " 时:");
    WinIO->IoWrite8(0x70,0x02);
    Time += (QString("%1").arg(WinIO->IoRead8(0x71),2,16,QChar('0')) + " 分 ");
    WinIO->IoWrite8(0x70,0x00);
    Time += (QString("%1").arg(WinIO->IoRead8(0x71),2,16,QChar('0')) + " 秒 ");

    ui->biostime->setText(Time);
    // Test 内存访问
#if 1
    tagPhysStruct *hPhysicalMemory = new tagPhysStruct;
    hPhysicalMemory->pvPhysAddress = 0x00000000;
    hPhysicalMemory->dwPhysMemSizeInBytes= 65536;

    PBYTE pbLinAddr = nullptr;
    qDebug()<< "物理内存映射前:"<< pbLinAddr;
    pbLinAddr = WinIO->MapMemoryToLin(*hPhysicalMemory);
    if(pbLinAddr)
    {
        // Now we can use pbLinAddr to access physical address 0xA0000
        qDebug()<< "物理内存映射后:" <<pbLinAddr;
        *pbLinAddr = 10;
        for(int i = 0;i<10;i++)
        {
            qDebug()<< "读取到的" << QString::number(i)<< QString::number(*(pbLinAddr+i),16).toUpper();
        }

        // When you're done with pbLinAddr, call UnmapPhysicalMemory

        WinIO->UnMapMemoryToLin(*hPhysicalMemory);
    }
    // 内存测试 GetMemory    经过测试没有问题
    qDebug()<<"内存函数 GetMemory 测试";
    PBYTE physAddr = (PBYTE)0xFD6E0830;
    DWORD physVal = 0x0;
    DWORD Set_physVal = 0xABCD;
//    WinIO->SetMemory(physAddr,Set_physVal);

    physVal = WinIO->GetMemory(physAddr);
    if(physVal)
    {
        qDebug() << "物理地址 0x00000000 对应的物理值为: " <<  QString::number(physVal,16).toUpper();
        this->Deal_Log("物理地址 0x00000000 对应的物理值为: " +  QString::number(physVal,16).toUpper());
    }
    else
    {
        qDebug() << "获取物理值失败";
    }
    ui->GetMemoty->setText( QString::number(physVal,16).toUpper());

#else
    DWORD Content = WinIO->GetMemory((BYTE*)COM1_EN_RS422);
    QString Read_Content = QString::number(Content, 16).toUpper();
    // 向UI中写入
    ui->MemValue->setText(Read_Content);

#endif
    // 页面内容初始化
    ui->MemAddr->setText("0x00000000");
    ui->MemValue->setText("0xABCD");

    ui->SuperIoAddr->setText("0x70");
    ui->SuperIOValue->setText("0x01");


}

Memory::~Memory()
{
    if(WinIO != nullptr)
    {
        delete WinIO;
        WinIO = nullptr;
    }
    delete ui;
}

void Memory::Deal_Log(QString Log)
{
    ui->log->insertPlainText(Log);
}


void Memory::on_MemWrite_clicked()
{
    bool ok;
    // 地址
    unsigned long long Addr = ui->MemAddr->text().toULongLong(&ok,16);

    PBYTE addr_num = reinterpret_cast<PBYTE>(Addr);
    // 写入的值
    int value = ui->MemValue->text().toInt(&ok,16);
    DWORD dwContent = static_cast<DWORD>(value);
    qDebug()<<"输入的地址"<<addr_num<< "读取的输入值"<< value <<"写入地址:"<<addr_num << "写入的值:"<<dwContent;

    WinIO->SetMemory((PBYTE)Addr,(DWORD)dwContent);

}


void Memory::on_MemRead_clicked()
{
    bool ok;
    QString Read_Content;
    unsigned long long Addr = ui->MemAddr->text().toULongLong(&ok,16);

    // PBYTE 指针所占的空间大小与指针的大小相同，通常为 4 字节或 8 字节，取决于操作系统的位数（32 位或 64 位）
    qDebug()<<"读取的地址"<<QString::number(Addr,16);
    PBYTE addr_num = reinterpret_cast<PBYTE>(Addr);
//    qDebug()<<"BYTE大小:"<< sizeof(addr_num)<<"值为:"<< addr_num ;

    DWORD Content = WinIO->GetMemory((PBYTE)addr_num);
    Read_Content = QString::number(Content, 16).toUpper();
    // 向UI中写入
    ui->MemValue->setText(Read_Content);

}


void Memory::on_SuperWrite_clicked()
{
    // 对现有值的读取，格式转换
    // 地址
    bool ok;
    int SuperIOAddr = ui->SuperIoAddr->text().toInt(&ok,16);
    WORD addr_num = static_cast<WORD>(SuperIOAddr);
    // 传回值
    int SuperIOValue = ui->SuperIOValue->text().toInt(&ok,16);
    DWORD addr_value = static_cast<DWORD>(SuperIOValue);


    WinIO->IoWrite8(addr_num,addr_value);
}


void Memory::on_SuperIORead_clicked()
{
    // 对现有值的读取，以及对应的格式转换
    // 从UI中读取数据
    bool ok;
    int SuperIOAddr = ui->SuperIoAddr->text().toInt(&ok,16);

    WORD addr_num = static_cast<WORD>(SuperIOAddr);

    QString SuperIoValue = QString("%1").arg(WinIO->IoRead8(addr_num),2,16,QChar('0'));
    ui->SuperIOValue->setText(SuperIoValue);
}

