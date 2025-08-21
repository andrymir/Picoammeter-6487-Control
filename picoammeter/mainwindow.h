#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QThread>
#include <QWidget>
#include <QDebug>
#include "QFile"
#include "QTextStream"
#include "QDateTime"
#include "connect_ps.h"
#include "lan_port_command.h"
#include "QStandardItem"
#include "QStandardItemModel"
#include "QItemDelegate"
#include <QMap>
#include <QMapIterator>
#include <QMutex>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    float MAX_Y= 0,MAX_X = 0;        //границы для масштаба графика
    float MIN_Y= 1000,MIN_X = 1000;
    int i=0;

    connect_PS *dia;                                                                        //диалоговое окно настройки
    //double len_wave = 0;
    bool LAN_connect = false, COM_connect = false;                                          //флаги подключения к пикамперметру и к серверу
    bool connect_answer = false;                                                            //флаг ответа по запросу подключения
    bool format_txt=true, format_bin=true, format_csv=true;                                 //флаги форматов сохранения

    QString name_port_COM= "ttyS0", ip_add_server = "127.0.0.1", port_lan_server="1233";    //первоначальные параметры подключения

    QString MIN_range="",MAX_range="", step_range="";                                       //для автоматического режима границы диапазона и шаг

    lan_port_command *object;   //объект класса взаимодействия с сервером
    QThread thread;             //новый поток

    QStandardItem *item;                                       //айтем и модель для таблицы
    QStandardItemModel *model= new QStandardItemModel;
    QMutex mMutex,mMutex_2;
    QWaitCondition mWait, mWait_2;



public slots:
    void on_pushButton_set_L_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_connect_clicked();

    void connect_to_ps(QString name_port, QString ip_add, QString port_lan,bool save_bin,bool save_csv,bool save_txt);

    void on_pushButton_open_file_clicked();

    void LAN_read_answer(float l, QString data_out);

    void LAN_connect_answer(bool con, bool con2);

    void on_pushButton_stop_clicked();

    void data_changed_slot();

    void slot_rewrite_data();

    void slot_len_wave_setted();

    void SLOT_server_error();

signals:
    void stop_0();                                      //остановка всего потока
    void signal_connect(QString,quint16,QString,bool,bool);       //подключение
    void signal_disconnect();
    void signal_set_len_wave(float);                    //установка длины волны и сразу измерение силы тока
    void signal_set_range_len_wave(float,float,float);  //прохождение диапазона длин волн с заданым шагом
    void signal_meas_curr(float);                       //измерение силы тока
    void signal_set_only_len_wave(float);               //только установка длины волны

private slots:
    void on_pushButton_connect_setting_clicked();

    void on_pushButton_clear_buffer_clicked();

    void on_pushButton_meas_curr_clicked();

    void on_pushButton_delete_string_clicked();

    void on_pushButton_disconnect_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
