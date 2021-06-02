#include "SerialPort.h"
#pragma execution_character_set("utf-8")

SerialPort::SerialPort(QWidget *parent)
    : QMainWindow(parent)
{
	ui = new Ui::SerialPortClass();
    ui->setupUi(this);

	//初始化参数、
	concentration = 0;
	ui->lineEdit_file->setText(tr("E:\\Result\\3 实时DCS.txt"));
	filename_of_txt = tr("E:/Result/3 实时DCS.txt");

	//串口相关
	concentration_of_send = 0;//发送数据初始化为0
	m_serialPort = new QSerialPort();
	//获取能用的串口名称
	m_portNameList = getPortNameList();
	//下拉框显示
	ui->comboBox_serialport->addItems(m_portNameList);
	//打开串口
	connect(ui->pushButton_open_serialport, &QPushButton::clicked, this, &SerialPort::openPort);

	timer_process = new QTimer(this);
	timer_process->start();
	timer_process->setInterval(1000);//设置触发时间


	//选择原始图片文件夹
	connect(ui->pushButton_choose_file, &QPushButton::clicked, [=]() {
		filename_of_txt = QFileDialog::getOpenFileName(this, tr("file_of_txt path"), "D:");
		ui->lineEdit_file->setText(filename_of_txt);
	});


	//每隔设定时间计算一次
	connect(timer_process, &QTimer::timeout, [=]() {
		//从txt读取数据
		QFile f(filename_of_txt);
		if (!f.open(QIODevice::ReadOnly | QIODevice::Text))//打开指定文件
		{
			ui->label_message->setText(tr("打开文件失败"));
		}
		else
		{
			ui->label_message->setText(tr("打开文件成功"));
			while (!f.atEnd()) {
				QByteArray line = f.readLine();
				QString str(line);
				concentration = str.toDouble();
			}
			f.close();

			//设置浓度值
			ui->lineEdit_test->setText(QString::number(concentration));

			//设置发送的电流信号
			set_concentration();

			//向串口发送数据
			sendMyMessage_test();
		}


	});


}



void SerialPort::openPort()
{
	if (m_serialPort->isOpen())//如果串口已经打开了 先给他关闭了
	{
		m_serialPort->clear();
		m_serialPort->close();
	}

	//设置串口名字
	m_serialPort->setPortName(ui->comboBox_serialport->currentText());//当前选择的串口名字
																	  //打开串口
	if (!m_serialPort->open(QIODevice::ReadWrite))//用ReadWrite 的模式尝试打开串口
	{
		return;
	}
	//设置串口通讯参数
	m_serialPort->setBaudRate(QSerialPort::Baud9600, QSerialPort::AllDirections);//设置波特率和读写方向
	m_serialPort->setDataBits(QSerialPort::Data8);      //数据位为8位
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);//无流控制
	m_serialPort->setParity(QSerialPort::NoParity); //无校验位
	m_serialPort->setStopBits(QSerialPort::OneStop); //一位停止位

	connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(receiveInfo()));
}



////接收到单片机发送的数据进行解析
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
//QString转16进制

//获取串口列表
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



//字符串转16进制
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
	0x30等于十进制的48，48也是0的ASCII值，，
	1-9的ASCII值是49-57，，所以某一个值－0x30，，
	就是将字符0-9转换为0-9

	*/
	if ((ch >= '0') && (ch <= '9'))
		return ch - 0x30;
	else if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else return (-1);
}

//向串口发送信息
void SerialPort::sendInfo(const QString &info)
{
	QByteArray sendBuf;

	convertStringToHex(info, sendBuf); //把QString 转换 为 hex

	m_serialPort->write(sendBuf);//这句是真正的给单片机发数据 用到的是QIODevice::write 具体可以看文档
}

//设置浓度信息
void SerialPort::set_concentration()
{
	//获取检测范围
	QString min_qstring = ui->lineEdit_min->text();
	double min = min_qstring.toDouble() ;

	QString max_qstring = ui->lineEdie_max->text();
	double max = max_qstring.toDouble();

	concentration_of_send = ((concentration-min)/(max-min)) * 3276 +819;

    //如果电流大于4095则将其设置为4095
	if (concentration_of_send > 4095)
	{
		concentration_of_send = 4095;
	}
}




//设置发送数据
void SerialPort::setSendMessage_test(int send_data)
{


	frame.clear();
	frame.resize(8);
	//从机地址
	frame[0] = 1;

	//功能码
	frame[1] = 6;

	//寄存器地址
	frame[2] = 0x00;
	frame[3] = 0x00;
	//写如的数据为88
	int data = send_data;
	uint8_t data_bit[2];
	data_bit[0] = data >> 8;
	data_bit[1] = data;
	frame[4] = data_bit[0];
	frame[5] = data_bit[1];

	//CRC校验
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


// 将16进制字符串转换为对应的字节序列

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
				//QMessageBox::warning(0,tr("错误："),QString("非法的16进制字符: \"%1\"").arg(s));
			}
		}
	}
	return ret;
}


void SerialPort::sendMyMessage_test()
{
	//实战
	int sendData = concentration_of_send;
	setSendMessage_test(sendData);//设置参数
	QString str = frame;
	str = str.toUpper();
	ui->label_send_message->setText(str);
	//处理字符串
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