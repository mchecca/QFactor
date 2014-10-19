#include "TOTP.h"
#include "liboath/oath.h"
#include <time.h>

TOTP::TOTP(QString name, QString key, QString website)
{
    this->m_name = name;
    this->m_key = key;
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

int TOTP::generate()
{
    if (oath_init() != OATH_OK)
        return -1;

    char *key = strdup(m_key.toStdString().c_str());
    size_t key_len= strlen(key);
    char *secret = NULL;
    size_t secret_len = 0;
    if (oath_base32_decode(key, key_len, &secret, &secret_len) != OATH_OK)
    {
        free(key);
        return TOTP_INVALID_KEY;
    }
    time_t time_now = time(0);
    char *output = (char *) calloc (sizeof(char*), 7);
    if (oath_totp_generate(secret, secret_len, time_now, 30, 0, 6, output) != OATH_OK)
    {
        free(key);
        free(secret);
        free(output);
        return TOTP_ERROR;
    }
    int token = atoi(output);
    free(key);
    free(secret);
    free(output);

    oath_done();
    return token;
}
