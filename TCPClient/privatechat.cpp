#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h""
#include "tcpclient.h"
#include <QMessageBox>

privateChat::privateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::privateChat)
{
    ui->setupUi(this);
}

privateChat::~privateChat()
{
    delete ui;
}

privateChat &privateChat::getinstance()
{
    static privateChat instance;
    return instance;
}

void privateChat::setChatName(QString strName)
{
    m_strChatName = strName;
    m_strLoginName = TcpClient::getinstance().getOnlineName();
}

void privateChat::updateMsg(const PDU *pdu)
{
    qDebug() << "我是udateMsg";
    if(pdu == NULL)
    {
        return;
    }
    char caSendName[32] = {'\0'};
    memcpy(caSendName, pdu->caData, 32);
    QString strMsg = QString("%1 : %2").arg(caSendName).arg((char*)(pdu->caMsg));
    ui->showMsg_te->append(strMsg);
}

void privateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_le->text();
    ui->inputMsg_le->clear();
    if(!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.size());
        memcpy(pdu->caData+32, m_strChatName.toStdString().c_str(), m_strChatName.size());

        memcpy(pdu->caMsg, strMsg.toStdString().c_str(),strMsg.size());

        TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "私聊", "输入信息不可为空");
    }
}

