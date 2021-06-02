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

	//������غ���
	QStringList getPortNameList();//��ȡ���п��õĴ����б�
	void convertStringToHex(const QString &str, QByteArray &byteData);//QStringת16����
	char convertCharToHex(char ch);
	void sendInfo(const QString &info);//�򴮿ڷ�����Ϣ
	//CRCУ��
	uint16_t MB_CRC16_calculate(uint8_t * array, uint8_t index);
	//void setSendMessage(int send_data);//���÷��͵ı���֡
	void openPort();//�򿪴���
	QByteArray HexStringToByteArray(QString HexString);// ��16�����ַ���ת��Ϊ��Ӧ���ֽ�����

	void sendMyMessage_test();
	void setSendMessage_test(int send_data);
	void set_concentration();

	//������ر���
	QSerialPort* m_serialPort; //������
	QStringList m_portNameList;//������
	QByteArray frame;
	double concentration_of_send;//���͵ĵ���ֵ
	bool if_send;
	QTimer *timer_process;
	double concentration;
	QString filename_of_txt;
	
	


private:
    Ui::SerialPortClass *ui;
};
