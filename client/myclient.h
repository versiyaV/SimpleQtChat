#pragma once
#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QObject>
#include <QMainWindow>
#include <QVBoxLayout>
#include "mainwindow.h"

class QTextEdit;
class QLineEdit;

class MyClient: public QWidget
{
    Q_OBJECT
private:
    QTcpSocket* m_pTcpSocket;
    QTextEdit* m_ptxtInfo;
    QLineEdit* m_ptxtInput;
    QLineEdit* m_ptxtLogin;
    QLineEdit* m_ptxtAddressServer;
    QLineEdit* m_ptxtPortServer;
    QVBoxLayout* pvbxLayout;
    quint16 m_nNextBlockSize;
    bool flag;
    void setLayoutChat();
    void setLayoutLogin();
    void sendLoginToServer(QString login);
    void printKLines(QString filename, int k);
public:
    MyClient(QWidget* pwgt = nullptr);
private slots:
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError);
    void slotSendToServer();
    void slotConnected();
    void slotLogin();
};
