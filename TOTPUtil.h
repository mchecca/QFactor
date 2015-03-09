#ifndef TOTPUTIL_H
#define TOTPUTIL_H

#include <QString>

#define TOTP_OK 0
#define TOTP_NOT_OK 1

QString totpGenerate(QString key, int token_length, int *result);

#endif // TOTPUTIL_H
