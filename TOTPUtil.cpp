#include "TOTPUtil.h"

#ifdef __linux__
#include "liboath/oath.h"
#endif

int base32_decode (const char *in, size_t inlen, char **out, size_t *outlen)
{
#ifdef __linux__
    return oath_base32_decode(in, inlen, out, outlen);
#else
    return TOTP_NOT_OK;
#endif
}

int totp_generate (const char *secret, size_t secret_length, time_t now,
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
