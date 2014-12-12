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
    QString token = "N/A";
    int tmp_result = TOTP_ERROR;
    char *key = strdup(m_key.toStdString().c_str());
    size_t key_len= strlen(key);
    char *secret = NULL;
    size_t secret_len = 0;

    if (base32_decode(key, key_len, &secret, &secret_len) != TOTP_OK)
    {
        tmp_result = TOTP_INVALID_KEY;
    }
    else
    {
        time_t time_now = time(0);
        char *output = (char *) calloc (sizeof(char*), 7);
        if (totp_generate(secret, secret_len, time_now, 30, 0, this->m_tokenLength, output) != TOTP_OK)
        {
            tmp_result = TOTP_ERROR;
        }
        else
        {
            tmp_result = TOTP_SUCCESS;
            token = QString(output);
        }
        free(output);
    }
    free(key);
    free(secret);

    if (result)
        *result = tmp_result;
    return token;
}
