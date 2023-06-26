#pragma once

#include <QObject>
#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QTcpServer>
#include <QSet>
#include <QString>
#include <QMap>
#include <QVBoxLayout>
#include <QLineEdit>

class QTcpServer;
class QTextEdit;
class QTcpSocket;

class MyServer : public QWidget
{
    Q_OBJECT
private:
    QTcpServer* m_ptcpServer;
    QTextEdit* m_ptxt;
    quint16 m_nNextBlockSize;
    QSet<QTcpSocket*> client;
    bool flag;
    QString nickname;
    QMap<QTcpSocket*, QString> clients;
    QVBoxLayout* pvBoxLayout;
    QLineEdit* m_ptxtAddressServer;
    QLineEdit* m_ptxtPortServer;
private:
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    void setLayoutConnect();
    void setLayoutSettings();
    void sendKLines(QTcpSocket* pClientSocket,QString filename, int k);
public:
    explicit MyServer(QWidget *parent = nullptr);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void slotDeleteClient(QSet<QTcpSocket*>::iterator);
    void slotStartServer();
};
