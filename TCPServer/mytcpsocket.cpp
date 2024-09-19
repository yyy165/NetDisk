#include "mytcpsocket.h"
#include <qdebug.h>
#include <QStringList>
#include "mytcpserver.h"
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            ,this, SLOT(clientOffline()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::recvMsg()
{
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu + sizeof(uint), uiPDULen - sizeof(uint));
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);
        bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData, REGIST_OK);
            QDir dir;
            qDebug() << "create dir" << dir.mkdir(QString("./%1").arg(caName));
        }
        else
        {
            strcpy(respdu->caData, REGIST_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData+32, 32);
        bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData, LOGIN_OK);
            m_strName = caName;
        }
        else
        {
            strcpy(respdu->caData, LOGIN_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        QStringList ret = OpeDB::getInstance().handleAllOnline();
        uint uiMsgLen = ret.size() * 32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i = 0;i < ret.size();i++)
        {
            memcpy((char*)respdu->caMsg+i*32, ret.at(i).toStdString().c_str(),ret.at(i).size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
    {
        int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if(ret == 1)
        {
            strcpy(respdu->caData, SEARCH_USR_ONLINE);
        }
        else if(ret == 0)
        {
            strcpy(respdu->caData, SEARCH_USR_OFFLINE);
        }
        else if(ret == -1)
        {
            strcpy(respdu->caData, SEARCH_USR_NO);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32);
        strncpy(caName, pdu->caData+32, 32);
        int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName);
        PDU *respdu = NULL;
        if(ret == -1)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, UNKNOWN_ERROR);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(ret == 0)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, EXISTED_FRIEND);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(ret == 1)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
            strcpy(respdu->caData, caPerName);
            strcpy(respdu->caData + 32, caName);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            // MyTcpServer::getInstance().resend(caPerName, pdu);
        }
        else if(ret == 2)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(ret == 3)
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData, ADD_FRIEND_NO_EXIST);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32);
        strncpy(caName, pdu->caData+32, 32);
        OpeDB::getInstance().handleAddAgree(caPerName, caName);
        qDebug() << caPerName << caName;
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_YOU;
        strcpy(respdu->caData, caPerName);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
    {
        char caPerName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REJECT_YOU;
        strcpy(respdu->caData, caPerName);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
    {
        char strName[32] = {'\0'};
        strncpy(strName, pdu->caData, 32);
        QStringList ret = OpeDB::getInstance().handleFlushFriend(strName);
        uint uiMsgLen = ret.size() * 32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        for(int i = 0;i < ret.size();i++)
        {
            memcpy((char*)(respdu->caMsg) + i*32, ret.at(i).toStdString().c_str(), ret.at(i).size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {
        char caFriendName[32] = {'\0'};
        char caSelfName[32] = {'\0'};
        strncpy(caFriendName, pdu->caData, 32);
        strncpy(caSelfName, pdu->caData + 32, 32);
        OpeDB::getInstance().handleDelFriend(caFriendName, caSelfName);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
        strcpy((char*)respdu->caData, DEL_FRIEND_OK);
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        char caPerName[32] = {'\0'};
        strncpy(caPerName, pdu->caData + 32, 32);
        qDebug() << caPerName;
        MyTcpServer::getInstance().resend(caPerName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        QStringList onlineUsr = OpeDB::getInstance().handleAllOnline();
        for(int i = 0;i < onlineUsr.size();i++)
        {
            MyTcpServer::getInstance().resend(onlineUsr.at(i).toStdString().c_str(), pdu);
        }
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
    {
        QDir dir;
        QString strCurPath = QString("%1").arg((char*)pdu->caMsg);
        PDU *respdu;
        qDebug() << strCurPath;
        bool ret = dir.exists(strCurPath);
        if(ret) //当前目录存在
        {
            char caNewDir[32] = {'\0'};
            memcpy(caNewDir, pdu->caData + 32, 32);
            QString strNewPath = strCurPath + "/" + caNewDir;
            qDebug() << strNewPath;
            ret = dir.exists(strNewPath);
            if(ret) //新目录存在
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, FILE_NAME_EXIST);
            }
            else    //新目录不存在
            {
                dir.mkdir(strNewPath);
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, CREATE_DIR_OK);
            }
        }
        else    //当前目录不存在
        {
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
            strcpy(respdu->caData, DIR_NO_EXIST);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_DIR_REQUEST:
    {
        char *pCurPath = new char[pdu->uiMsgLen];
        memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
        QDir dir(pCurPath);
        QFileInfoList fileInfoList =  dir.entryInfoList();
        int iFileCount = fileInfoList.size();
        PDU *respdu = mkPDU(sizeof(FileInfo) * (iFileCount));
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_RESPOND;
        FileInfo *pFileInfo = NULL;
        QString strFileName;
        for(int i = 0;i < iFileCount;i++)
        {
            // if(QString(".") == fileInfoList[i].fileName()
            // ||QString("..") == fileInfoList[i].fileName())
            // {
            //     continue;
            // }
            pFileInfo = (FileInfo*)(respdu->caMsg) + i;
            strFileName = fileInfoList[i].fileName();
            memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
            if(fileInfoList[i].isDir())
            {
                pFileInfo->iFileType = 0;
            }
            else if(fileInfoList[i].isFile())
            {
                pFileInfo->iFileType = 1;
            }
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
    {
        char dirName[32] = {'\0'};
        strcpy(dirName, pdu->caData);
        char *curPath = new char[pdu->uiMsgLen];
        memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(curPath).arg(dirName);
        qDebug() << strPath;

        QFileInfo fileInfo(strPath);
        bool ret = false;
        if(fileInfo.isDir())
        {
            QDir dir;
            dir.setPath(strPath);
            ret = dir.removeRecursively();
        }
        else if(fileInfo.isFile())  //常规文件
        {
            ret = false;
        }
        PDU *respdu = NULL;
        if(ret)
        {
            respdu = mkPDU(0);
            memcpy(respdu->caData, DEL_DIR_OK, strlen(DEL_DIR_OK));
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
        }
        else
        {
            respdu = mkPDU(0);
            memcpy(respdu->caData, DEL_DIR_FAIL, strlen(DEL_DIR_FAIL));
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
    {
        char caOldName[32] = {'\0'};
        char caNewName[32] = {'\0'};
        strncpy(caOldName, pdu->caData, 32);
        strncpy(caNewName, pdu->caData + 32, 32);
        char *pPath = new char[pdu->uiMsgLen + 1];
        memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
        QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
        QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);
        QDir dir;
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
        bool ret = dir.rename(strOldPath, strNewPath);
        if(ret)
        {
            strcpy(respdu->caData, RENAME_FILE_OK);
        }
        else
        {
            strcpy(respdu->caData, RENAME_FILE_FAIL);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default:
        break;
    }

    free(pdu);
    pdu = NULL;
    // qDebug() << caName << caPwd << pdu->uiMsgType;
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}
