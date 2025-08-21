#ifndef LAN_PORT_COMMAND_H
#define LAN_PORT_COMMAND_H
#include <QTcpSocket>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include "com_port_command.h"

// состояния соединённости
enum ConnectionCondition
{
    DISCONNECTED = 0,
    CONNECTING      = 1,
    CONNECTED       = 2,
};

#define RCV_MARKER    0xf6          // маркер начала данных
#define GOOD_EXCHANGE_MARKER 0x55;
#define BAD_EXCHANGE_MARKER 0xCC;

// команды, прилетающие из ehternet
enum EthernetClientCmd
{
    SET_WAVE_LEN = 0,	// установить длину волны
    GET_WAVE_LEN = 1,	// получить текущую длину волны
    STOP_MOVING  = 2,   // прекратить движение
};

// команды, улетающие в ehternet
enum EthernetServerCmd
{
    WAVE_LEN_SETTED = 0,	// заданная длина волны установлена
    WAVE_LEN_SET_STOPED = 1,
    TAKE_WAVE_LEN   = 2,	// возвращаем длину волны на приборе
    ERROR_WHILE_WORKING=3,  // ошибка работы приложения
    APP_BISY        = 4,    // занятость
    CANT_MAKE_CMD   = 5,    // невозможно выполнить команду
    ERROR_EXCHANGE  = 6,    // ошибка обмена
};

class lan_port_command :public QObject
{
    Q_OBJECT
public:
    explicit lan_port_command(QMutex* ,QWaitCondition *,QWaitCondition *);
    ~lan_port_command();


    QByteArray data;    //массив данных передаваемый по интерфейсу
    // Буфер для передачи команд
    char* m_pTransmData;
    char* m_pRecieveData;

    bool stop_read = false; //переменная для остановки цикла
    bool stop_read_1 = false; //переменная для остановки цикла
    float cur_len_wave = 0;
    bool range_len = false;
    float l_max,l_min,l_step;
    QTcpSocket *socket; //сокет по котрому будем подключаться к серверу
    QString ip_add_pls;
    QString name_port_COM;
    quint16 port_pls;
    float len_wave_const =0;
    //bool connect_pls= false,set_l_wave_pls = false,set_only_l_wave_pls = false, continue_range = false, meas_curr = false;
    bool continue_range = false;
    bool write_answer=true;
    QMutex *mutex;
    QWaitCondition *wait_signal, *wait_signal_2;
    int choose_function=0;
    //1 -
    //2 -
    //3 -
    //4 -
    //5 -

    bool COM_connect, LAN_connect;


public slots:
    void connect_lan(QString ip_add, quint16 port, QString name_port_COM_port,bool state_LAN_con,bool state_COM_con);     //функция подключения
    void disconnect_all();
    void set_l_wave(float m_fWaveLenght);               //функция установки длины волны
    void set_only_l_wave(float m_fWaveLenght);          //функция установки длины волны
    void meas_curr_value(float m_fWaveLenght);
    void set_range_l_wave(float len_min, float len_max, float len_step);         //функция установки диапазона длин волн
    //QString disconnect();  //функция отключения
    void OnReadDataSignalEmited();
    void stop();
    void run();
    QString read_str_pic(COM_port_command *port);

signals:
    void signal_read_answer(float,QString);
    void signal_connect_answer(bool,bool);
    void signal_finish_thread();
    void signal_wave_setted();
    void error_server(); //сигнал ошибки отключения от сервера в дальнейшем можно добавить параметр и отправлять тип ошибки
    void end_meas();

};

#endif // LAN_PORT_COMMAND_H
