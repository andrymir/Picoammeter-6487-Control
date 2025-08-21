#include "lan_port_command.h"


lan_port_command::lan_port_command(QMutex*m, QWaitCondition *w, QWaitCondition *w2) :mutex(m),wait_signal(w),wait_signal_2(w2)
{

}

lan_port_command::~lan_port_command()
{

}

void lan_port_command::connect_lan(QString ip_add, quint16 port, QString name_port_COM_port,bool state_LAN_con,bool state_COM_con){
    //connect_pls = true;
    choose_function=1;
    ip_add_pls = ip_add;
    port_pls = port;
    name_port_COM = name_port_COM_port;
    LAN_connect = state_LAN_con;
    COM_connect = state_COM_con;
}

void lan_port_command::disconnect_all(){
    //connect_pls = true;
    choose_function=6;
}


void lan_port_command::set_l_wave(float m_fWaveLenght)
{
    len_wave_const = m_fWaveLenght;
    //set_l_wave_pls=true;
    choose_function=2;
}

void lan_port_command::set_only_l_wave(float m_fWaveLenght)
{
    len_wave_const = m_fWaveLenght;
    //set_only_l_wave_pls=true;
    choose_function=3;
}

void lan_port_command::meas_curr_value(float m_fWaveLenght)
{
    len_wave_const = m_fWaveLenght;
    //meas_curr=true;
    choose_function=4;
}

void lan_port_command::set_range_l_wave(float len_min, float len_max, float len_step){
    //range_len = true;
    choose_function=5;
    l_max = len_max;
    l_min = len_min;
    l_step = len_step;

}


//QString lan_port_command::disconnect(){

//  return "Вы отключились";
//}



void lan_port_command::OnReadDataSignalEmited()
{
    m_pRecieveData =  new char[8]{};
    // читаем
    qint64 nReadedCnt = socket->read(m_pRecieveData, 8);

    // если это хоть похоже на нашу команду
    if(nReadedCnt >= 3)
    {
        // маркер проверим
        if(m_pRecieveData[0] != char(RCV_MARKER))
        {
            qDebug()<<QString("incorrect msg marker: %1!\n").arg(int(m_pRecieveData[0]));
            return;
        }

        // размер проверим
        if((m_pRecieveData[1]+2) != nReadedCnt)
        {
            qDebug()<<QString("incorrect msg size: %1!\n").arg(int(m_pRecieveData[1]));
            return;
        }

        // теперь смотрим код ответа
        switch (int(m_pRecieveData[2]))
        {
        case WAVE_LEN_SETTED:// = 0 заданная длина волны установлена
           qDebug()<<"Server: wave setted\n";
            break;
        case WAVE_LEN_SET_STOPED: // = 1 установка волны остановлена
            qDebug()<<"Server: wave set stoped\n";
            break;
        case TAKE_WAVE_LEN:// = 2 возвращаем длину волны на приборе
        {
            float fBlockLen;
            memcpy(&fBlockLen, m_pRecieveData+3, 4);
            QString str = QString("Server: my wave L is: %1nm\n").arg(fBlockLen, 0, 'f', 3);
           qDebug()<<str;
            break;
        }
        case ERROR_WHILE_WORKING:// =3 ошибка работы приложения
            qDebug()<<"Server: an error occured, while working!\n";
            break;
        case APP_BISY: // = 4 занят
            qDebug()<<"Server: wait a minute..\n";
            break;
        case CANT_MAKE_CMD:// = 5 невозможно выполнить команду
            qDebug()<<"Server: cant make cmd now!\n";
            break;
        case ERROR_EXCHANGE:// = 6 ошибка инет обмена
            qDebug()<<"Server: I have network error!\n";
            break;
        default:
            qDebug()<<"Server:wtf !\n";
            break;
        }
    }
    else
    {
        qDebug()<<QString("incorrect msg reded bytes: %1!\n").arg(nReadedCnt);
    }

    // дочитаем тут всё..
    QByteArray ba = socket->readAll();
    delete [] m_pRecieveData;
}

