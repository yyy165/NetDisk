#include "friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include "privatechat.h"

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pShowMsgTE = new QTextEdit;
    m_pFriendListWiget = new QListWidget;
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWiget);
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    connect(m_pShowOnlineUsrPB, SIGNAL(clicked())
            , this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked(bool))
            , this, SLOT(searchUsr()));
    connect(m_pFlushFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(flushFriend()));
    connect(m_pDelFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(delFriend()));
    connect(m_pPrivateChatPB, SIGNAL(clicked(bool))
            , this, SLOT(privateChat()));
    connect(m_pMsgSendPB, SIGNAL(clicked(bool))
            , this, SLOT(groupChat()));
}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(pdu == NULL)
    {
        return;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(pdu == NULL)
    {
        return;
    }
    uint uiSize = pdu->uiMsgLen / 32;
    char caName[32] = {'\0'};
    for(uint i= 0;i < uiSize;i++)
    {
        memcpy(caName, (char*)(pdu->caMsg) + i*32, 32);
        m_pFriendListWiget->addItem(caName);
    }
}

void Friend::updateGroupChat(PDU *pdu)
{
    QString recvMsg = QString("%1 : %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_pShowMsgTE->append(recvMsg);
}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWiget;
}

void Friend::flushFriend()
{
    m_pFriendListWiget->clear();
    QString strName = TcpClient::getinstance().getOnlineName();
    PDU *pdu = mkPDU(0);
    memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::delFriend()
{
    if(m_pFriendListWiget->currentItem() == nullptr)
    {
        QMessageBox::critical(this, "删除失败", "请选择好友");
    }
    else
    {
        QString strFriendName = m_pFriendListWiget->currentItem()->text();
        PDU *pdu = mkPDU(0);
        QString strSelfName = TcpClient::getinstance().getOnlineName();
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(pdu->caData, strFriendName.toStdString().c_str(), strFriendName.size());
        memcpy(pdu->caData + 32, strSelfName.toStdString().c_str(), strSelfName.size());
        TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }

}

void Friend::privateChat()
{
    if(m_pFriendListWiget->currentItem() == nullptr)
    {
        QMessageBox::critical(this, "私聊", "请选择私聊对象");
    }
    else
    {
        QString strChatName = m_pFriendListWiget->currentItem()->text();
        privateChat::getinstance().setChatName(strChatName);
        if(privateChat::getinstance().isHidden())
        {
            privateChat::getinstance().show();
        }
    }
}

void Friend::groupChat()
{
    if(m_pInputMsgLE->text().isEmpty())
    {
        QMessageBox::warning(this, "群聊", "输入信息不可为空");
    }
    else
    {
        QString sendMsg = m_pInputMsgLE->text();
        m_pInputMsgLE->clear();
        QString sendName = TcpClient::getinstance().getOnlineName();
        PDU *pdu = mkPDU(sendMsg.toUtf8().size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        strncpy(pdu->caData, sendName.toStdString().c_str(), sendName.size());
        strncpy((char*)pdu->caMsg, sendMsg.toStdString().c_str(), sendMsg.size());
        TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden())
    {
        m_pOnline->show();

        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名:");
    if(!m_strSearchName.isEmpty())
    {
        qDebug() << m_strSearchName;
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}
