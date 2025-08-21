#include "mainwindow.h"
#include "ui_mainwindow.h"

QVector<double> X_L, Y_I;

QMap <float,double> Values;

QList<double> val_list;
QList<float> key_list;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //производим настройку графиков для первого окна
  ui->widget->clearGraphs();
  ui->widget->addGraph();     //добавляем график
  ui->widget->xAxis->setLabel("L, нм");     //подписываем ос ox как длина волны
  ui->widget->yAxis->setLabel("I , A"); //подписываем ос oy как ток
  ui->widget->xAxis->setRange(400,1000);    //вводим начальный диапазон значений графиков
  ui->widget->yAxis->setRange(0,1);
  ui->widget->graph(0)->setName("Чувствительность фотодиода");    //изменяем названия графика


  dia = new connect_PS(this); //окно для подключения фотодиода
  connect(dia,SIGNAL(connect_to_ps(QString, QString ,QString,bool,bool,bool)),this,SLOT(connect_to_ps(QString, QString, QString,bool,bool,bool))); //также сигнал нажатия подключения со слотом подключения

  ui->widget->setInteraction(QCP::iRangeZoom,true); //добавление зума
  ui->widget->setInteraction(QCP::iRangeDrag,true);

  ui->pushButton_set_L->setEnabled(false);     //делаем неактивными кнопки до подключения
  ui->pushButton_start->setEnabled(false);
  ui->pushButton_meas_curr->setEnabled(false);
  ui->pushButton_stop->setEnabled(false);

  ui->label_connect_pic->setText("ПИК-амперметр\t - НЕ ПОДКЛЮЧЕНО");
  ui->label_connect_server->setText("Сервер\t\t - НЕ ПОДКЛЮЧЕНО");

  object = new lan_port_command(&mMutex, &mWait, &mWait_2);
  object->moveToThread(&thread);  //передаем наш класс в отдельный поток

  connect(&thread, &QThread::started, object, &lan_port_command::run);


  connect(this,SIGNAL(stop_0()),object,SLOT(stop()),Qt::DirectConnection); //остановка
  connect(this,SIGNAL(signal_connect(QString,quint16,QString,bool,bool)),object,SLOT(connect_lan(QString,quint16,QString,bool,bool)),Qt::DirectConnection);        //соединение
  connect(this,SIGNAL(signal_disconnect()),object,SLOT(disconnect_all()),Qt::DirectConnection);        //отключение
  connect(this,SIGNAL(signal_set_len_wave(float)),object,SLOT(set_l_wave(float)),Qt::DirectConnection);                                        //установка длины волны
  connect(this,SIGNAL(signal_set_only_len_wave(float)),object,SLOT(set_only_l_wave(float)),Qt::DirectConnection);                              //установка только длины волны
  connect(this,SIGNAL(signal_set_range_len_wave(float,float,float)),object,SLOT(set_range_l_wave(float,float,float)),Qt::DirectConnection);    //установка диапазона
  connect(this,SIGNAL(signal_meas_curr(float)),object,SLOT(meas_curr_value(float)),Qt::DirectConnection);                                      //установка диапазона


  connect(object,SIGNAL(signal_read_answer(float,QString)),this,SLOT(LAN_read_answer(float,QString)),Qt::QueuedConnection);                    //прочитал ответ
  connect(object,SIGNAL(signal_wave_setted()),this,SLOT(slot_len_wave_setted()),Qt::QueuedConnection);                                         //длина волны установлена
  connect(object,SIGNAL(signal_connect_answer(bool,bool)),this,SLOT(LAN_connect_answer(bool,bool)),Qt::DirectConnection);                      //ответ по подключению
  connect(object,SIGNAL(error_server()),this,SLOT(SLOT_server_error()),Qt::QueuedConnection);                                                  //ошибка сервера

  connect(object,SIGNAL(end_meas()),this,SLOT(on_pushButton_stop_clicked()),Qt::QueuedConnection);                                                  //ошибка сервера

  thread.start(); //начинаем работу потока


  //настройка таблицы
  model->clear();
  model->setColumnCount(2);     //устанавливаем 2 столбца
  model->setHorizontalHeaderLabels(QStringList() <<"L (длина волны)"<<"I (сила тока)"); //задаем названия
  ui->tableView->resizeRowsToContents(); //корректируем ширину столбцов
  ui->tableView->horizontalHeader()->setStretchLastSection(true); //последний столбец растягиваем до конца таблицы

  ui->tableView->setModel(model);           //применяем настроенную модель к таблице
  ui->tableView->resizeColumnsToContents(); //корректируем ширину столбцов

  // пробуем прочесть ip
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


}

