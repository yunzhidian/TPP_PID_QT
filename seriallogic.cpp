#include "seriallogic.h"
#include <qdebug.h>

extern bool g_EnablePlotFlag;

SerialLogic::SerialLogic()
{
    serial = new QSerialPort();
    timer = new QTimer(this);
    connect(serial,SIGNAL(readyRead()),this,SLOT(serialRead()) );   //连接槽
    connect(timer, SIGNAL(timeout()), this, SLOT(recordTime()) );
    timer->setInterval(10);
    timer->start();
}

SerialLogic::~SerialLogic()
{
    serial->close();
    delete serial;
}

void SerialLogic::recordTime()
{
    TPP_SystemTime += 10;
}

bool SerialLogic::openSerialPort()
{
    return serial->open(QIODevice::ReadWrite);
}

void SerialLogic::closeSerialPort()
{
    return serial->close();
}

bool SerialLogic::setSerialPort(const QString &arg1)
{
    QSerialPortInfo info;
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    int i = 0;
    foreach (info, infos) {
        if(info.portName() == arg1) break;
        i++;
    }
    serial->close();//串口初始化时处于关闭状态
    if(i != infos.size ()){//找到可用串口
        serial->setPort(info);
        //设置串口信息
        serial->setBaudRate(115200, QSerialPort::AllDirections);
        serial->setDataBits(QSerialPort::Data8);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setParity(QSerialPort::NoParity);

        return SERIAL_PORT_SUCCESS;
    }
    else
        return SERIAL_PORT_FAIL;
}

void SerialLogic::serialRead()
{
    QByteArray recDataBufferTemp;

    while(!serial->atEnd())
    {
//        qDebug("Bytes in IO device: %lld; Index:%d\n", serial->bytesAvailable(),i);
        recDataBufferTemp = serial->readAll();

        if(recDataBufferTemp.contains(0xDE) || (SerialReadDataNum > ReceiveData_Head && SerialReadDataNum < ReceiveData_Len) )
        {
            //2.0版本需要改进：
            //考虑到如果包头跟其他数据混在一起(可能性极低),将包头之后的数据分离出来
            //同样考虑到包尾与其他数据混在一起的可能性,分离包尾,如此得到完整的数据帧
            recDataBuffer += recDataBufferTemp;

            SerialReadDataNum += recDataBufferTemp.size();
            if(SerialReadDataNum >= ReceiveData_Len)
            {
                recDataBuffer = recDataBuffer.left(recDataBuffer.length()-
                                       recDataBuffer.indexOf(0xDE));
                SerialReadDataNum = 0;

                char *data= recDataBuffer.data();
//                for(int j= 0; j<recDataBuffer.size(); j++)
//                {
//                    qDebug("data:0x%0x\n", (unsigned char)data[j]);
//                }
                recDataBuffer.clear();

                //直接传递字符指针不是很安全，后续版本改进这个函数,对数组越界进行保护
                parseSerialData(data);

                //发送信号，使用QwtPlot画图
                if(g_EnablePlotFlag)
                    emit(sigUpdatePlot(TPP_SystemTime, MotorID, PositionData, SpeedData) );
            }
        }
        else if(recDataBufferTemp.contains(0xED) || (SerialAckDataNum > SendData_Head && SerialAckDataNum < SendData_Len))
        {//测试用，在提示框中显示发送下去的数据(由STM32端返回的字段)
            recDataBuffer += recDataBufferTemp;

            SerialAckDataNum += recDataBufferTemp.size();
            if(SerialAckDataNum >= SendData_Len)
            {
                recDataBuffer = recDataBuffer.left(recDataBuffer.length()-
                                       recDataBuffer.indexOf(0xED));
                SerialAckDataNum = 0;

                char *data= recDataBuffer.data();
                recDataBuffer.clear();

                parseAckData(data);

                emit(sigShowAckData(Ack_P, Ack_I, Ack_D));

            }

        }

    }
}

int SerialLogic::parseSerialData(char *pArray)
{
    int FrameTailSum = 0;
    char FrameTail;
    int *pData = (int *)(&pArray[ReceiveData_PositionData]);

    if(0 == (pArray[ReceiveData_Head] - 0xDE) )//检验包头
        return SerialParse_HeadError;

    if(0 == (pArray[ReceiveData_DeviceID] & 0x3F) )//检验电机ID
        return SerialParse_NoMotorFill;

    for(int i=0; i<ReceiveData_Tail; i++)//检验包尾
        FrameTailSum += pArray[i];
    FrameTail = (char)FrameTailSum;
    if(0 != (pArray[ReceiveData_Tail] - FrameTail) )
        return SerialParse_TailError;

    //提取数据
    PositionData = pData[0] / 100;
    SpeedData = pData[1] / 100;
    MotorID = pArray[ReceiveData_DeviceID] & 0x3F;

//    qDebug()<<"Position: "<<PositionData;

    return SerialParse_Success;
}

//测试用，分析STM32返回来的数据帧
int SerialLogic::parseAckData(char *pArray)
{
    int FrameTailSum = 0;
    char FrameTail;
    int *pData = (int *)(&pArray[SendData_DataFrame]);

    if(0 == (pArray[SendData_Head] - 0xED) )//检验包头
        return SerialParse_HeadError;

    if(0 == (pArray[SendData_DeviceID] & 0x3F) )//检验电机ID
        return SerialParse_NoMotorFill;

    for(int i=0; i<SendData_Tail; i++)//检验包尾
        FrameTailSum += pArray[i];
    FrameTail = (char)FrameTailSum;
    if(0 != (pArray[SendData_Tail] - FrameTail) )
        return SerialParse_TailError;

    //提取数据
    Ack_P = pData[0] / 100;
    Ack_I = pData[1] / 100;
    Ack_D = pData[2] / 100;

//    qDebug("ACK_P:%f\n", Ack_P);

    return SerialParse_Success;
}


double SerialLogic::getPositionData()
{
    return PositionData;
}

double SerialLogic::getSpeedData()
{
    return SpeedData;
}

int SerialLogic::serialWrite()
{
    int i;
    long FrameTail= 0;

    //填充包头
    serialDataToSend[SendData_Head] = 0xED;
    //校验是否选中位置环/速度环与电机类型
    if( !(serialDataToSend[SendData_DeviceID] & 0xC0) )
        return SerialSend_NoLoopChoice;
    if( !(serialDataToSend[SendData_DeviceID] & 0x3F) )
        return SerialSend_NoMotorChoice;
    //填充包尾:暂时使用和校验(后续版本改成CRC8校验符)
    for(i = 0; i < SendData_Tail; i++)
        FrameTail += serialDataToSend[i];
    serialDataToSend[SendData_Tail] = (char)FrameTail;

//    for(int j= 0; j< SendData_Len; j++)
//        qDebug("Data to Send:0x%0x\n", (unsigned char)serialDataToSend[j]);

    if(SendData_Len == serial->write(serialDataToSend, SendData_Len))
    {
        for(int j= 0;j<SendData_Len;j++)
            qDebug("Data:0x%0x, index:%d\n", (unsigned char)serialDataToSend[j], j);
        serial->flush();
        return SerialSend_Success;
    }
    else
        return SerialSend_WriteError;
}
