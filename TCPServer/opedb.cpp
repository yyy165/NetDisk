#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\QT\\Project\\NetDisk\\TCPServer\\cloud.db");
    if(m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(name == NULL || pwd == NULL)
    {
        qDebug() << "name || pwd == NULL";
        return false;
    }
    QString data = QString("insert into usrInfo(name, pwd) values('%1', '%2')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(name == NULL || pwd == NULL)
    {
        qDebug() << "name || pwd == NULL";
        return false;
    }
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery query;
    qDebug() << data;
    query.exec(data);
    if(query.next())
    {
        QString data = QString("update usrInfo set online = 1 where name = \'%1\' and pwd = \'%2\' ").arg(name).arg(pwd);
        QSqlQuery query;
        qDebug() << data;
        return query.exec(data);
    }
    else
    {
        return false;
    }
}

void OpeDB::handleOffline(const char *name)
{
    if(name == NULL)
    {
        qDebug() << "name is NULL";
        return;
    }
    else
    {
        QString data = QString("update usrInfo set online = 0 where name = \'%1\'").arg(name);
        QSqlQuery query;
        query.exec(data);
    }
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online = 1");
    QSqlQuery query;
    qDebug() << data;
    query.exec(data);
    QStringList result;
    result.clear();
    while(query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(name == NULL)
    {
        return -1;
    }
    QString data = QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        int ret = query.value(0).toInt();
        qDebug() << ret;
        return ret;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(pername == NULL || name == NULL)
    {
        return -1;
    }
    QString data = QString("select * from friend where (id = (select id from usrInfo where name = \'%1\' ) and friendid = (select id from usrInfo where name = \'%2\'))"
                           "or (id = (select id from usrInfo where name = \'%3\' ) and friendid = (select id from usrInfo where name = \'%4\'))").arg(pername).arg(name).arg(name).arg(pername);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        return 0;  //双方已经是好友
    }
    else
    {
        QString data = QString("select online from usrInfo where name=\'%1\'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            int ret = query.value(0).toInt();
            if(ret == 1)
            {
                return 1;   //对方在线
            }
            else if(ret == 0)
            {
                return 2;   //对方不在线
            }
        }
        else
        {
            return 3;   //对方不存在
        }
    }
    return -1;
}

void OpeDB::handleAddAgree(const char *pername, const char *name)
{
    if(pername == NULL || name == NULL)
    {
        return;
    }
    else
    {
        QString data = QString("insert into friend(id, friendid) select(select id from usrInfo where name = \'%1\') as id,(select id from usrInfo where name = \'%2\') as friendid union all select(select id from usrInfo where name = \'%2\') as id,(select id from usrInfo where name = \'%1\') as friendid").arg(name).arg(pername);
        qDebug() << data;
        QSqlQuery query;
        query.exec(data);
        qDebug() << pername << name << "插入成功";
    }
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name == NULL)
    {
        return strFriendList;
    }
    QString data = QString("select name from usrInfo where id in (select friendid from friend where id in (select id from usrInfo where name = \'%1\')) and online = 1").arg(name);
    QSqlQuery query;
    query.exec(data);
    qDebug() << data;
    while(query.next())
    {
        strFriendList.append(query.value(0).toString());
        qDebug() << query.value(0);
    }
    return strFriendList;
}

bool OpeDB::handleDelFriend(const char *pername, const char *name)
{
    if(pername == NULL || name == NULL)
    {
        return false;
    }
    QString data = QString("delete from friend where (id in (select id from usrInfo where name = \'%1\') and friendid = (select id from usrInfo where name = \'%2\')) or (id = (select id from usrInfo where name = \'%2\') and friendid = (select id from usrInfo where name = \'%1\'))").arg(pername).arg(name);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    return true;
}