MainWindow::~MainWindow()
{
    //параметры ini файла
    QString ini_save="";
    QSettings stgs("conf.ini", QSettings::IniFormat);
    stgs.beginGroup("PARAM");
    stgs.setValue("Server_IP", ip_add_server);
    stgs.setValue("Server_port", port_lan_server);
    stgs.setValue("COM_port", name_port_COM);
    stgs.setValue("MIN_range", MIN_range);
    stgs.setValue("MAX_range", MAX_range);
    stgs.setValue("step_range", step_range);
    if (format_bin) ini_save+=" bin,";
    if (format_csv) ini_save+=" csv,";
    if (format_txt) ini_save+=" txt,";
    ini_save.chop(1);
    stgs.setValue("format_save", ini_save);
    stgs.endGroup();
    //if(mMutex.tryLock()) mMutex.unlock(); //что то не получилось использовать
    mWait.wakeOne();

    emit stop_0();  //заканчиваем работу потока и выходим
    thread.quit();
    int tryCnt = 0;
    while(!thread.isFinished())     // Modified
    {
        QThread::msleep(100);
        if(tryCnt == 10)
        {
            // что-то не завершается..
            thread.terminate();
            break;
        }
        else
            ++tryCnt;
    }

    if(dia != nullptr)  // Modified
        delete dia;

    delete ui;

}


void MainWindow::on_pushButton_set_L_clicked()
{

    float len = ui->lineEdit_L->text().toFloat();

        //проверка числа по формату и значению
    if(len==0) {
        ui->lineEdit_L->clear();
        QMessageBox::information(this, "Внимание", "Введено неверное по формату значение"); //выводим информацию об подключении в всплывающем окне
    }
    else if(len<400 || len>1000){
        ui->lineEdit_L->clear();
        QMessageBox::information(this, "Внимание", "Значение должно быть в промежутке от 400нм до 1000нм"); //выводим информацию об подключении в всплывающем окне
    }
    else{
         ui->pushButton_stop->setEnabled(true);
         if(ui->checkBox_fast_way->checkState()){   //проверяем установлено ли быстрое измерение
         emit signal_set_len_wave(len);               //установить длину волны
         mWait.wakeAll();}
         else {
             ui->pushButton_meas_curr->setEnabled(false);  //установим только длину волны пока она не установлена кнопки измерения и начала диапазона сделаем неактивными
             ui->pushButton_start->setEnabled(false);
             emit signal_set_only_len_wave(len);
             mWait.wakeAll();
         }
   }
}

void MainWindow::on_pushButton_meas_curr_clicked() //слот только измерения значения пикамперметра, значение длины волны берем из введенного (необходимо при отключенном сервере или просто для бысрого получения значения тока)
{
    float len = ui->lineEdit_L->text().toFloat();
     //проверка числа по формату и значению
    if(len==0) {
        ui->lineEdit_L->clear();
        QMessageBox::information(this, "Внимание", "Введено неверное по формату значение"); //выводим информацию об подключении в всплывающем окне
    }
    else if(len<400 || len>1000){
        ui->lineEdit_L->clear();
        QMessageBox::information(this, "Внимание", "Значение должно быть в промежутке от 400нм до 1000нм"); //выводим информацию об подключении в всплывающем окне
    }
    else{
         ui->pushButton_stop->setEnabled(true);
         emit signal_meas_curr(len); //измерить значение тока
         mWait.wakeAll();
    }
}

