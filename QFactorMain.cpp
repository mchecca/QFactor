#include "QFactorMain.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include "ui_QFactorMain.h"

QFactorMain::QFactorMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QFactorMain)
{
    ui->setupUi(this);

    /* center the window */
    QRect screenFrame = frameGeometry();
    screenFrame.moveCenter(QDesktopWidget().availableGeometry().center());
    move(screenFrame.topLeft());

    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(ui->lstTotp, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(totpItemChanged(QListWidgetItem*)));
}

QFactorMain::~QFactorMain()
{
    delete ui;
    foreach (TOTP *t, totpList)
        delete t;
}

void QFactorMain::addClicked()
{
    QString name = ui->txtName->text().trimmed();
    QString key = ui->txtKey->text().trimmed();
    if (name.length() == 0 || key.length() == 0) {
        QMessageBox::information(this, "QFactor", "Name and key cannot be empty");
        return;
    }
    TOTP *totp = new TOTP(key);
    totpList.append(totp);
    QListWidgetItem *item = new QListWidgetItem(totp->key());
    item->setData(Qt::UserRole, qVariantFromValue((void *) totp));
    ui->lstTotp->addItem(item);
    ui->txtName->clear();
    ui->txtKey->clear();
}

void QFactorMain::totpItemChanged(QListWidgetItem *item)
{
    TOTP *totp = (TOTP*) item->data(Qt::UserRole).value<void*>();
    qDebug("%s: %p", totp->key().toStdString().c_str(), totp);
}
