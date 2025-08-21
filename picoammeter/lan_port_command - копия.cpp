#include "lan_port_command.h"

lan_port_command::lan_port_command()
{

}

lan_port_command::~lan_port_command()
{

}
void lan_port_command::test(QString ip_add, quint16 port){
    connect_pls = true;
    ip_add_pls = ip_add;
    port_pls = port;
}

void lan_port_command::connect_lan(QString ip_add, quint16 port){
    connect_pls = true;
    ip_add_pls = ip_add;
    port_pls = port;

//    QString str = "Вам не удалось подключиться";
//    socket -> connectToHost(ip_add,port);            //подключение к хосту по заданному ip адресу
//    if (socket ->waitForConnected(1000) == false) {
//        return str;  //если не удалось подключиться
//        emit signal_connect_answer(false);
//    }
//    else{
//        str = "Вы подключились";     //если удалось подключиться
//        emit signal_connect_answer(true);
//        return str;
//    }


}


void lan_port_command::set_l_wave(float m_fWaveLenght)
{
    len_wave_const = m_fWaveLenght;
    set_l_wave_pls=true;
//    cur_len_wave = m_fWaveLenght;
//    m_pTransmData = new char[8];
//    memset(m_pTransmData, 0, 8);
//    m_pTransmData[0] = RCV_MARKER;
//    m_pTransmData[1] = 5;
//    m_pTransmData[2] = EthernetClientCmd::SET_WAVE_LEN;

//    // записываем полученную длину волны и погнали
//    memcpy(m_pTransmData+3, &m_fWaveLenght, 4);
//    socket->write(m_pTransmData, 7);
//    socket ->waitForBytesWritten();      //ждем пока данные отправятся
//    // сообщим что получилось
//    qDebug()<<QString("Send wave len: %1").arg(m_fWaveLenght, 0, 'f', 2);
//    QString str;

//    if(socket ->waitForReadyRead(10000) == false){
//        str = "fail not read";    //если на запрос не ответили
//    }
//    else{
//    //OnReadDataSignalEmited();
//     }
    //return str;
}

void lan_port_command::set_range_l_wave(float len_min, float len_max, float len_step){
    range_len = true;
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
    qDebug()<<"что то пришло";
    // читаем чего там е
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
        case APP_BISY: // = 4 обожжи
            qDebug()<<"Server: wait a minute..\n";
            break;
        case CANT_MAKE_CMD:// = 5 невозможно выполнить команду
            qDebug()<<"Server: cant make cmd now!\n";
            break;
        case ERROR_EXCHANGE:// = 6 ошибка инет обмена
            qDebug()<<"Server: I have network error!\n";
            break;
        default:
            qDebug()<<"Server: bleeaattt wtf buuueee!\n";
            break;
        }
    }
    else
    {
        qDebug()<<QString("incorrect msg reded bytes: %1!\n").arg(nReadedCnt);
    }

    // дочитаем тут всё..
    QByteArray ba = socket->readAll();
    if(ba.size() != 0)
    {
        int a = 0;
    }
}

