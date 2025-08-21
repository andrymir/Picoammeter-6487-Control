#ifndef COM_PORT_COMMAND_H
#define COM_PORT_COMMAND_H
#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QDebug>

class COM_port_command
{
public:
    COM_port_command();
    QString IDN();                      //функция индефикации устройства
    QString CONFigure();                      //функция индефикации устройства

    QSerialPort serialPort;             //порт по которому будем подключаться
    bool connect(QString Name);      //функция подключения
    QString disconnect();               //функция отключения
    QString Curr();                     //функция об информации текущей силы тока
    QString read_str(bool curr_meas);

};

#endif // COM_PORT_COMMAND_H
