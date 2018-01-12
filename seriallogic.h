#ifndef SERIALLOGIC_H
#define SERIALLOGIC_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QMutex>

#define SERIAL_PORT_SUCCESS  1
#define SERIAL_PORT_FAIL     0

enum SerialSendReturn{
    SerialSend_Success = 0,
    SerialSend_WriteError,
    SerialSend_NoLoopChoice,
    SerialSend_NoMotorChoice
};

enum SerialParseReturn{
    SerialParse_Success = 0,
    SerialParse_HeadError,
    SerialParse_NoLoopFill,//保留，作为2.0版本用
    SerialParse_NoMotorFill,
    SerialParse_TailError
};

enum SerialSendDataNum{
    SendData_Head     = 0,
    SendData_DeviceID = 1,
    SendData_DataFrame= 2,
    SendData_Tail     =14,
    SendData_Len      =15
};

enum SerialReceiveDataNum{
    ReceiveData_Head        = 0,
    ReceiveData_DeviceID    = 1,
    ReceiveData_PositionData= 2,
    ReceiveData_SpeedData   = 6,
    ReceiveData_Tail        =10,
    ReceiveData_Len         =11
};

enum SerialLoopID{
    POSTION_BUTTON_ID= 1,
    SPEED_BUTTON_ID
};

enum SerialMotorID{
    YAW_BUTTON_ID = 1,
    PITCH_BUTTON_ID,
    CHASSIS1_BUTTON_ID,
    CHASSIS2_BUTTON_ID,
    CHASSIS3_BUTTON_ID,
    CHASSIS4_BUTTON_ID
};

class SerialLogic : public QObject
{
    Q_OBJECT

public:
    char serialDataToSend[SendData_Len];

    SerialLogic();
    ~SerialLogic();
    bool setSerialPort(const QString &arg1);
    bool openSerialPort();
    void closeSerialPort();
    int serialWrite();
    int parseSerialData(char *pArray);
    int parseAckData(char *pArray);

    double getPositionData();
    double getSpeedData();


private:
    QSerialPort *serial;
    double PositionData;
    double SpeedData;
    double Ack_P;
    double Ack_I;
    double Ack_D;
    int MotorID = 0;
    QTimer *timer;
    long TPP_SystemTime = 0;
    QByteArray recDataBuffer;
    int SerialReadDataNum = 0;
    int SerialAckDataNum = 0;

private slots:
    void serialRead();
    void recordTime();

signals:
    void sigUpdatePlot(long _SystemTime, int _MotorID, double _PostionData, double _SpeedData);
    void sigShowAckData(double _ACK_P, double _ACK_I, double _ACK_D);
};

#endif // SERIALLOGIC_H
