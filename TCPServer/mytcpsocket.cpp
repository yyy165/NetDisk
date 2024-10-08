#include "mytcpsocket.h"
#include <qdebug.h>
#include <QStringList>
#include "mytcpserver.h"
#include <QFileInfoList>
#include <QIODevice>
#include <QTimer>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            ,this, SLOT(clientOffline()));
    connect(m_pTimer, SIGNAL(timeout())
            ,this, SLOT(sendFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload)
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
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char caDirName[32] = {'\0'};
            char *caCurPath = new char[pdu->uiMsgLen];
            strncpy(caDirName, pdu->caData, 32);
            memcpy(caCurPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(caCurPath).arg(caDirName);

            QFileInfo fileInfo(strPath);
            PDU *respdu = NULL;
            if(fileInfo.isDir())
            {
                QDir dir(strPath);
                QFileInfoList fileInfoList =  dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                respdu = mkPDU(sizeof(FileInfo) * (iFileCount));
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
            }
            else if(fileInfo.isFile())
            {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, ENTER_DIR_FAIL);
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData, "%s %lld", caFileName, &fileSize);
            char *curPath = new char[pdu->uiMsgLen];
            memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(curPath).arg(caFileName);
            delete []curPath;
            curPath = NULL;
            m_file.setFileName(strPath);
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
                m_iCount = 1;
            }
        }
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
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
                ret = false;
            }
            else if(fileInfo.isFile())  //常规文件
            {
                QDir dir;
                ret = dir.remove(strPath);
            }
            PDU *respdu = NULL;
            if(ret)
            {
                respdu = mkPDU(0);
                memcpy(respdu->caData, DEL_FILE_OK, strlen(DEL_FILE_OK));
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
            }
            else
            {
                respdu = mkPDU(0);
                memcpy(respdu->caData, DEL_FILE_FAIL, strlen(DEL_FILE_FAIL));
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            strcpy(caFileName, pdu->caData);
            char *curPath = new char[pdu->uiMsgLen];
            memcpy(curPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(curPath).arg(caFileName);
            delete []curPath;
            curPath = NULL;

            QFileInfo fileInfo(strPath);
            qint64 filesize = fileInfo.size();
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData, "%s %lld", caFileName, filesize);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32] = {'\0'};
            int num = 0;
            sscanf(pdu->caData, "%s %d", caSendName, &num);
            int size = num * 32;
            PDU *retPdu = mkPDU(pdu->uiMsgLen - size);//下载文件的路径的大小
            retPdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(retPdu->caData, caSendName);
            memcpy((char*)retPdu->caMsg, (char*)(pdu->caMsg) + size, pdu->uiMsgLen - size);
            char caRecvName[32] = {'\0'};
            for(int i = 0; i < num ; i++)
            {
                memcpy(caRecvName, (char*)(pdu->caMsg) + i * 32, 32);
                qDebug() << "要转发给该好友文件:"
                         <<caRecvName;
                MyTcpServer::getInstance().resend(caRecvName, retPdu);
            }
            free(retPdu);
            retPdu = NULL;
            retPdu = mkPDU(0);
            retPdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(retPdu->caData, "send share file msg ok");
            write((char*)retPdu, retPdu->uiPDULen);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST:
        {
            QString strRecvPath = QString("./%1").arg(pdu->caData);
            QString strShareFilePath = QString("%1").arg((char*)pdu->caMsg);
            int index = strShareFilePath.lastIndexOf('/');
            QString fileName = strShareFilePath.right(strShareFilePath.size() - index - 1);
            strRecvPath = strRecvPath + "/" + fileName;
            QFileInfo qFileInfo(strShareFilePath);
            if(qFileInfo.isFile())
            {
                QFile::copy(strShareFilePath, strRecvPath);
            }
            else if(qFileInfo.isDir())
            {
                copyDir(strShareFilePath, strRecvPath);
            }
            PDU *retPdu = mkPDU(0);
            retPdu->uiMsgLen = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
            strcpy(retPdu->caData, "copy file ok");
            write((char*)retPdu, retPdu->uiPDULen);
            free(retPdu);
            retPdu = NULL;
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
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        // 写入文件
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();

        qDebug() << QString("第 %1 次传入文件,接受数据大小:%2").arg(m_iCount).arg(buff.size());
        m_iCount++;
        if(m_iTotal == m_iRecved)
        {
            m_bUpload = false;
            m_file.close();
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iRecved)
        {
            m_bUpload = false;
            m_file.close();
            strcpy(respdu->caData, UPLOAD_FILE_FAIL);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        return ;
    }

    // qDebug() << caName << caPwd << pdu->uiMsgType;
}

void MyTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    char *pData = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(pData, 4096);
        if(ret > 0 && ret <= 4096)
        {
            write(pData, ret);
        }
        else if(ret == 0)
        {
            m_file.close();
            break;
        }
        else if(ret < 0)
        {
            qDebug() << "发送文件内容给客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = NULL;
}

void MyTcpSocket::copyDir(QString sourceDir, QString targetDir)
{
    QDir dir;
    dir.mkdir(targetDir);//创建目标文件夹，防止文件夹不存在
    dir.setPath(sourceDir);
    QFileInfoList fileInfoList = dir.entryInfoList();
    QString sourceTemp;
    QString targetTemp;
    for(int i = 0; i < fileInfoList.size(); i++)
    {
        sourceTemp = sourceDir + "/" + fileInfoList.at(i).fileName();
        targetTemp = targetDir + "/" + fileInfoList.at(i).fileName();
        if(fileInfoList.at(i).isFile())
        {
            QFile::copy(sourceTemp, targetTemp);
        }
        else if(fileInfoList.at(i).isDir())
        {
            // 不复制 . 和 ..目录
            if(QString(".") == fileInfoList.at(i).fileName()
                || QString("..") == fileInfoList.at(i).fileName())
            {
                continue;
            }
            copyDir(sourceTemp, targetTemp);
        }
    }
}
