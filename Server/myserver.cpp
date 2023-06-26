#include "myserver.h"
#include <QMessageBox>
#include <QLabel>
#include <QTime>
#include <QPushButton>
#include <fstream>

MyServer::MyServer(QWidget *parent)
    : QWidget{parent}, m_nNextBlockSize(0)
{
    m_ptcpServer = new QTcpServer(this);
    m_ptxt = new QTextEdit;
    m_ptxtAddressServer = new QLineEdit;
    m_ptxtPortServer = new QLineEdit;
    setLayoutSettings();
}

void MyServer::setLayoutSettings()
{
    QPushButton* pcmd = new QPushButton("&Принять");
    connect(pcmd, SIGNAL(clicked()), SLOT(slotStartServer()));

    pvBoxLayout = new QVBoxLayout;
    pvBoxLayout->addWidget(new QLabel("<H1>Сервер</H1>"));
    pvBoxLayout->addWidget(new QLabel("<H5>Адрес сервера</H5>"));
    pvBoxLayout->addWidget(m_ptxtAddressServer);
    pvBoxLayout->addWidget(new QLabel("<H5>Порт сервера</H5>"));
    pvBoxLayout->addWidget(m_ptxtPortServer);
    pvBoxLayout->addWidget(pcmd);
    setLayout(pvBoxLayout);
}

void MyServer::slotStartServer()
{
    delete pvBoxLayout;
    setLayoutConnect();
}

void MyServer::setLayoutConnect()
{
    QHostAddress hostAddr;
    hostAddr.setAddress(m_ptxtAddressServer->text());

    if(!m_ptcpServer->listen(hostAddr, m_ptxtPortServer->text().split(" ")[0].toInt()))
    {
        QMessageBox::critical(0,
                              "Ошибка сервера",
                              "Невозможно запустить сервер: " + m_ptcpServer->errorString());
        m_ptcpServer->close();
        return;
    }

    connect(m_ptcpServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection()));
    m_ptxt->setReadOnly(true);

    pvBoxLayout = new QVBoxLayout;
    pvBoxLayout->addWidget(new QLabel("<H1>Сервер</H1>"));
    pvBoxLayout->addWidget(m_ptxt);
    setLayout(pvBoxLayout);
}

void MyServer::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    clients.insert(pClientSocket, "");
    client.insert(pClientSocket);
    flag = true;
    for(auto iter = client.begin(); iter != client.end(); iter++)
    {
        connect(*iter, SIGNAL(disconnected()),*iter, SLOT(slotDeleteClient(iter)));
        connect(*iter, SIGNAL(readyRead()),this, SLOT(slotReadClient()));
    }
}

void MyServer::slotDeleteClient(QSet<QTcpSocket*>::iterator iter)
{
    QString disconnected = clients[*iter] + " отключился";
    for(auto it = client.begin(); it != client.end(); it++)
        if(it != iter)
            sendToClient(*it, disconnected);
    client.erase(iter);
    clients[*iter] = "";
}

void MyServer::sendKLines(QTcpSocket* pClientSocket, QString filename, int k)
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
            sendToClient(pClientSocket, temp);
    }
}

void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_3);
    std::fstream FILE("history.txt", std::ios::app | std::ios::in | std::ios::out);
    while(true)
    {
        if(!m_nNextBlockSize)
        {
            if(((quint16)pClientSocket->bytesAvailable() < sizeof(quint16)))
                break;
            in >> m_nNextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < m_nNextBlockSize)
            break;

        QTime time;
        QString str;
        in >> time >> str;
        if(flag)
        {
            nickname = str;
            flag = false;
            clients[pClientSocket] = str;
            m_nNextBlockSize = 0;
            nickname = nickname + " подключился";
            sendToClient(pClientSocket, nickname);
            sendToClient(pClientSocket, "Последние 10 сообщений от сервера");
            sendKLines(pClientSocket,"history.txt",11);
            break;
        }
        QString responce = clients[pClientSocket] + ">>" + str;


        QString strMessage = time.toString() + " " + clients[pClientSocket] + " отправил сообщение - " + str;
        m_ptxt->append(strMessage);
        QString file_str = time.toString() + " " + responce+ "\n";
        FILE << responce.toStdString() + "\n";

        m_nNextBlockSize = 0;

        for(auto iter = client.begin(); iter != client.end(); iter++)
        {
            sendToClient(*iter, responce);
        }
    }
}

void MyServer::sendToClient(QTcpSocket *pSocket, const QString &str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    out << quint16(0) << QTime::currentTime() << str;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    pSocket->write(arrBlock);
}
