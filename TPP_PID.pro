#-------------------------------------------------
#
# Project created by QtCreator 2018-01-05T15:02:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TPP_PID
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    seriallogic.cpp \
    motorstatus.cpp

HEADERS  += mainwindow.h \
    seriallogic.h \
    motorstatus.h

FORMS    += mainwindow.ui

QT       += serialport

RESOURCES += \
    res.qrc

RC_ICONS  += \
    tpp.ico

DEFINES += QT_DLL QWT_DLL

#LIBS += -L"D:\Users\QT\5.6.3\mingw49_32\lib" -lqwtd
LIBS += -L"D:\Users\QT\5.6.3\mingw49_32\lib" -lqwt
INCLUDEPATH += D:\Users\QT\5.6.3\mingw49_32\include\Qwt
