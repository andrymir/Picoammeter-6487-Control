#ifndef CONNECT_PS_H
#define CONNECT_PS_H
#include <QWidget>
#include <QSerialPortInfo>
#include <QMessageBox>
#include "QFile"
#include <QSettings>
#include <QDebug>

namespace Ui {
class connect_PS;
}

class connect_PS : public QWidget  //окно подключения
{
    Q_OBJECT
public:
    explicit connect_PS(QWidget *parent = nullptr);
    ~connect_PS();
    QString name_port_COM= "ttyS0", ip_add_server = "127.0.0.1", port_lan_server="1233"; //первоначальные параметры подключения
    bool format_txt=true, format_bin=true, format_csv=true; //флаги форматов сохранения

signals:
    void connect_to_ps(QString data,QString ip,QString port,bool,bool,bool);
    void close_window();

private slots:

    void on_pushButton_Connect_clicked();

    void on_pushButton_Close_window_clicked();

public slots:

    void slot_update_clicked();

private:
    Ui::connect_PS *ui;


};

#endif // CONNECT_PS_H
