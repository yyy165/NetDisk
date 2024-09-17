#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class privateChat;
}

class privateChat : public QWidget
{
    Q_OBJECT

public:
    explicit privateChat(QWidget *parent = nullptr);
    ~privateChat();

    static privateChat &getinstance();

    void setChatName(QString strName);
    void updateMsg(const PDU *pdu);

private slots:
    void on_sendMsg_pb_clicked();

private:
    Ui::privateChat *ui;
    QString m_strChatName;
    QString m_strLoginName;
};

#endif // PRIVATECHAT_H
