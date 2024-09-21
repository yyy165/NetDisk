#include "tcpclient.h"
#include "ui_tcpclient.h"
#include "opewidget.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "privatechat.h"
#include <QIODevice>

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

QString TcpClient::getCurPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器" , "连接服务器成功");
}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getInstance().getBook()->getDownLoadStatus())
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
                m_strCurPath = QString("./%1").arg(onlineName);
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
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            qDebug() << "我是case";
            if(privateChat::getinstance().isHidden())
            {
                privateChat::getinstance().show();
            }
            char caSendName[32] = {'\0'};
            strncpy(caSendName, pdu->caData, 32);
            QString strSendName = caSendName;
            privateChat::getinstance().setChatName(strSendName);
            privateChat::getinstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            OpeWidget::getInstance().getFriend()->updateGroupChat(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            QMessageBox::information(this, "新建文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_DIR_RESPOND:
        {
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            QString strEnterDir = OpeWidget::getInstance().getBook()->enterDir();
            if(!strEnterDir.isEmpty())
            {
                m_strCurPath = m_strCurPath + '/' + strEnterDir;
            }
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_RESPOND:
        {
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            QMessageBox::information(this, "重命名文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            OpeWidget::getInstance().getBook()->clearEnterName();
            QMessageBox::information(this, "进入文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
        {
            QMessageBox::information(this, "删除文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            QMessageBox::information(this, "上传文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug() << pdu->caData;
            char caFileName[32] = {'\0'};
            sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
            if(strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal > 0)
            {
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if(!m_file.open(QIODevice::WriteOnly))
                {
                    QMessageBox::warning(this, "下载文件", "获得保存文件的路径失败");
                }
            }
            break;
        }
        default:
            break;
        }


        free(pdu);
        pdu = NULL;
    }
    else
    {
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        if(pBook->m_iTotal == pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this, "下载文件", "下载文件成功");
        }
        else if(pBook->m_iTotal < pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }
}

#if 0
void TcpClient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.toUtf8().size());
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

