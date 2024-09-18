/********************************************************************************
** Form generated from reading UI file 'privatechat.ui'
**
** Created by: Qt User Interface Compiler version 6.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRIVATECHAT_H
#define UI_PRIVATECHAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_privateChat
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTextEdit *showMsg_te;
    QHBoxLayout *horizontalLayout;
    QLineEdit *inputMsg_le;
    QPushButton *sendMsg_pb;

    void setupUi(QWidget *privateChat)
    {
        if (privateChat->objectName().isEmpty())
            privateChat->setObjectName("privateChat");
        privateChat->resize(334, 285);
        verticalLayout_2 = new QVBoxLayout(privateChat);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        showMsg_te = new QTextEdit(privateChat);
        showMsg_te->setObjectName("showMsg_te");
        QFont font;
        font.setPointSize(24);
        showMsg_te->setFont(font);

        verticalLayout->addWidget(showMsg_te);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        inputMsg_le = new QLineEdit(privateChat);
        inputMsg_le->setObjectName("inputMsg_le");
        inputMsg_le->setFont(font);

        horizontalLayout->addWidget(inputMsg_le);

        sendMsg_pb = new QPushButton(privateChat);
        sendMsg_pb->setObjectName("sendMsg_pb");
        sendMsg_pb->setFont(font);

        horizontalLayout->addWidget(sendMsg_pb);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(privateChat);

        QMetaObject::connectSlotsByName(privateChat);
    } // setupUi

    void retranslateUi(QWidget *privateChat)
    {
        privateChat->setWindowTitle(QCoreApplication::translate("privateChat", "Form", nullptr));
        sendMsg_pb->setText(QCoreApplication::translate("privateChat", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class privateChat: public Ui_privateChat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRIVATECHAT_H
