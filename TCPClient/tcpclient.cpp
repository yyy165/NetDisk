#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "opewidget.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);

    resize(350, 250);
    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected())
            , this, SLOT(showConnect()));

    connect(&m_tcpSocket, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));

    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData =  file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();

        strData.replace("\n", " ");

        QStringList strList = strData.split(" ");

        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << "ip:" << m_strIP << "port:" << m_usPort;
    }
    else
    {
        QMessageBox::critical(this, "open config", "open config fail");
    }
}

TcpClient &TcpClient::getinstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::getOnlineName()
{
    return onlineName;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器" , "连接服务器成功");
}

void TcpClient::recvMsg()
{
    qDebug() << m_tcpSocket.bytesAvailable();
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(0 == strcmp(pdu->caData, REGIST_OK))
        {
            QMessageBox::information(this, "注册", REGIST_OK);
        }
        else if(0 == strcmp(pdu->caData, REGIST_FAILED))
        {
            QMessageBox::warning(this, "注册", REGIST_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0 == strcmp(pdu->caData, LOGIN_OK))
        {
            QMessageBox::information(this, "登录", LOGIN_OK);
            OpeWidget::getInstance().show();
            hide();
        }
        else if(0 == strcmp(pdu->caData, LOGIN_FAILED))
        {
            QMessageBox::warning(this, "登录", LOGIN_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {
        OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);

        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
    {
        if(0 == strcmp(SEARCH_USR_NO, pdu->caData))
        {
            QMessageBox::information(this, "搜索", QString("%1 : 用户不存在").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(SEARCH_USR_ONLINE, pdu->caData))
        {
            QMessageBox::information(this, "搜索", QString("%1 : 在线").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(SEARCH_USR_OFFLINE, pdu->caData))
        {
            QMessageBox::information(this, "搜索", QString("%1 : 离线").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        qDebug() << "ENUM_MSG_TYPE_ADD_FRIEND_REQUEST";
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData+32, 32);
        int ret = QMessageBox::information(this, "好友请求", QString("\'%1\' : 想要添加你为好友").arg(caName), QMessageBox::Yes, QMessageBox::No);
        PDU *respdu = mkPDU(0);
        memcpy(respdu->caData, pdu->caData, 64);
        if(ret == QMessageBox::Yes)
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        }
        else
        {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        }
        m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        QMessageBox::information(this, "添加好友", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
    {
        OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        QMessageBox::information(this, "删除好友", DEL_FRIEND_OK);
        break;
    }
    case ENUM_MSG_TYPE_ADD_YOU:
    {
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        QMessageBox::information(this, "添加好友", QString("%1同意了你的好友申请").arg(caName));
        break;
    }
    case ENUM_MSG_TYPE_REJECT_YOU:
    {
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        QMessageBox::information(this, "添加好友", QString("%1拒绝了你的好友申请...").arg(caName));
        break;
    }
    default:
        break;
    }

    free(pdu);
    pdu = NULL;
}

#if 0
void TcpClient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size());
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "信息发送", "发送的信息不能为空");
    }
}
#endif


void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        onlineName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "登录", "登录失败:用户名或密码为空");
    }
}


void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "注册", "注册失败:用户名或密码为空");
    }
}


void TcpClient::on_cancel_pb_clicked()
{

}