void MainWindow::on_pushButton_save_clicked() //сохранение в файл
{
    QString a="",ADD_save;
    QString ADD= QFileDialog::getSaveFileName(this, tr("Выбрать файл"), "","");

    if(format_txt){                                     //сохранение в текстовом формате
    ADD_save = ADD+".txt";
    QFile tFile(ADD_save);
    tFile.open(QIODevice::WriteOnly|QFile::Text);

    QTextStream stream(&tFile);

    for(int i=0; i<X_L.size();i++)
    {
    a=QString::number(X_L[i]);
    QString ab=QString::number(Y_I[i]);
    stream<<a+" "+ab+"\n";
    }
    tFile.close();}


    if(format_bin){
    ADD_save = ADD+".bin";                               //сохранение в бинарном виде
    QFile tFile2(ADD_save);
    tFile2.open(QIODevice::WriteOnly);

    QDataStream stream2(&tFile2);
    stream2.setByteOrder(QDataStream::LittleEndian);
    for(int i=0; i<X_L.size();i++)
    {
    stream2<<X_L[i]<<Y_I[i];
    }
    tFile2.close();}

    if(format_csv){
        ADD_save = ADD+".csv";                               //сохранение в csv виде
        QFile tFile3(ADD_save);
        tFile3.open(QIODevice::WriteOnly);

        QTextStream stream3(&tFile3);
        //QString a3="";
        for(int i=0; i<X_L.size();i++)
        {
            a=QString::number(X_L[i]);
            QString ab=QString::number(Y_I[i]);
            a.replace(".",",");
            ab.replace(".",",");
            stream3<<a<<";"<<ab<<"\n";
        }
        tFile3.close();
    }

}

void MainWindow::on_pushButton_start_clicked() //слот начала измерения диапазона значений
{
    float l_max = ui->lineEdit_L_MAX->text().toFloat();         //считываем значения границ диапазона и шага
    float l_min = ui->lineEdit_L_MIN->text().toFloat();
    float l_step = ui->lineEdit_L_step->text().toFloat();
    if(l_max>=400 && l_min>=400 && l_step>0 && l_max<=1000 && l_min<=1000 && l_min<=l_max){ //проверяем на правильность
        ui->pushButton_stop->setEnabled(true);

        emit signal_set_range_len_wave(l_min,l_max,l_step);
        mWait.wakeAll();
        MIN_range=ui->lineEdit_L_MIN->text(); //запоминаем строки для сохранения в ini файл
        MAX_range=ui->lineEdit_L_MAX->text();
        step_range=ui->lineEdit_L_step->text();
        ui->pushButton_start->setEnabled(false);
    }
    else {
        ui->lineEdit_L_MAX->clear();
        ui->lineEdit_L_MIN->clear();
        ui->lineEdit_L_step->clear();
        QMessageBox::information(this, "Внимание", "Значения границ диапазона должны быть в промежутке от 400нм до 1000нм, а значение шага больше 0"); //выводим информацию об подключении в всплывающем окне
    }
}

