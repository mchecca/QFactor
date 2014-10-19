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

    /* set up TOTP table */
    ui->tblTotp->setColumnCount(4);
    ui->tblTotp->setHorizontalHeaderLabels(QStringList() <<
                                           "Account" <<
                                           "Token" <<
                                           "Website" <<
                                           "Actions");

    loadSettings();

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(1000);
    refreshTimer->start();

    clipboardTimer = new QTimer(this);
    clipboardTimer->setInterval(5000);

    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTimerTimeout()));
    connect(clipboardTimer, SIGNAL(timeout()), this, SLOT(clipboardTimerTimeout()));
    connect(ui->tblTotp, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)), this, SLOT(totpItemChanged(QTableWidgetItem*,QTableWidgetItem*)));
    connect(ui->tblTotp, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(totpDoubleClicked(QModelIndex)));
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
    QString name, key, website;
    if (NewTOTPDialog(this, &name, &key, &website).exec() == QDialog::Accepted)
        addTOTP(name, key, website);
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

void QFactorMain::totpItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous);
    bool enabled = (current && (ui->tblTotp->rowCount() > 0)  ? true : false);
    ui->btnDelete->setEnabled(enabled);
}

void QFactorMain::totpDoubleClicked(QModelIndex index)
{
    TOTP *t = (TOTP*) index.data(Qt::UserRole).value<void*>();
    QClipboard *clipboard = QApplication::clipboard();
    if (!t || !clipboard)
        return;
    int key = t->generate();
    if (key == TOTP_INVALID_KEY)
        return;
    QString key_str = QString::number(t->generate());
    clipboard->setText(key_str);
    clipboardTimer->stop();
    clipboardTimer->start();
    ui->lblStatus->setText(QString("Copied %1 to clipboard").arg(key_str));
}

void QFactorMain::deleteClicked()
{
    QTableWidgetItem *item = ui->tblTotp->item(ui->tblTotp->currentRow(), 0);
    if (!item)
        return;
    TOTP *t = (TOTP*) item->data(Qt::UserRole).value<void*>();
    totpList.removeOne(t);
    ui->tblTotp->removeRow(ui->tblTotp->currentRow());
    ui->tblTotp->raise();
    refreshTotps();
    saveSettings();
}

void QFactorMain::addTOTP(QString name, QString key, QString website)
{
    TOTP *totp = new TOTP(name, key, website);
    totpList.append(totp);
    int rowCount = ui->tblTotp->rowCount();
    ui->tblTotp->insertRow(rowCount);
    QTableWidgetItem *item = new QTableWidgetItem(totp->name());
    QTableWidgetItem *keyItem = new QTableWidgetItem();
    item->setData(Qt::UserRole, qVariantFromValue((void *) totp));
    keyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tblTotp->setItem(rowCount, 0, item);
    ui->tblTotp->setItem(rowCount, 1, keyItem);
    ui->tblTotp->setItem(rowCount, 2, new QTableWidgetItem());
    refreshTotps();
    saveSettings();
}

void QFactorMain::refreshTotps()
{
    for (int i = 0; i < ui->tblTotp->rowCount(); i++)
    {
        QTableWidgetItem *account = ui->tblTotp->item(i, 0);
        QTableWidgetItem *key_item = ui->tblTotp->item(i, 1);
        QTableWidgetItem *website = ui->tblTotp->item(i, 2);
        TOTP *t = (TOTP*) account->data(Qt::UserRole).value<void*>();
        int key = t->generate();
        QString key_str = QString((key == TOTP_INVALID_KEY) ? "Invalid key" : QString::number(key));
        account->setText(t->name());
        key_item->setText(key_str);
        website->setText(t->website());
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
        QString websiteKey = QString("totp/%1/website").arg(QString::number(i));
        QString name = settings->value(nameKey, QString()).toString();
        QString key = settings->value(keyKey, QString()).toString();
        QString website = settings->value(websiteKey, QString()).toString();
        addTOTP(name, key, website);
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
        QString websiteKey = QString("totp/%1/website").arg(QString::number(i));
        settings->setValue(nameKey, t->name());
        settings->setValue(keyKey, t->key());
        settings->setValue(websiteKey, t->website());
    }
}
