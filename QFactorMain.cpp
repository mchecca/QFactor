#include "QFactorMain.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QClipboard>
#include "NewTOTPDialog.h"
#include "ui_QFactorMain.h"

QFactorMain::QFactorMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QFactorMain)
{
    ui->setupUi(this);
    ui->lblStatus->setText(QString());
    settings = new QSettings("Michael Checca", "QFactor");

    /* center the window */
    QRect screenFrame = frameGeometry();
    screenFrame.moveCenter(QDesktopWidget().availableGeometry().center());
    move(screenFrame.topLeft());

    loadSettings();

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(1000);
    refreshTimer->start();

    clipboardTimer = new QTimer(this);
    clipboardTimer->setInterval(5000);

    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout()));
    connect(clipboardTimer, SIGNAL(timeout()), this, SLOT(clipboardTimerTimeout()));
    connect(ui->lstTotp, SIGNAL(currentRowChanged(int)), this, SLOT(totpItemRowChanged(int)));
    connect(ui->lstTotp, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(totpDoubleClicked(QModelIndex)));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));

    refreshTimerTimeout();
}

QFactorMain::~QFactorMain()
{
    saveSettings();
    delete ui;
    foreach (TOTP *t, totpList)
        delete t;
    refreshTimer->stop();
    clipboardTimer->stop();
    delete refreshTimer;
    delete clipboardTimer;
    delete settings;
}

void QFactorMain::addClicked()
{
    QString name, key;
    if (NewTOTPDialog(this, &name, &key).exec() == QDialog::Accepted)
        addTOTP(name, key);
}

void QFactorMain::refreshTimerTimeout()
{
    static qint64 time = 0;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    int untilRefresh = TOKEN_REFRESH_RATE - (currentTime % TOKEN_REFRESH_RATE);
    ui->lblRefresh->setText(QString("Seconds until refresh: %1").arg(QString::number(untilRefresh)));
    if ((currentTime % TOKEN_REFRESH_RATE) == 0 || ((currentTime - time) > TOKEN_REFRESH_RATE))
    {
        time = currentTime;
        refreshTotps();
    }
}

void QFactorMain::clipboardTimerTimeout()
{
    ui->lblStatus->setText(QString());
    clipboardTimer->stop();
}

void QFactorMain::totpItemRowChanged(int row)
{
    ui->btnDelete->setEnabled(ui->lstTotp->item(row) ? true : false);
}

void QFactorMain::totpDoubleClicked(QModelIndex index)
{
    TOTP *t = (TOTP*) index.data(Qt::UserRole).value<void*>();
    QClipboard *clipboard = QApplication::clipboard();
    if (!t || !clipboard)
        return;
    QString token = QString::number(t->generate());
    clipboard->setText(token);
    clipboardTimer->stop();
    clipboardTimer->start();
    ui->lblStatus->setText(QString("Copied %1 to clipboard").arg(token));
}

void QFactorMain::deleteClicked()
{
    QListWidgetItem *item = ui->lstTotp->currentItem();
    if (!item)
        return;
    TOTP *t = (TOTP*) item->data(Qt::UserRole).value<void*>();
    totpList.removeOne(t);
    ui->lstTotp->takeItem(ui->lstTotp->currentRow());
    ui->lstTotp->raise();
    refreshTotps();
    saveSettings();
}

void QFactorMain::addTOTP(QString name, QString key)
{
    TOTP *totp = new TOTP(name, key);
    totpList.append(totp);
    QListWidgetItem *item = new QListWidgetItem(totp->name());
    item->setData(Qt::UserRole, qVariantFromValue((void *) totp));
    ui->lstTotp->addItem(item);
    refreshTotps();
    saveSettings();
}

void QFactorMain::refreshTotps()
{
    for (int i = 0; i < ui->lstTotp->count(); i++)
    {
        QListWidgetItem *item = ui->lstTotp->item(i);
        TOTP *t = (TOTP*) item->data(Qt::UserRole).value<void*>();
        int key = t->generate();
        QString text = QString("%1 -- %2").arg(t->name(), (key == TOTP_INVALID_KEY) ? "Invalid key" : QString::number(key));
        item->setText(text);
    }
}

void QFactorMain::loadSettings()
{
    QPoint location = settings->value("ui/location", this->pos()).toPoint();
    QSize size = settings->value("ui/size", this->size()).toSize();
    bool isMaximized = settings->value("ui/maximized", false).toBool();
    this->move(location);
    this->resize(size);
    if (isMaximized)
        this->showMaximized();
    int totp_count = settings->value("totp/count", QVariant(0)).toInt();
    for (int i = 0; i < totp_count; i++)
    {
        QString nameKey = QString("totp/%1/name").arg(QString::number(i));
        QString keyKey = QString("totp/%1/key").arg(QString::number(i));
        QString name = settings->value(nameKey, QString()).toString();
        QString key = settings->value(keyKey, QString()).toString();
        addTOTP(name, key);
    }
    refreshTotps();
}

void QFactorMain::saveSettings()
{
    settings->setValue("ui/location", this->pos());
    settings->setValue("ui/size", this->size());
    settings->setValue("ui/maximized", this->isMaximized());
    settings->setValue("totp/count", totpList.count());
    TOTP *t = NULL;
    for (int i = 0; i < totpList.count(); i++)
    {
        t = totpList.at(i);
        QString nameKey = QString("totp/%1/name").arg(QString::number(i));
        QString keyKey = QString("totp/%1/key").arg(QString::number(i));
        settings->setValue(nameKey, t->name());
        settings->setValue(keyKey, t->key());
    }
}
