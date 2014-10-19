#ifndef TOTP_H
#define TOTP_H

#include <QObject>
#include <QString>

#define TOTP_ERROR -1
#define TOTP_INVALID_KEY -2

class TOTP
{

public:
    TOTP(QString name, QString key, QString website);
    QString name();
    QString key();
    QString website();
    int generate();

private:
    QString m_name;
    QString m_key;
    QString m_website;
};

#endif // TOTP_H
