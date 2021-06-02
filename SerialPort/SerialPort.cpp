#include "SerialPort.h"
#pragma execution_character_set("utf-8")

SerialPort::SerialPort(QWidget *parent)
    : QMainWindow(parent)
{
	ui = new Ui::SerialPortClass();
    ui->setupUi(this);

	//��ʼ��������
	concentration = 0;
	ui->lineEdit_file->setText(tr("E:\\Result\\3 ʵʱDCS.txt"));
	filename_of_txt = tr("E:/Result/3 ʵʱDCS.txt");

	//�������
	concentration_of_send = 0;//�������ݳ�ʼ��Ϊ0
	m_serialPort = new QSerialPort();
	//��ȡ���õĴ�������
	m_portNameList = getPortNameList();
	//��������ʾ
	ui->comboBox_serialport->addItems(m_portNameList);
	//�򿪴���
	connect(ui->pushButton_open_serialport, &QPushButton::clicked, this, &SerialPort::openPort);

	timer_process = new QTimer(this);
	timer_process->start();
	timer_process->setInterval(1000);//���ô���ʱ��


	//ѡ��ԭʼͼƬ�ļ���
	connect(ui->pushButton_choose_file, &QPushButton::clicked, [=]() {
		filename_of_txt = QFileDialog::getOpenFileName(this, tr("file_of_txt path"), "D:");
		ui->lineEdit_file->setText(filename_of_txt);
	});


	//ÿ���趨ʱ�����һ��
	connect(timer_process, &QTimer::timeout, [=]() {
		//��txt��ȡ����
		QFile f(filename_of_txt);
		if (!f.open(QIODevice::ReadOnly | QIODevice::Text))//��ָ���ļ�
		{
			ui->label_message->setText(tr("���ļ�ʧ��"));
		}
		else
		{
			ui->label_message->setText(tr("���ļ��ɹ�"));
			while (!f.atEnd()) {
				QByteArray line = f.readLine();
				QString str(line);
				concentration = str.toDouble();
			}
			f.close();

			//����Ũ��ֵ
			ui->lineEdit_test->setText(QString::number(concentration));

			//���÷��͵ĵ����ź�
			set_concentration();

			//�򴮿ڷ�������
			sendMyMessage_test();
		}


	});


}



void SerialPort::openPort()
{
	if (m_serialPort->isOpen())//��������Ѿ����� �ȸ����ر���
	{
		m_serialPort->clear();
		m_serialPort->close();
	}

	//���ô�������
	m_serialPort->setPortName(ui->comboBox_serialport->currentText());//��ǰѡ��Ĵ�������
																	  //�򿪴���
	if (!m_serialPort->open(QIODevice::ReadWrite))//��ReadWrite ��ģʽ���Դ򿪴���
	{
		return;
	}
	//���ô���ͨѶ����
	m_serialPort->setBaudRate(QSerialPort::Baud9600, QSerialPort::AllDirections);//���ò����ʺͶ�д����
	m_serialPort->setDataBits(QSerialPort::Data8);      //����λΪ8λ
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//��������
	m_serialPort->setParity(QSerialPort::NoParity); //��У��λ
	m_serialPort->setStopBits(QSerialPort::OneStop); //һλֹͣλ

	connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receiveInfo()));
}



////���յ���Ƭ�����͵����ݽ��н���
//void MyMainWindow::receiveInfo()
//{
//	info = m_serialPort->readAll();
//	//info.resize(8);
//	test_refer.resize(8);
//	test_refer[0] = 0x01;
//	test_refer[1] = 0x04;
//	test_refer[2] = 0x00;
//	test_refer[3] = 0x02;
//	test_refer[4] = 0x00;
//	test_refer[5] = 0x01;
//	test_refer[6] = 0x90;
//	test_refer[7] = 0x0A;
//
//	int a = info.size();
//
//	if (info == test_refer)
//	{
//		sendMyMessage_test();
//	}
//}
//QStringת16����

//��ȡ�����б�
QStringList SerialPort::getPortNameList()
{
	QStringList m_serialPortName;
	QSerialPortInfo info;
	QList<QSerialPortInfo> listport;
	listport = QSerialPortInfo::availablePorts();
	for (int i = 0; i<listport.size(); i++)
	{
		m_serialPortName << listport[i].portName();
	}
	return m_serialPortName;
}