void lan_port_command::run(){

    COM_port_command port;

    range_len = false;
    socket = new QTcpSocket(this);                           //создание сокета
    m_pTransmData = new char[8];


    //connect(socket, SIGNAL(readyRead()), this , SLOT(OnReadDataSignalEmited()),Qt::DirectConnection);
    qDebug()<<"start ";
    while (1)   //цикл не завершится пока не придут данные или сигнал о завершении
    {
        mutex->lock();
        wait_signal->wait(mutex);

        stop_read_1 = false;
        //переделаем в switch
        switch (choose_function) {
            case 1:{ //подключение
            if(!LAN_connect){
            socket ->connectToHost(ip_add_pls,port_pls);            //подключение к хосту по заданному ip адресу
            if(socket->state() == 2) socket->waitForConnected(100);

            if (socket ->state() == 3) {
                LAN_connect = true;
            }
            else{
                LAN_connect = false;
            }
            }
            if(!COM_connect){
            port.disconnect();    //подключение к пик-амперметру по COM порту
            COM_connect = port.connect(name_port_COM);
            COM_connect = true; //пока для проверки без пик-амперметра потом убрать надо
            }

            emit signal_connect_answer(LAN_connect,COM_connect);
            //connect_pls=false;

            choose_function=0;
            wait_signal_2->wakeAll();
            break;
        };
            case 2:{ // установить длину волны и измерить
            cur_len_wave = len_wave_const;
            //stop_read_1 = false;

            memset(m_pTransmData, 0, 8);
            m_pTransmData[0] = RCV_MARKER;
            m_pTransmData[1] = 5;
            m_pTransmData[2] = EthernetClientCmd::SET_WAVE_LEN;

            // записываем полученную длину волны и погнали
            memcpy(m_pTransmData+3, &cur_len_wave, 4);
            socket->write(m_pTransmData, 7);
            socket ->waitForBytesWritten();      //ждем пока данные отправятся
            // сообщим что получилось
            qDebug()<<QString("Send wave len: %1").arg(cur_len_wave, 0, 'f', 2);

            continue_range = false;
            write_answer=true;

            while (1)   //цикл не завершится пока не придут данные или сигнал о завершении  // выглядит костыльно и тупо но другое решение не придумал
            {
                if (stop_read_1){         //если сигнал стоп пришел завершаем бесконечный цикл
                    qDebug()<<"ok";
                    write_answer=false;
                    break;
                }
                else if(socket->waitForReadyRead(1000) == true){
                    OnReadDataSignalEmited();
                 break;
               }
                else if(socket ->state() == 0) {
                    qDebug()<<"gg servery";
                    LAN_connect = false;
                    write_answer = false;
                    break;
                }
            }
            if(write_answer){
            qDebug()<<"точка установлена";
            QString data_out = read_str_pic(&port);


            if(!stop_read_1) {
                emit signal_read_answer(cur_len_wave,data_out);
                //emit end_meas();
            }
            }
            else if (LAN_connect){
                qDebug()<<"стоп установка";
                m_pTransmData[0] = RCV_MARKER;
                m_pTransmData[1] = 1;
                m_pTransmData[2] = EthernetClientCmd::STOP_MOVING;

                // записываем полученную длину волны и погнали
                //memcpy(m_pTransmData+3, &cur_len_wave, 4);
                socket->write(m_pTransmData, 3);
                socket ->waitForBytesWritten();      //ждем пока данные отправятся
                if(socket->waitForReadyRead(7000) == true) OnReadDataSignalEmited();

            }

            if(!LAN_connect) emit error_server();
            //stop_read_1 = false;
            choose_function=0;
            break;
        };
            case 3:{
            cur_len_wave = len_wave_const;
            //stop_read_1 = false;

            memset(m_pTransmData, 0, 8);
            m_pTransmData[0] = RCV_MARKER;
            m_pTransmData[1] = 5;
            m_pTransmData[2] = EthernetClientCmd::SET_WAVE_LEN;

            // записываем полученную длину волны и погнали
            memcpy(m_pTransmData+3, &cur_len_wave, 4);
            socket->write(m_pTransmData, 7);
            socket ->waitForBytesWritten();      //ждем пока данные отправятся
            // сообщим что получилось
            qDebug()<<QString("Send wave len: %1").arg(cur_len_wave, 0, 'f', 2);

            continue_range = false;

            while (1)   //цикл не завершится пока не придут данные или сигнал о завершении
            {
                if (stop_read_1){         //если сигнал стоп пришел завершаем бесконечный цикл
                    qDebug()<<"стоп установка";
                    m_pTransmData[0] = RCV_MARKER;
                    m_pTransmData[1] = 1;
                    m_pTransmData[2] = EthernetClientCmd::STOP_MOVING;

                    // записываем полученную длину волны и погнали
                    //memcpy(m_pTransmData+3, &cur_len_wave, 4);
                    socket->write(m_pTransmData, 3);
                    socket ->waitForBytesWritten();      //ждем пока данные отправятся
                    if(socket->waitForReadyRead(7000) == true) OnReadDataSignalEmited();

                    break;
                }
                else if(socket->waitForReadyRead(1000) == true){
                    OnReadDataSignalEmited();
                 break;
               }
               else if(socket ->state() == 0) {
                    qDebug()<<"gg servery";
                    LAN_connect = false;
                    break;
                }
            }
            qDebug()<<"точка end";
            if(!LAN_connect) emit error_server();
            else {
                emit signal_wave_setted();
            }


            //stop_read_1 = false;
            //set_only_l_wave_pls=false;
            choose_function=0;

            break;
        };
            case 4:{
            cur_len_wave = len_wave_const;
            //stop_read_1 = false;
            QString data_out = read_str_pic(&port);
            if(!stop_read_1) {emit signal_read_answer(cur_len_wave,data_out);
            //emit end_meas();
            }
            //meas_curr=false;
            choose_function=0;
            break;
        };
            case 5:{
            qDebug()<<"начинается диапазон ";
            write_answer=true;
            //stop_read_1 = false;

            for (float i = l_min;i<=l_max; i+=l_step) {
                if (stop_read_1){         //если сигнал стоп пришел завершаем цикл
                    qDebug()<<"ok";

                    break;
                }
                continue_range = false;

                cur_len_wave = i;
                //m_pTransmData = new char[8];
                memset(m_pTransmData, 0, 8);
                m_pTransmData[0] = RCV_MARKER;
                m_pTransmData[1] = 5;
                m_pTransmData[2] = EthernetClientCmd::SET_WAVE_LEN;

                // записываем полученную длину волны и погнали
                memcpy(m_pTransmData+3, &i, 4);
                socket->write(m_pTransmData, 7);
                socket->waitForBytesWritten();      //ждем пока данные отправятся
                // сообщим что получилось
                qDebug()<<QString("Send wave len: %1").arg(i, 0, 'f', 2);
                //msleep(500);
                while (1)   //цикл не завершится пока не придут данные или сигнал о завершении
                {
                    if (stop_read_1){         //если сигнал стоп пришел завершаем бесконечный цикл
                        qDebug()<<"ok";
                        write_answer=false;
                        break;
                    }
                    else if(socket->waitForReadyRead(1000) == true){
                        OnReadDataSignalEmited();
                     break;
                   }
                    else if(socket ->state() == 0) {
                        qDebug()<<"gg servery";
                        LAN_connect = false;
                        write_answer=false;
                        break;
                    }
                }
                if(!write_answer && LAN_connect){
                    qDebug()<<"стоп установка";
                    m_pTransmData[0] = RCV_MARKER;
                    m_pTransmData[1] = 1;
                    m_pTransmData[2] = EthernetClientCmd::STOP_MOVING;

                    // записываем полученную длину волны и погнали
                    //memcpy(m_pTransmData+3, &cur_len_wave, 4);
                    socket->write(m_pTransmData, 3);
                    socket ->waitForBytesWritten();      //ждем пока данные отправятся
                    if(socket->waitForReadyRead(7000) == true) OnReadDataSignalEmited();
                }
                if (!write_answer) break;
                qDebug()<<"вышли из цикла";
                QString data_out = read_str_pic(&port);
                if (!write_answer) break;
                emit signal_read_answer(cur_len_wave,data_out);

            }

            qDebug()<<"диапазон закончился";
            if(!LAN_connect) emit error_server();
            else if(!stop_read_1) emit end_meas();
            //stop_read_1 = false;
            range_len = false;

            choose_function=0;
            break;
        };
        case 6:{ //отключение
        socket->disconnectFromHost();
        if(socket->state() != 0) socket->waitForDisconnected(1000);
        port.disconnect();    //отключение от пик-амперметра по COM порту
        choose_function=0;
        wait_signal_2->wakeAll();
        break;
    };
        default: break;
        }

        if (stop_read){         //если сигнал стоп пришел завершаем бесконечный цикл
            mutex->unlock();
            break;
        }
        mutex->unlock();
    }
    qDebug()<<"поток завершился";
    socket->close();     //при отключении закрываем сокет

    //emit signal_finish_thread();
    delete [] m_pTransmData;

    if(socket != nullptr) delete socket;

}

void lan_port_command::stop(){
    this->stop_read = true;
    qDebug()<<"stop";

}

QString lan_port_command::read_str_pic(COM_port_command *port){
    int tryCnt = 0;
    QString data_out = "";
    while(data_out.isEmpty())     // Modified
    {
        if (stop_read_1){         //если сигнал стоп пришел завершаем бесконечный цикл
            write_answer = false;
            break;
        }
        data_out = port->Curr(); //запрашиваем информаци о токе
        if(tryCnt == 2)
        {
            // что-то не пришло
            qDebug()<<"данные так и не пришли";
            break;
        }
        else
            ++tryCnt;
    }
    return data_out;

}