void MainWindow::on_pushButton_connect_clicked() //слот подключения
{

    emit signal_connect(ip_add_server, port_lan_server.toInt(),name_port_COM,LAN_connect,COM_connect); //отправляем сигнал подключения к серверу и пикамперметру
    mWait.wakeAll();

    mMutex_2.lock();
    mWait_2.wait(&mMutex_2);
    //qDebug()<<LAN_connect<<" "<<COM_connect;
    mMutex_2.unlock();

    if(!LAN_connect) qDebug()<<"ошибка подключения сервера";            //при ошибки подкючения выводим сообщения
    if(!COM_connect) qDebug()<<"ошибка подключения ПИК-амперметра";

    if(LAN_connect && COM_connect){             //при успешном подключении делаем активными кнопки
       qDebug()<<"successful connection";
    ui->pushButton_set_L->setEnabled(true);
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_meas_curr->setEnabled(true);
    ui->label_connect_pic->setText("ПИК-амперметр\t - ПОДКЛЮЧЕНО");
    ui->label_connect_server->setText("Сервер\t\t - ПОДКЛЮЧЕНО");

    }
    else{
        if(COM_connect) {
            ui->pushButton_meas_curr->setEnabled(true); //при поключении  только к пикамперметру делаем активной только кнопку измерения
            ui->label_connect_pic->setText("ПИК-амперметр\t - ПОДКЛЮЧЕНО");
        }
        else ui->pushButton_meas_curr->setEnabled(false);
        ui->pushButton_start->setEnabled(false);
        ui->pushButton_set_L->setEnabled(false);
    }

}
void MainWindow::connect_to_ps(QString name_port, QString ip_add, QString port_lan,bool save_bin,bool save_csv,bool save_txt){ //слот обработки информации настройки
    //считываем значения пришедшие с окна настройки
        name_port_COM = name_port;
        ip_add_server = ip_add;
        port_lan_server = port_lan;
        format_bin = save_bin;
        format_csv = save_csv;
        format_txt = save_txt;
}


void MainWindow::on_pushButton_open_file_clicked()  //слот открытия файла
{
    Values.clear();
    MAX_Y=0;
    MIN_Y=1000;
    MAX_X=0;
    MIN_X=1000;  // сбрасываем значения
    QString ADD= QFileDialog::getOpenFileName(this, tr("Выбрать файл"), "","(Text files *.txt)"); //значения читаем с текстового файла если надо будет переделаю на бинарник
    QFile tFile(ADD);
    tFile.open(QIODevice::ReadOnly|QFile::Text);

    QTextStream stream(&tFile);
    QString a="";
    QStringList lst;
    float x;
    double y;

    while(!stream.atEnd()){
        a=stream.readLine();
        lst = a.split(" ");

        x=lst[0].toFloat();
        y=lst[1].toDouble();

        Values.insert(x,y);

        if(MAX_Y<y) MAX_Y = y;      //мониторим максимальное значение для нормального отображение на графике значений
        if(MIN_Y>y) MIN_Y = y;      //мониторим минимальное значение для нормального отображение на графике значений
        if(MAX_X<x) MAX_X = x;      //мониторим максимальное значение для нормального отображение на графике значений
        if(MIN_X>x) MIN_X = x;      //мониторим минимальное значение для нормального отображение на графике значений
    }
    tFile.close();

    slot_rewrite_data();
}

void MainWindow::LAN_read_answer(float l,QString data_out){  //ответ с потока
    if(!object->stop_read_1 || !(object->choose_function==2)){  // не заходим только если принудительно прервали ожидание от одиночного быстрого измерения
    //if(data_out != "fail not read" && data_out != ""){  //расскоментить в итоге
//    ui->textBrowser_Curr->clear();
//    ui->textBrowser_Curr->append(data_out);         //то выводим информацию на экран


    Values.insert(l,data_out.toDouble());



    if(MAX_Y<data_out.toDouble()) MAX_Y = data_out.toDouble();      //мониторим максимальное значение для нормального отображение на графике значений
    if(MIN_Y>data_out.toDouble()) MIN_Y = data_out.toDouble();      //мониторим минимальное значение для нормального отображение на графике значений

    if(MAX_X<l) MAX_X = l;      //мониторим максимальное значение для нормального отображение на графике значений
    if(MIN_X>l) MIN_X = l;      //мониторим минимальное значение для нормального отображение на графике значений

    slot_rewrite_data();  // вызываем слот построения графика и занесения в таблицу полученых в массив значений
    //}
    }


    object->continue_range = true; //для диапазона продолжаем измерение
}

