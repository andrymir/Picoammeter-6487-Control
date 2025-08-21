#ifndef LAN_PORT_COMMAND_H
#define LAN_PORT_COMMAND_H
#include <QTcpSocket>
#include <QObject>
#include <QThread>
#include <QDebug>

// состояния соединённости
enum ConnectionCondition
{
    DISCONNECTED = 0,
    CONNECTING      = 1,
    CONNECTED       = 2,
};

// про интернет всякое
#define RCV_MARKER    0xf6          // маркер начала данных
#define GOOD_EXCHANGE_MARKER 0x55;
#define BAD_EXCHANGE_MARKER 0xCC;

// команды, прилетающие из инета
enum EthernetClientCmd
{
    SET_WAVE_LEN = 0,	// установи ка, брат. вот такую длину волны
    GET_WAVE_LEN = 1,	// дай посмотреть. чото там выставилось
    STOP_MOVING  = 2,   // прекратить движение
};

// команды, улетающие в инет
enum EthernetServerCmd
{
    WAVE_LEN_SETTED = 0,	// заданная длина волны установлена
    WAVE_LEN_SET_STOPED = 1,
    TAKE_WAVE_LEN   = 2,	// возвращаем длину волны на приборе
    ERROR_WHILE_WORKING=3,  // ошибка работы приложения
    APP_BISY        = 4,    // обожжи
    CANT_MAKE_CMD   = 5,    // невозможно выполнить команду
    ERROR_EXCHANGE  = 6,    // ошибка инет обмена
};

class lan_port_command :public QObject
{
    Q_OBJECT
public:
    explicit lan_port_command();
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
    quint16 port_pls;
    float len_wave_const =0;
    bool connect_pls= false,set_l_wave_pls = false, continue_range = false;

public slots:
    void connect_lan(QString ip_add, quint16 port);     //функция подключения
    void set_l_wave(float m_fWaveLenght);         //функция установки длины волны
    void set_range_l_wave(float len_min, float len_max, float len_step);         //функция установки диапазона длин волн
    //QString disconnect();  //функция отключения
    void OnReadDataSignalEmited();
    void stop();
    void run();
    void test(QString ip_add, quint16 port);

signals:
    void signal_read_answer(float l);
    void signal_connect_answer(bool con);
    void signal_finish_thread();

};

#endif // LAN_PORT_COMMAND_H
