#ifndef MEMORY_H
#define MEMORY_H

#include <QWidget>
#include "winapi.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Memory; }
QT_END_NAMESPACE

class Memory : public QWidget
{
    Q_OBJECT

public:
    Memory(QWidget *parent = nullptr);
    ~Memory();

public slots:
    void Deal_Log(QString Log);
private slots:

    void on_MemWrite_clicked();

    void on_MemRead_clicked();

    void on_SuperWrite_clicked();

    void on_SuperIORead_clicked();

signals:

private:
    Ui::Memory *ui;

    WinAPI *WinIO = nullptr;
};
#endif // MEMORY_H
