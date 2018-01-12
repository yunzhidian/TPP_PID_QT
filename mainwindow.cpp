#include "mainwindow.h"

bool g_EnablePlotFlag = 1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial_Logic = new SerialLogic();

    motor_Status = new MotorStatus(ui->positionWidget, ui->speedWidget);
    connect(serial_Logic, SIGNAL(sigUpdatePlot(long,int,double,double) ), motor_Status, SLOT(updatePlot(long,int,double,double)) );
    connect(serial_Logic, SIGNAL(sigShowAckData(double,double,double)), this, SLOT(text_ackData_Show(double, double, double)) );

    initSeialPort();

    setButtonGroups();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete loop_Group;
    delete motor_Group;
}

void MainWindow::initSeialPort()
{
    //get name to choose
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    if(infos.isEmpty())
    {
        ui->portBox->addItem("无串口");
        return;
    }
    ui->portBox->addItem("端口号");
    foreach (QSerialPortInfo info, infos) {
        ui->portBox->addItem(info.portName());
    }

    ui->connectButton->setEnabled(false);
}

void MainWindow::on_portBox_currentIndexChanged(const QString &arg1)
{
    int resultSettingPort;
    resultSettingPort = serial_Logic->setSerialPort(arg1);
    if(resultSettingPort == SERIAL_PORT_SUCCESS)
    {
        ui->hintText->setText("串口设置成功!\n");
        ui->connectButton->setEnabled(true);
    }
    else
    {
        ui->hintText->setText("串口设置失败!\n");
    }
}

void MainWindow::on_connectButton_clicked()
{
    QImage showConnectMsg[2] = {QImage(":/icon/connect_sucess"),
                                QImage(":/icon/connect_fail")};
//    for(int i=0; i<2; i++)
//        showConnectMsg[i].scaled(20, 20, Qt::KeepAspectRatio);

    if(serial_Logic->openSerialPort())
        ui->showConnectLabel->setPixmap(QPixmap::fromImage(showConnectMsg[0]));
    else
        ui->showConnectLabel->setPixmap(QPixmap::fromImage(showConnectMsg[1]));
}

void MainWindow::setButtonGroups()
{
    loop_Group = new QButtonGroup(this);
    loop_Group->addButton(ui->positionLoopRadioButton, POSTION_BUTTON_ID);
    loop_Group->addButton(ui->speedLoopRadioButton, SPEED_BUTTON_ID);
    loop_Group->setExclusive(true);//設置互斥
    connect(ui->positionLoopRadioButton, SIGNAL(clicked()), this, SLOT(onLoopGroupButtonClicked()));
    connect(ui->speedLoopRadioButton, SIGNAL(clicked()), this, SLOT(onLoopGroupButtonClicked()));

    motor_Group = new QButtonGroup(this);
    motor_Group->addButton(ui->yawRadioButton, YAW_BUTTON_ID);
    motor_Group->addButton(ui->pitchRadioButton, PITCH_BUTTON_ID);
    motor_Group->addButton(ui->chassis1RadioButton, CHASSIS1_BUTTON_ID);
    motor_Group->addButton(ui->chassis2RadioButton, CHASSIS2_BUTTON_ID);
    motor_Group->addButton(ui->chassis3RadioButton, CHASSIS3_BUTTON_ID);
    motor_Group->addButton(ui->chassis4RadioButton, CHASSIS4_BUTTON_ID);
    motor_Group->setExclusive(true);//設置互斥
    connect(ui->yawRadioButton, SIGNAL(clicked()), this, SLOT(onMotorGroupButtonClicked()));
    connect(ui->pitchRadioButton, SIGNAL(clicked()), this, SLOT(onMotorGroupButtonClicked()));
    connect(ui->chassis1RadioButton, SIGNAL(clicked()), this, SLOT(onMotorGroupButtonClicked()));
    connect(ui->chassis2RadioButton, SIGNAL(clicked()), this, SLOT(onMotorGroupButtonClicked()));
    connect(ui->chassis3RadioButton, SIGNAL(clicked()), this, SLOT(onMotorGroupButtonClicked()));
    connect(ui->chassis4RadioButton, SIGNAL(clicked()), this, SLOT(onMotorGroupButtonClicked()));
}

void MainWindow::onLoopGroupButtonClicked()
{
    char loopChoice= 0x00;

    switch(loop_Group->checkedId())
    {
    case POSTION_BUTTON_ID:
        loopChoice |= 0x80;
        qDebug()<<"位置环选中!\n";
        break;
    case SPEED_BUTTON_ID:
        loopChoice |= 0x40;
        break;
    default:
        loopChoice = 0x00;
    }
    serial_Logic->serialDataToSend[SendData_DeviceID] |= loopChoice;
}

void MainWindow::onMotorGroupButtonClicked()
{
    char motorChoice= 0x00;

    switch (motor_Group->checkedId()) {
    case YAW_BUTTON_ID:
        motorChoice = 1;
        qDebug()<<"Yaw俯仰电机选中!\n";
        break;
    case PITCH_BUTTON_ID:
        motorChoice = 2;
        break;
    case CHASSIS1_BUTTON_ID:
        motorChoice = 3;
        break;
    case CHASSIS2_BUTTON_ID:
        motorChoice = 4;
        break;
    case CHASSIS3_BUTTON_ID:
        motorChoice = 5;
    case CHASSIS4_BUTTON_ID:
        motorChoice = 6;
    default:
        motorChoice = 0;
        break;
    }
    serial_Logic->serialDataToSend[SendData_DeviceID] |= motorChoice;
}

void MainWindow::on_writePushButton_clicked()
{
    int32_t *pSerialData=(int32_t *) &serial_Logic->serialDataToSend[SendData_DataFrame];
    pSerialData[0] = (int32_t)(ui->pSpinBox->text().toDouble()*100);
    pSerialData[1] = (int32_t)(ui->iSpinBox->text().toDouble()*100);
    pSerialData[2] = (int32_t)(ui->dSpinBox->text().toDouble()*100);
    qDebug()<<"P:"<<pSerialData[0]<<endl;

    switch (serial_Logic->serialWrite()) {
    case SerialSend_Success:
        ui->hintText->setText("PID参数发送成功!");
        break;
    case SerialSend_WriteError:
        ui->hintText->setText("PID参数写错误!");
        break;
    case SerialSend_NoLoopChoice:
        ui->hintText->setText("未选择位置环或速度环!");
        break;
    case SerialSend_NoMotorChoice:
        ui->hintText->setText("未选择电机!");
        break;
    default:
        break;
    }
}


void MainWindow::on_controlPlotButton_clicked(bool checked)
{
    if(checked)
    {
        ui->controlPlotButton->setText("开始");
        g_EnablePlotFlag = 0;
    }
    else
    {
        ui->controlPlotButton->setText("暂停");
        g_EnablePlotFlag = 1;
    }
}

void MainWindow::text_ackData_Show(double ack_P, double ack_I, double ack_D)
{
    ui->hintText->setText("P:"+QString::number(ack_P, 10, 2)+"\n"+
                          "I:"+QString::number(ack_I, 10, 2)+"\n"+
                          "D:"+QString::number(ack_D, 10, 2)     );
}
