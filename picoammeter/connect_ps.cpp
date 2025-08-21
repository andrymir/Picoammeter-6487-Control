#include "connect_ps.h"
#include "ui_connect_PS.h"


connect_PS::connect_PS(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::connect_PS)
{
  ui->setupUi(this);
  this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint); //убираем кнопки закрытия и свертывания, чтобы основной окно не зависло в бесконечном цикле
  foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
  {
      ui->cmbPort->addItem(serialPortInfo.portName());

  }
  ui->lineEdit_IP_adress->setInputMask(QStringLiteral("009.009.009.009"));
  // пробуем прочесть сохраненные данные
  QString find_string_format;
  QFile scriptFile("conf.ini");
  if(scriptFile.exists())
  {
     QSettings stgs("conf.ini", QSettings::IniFormat);
     stgs.beginGroup("PARAM");
      ip_add_server           = stgs.value("Server_IP", ip_add_server).toString();
      port_lan_server           = stgs.value("Server_port", port_lan_server).toString();
      name_port_COM           = stgs.value("COM_port", name_port_COM).toString();
      find_string_format = stgs.value("format_save", find_string_format).toString();
      stgs.endGroup();

      if(find_string_format.indexOf("bin") == -1) format_bin = false;
      if(find_string_format.indexOf("csv") == -1) format_csv = false;
      if(find_string_format.indexOf("txt") == -1) format_txt = false;
  }
  ui->lineEdit_IP_adress->setText(ip_add_server);
  ui->lineEdit_port->setText(port_lan_server);
  ui->checkBox_BIN->setChecked(format_bin);
  ui->checkBox_CSV->setChecked(format_csv);
  ui->checkBox_TXT->setChecked(format_txt);
  ui->cmbPort->setCurrentText(name_port_COM);

}
connect_PS::~connect_PS()
{
    delete ui;

}

void connect_PS::on_pushButton_Connect_clicked() //нажатие кнопки подключения
{
    if(ui->checkBox_BIN->checkState() || ui->checkBox_CSV->checkState() || ui->checkBox_TXT->checkState()){ //смотрим чтобы хотя бы один из форматов сохранения был выбран
        ip_add_server           = ui->lineEdit_IP_adress->text();
        port_lan_server           = ui->lineEdit_port->text();
        name_port_COM           = this->ui->cmbPort->currentText();
        format_bin = ui->checkBox_BIN->checkState();
        format_csv = ui->checkBox_CSV->checkState();
        format_txt = ui->checkBox_TXT->checkState();
    emit connect_to_ps(this->ui->cmbPort->currentText(),ui->lineEdit_IP_adress->text(),ui->lineEdit_port->text(),ui->checkBox_BIN->checkState(),ui->checkBox_CSV->checkState(),ui->checkBox_TXT->checkState());
    this->close();}
    else QMessageBox::warning(this, "Внимание", "Выберите хотя бы один из форматов для сохранения");

}

void connect_PS::slot_update_clicked() //нажатие кнопки обновления
{
    ui->cmbPort->clear();
    // обновляем информацию о доступных портах
        foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
        {
            ui->cmbPort->addItem(serialPortInfo.portName());
        }
   ui->cmbPort->setCurrentText(name_port_COM);
}


void connect_PS::on_pushButton_Close_window_clicked()
{
    ui->lineEdit_IP_adress->setText(ip_add_server);
    ui->lineEdit_port->setText(port_lan_server);
    ui->checkBox_BIN->setChecked(format_bin);
    ui->checkBox_CSV->setChecked(format_csv);
    ui->checkBox_TXT->setChecked(format_txt);
    ui->cmbPort->setCurrentText(name_port_COM);
    emit close_window();
    this->close();
}
