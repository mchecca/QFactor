#include "NewTOTPDialog.h"
#include "ui_NewTOTPDialog.h"
#include <QMessageBox>

NewTOTPDialog::NewTOTPDialog(QWidget *parent, QString *name, QString *key, int *tokenLength, QString *website) :
    QDialog(parent),
    ui(new Ui::NewTOTPDialog)
{
    ui->setupUi(this);
    ui->txtName->setFocus();
    ui->txtTokenLength->setText(QString::number(DEFAULT_TOTP_KEY_LEN));
    this->m_name = name;
    this->m_key = key;
    this->m_tokenLength = tokenLength;
    this->m_website = website;
}

NewTOTPDialog::~NewTOTPDialog()
{
    delete ui;
}

void NewTOTPDialog::accept()
{
    QString name_str = ui->txtName->text().trimmed();
    QString key_str = ui->txtKey->text().trimmed();
    QString tokenlength_str = ui->txtTokenLength->text().trimmed();
    QString website_str = ui->txtWebsite->text().trimmed();
    bool ok;
    int tokenLength = tokenlength_str.toInt(&ok);
    if (name_str.length() == 0 || key_str.length() == 0)
    {
        QMessageBox::information(this, "QFactor", tr("Name and key cannot be empty"));
    }
    if (!ok)
    {
        QMessageBox::information(this, "QFactor", tr("Token length must be a number"));
    }
    else
    {
        if (m_name && m_key && m_website && m_tokenLength)
        {
            *m_name = name_str;
            *m_key = key_str;
            *m_tokenLength = tokenLength;
            *m_website = website_str;
            done(QDialog::Accepted);
        }
    }
}