void MainWindow::LAN_connect_answer(bool con, bool con2) //слот ответа по подключению сервера и пик амперметра
{
    LAN_connect = con;  //читаем полученные данные по подключению
    COM_connect = con2;
    qDebug()<<"connecting...";
}

void MainWindow::on_pushButton_stop_clicked()
{
    ui->pushButton_stop->setEnabled(false);
    object->stop_read_1 = true; //принудительно останавливаем работу одной из функций потока
    QMessageBox::information(this,"Внимание","Измерение закончено");
    if(LAN_connect) ui->pushButton_start->setEnabled(true);
    //mWait.wakeAll();
}

void MainWindow::data_changed_slot() //слот изменения таблицы - вызывается при изменении значения в таблице
{
    qDebug()<<"changed";
    //сделаем по тупому - после изменения значения таблицы полностью очистим наш массив значений и заполним занового из значений с таблицы
    Values.clear();
    MAX_Y= 0,MAX_X = 0;
    MIN_Y= 1000,MIN_X = 1000;
    QMap<int, QVariant> map_changed_1;
    QMap<int, QVariant> map_changed;
    QString str, str_1;
    QList <QVariant> list_changed, list_changed_1;



    for (int k=0;k<model->rowCount();k++) {
        map_changed_1 = model->itemData(model->index(k,1));
        map_changed = model->itemData(model->index(k,0));
        list_changed = map_changed.values();
        for(auto j:list_changed) str = j.toString();

        list_changed_1 = map_changed_1.values();
        for(auto j:list_changed_1) str_1 = j.toString();

        Values.insert(str.toFloat(),str_1.toDouble());

        if(MAX_Y<str_1.toDouble()) MAX_Y = str_1.toDouble();      //мониторим максимальное значение для нормального отображение на графике значений
        if(MIN_Y>str_1.toDouble()) MIN_Y = str_1.toDouble();      //мониторим минимальное значение для нормального отображение на графике значений

        if(MAX_X<str.toFloat()) MAX_X = str.toFloat();      //мониторим максимальное значение для нормального отображение на графике значений
        if(MIN_X>str.toFloat()) MIN_X = str.toFloat();      //мониторим минимальное значение для нормального отображение на графике значений

        //qDebug()<<MIN_X<<" "<<MAX_X<<" "<<MIN_Y<<" "<<MAX_Y;

    }
    slot_rewrite_data(); // вызываем слот построения графика и занесения в таблицу полученых в массив значений


}



void MainWindow::on_pushButton_connect_setting_clicked() // слот вызова окна настройки
{
    QEventLoop loop(this);
    connect(dia,SIGNAL(connect_to_ps(QString, QString ,QString,bool,bool,bool)),&loop,SLOT(quit())); //связываем нажатия кнопки подключения с выходом бесконечного цикла
    connect(dia,SIGNAL(close_window()),&loop,SLOT(quit())); //связываем нажатия кнопки подключения с выходом бесконечного цикла

    dia->setWindowModality(Qt::WindowModality::WindowModal);  // Modified

    dia->show(); //подключение к пикоамперметру
    dia->slot_update_clicked();


    loop.exec();
}

