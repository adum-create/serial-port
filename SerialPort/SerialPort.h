#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SerialPort.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
class SerialPort : public QMainWindow
{
    Q_OBJECT

public:
	explicit SerialPort(QWidget *parent = 0);

	//串口相关函数
	QStringList getPortNameList();//获取所有可用的串口列表
	void convertStringToHex(const QString &str, QByteArray &byteData);//QString转16进制
	char convertCharToHex(char ch);
	void sendInfo(const QString &info);//向串口发送信息
	//CRC校验
	uint16_t MB_CRC16_calculate(uint8_t * array, uint8_t index);
	//void setSendMessage(int send_data);//设置发送的报文帧
	void openPort();//打开串口
	QByteArray HexStringToByteArray(QString HexString);// 将16进制字符串转换为对应的字节序列

	void sendMyMessage_test();
	void setSendMessage_test(int send_data);
	void set_concentration();

	//串口相关变量
	QSerialPort* m_serialPort; //串口类
	QStringList m_portNameList;//串口名
	QByteArray frame;
	double concentration_of_send;//发送的电流值
	bool if_send;
	QTimer *timer_process;
	double concentration;
	QString filename_of_txt;
	
	


private:
    Ui::SerialPortClass *ui;
};
