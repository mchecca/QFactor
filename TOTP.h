#ifndef TOTP_H
#define TOTP_H

#include <QObject>
#include <QString>

#define TOTP_ERROR -1
#define TOTP_INVALID_KEY -2

class TOTP
{

public:
    TOTP(QString name, QString key);
    QString name();
    QString key();
    int generate();

private:
    QString m_name;
    QString m_key;
};

#endif // TOTP_H