void MainWindow::slot_rewrite_data() // слот построения графика и занесения в таблицу полученых в массив значений
{
    disconnect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(data_changed_slot())); //так как таблица будет меняться отключим сигнал изменения таблицы
    X_L.clear();
    Y_I.clear();
    val_list = Values.values();
    for(auto j:val_list) Y_I.push_back(j);

    key_list = Values.keys();
    for(auto j:key_list) X_L.push_back(j);

    ui->widget->yAxis->setRange(0.8*MIN_Y,(1.1*MAX_Y));     //относительно максимального значения выставляем диапазон показываемых значений
    ui->widget->xAxis->setRange(0.98*MIN_X,(1.01*MAX_X));   //относительно максимального значения выставляем диапазон показываемых значений

    ui->widget->graph(0)->setData(X_L,Y_I);

    ui->widget->replot();           //перерисовываем график с новыми значениями

    i=0;
    model->clear();
    model->setColumnCount(2);       //устанавливаем 2 столбца
    model->setHorizontalHeaderLabels(QStringList() <<"L (длина волны)"<<"I (сила тока)");       //задаем названия
    ui->tableView->resizeRowsToContents();      //корректируем ширину строк

    ui->tableView->setModel(model);             //применяем настроенную модель к таблице
    ui->tableView->resizeColumnsToContents();   //корректируем ширину столбцов

    //заполняем таблицу
    for(auto j:key_list){
    item = new QStandardItem(QString::number(j));
    //item->setEditable(0);
    model->setItem(i,0,item);
    i++;}
    i=0;
    for(auto j:val_list){
    item = new QStandardItem(QString::number(j));
    //item->setEditable(0);
    model->setItem(i,1,item);
    i++;}

    ui->tableView->resizeColumnsToContents();   //корректируем строки по ширине
    ui->tableView->resizeRowsToContents();      //корректируем ширину строк
    connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(data_changed_slot())); //обратно конектим сигнал изменения таблицы
}

void MainWindow::on_pushButton_clear_buffer_clicked() //слот очистки значений массива и таблицы
{
    QMessageBox::StandardButton answer = QMessageBox::question(this,"!?!","Полностью очистить таблицу?");

    if(answer == QMessageBox::Yes){
    Values.clear();
    MAX_Y=0;
    MIN_Y=10;
    MAX_X=0;
    MIN_X=1000;
    slot_rewrite_data();}
}

void MainWindow::slot_len_wave_setted() // серевер ответил что установил длину волны
{
    ui->pushButton_meas_curr->setEnabled(true);
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
}

void MainWindow::SLOT_server_error() // ошибка от сервера - в данной версии только потеряно соединение
{
    LAN_connect = false;
    //qDebug()<<"ошибка подключения сервера";            //при ошибки подкючения выводим сообщения
    QMessageBox::warning(this,"Внимание","Потеряно сединение с сервером");
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_set_L->setEnabled(false);
    ui->label_connect_server->setText("Сервер\t\t - НЕ ПОДКЛЮЧЕНО");

}



void MainWindow::on_pushButton_delete_string_clicked() //слот удаления выделенных строк
{
    QMessageBox::StandardButton answer = QMessageBox::question(this,"!?!","Удалить выбранные строки?");

    if(answer == QMessageBox::Yes){
    QSet <int> rows;
    QItemSelectionModel *selectModel;
    QModelIndexList indexes;
    QModelIndex index;

    selectModel = ui->tableView->selectionModel();
    indexes = selectModel->selectedIndexes();

    foreach(index, indexes){
        rows.insert(index.row());
    }
    QList<int> new_rows = rows.toList();
    std::sort(new_rows.begin(),new_rows.end(),std::greater<float>());
    for (auto i:new_rows) {
        ui->tableView->model()->removeRows(i,1);
        i++;
    }
    data_changed_slot();
    }

}

void MainWindow::on_pushButton_disconnect_clicked()
{
    mMutex_2.lock();
    emit signal_disconnect();
    mWait.wakeAll();
    mWait_2.wait(&mMutex_2);
    //qDebug()<<LAN_connect<<" "<<COM_connect;
    mMutex_2.unlock();
    COM_connect=false;
    LAN_connect=false;
    ui->pushButton_set_L->setEnabled(false);
    ui->pushButton_start->setEnabled(false);
    ui->pushButton_meas_curr->setEnabled(false);
    ui->label_connect_pic->setText("ПИК-амперметр\t - НЕ ПОДКЛЮЧЕНО");
    ui->label_connect_server->setText("Сервер\t\t - НЕ ПОДКЛЮЧЕНО");
    QMessageBox::information(this,"!Внимание!","Вы отключились");
}
