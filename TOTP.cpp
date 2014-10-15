#include "TOTP.h"

TOTP::TOTP(QString key)
{
    this->m_key = key;
}

QString TOTP::key()
{
    return this->m_key;
}

int TOTP::generate()
{
    return -1;
}
