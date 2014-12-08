#ifndef NEWTOTPDIALOG_H
#define NEWTOTPDIALOG_H

#include <QDialog>
#include "TOTP.h"

namespace Ui {
class NewTOTPDialog;
}

class NewTOTPDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTOTPDialog(QWidget *parent, QString *name, QString *key, int *tokenLength, QString *website);
    ~NewTOTPDialog();

public slots:
    void accept();

private:
    Ui::NewTOTPDialog *ui;

    QString *m_name;
    QString *m_key;
    int *m_tokenLength;
    QString *m_website;
};

#endif // NEWTOTPDIALOG_H
