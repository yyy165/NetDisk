#include "book.h"
#include "tcpclient.h"
#include <QMessageBox>
#include <QInputDialog>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
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
