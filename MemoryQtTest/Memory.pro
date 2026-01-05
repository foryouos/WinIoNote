QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += console
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    memory.cpp \
    winapi.cpp

HEADERS += \
    memory.h \
    winapi.h \
    WinIo.h \
    winio_nt.h \

FORMS += \
    memory.ui

RC_ICONS = RB.ico
# 启动时 - 申请管理员权限
RC_FILE = getAdmin.rc

DEFINES += WINIO_DLL
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
