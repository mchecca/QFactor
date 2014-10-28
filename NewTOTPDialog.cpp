#include "NewTOTPDialog.h"
#include "ui_NewTOTPDialog.h"
#include <QMessageBox>

NewTOTPDialog::NewTOTPDialog(QWidget *parent, QString *name, QString *key, QString *website) :
    QDialog(parent),
    ui(new Ui::NewTOTPDialog)
{
    ui->setupUi(this);
    ui->txtName->setFocus();
    this->m_name = name;
    this->m_key = key;
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
    QString website_str = ui->txtWebsite->text().trimmed();
    if (name_str.length() == 0 || key_str.length() == 0)
    {
        QMessageBox::information(this, "QFactor", tr("Name and key cannot be empty"));
    }
    else
    {
        if (m_name && m_key && m_website)
        {
            *m_name = name_str;
            *m_key = key_str;
            *m_website = website_str;
            done(QDialog::Accepted);
        }
    }
}
