#include "com_port_command.h"
#include <QElapsedTimer>

COM_port_command::COM_port_command()
{

}

QString COM_port_command::IDN(){

    QString data_str="";

    QByteArray data;                                //массив данных передаваемых по интерфейсу



    serialPort.write("*RST\n");       //сбрасываем и заново настраиваем пикамперметр
    serialPort.write("SENS:FUNC 'CURR'\n");             //будем читать значения тока
    serialPort.write("SENS:CURR:RANGE:AUTO ON\n");      //автоматически выставляем диапазон измерения
    serialPort.write("SYST:ZCH OFF\n");     //выключаем сравнение относительно 0 - в доку написано:если сравнение 0 включено входной усилитель переконфигурирован для понижения входного сигнала в соответсвии с входным импедансом (я ничего не понял)
    serialPort.waitForBytesWritten();

    serialPort.write("*IDN?\n");                    //посылаем запрос информации об устройстве
    serialPort.waitForBytesWritten();               //ждем пока сообщение отправится
     data_str = read_str(false);

    return data_str;
}

QString COM_port_command::CONFigure(){

    QString data_str="";

    QByteArray data;                                //массив данных передаваемых по интерфейсу

    serialPort.write("CONFigure?\n");                    //посылаем запрос информации об устройстве
    serialPort.waitForBytesWritten();               //ждем пока сообщение отправится

    data_str = read_str(false);

    return data_str;
}

bool COM_port_command::connect(QString Name){ //подключение к пикамперметру

    // указали имя к какому порту будем подключаться
     serialPort.setPortName(Name);
    // указали скорость
    serialPort.setBaudRate(QSerialPort::Baud57600);

    // пробуем подключится
    if (!serialPort.open(QIODevice::ReadWrite)) {
       // если подключится не получится, то покажем сообщение с ошибкой
       QString warning= "Не удалось подключится к порту";
       return false;
    }
    else {
        //CONFigure();
        QString information;
        QString str = IDN();
        if(str.indexOf("KEITHLEY INSTRUMENTS INC.,MODEL 6487") != -1) return true;
        else return false;

}



}
QString COM_port_command::disconnect(){

    QString close = "Вы отключились от порта";
    serialPort.close();                     //при отключении закрываем порт
    return close;

}

QString COM_port_command::Curr(){ //чтение значения тока
    QString data_str="";

    QByteArray data;
    //либо как в проге примере

    serialPort.write("READ?\n");
    //ждем пока данные отправятся
    if(serialPort.waitForBytesWritten(2000)) // Modified
    {
        data_str = read_str(true);
    }
    else
        data_str = "";

   return data_str;

}

QString COM_port_command::read_str(bool curr_meas){ //чтение ответа
    QString data_str;
    QByteArray data;

    //читаем по таймеру 3с или пока не придет конец строки "\r"
    QElapsedTimer timer;
    QString str_curr="";
    int delay = 3000;
    bool bCathed = false;

    timer.start();
    while(timer.elapsed() < delay && bCathed == false)
    {
        if(serialPort.waitForReadyRead(100) == false){
            data_str="fail not read";           //если на запрос не ответили
            //qDebug()<<data_str;
            //break;
        }
        else {
        data = serialPort.readAll();            //если на запрос ответили
        data_str=QString(data);                 //то читаем данные
        str_curr+=data_str;
        if (str_curr.indexOf("\r") != -1) bCathed = true;

        }
    }
    //qDebug()<<timer.elapsed()<<" "<<bCathed;

    //if(curr_meas) str_curr.chop(30); //не уверен что это надо
    if(curr_meas) str_curr.chop(str_curr.size() - str_curr.indexOf("A"));

    qDebug()<<str_curr;


    return str_curr;

}
