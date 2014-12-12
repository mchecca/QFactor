#ifndef TOTPUTIL_H
#define TOTPUTIL_H

#include <stddef.h>
#include <stdlib.h>

#define TOTP_OK 0
#define TOTP_NOT_OK 1

int
base32_decode (const char *in, size_t inlen, char **out, size_t *outlen);

int
totp_generate (const char *secret, size_t secret_length, time_t now,
                    unsigned time_step_size, time_t start_offset,
                    unsigned digits, char *output_otp);

#endif // TOTPUTIL_H
