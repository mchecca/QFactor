#include "TOTPUtil.h"

#ifdef __linux__
#include "liboath/oath.h"
#endif

static int base32_decode (const char *in, size_t inlen, char **out, size_t *outlen)
{
#ifdef __linux__
    return oath_base32_decode(in, inlen, out, outlen);
#else
    return TOTP_NOT_OK;
#endif
}

static int totp_generate (const char *secret, size_t secret_length, time_t now,
                    unsigned time_step_size, time_t start_offset,
                    unsigned digits, char *output_otp)
{
#ifdef __linux__
    return oath_totp_generate(secret, secret_length, now, time_step_size,
                              start_offset, digits, output_otp);
#else
    return TOTP_NOT_OK;
#endif
}

QString totpGenerate(QString key, int token_length, int *result)
{
    QString token = "N/A";
    int tmp_result = TOTP_NOT_OK;
#ifdef __linux__
    char *secret = NULL;
    size_t secret_len = 0;

    if (base32_decode(key.toStdString().c_str(), key.length(), &secret, &secret_len) != TOTP_OK)
    {
        tmp_result = TOTP_NOT_OK;
    }
    else
    {
        time_t time_now = time(0);
        char *output = (char *) calloc (sizeof(char*), token_length);
        if (totp_generate(secret, secret_len, time_now, 30, 0, token_length, output) != TOTP_OK)
        {
            tmp_result = TOTP_NOT_OK;
        }
        else
        {
            tmp_result = TOTP_OK;
            token = QString(output);
        }
        free(output);
    }
    if (result)
#else
    tmp_result = TOTP_NOT_OK;
    token = "Windows version not implemented";
#endif
    *result = tmp_result;
    return token;
}
