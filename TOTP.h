#ifndef TOTP_H
#define TOTP_H

#include <QObject>
#include <QString>

class TOTP
{

public:
    TOTP(QString key);
    QString key();

private:
    QString m_key;
};

#endif // TOTP_H
