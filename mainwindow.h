#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ui_mainwindow.h>
#include <QPaintEvent> //用于绘画事件
#include <QtGui> //引入用到的控件
#include "seriallogic.h"
#include <QDebug>
#include "motorstatus.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    SerialLogic* serial_Logic;

private:
    Ui::MainWindow *ui;
    QButtonGroup *loop_Group;
    QButtonGroup *motor_Group;
    MotorStatus *motor_Status;

    void initSeialPort();
    void setButtonGroups();


private slots:
    void onLoopGroupButtonClicked();
    void onMotorGroupButtonClicked();
    void on_writePushButton_clicked();
    void on_connectButton_clicked();
    void on_portBox_currentIndexChanged(const QString &arg1);
    void on_controlPlotButton_clicked(bool checked);
    void text_ackData_Show(double ack_P, double ack_I, double ack_D);
};

#endif // MAINWINDOW_H
