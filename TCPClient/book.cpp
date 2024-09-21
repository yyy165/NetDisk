#include "book.h"
#include "tcpclient.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTimer>
#include <QIODevice>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear();

    m_pTimer = new QTimer;
    m_pDownload = false;

    m_pBookListLW = new QListWidget;
    m_pTurnBackPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件夹");
    m_pFlushDirPB = new QPushButton("刷新文件夹");

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pTurnBackPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushDirPB);

    m_pUpLoadPB = new QPushButton("上传文件");
    m_pDownLoadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("共享文件");

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUpLoadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListLW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB, SIGNAL(clicked(bool))
            , this, SLOT(createDir()));
    connect(m_pFlushDirPB, SIGNAL(clicked(bool))
            , this, SLOT(flushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked(bool))
            , this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked(bool))
            , this, SLOT(renameFile()));
    connect(m_pBookListLW, SIGNAL(doubleClicked(QModelIndex))
            , this, SLOT(enterDir(QModelIndex)));
    connect(m_pTurnBackPB, SIGNAL(clicked(bool))
            , this, SLOT(returnPre()));
    connect(m_pUpLoadPB, SIGNAL(clicked(bool)),
            this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()),
            this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool)),
            this, SLOT(delFile()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool)),
            this, SLOT(downloadFile()));
}

void Book::updateFileList(const PDU *pdu)
{
    if(pdu == NULL)
    {
        return;
    }
    m_pBookListLW->clear();
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    for(int i = 0;i < iCount;i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg) + i;
        qDebug() << pFileInfo->caFileName << pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(pFileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.png")));
        }
        else if(pFileInfo->iFileType == 1)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/reg.jfif")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListLW->addItem(pItem);
    }
}

void Book::clearEnterName()
{
    m_strEnterDir.clear();
}

QString Book::enterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadStatus(bool status)
{
    m_pDownload = status;
}

bool Book::getDownLoadStatus()
{
    return m_pDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this, "新建文件夹", "新建文件夹名称");
    if(strNewDir.isEmpty())
    {
        QMessageBox::warning(this, "新建文件夹", "新建文件夹名不可为空");
    }
    else
    {   if(strNewDir.size() > 32)
        {
            QMessageBox::warning(this, "新建文件夹", "文件夹名称过长");
        }
        else
        {
            QString strName = TcpClient::getinstance().getOnlineName();
            QString strCurPath = TcpClient::getinstance().getCurPath();
            PDU *pdu = mkPDU(strCurPath.toUtf8().size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            strncpy(pdu->caData + 32, strNewDir.toStdString().c_str(), strNewDir.size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }

    }
}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_REQUEST;
    strncpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListLW->currentItem();
    if(pItem == NULL)
    {
        QMessageBox::warning(this, "删除目录", "请选择要删除的目录");
    }
    else
    {
        QString dirName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
        strncpy(pdu->caData, dirName.toStdString().c_str(), dirName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::renameFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListLW->currentItem();
    if(pItem == NULL)
    {
        QMessageBox::warning(this, "重命名文件", "请选择要重命名的文件");
    }
    else
    {
        QString strOldName = pItem->text();
        if(strOldName.isEmpty())
        {
            QMessageBox::warning(this, "重命名文件", "重命名不能为空");
        }
        else
        {
            QString strNewName = QInputDialog::getText(this, "重命名文件", "请输入新名称");
            PDU *pdu = mkPDU(strCurPath.size() + 1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
            strncpy(pdu->caData + 32, strNewName.toStdString().c_str(), strNewName.size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
            TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    m_strEnterDir = strDirName;
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QString strRootPath = "./" + TcpClient::getinstance().getOnlineName();
    if(strCurPath == strRootPath)
    {
        if(m_pBookListLW->currentItem()->text() == "..")
        {
            QMessageBox::warning(this, "返回", "返回失败:已经在根目录");
            return;
        }
    }
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData, strDirName.toStdString().c_str(), strDirName.size());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::returnPre()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QString strRootPath = "./" + TcpClient::getinstance().getOnlineName();
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回", "返回失败:已经在根目录");
    }
    else
    {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index, strCurPath.size() - index);
        TcpClient::getinstance().setCurPath(strCurPath);

        clearEnterName();

        flushFile();
    }
}

void Book::uploadFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(m_strUploadFilePath.isEmpty())
    {
        QMessageBox::warning(this, "上传文件", "上传文件名称不能为空");
        return ;
    }
    int idx = m_strUploadFilePath.lastIndexOf('/');
    QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - idx - 1);
    QFile file(m_strUploadFilePath);
    qint64 fileSize = file.size();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    sprintf(pdu->caData, "%s %lld", fileName.toStdString().c_str(), fileSize);
    qDebug() << pdu->caData;
    qDebug() << (char*)pdu->caMsg;
    TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
    m_pTimer->start(1000);
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = file.read(pBuffer, 4096);
        if(ret > 0 && ret <= 4096)
        {
            TcpClient::getinstance().getTcpSocket().write(pBuffer, ret);
        }
        else if(ret == 0)
        {
            break;
        }
        else
        {
            QMessageBox::warning(this, "上传文件", "上传文件失败：读取文件内容失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;
}

void Book::delFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListLW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件", "请选择要删除的文件");
        return ;
    }
    QString strDelName = pItem->text();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
    strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::downloadFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListLW->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
        return ;
    }
    QString strSaveFilePath = QFileDialog::getSaveFileName();
    if(strSaveFilePath.isEmpty())
    {
        QMessageBox::warning(this, "下载文件", "请选择保存位置");
        m_strSaveFilePath.clear();
    }
    else
    {
        m_strSaveFilePath = strSaveFilePath;
    }
    QString fileName = pItem->text();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    strncpy(pdu->caData, fileName.toStdString().c_str(), fileName.size());
    memcpy((char*)pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}
