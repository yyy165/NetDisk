#include "mytcpserver.h"
#include "qobjectdefs.h"
#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client conneted";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, SIGNAL(offline(MyTCPSocket*))
            ,this, SLOT(deleteSocket(MyTcpSocket*)));
}

void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    qDebug() << "我是resend";
    if(pername == NULL || pdu == NULL)
    {
        return;
    }
    QString strName = pername;
    for(int i = 0;i < m_tcpSocketList.size();i++)
    {
        if(strName == m_tcpSocketList.at(i)->getName())
        {
            m_tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }
    }

}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    QList<MyTcpSocket*>::Iterator iter = m_tcpSocketList.begin();
    for(;iter != m_tcpSocketList.end();iter++)
    {
        if(mysocket == *iter)
        {
            delete *iter;
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
    // for(int i = 0;i < m_tcpSocketList.size();i++)
    // {
    //     qDebug() << m_tcpSocketList.at(i)->getName();
    // }
}
