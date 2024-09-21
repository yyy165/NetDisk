#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include<QTimer>
#include<QFile>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);
    void clearEnterName();
    QString enterDir();
    void setDownloadStatus(bool status);
    bool getDownLoadStatus();
    QString getSaveFilePath();
    void updateLocalDownloadFileName();//更新本地下载文件的文件名称
    QString getShareFileName();

    qint64 m_iTotal;
    qint64 m_iRecved;
    QFile m_pFile;//用于本地下载文件使用

signals:

public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void uploadFile();
    void uploadFileData();
    void delFile();
    void downloadFile();
    void shareFile();

private:
    QListWidget *m_pBookListLW;
    QPushButton *m_pTurnBackPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushDirPB;
    QPushButton *m_pUpLoadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QTimer *m_pTimer;
    QString m_strSaveFilePath;
    bool m_pDownload;
    QString m_shareFileName;


};

#endif // BOOK_H
