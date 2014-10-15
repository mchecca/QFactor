#include "QFactorMain.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
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

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(1000);
    refreshTimer->start();

    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(ui->lstTotp, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(totpItemChanged(QListWidgetItem*)));
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout()));
}

QFactorMain::~QFactorMain()
{
    delete ui;
    foreach (TOTP *t, totpList)
        delete t;
    refreshTimer->stop();
    delete refreshTimer;
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
    refreshTotps();
}

void QFactorMain::totpItemChanged(QListWidgetItem *item)
{
    TOTP *totp = (TOTP*) item->data(Qt::UserRole).value<void*>();
}

void QFactorMain::refreshTimerTimeout()
{
    static qint64 time = 0;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    if ((currentTime % TOKEN_REFRESH_RATE) == 0 || ((currentTime - time) > TOKEN_REFRESH_RATE))
    {
        time = currentTime;
        refreshTotps();
    }
}

void QFactorMain::refreshTotps()
{
    for (int i = 0; i < ui->lstTotp->count(); i++)
    {
        QListWidgetItem *item = ui->lstTotp->item(i);
        TOTP *t = (TOTP*) item->data(Qt::UserRole).value<void*>();
        item->setText(QString("%1 -- %2").arg(t->key(), QString::number(t->generate())));
    }
}
