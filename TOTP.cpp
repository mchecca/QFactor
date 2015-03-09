#include "TOTP.h"
#include "TOTPUtil.h"
#include <time.h>

TOTP::TOTP(QString name, QString key, int tokenLength, QString website)
{
    this->m_name = name;
    this->m_key = key;
    this->m_tokenLength = tokenLength;
    this->m_website = website;
}

QString TOTP::name()
{
    return this->m_name;
}

QString TOTP::key()
{
    return this->m_key;
}

int TOTP::tokenLength()
{
    return this->m_tokenLength;
}

QString TOTP::website()
{
    return this->m_website;
}

void TOTP::setName(QString name)
{
    this->m_name = name;
}

void TOTP::setWebsite(QString website)
{
    this->m_website = website;
}

QString TOTP::generateToken(int *result)
{
    return totpGenerate(m_key, this->m_tokenLength, result);
}
