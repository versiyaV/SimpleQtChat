#include "myclient.h"
#include <QPushButton>
#include <QLabel>
#include <QTime>
#include <QString>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

MyClient::MyClient(QWidget* pwgt)
    :QWidget(pwgt),m_nNextBlockSize(0)
{
    m_pTcpSocket = new QTcpSocket(this);
    m_ptxtInfo = new QTextEdit;
    m_ptxtInput = new QLineEdit;
    m_ptxtLogin = new QLineEdit;
    m_ptxtAddressServer = new QLineEdit;
    m_ptxtPortServer = new QLineEdit;
    setLayoutLogin();
}

void MyClient::setLayoutLogin()
{
    QPushButton* pcmd = new QPushButton("&Принять");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotLogin()));

    pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Клиент</H1>"));
    pvbxLayout->addWidget(new QLabel("<H5>Адрес сервера</H5>"));
    pvbxLayout->addWidget(m_ptxtAddressServer);
    pvbxLayout->addWidget(new QLabel("<H5>Порт сервера</H5>"));
    pvbxLayout->addWidget(m_ptxtPortServer);
    pvbxLayout->addWidget(new QLabel("<H5>Логин</H5>"));
    pvbxLayout->addWidget(m_ptxtLogin);
    pvbxLayout->addWidget(pcmd);
    setLayout(pvbxLayout);
}

void MyClient::slotLogin()
{
    delete pvbxLayout;

    m_pTcpSocket->connectToHost(m_ptxtAddressServer->text(),m_ptxtPortServer->text().split(" ")[0].toInt());
    connect(m_pTcpSocket,SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket,SIGNAL(readyRead()), SLOT(slotReadyRead()));
    sendLoginToServer(m_ptxtLogin->text());
    setLayoutChat();
}

void MyClient::sendLoginToServer(QString login)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime() << login;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
}

void MyClient::printKLines(QString filename, int k)
{
    std::ifstream file(filename.toStdString());
    std::string L[k];
    int size = 0;

    while (file.good())
    {
        std::getline(file, L[size % k]);
        size++;
    }

    int start = size > k ? (size % k) : 0;
    int count = std::min(k, size);
    for (int i = 0; i < count; i++)
    {
        QString temp = QString::fromStdString(L[(start + i) % k]);
        if(temp != "")
            m_ptxtInfo->append(temp);
    }
}

void MyClient::setLayoutChat()
{
    flag = false;

    QPushButton* pcmd = new QPushButton("&Отправить");
    std::fstream FILE(m_ptxtLogin->text().toStdString(), std::ios::app | std::ios::out | std::ios::in);
    m_ptxtInfo->append("Ваши последние 5 сообщений");
    if(FILE.peek() == std::ifstream::traits_type::eof())
    {
        m_ptxtInfo->append("Вы новый пользователь");
    }
    else
    {
        printKLines(m_ptxtLogin->text(), 6);
    }

    connect(pcmd, SIGNAL(clicked()), SLOT(slotSendToServer()));
    connect(m_ptxtInput, SIGNAL(returnPressed()), this, SLOT(slotSendToServer()));

    pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Клиент</H1>"));
    pvbxLayout->addWidget(m_ptxtInfo);
    pvbxLayout->addWidget(m_ptxtInput);
    pvbxLayout->addWidget(pcmd);
    setLayout(pvbxLayout);
}

void MyClient::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_5_3);
    while(true)
    {
        if(!m_nNextBlockSize)
        {
            if((quint16)m_pTcpSocket->bytesAvailable() < sizeof(quint16))
                break;
            in >> m_nNextBlockSize;
        }

        if(m_pTcpSocket->bytesAvailable() < m_nNextBlockSize)
            break;

        QTime time;
        QString str;

        in >> time >> str;
        QString file_str = time.toString() + " " + str;
        m_ptxtInfo->append(file_str);
        m_nNextBlockSize = 0;
    }
}

void MyClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError = "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                                        "Сервер не найден" :
                                        err == QAbstractSocket::RemoteHostClosedError ?
                                            "Сервер отключен" :
                                            err == QAbstractSocket::ConnectionRefusedError ?
                                                "Подключение сброшено" : QString(m_pTcpSocket->errorString()));
    m_ptxtInfo->append(strError);
}

void MyClient::slotSendToServer()
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    std::fstream FILE(m_ptxtLogin->text().toStdString(), std::ios::app | std::ios::out | std::ios::in);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime() << m_ptxtInput->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    if(m_ptxtInput->text() != "")
        FILE << m_ptxtInput->text().toStdString() + "\n";
    FILE.close();
    m_pTcpSocket->write(arrBlock);
    m_ptxtInput->setText("");
}

void MyClient::slotConnected()
{
    //m_ptxtInfo->append(m_ptxtLogin->text() + " подключен к чату");
}