//�ַ���ת16����
void SerialPort::convertStringToHex(const QString &str, QByteArray &byteData)
{
	int hexdata, lowhexdata;
	int hexdatalen = 0;
	int len = str.length();
	byteData.resize(len / 2);
	char lstr, hstr;
	for (int i = 0; i<len; )
	{
		//char lstr,
		hstr = str[i].toLatin1();
		if (hstr == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		lstr = str[i].toLatin1();
		hexdata = convertCharToHex(hstr);
		lowhexdata = convertCharToHex(lstr);
		if ((hexdata == 16) || (lowhexdata == 16))
			break;
		else
			hexdata = hexdata * 16 + lowhexdata;
		i++;
		byteData[hexdatalen] = (char)hexdata;
		hexdatalen++;
	}
	byteData.resize(hexdatalen);
}

//
char SerialPort::convertCharToHex(char ch)
{
	/*
	0x30����ʮ���Ƶ�48��48Ҳ��0��ASCIIֵ����
	1-9��ASCIIֵ��49-57��������ĳһ��ֵ��0x30����
	���ǽ��ַ�0-9ת��Ϊ0-9

	*/
	if ((ch >= '0') && (ch <= '9'))
		return ch - 0x30;
	else if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else return (-1);
}

//�򴮿ڷ�����Ϣ
void SerialPort::sendInfo(const QString &info)
{
	QByteArray sendBuf;

	convertStringToHex(info, sendBuf); //��QString ת�� Ϊ hex

	m_serialPort->write(sendBuf);//����������ĸ���Ƭ�������� �õ�����QIODevice::write ������Կ��ĵ�
}

//����Ũ����Ϣ
void SerialPort::set_concentration()
{
	//��ȡ��ⷶΧ
	QString min_qstring = ui->lineEdit_min->text();
	double min = min_qstring.toDouble() ;

	QString max_qstring = ui->lineEdie_max->text();
	double max = max_qstring.toDouble();

	concentration_of_send = ((concentration-min)/(max-min)) * 3276 +819;

    //�����������4095��������Ϊ4095
	if (concentration_of_send > 4095)
	{
		concentration_of_send = 4095;
	}
}




//���÷�������
void SerialPort::setSendMessage_test(int send_data)
{


	frame.clear();
	frame.resize(8);
	//�ӻ���ַ
	frame[0] = 1;

	//������
	frame[1] = 6;

	//�Ĵ�����ַ
	frame[2] = 0x00;
	frame[3] = 0x00;
	//д�������Ϊ88
	int data = send_data;
	uint8_t data_bit[2];
	data_bit[0] = data >> 8;
	data_bit[1] = data;
	frame[4] = data_bit[0];
	frame[5] = data_bit[1];

	//CRCУ��
	uint8_t  crc16[6];
	crc16[0] = 1;
	crc16[1] = 6;
	crc16[2] = 0x00;
	crc16[3] = 0x00;
	crc16[4] = data_bit[0];
	crc16[5] = data_bit[1];

	uint16_t res = MB_CRC16_calculate(crc16, 6);

	int tmp = res;
	uint8_t data_bit_tmp[2];
	data_bit_tmp[0] = tmp >> 8;
	data_bit_tmp[1] = tmp;
	frame[6] = data_bit_tmp[1];

	frame[7] = data_bit_tmp[0];
	frame = frame.toHex();
}

//CRC
uint16_t SerialPort::MB_CRC16_calculate(uint8_t * array, uint8_t index)
{
	uint16_t tmp = 0xffff;
	uint16_t ret1 = 0;

	for (int n = 0; n < index; n++) {
		tmp = array[n] ^ tmp;
		for (int i = 0; i < 8; i++) {
			if (tmp & 0x01) {
				tmp = tmp >> 1;
				tmp = tmp ^ 0xa001;
			}
			else {
				tmp = tmp >> 1;
			}
		}
	}


	ret1 = tmp >> 8;
	ret1 = ret1 | (tmp << 8);
	return tmp;
}


// ��16�����ַ���ת��Ϊ��Ӧ���ֽ�����

QByteArray SerialPort::HexStringToByteArray(QString HexString)
{
	bool ok;
	QByteArray ret;
	HexString = HexString.trimmed();
	HexString = HexString.simplified();
	QStringList sl = HexString.split(" ");
	sl.size();
	//foreach(QString s, sl) 
	for (int i = 0; i< sl.size(); i++)
	{
		QString s = sl[i];
		if (!s.isEmpty())
		{
			char c = s.toInt(&ok, 16) & 0xFF;
			if (ok) {
				ret.append(c);
			}
			else {
				//QMessageBox::warning(0,tr("����"),QString("�Ƿ���16�����ַ�: \"%1\"").arg(s));
			}
		}
	}
	return ret;
}


void SerialPort::sendMyMessage_test()
{
	//ʵս
	int sendData = concentration_of_send;
	setSendMessage_test(sendData);//���ò���
	QString str = frame;
	str = str.toUpper();
	ui->label_send_message->setText(str);
	//�����ַ���
	QString end = "";
	end = end + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);
	str.remove(0, 2);
	end = end + " " + str.left(2);


	QByteArray myDate = HexStringToByteArray(end);
	qint64 abcd = m_serialPort->write(myDate);
	//	qDebug() << "xixi";

}