void lan_port_command::run(){ //поменяем тут вообще все

    range_len = false;
    socket = new QTcpSocket(this);                           //создание сокета
    m_pRecieveData =  new char[8]{};
    m_pTransmData = new char[8];

    //connect(socket, SIGNAL(readyRead()), this , SLOT(OnReadDataSignalEmited()),Qt::DirectConnection);
    qDebug()<<"start ";
    while (1)   //цикл не завершится пока не придут данные или сигнал о завершении
    {
        if (stop_read){         //если сигнал стоп пришел завершаем бесконечный цикл
            qDebug()<<"ok";
            break;
        }
        if(connect_pls) {
            socket->disconnectFromHost();
            QString str = "Вам не удалось подключиться";
            socket -> connectToHost(ip_add_pls,port_pls);            //подключение к хосту по заданному ip адресу
            if (socket ->waitForConnected(1000) == false) {
                //return str;  //если не удалось подключиться
                emit signal_connect_answer(false);
            }
            else{
                str = "Вы подключились";     //если удалось подключиться
                emit signal_connect_answer(true);
                //return str;
            }
            connect_pls=false;
        }
        if(set_l_wave_pls) {
            cur_len_wave = len_wave_const;

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
                    qDebug()<<"ok";
                    break;
                }
                else if(socket->waitForReadyRead(1000) == true){


                    //qDebug()<<"что то пришло";

                    OnReadDataSignalEmited();

//                    // читаем чего там е
//                    qint64 nReadedCnt = socket->read(m_pRecieveData, 8);

//                    // если это хоть похоже на нашу команду
//                    if(nReadedCnt >= 3)
//                    {
//                        // маркер проверим
//                        if(m_pRecieveData[0] != char(RCV_MARKER))
//                        {
//                            qDebug()<<QString("incorrect msg marker: %1!\n").arg(int(m_pRecieveData[0]));
//                            return;
//                        }

//                        // размер проверим
//                        if((m_pRecieveData[1]+2) != nReadedCnt)
//                        {
//                            qDebug()<<QString("incorrect msg size: %1!\n").arg(int(m_pRecieveData[1]));
//                            return;
//                        }

//                        // теперь смотрим код ответа
//                        switch (int(m_pRecieveData[2]))
//                        {
//                        case WAVE_LEN_SETTED:// = 0 заданная длина волны установлена
//                           qDebug()<<"Server: wave setted\n";
//                            break;
//                        case WAVE_LEN_SET_STOPED: // = 1 установка волны остановлена
//                            qDebug()<<"Server: wave set stoped\n";
//                            break;
//                        case TAKE_WAVE_LEN:// = 2 возвращаем длину волны на приборе
//                        {
//                            float fBlockLen;
//                            memcpy(&fBlockLen, m_pRecieveData+3, 4);
//                            QString str = QString("Server: my wave L is: %1nm\n").arg(fBlockLen, 0, 'f', 3);
//                           qDebug()<<str;
//                            break;
//                        }
//                        case ERROR_WHILE_WORKING:// =3 ошибка работы приложения
//                            qDebug()<<"Server: an error occured, while working!\n";
//                            break;
//                        case APP_BISY: // = 4 обожжи
//                            qDebug()<<"Server: wait a minute..\n";
//                            break;
//                        case CANT_MAKE_CMD:// = 5 невозможно выполнить команду
//                            qDebug()<<"Server: cant make cmd now!\n";
//                            break;
//                        case ERROR_EXCHANGE:// = 6 ошибка инет обмена
//                            qDebug()<<"Server: I have network error!\n";
//                            break;
//                        default:
//                            qDebug()<<"Server: bleeaattt wtf buuueee!\n";
//                            break;
//                        }
//                    }
//                    else
//                    {
//                        qDebug()<<QString("incorrect msg reded bytes: %1!\n").arg(nReadedCnt);
//                    }

//                    // дочитаем тут всё..
//                    QByteArray ba = socket->readAll();
//                    if(ba.size() != 0)
//                    {
//                        int a = 0;
//                    }
                 break;
               }
            }
            qDebug()<<"точка установлена";
            emit signal_read_answer(cur_len_wave);
            while(!continue_range){ //ждем пока измерется значение на ПИК амперметре

            }

            stop_read_1 = false;
            set_l_wave_pls=false;
        }

        if(range_len){
            qDebug()<<"начинается диапазон ";


            for (float i = l_min;i<=l_max; i+=l_step) {
                if (stop_read_1){         //если сигнал стоп пришел завершаем бесконечный цикл
                    qDebug()<<"ok";
                    goto mark_out;
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
                        goto mark_out;
                    }
                    else if(socket->waitForReadyRead(1000) == true){
                        //m_pRecieveData =  new char[8]{};
                        //qDebug()<<"что то пришло";
                        OnReadDataSignalEmited();
//                        // читаем чего там е
//                        qint64 nReadedCnt = socket->read(m_pRecieveData, 8);

//                        // если это хоть похоже на нашу команду
//                        if(nReadedCnt >= 3)
//                        {
//                            // маркер проверим
//                            if(m_pRecieveData[0] != char(RCV_MARKER))
//                            {
//                                qDebug()<<QString("incorrect msg marker: %1!\n").arg(int(m_pRecieveData[0]));
//                                return;
//                            }

//                            // размер проверим
//                            if((m_pRecieveData[1]+2) != nReadedCnt)
//                            {
//                                qDebug()<<QString("incorrect msg size: %1!\n").arg(int(m_pRecieveData[1]));
//                                return;
//                            }

//                            // теперь смотрим код ответа
//                            switch (int(m_pRecieveData[2]))
//                            {
//                            case WAVE_LEN_SETTED:// = 0 заданная длина волны установлена
//                               qDebug()<<"Server: wave setted\n";
//                                break;
//                            case WAVE_LEN_SET_STOPED: // = 1 установка волны остановлена
//                                qDebug()<<"Server: wave set stoped\n";
//                                break;
//                            case TAKE_WAVE_LEN:// = 2 возвращаем длину волны на приборе
//                            {
//                                float fBlockLen;
//                                memcpy(&fBlockLen, m_pRecieveData+3, 4);
//                                QString str = QString("Server: my wave L is: %1nm\n").arg(fBlockLen, 0, 'f', 3);
//                               qDebug()<<str;
//                                break;
//                            }
//                            case ERROR_WHILE_WORKING:// =3 ошибка работы приложения
//                                qDebug()<<"Server: an error occured, while working!\n";
//                                break;
//                            case APP_BISY: // = 4 обожжи
//                                qDebug()<<"Server: wait a minute..\n";
//                                break;
//                            case CANT_MAKE_CMD:// = 5 невозможно выполнить команду
//                                qDebug()<<"Server: cant make cmd now!\n";
//                                break;
//                            case ERROR_EXCHANGE:// = 6 ошибка инет обмена
//                                qDebug()<<"Server: I have network error!\n";
//                                break;
//                            default:
//                                qDebug()<<"Server: bleeaattt wtf buuueee!\n";
//                                break;
//                            }
//                        }
//                        else
//                        {
//                            qDebug()<<QString("incorrect msg reded bytes: %1!\n").arg(nReadedCnt);
//                        }

//                        // дочитаем тут всё..
//                        QByteArray ba = socket->readAll();
//                        if(ba.size() != 0)
//                        {
//                            int a = 0;
//                        }
                     break;
                   }
                }
                qDebug()<<"вышли из цикла";
                emit signal_read_answer(cur_len_wave);
                while(!continue_range){ //ждем пока измерется значение на ПИК амперметре

                }

            }
            mark_out:
            qDebug()<<"диапазон закончился";
            stop_read_1 = false;
            range_len = false;

        }
    }
    qDebug()<<"поток завершился";
    socket->close();     //при отключении закрываем сокет

    //emit signal_finish_thread();
    delete [] m_pTransmData;
    delete [] m_pRecieveData;
    if(socket != nullptr) delete socket;

}

void lan_port_command::stop(){
    this->stop_read = true;
    qDebug()<<"stop";

}
