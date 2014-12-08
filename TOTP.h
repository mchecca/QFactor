#ifndef TOTP_H
#define TOTP_H

#include <QObject>
#include <QString>

#define TOTP_SUCCESS 0
#define TOTP_ERROR -1
#define TOTP_INVALID_KEY -2

#define DEFAULT_TOTP_KEY_LEN 6
class TOTP
{

public:
    TOTP(QString name, QString key, int tokenLength, QString website);
    QString name();
    QString key();
    int tokenLength();
    QString website();
    void setName(QString name);
    void setWebsite(QString website);
    QString generateToken(int *result);

private:
    QString m_name;
    QString m_key;
    QString m_website;
    int m_tokenLength;
};

#endif // TOTP_H
