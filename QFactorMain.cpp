#include "QFactorMain.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
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
    ui->tblTotp->setColumnCount(3);
    ui->tblTotp->setHorizontalHeaderLabels(QStringList() <<
                                           "Account" <<
                                           "Token" <<
                                           "Website");

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
    connect(ui->tblTotp, SIGNAL(cellChanged(int,int)), this, SLOT(totpItemCellChanged(int,int)));
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
    TOTP *t = getTotpFromItemRow(index.row());
    /* copy to clipboard only if the token column was clicked */
    if (index.column() == 1)
    {
        QClipboard *clipboard = QApplication::clipboard();
        if (!t || !clipboard)
            return;
        int token = t->generateToken();
        if (token == TOTP_INVALID_KEY)
            return;
        QString token_str = QString::number(t->generateToken());
        clipboard->setText(token_str);
        clipboardTimer->stop();
        clipboardTimer->start();
        ui->lblStatus->setText(QString("Copied %1 to clipboard").arg(token_str));
    }
    /* if website was clicked, open in default web browser */
    else if (index.column() == 2)
    {
        QDesktopServices::openUrl(QUrl(t->website()));
    }
}

void QFactorMain::totpItemCellChanged(int row, int column)
{
    TOTP *t = getTotpFromItemRow(row);
    if (!t)
        return;
    QString text = ui->tblTotp->item(row, column)->text().trimmed();
    QString fallback = QString();
    bool edit = (text.length() > 0);
    switch (column) {
        case 0: /* account name */
            if (edit)
                t->setName(text);
            else
                fallback = t->name();
            break;
        case 2:
            if (edit)
                t->setWebsite(text);
            else
                fallback = t->website();
            break;
        default:
            break;
    }
    if (!edit)
        ui->tblTotp->item(row, column)->setText(fallback);
}

void QFactorMain::deleteClicked()
{
    TOTP *t = getTotpFromItemRow(ui->tblTotp->currentRow());
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
    QTableWidgetItem *tokenItem = new QTableWidgetItem();
    item->setData(Qt::UserRole, qVariantFromValue((void *) totp));
    tokenItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->tblTotp->setItem(rowCount, 0, item);
    ui->tblTotp->setItem(rowCount, 1, tokenItem);
    ui->tblTotp->setItem(rowCount, 2, new QTableWidgetItem());
    refreshTotps();
    saveSettings();
}

void QFactorMain::refreshTotps()
{
    for (int i = 0; i < ui->tblTotp->rowCount(); i++)
    {
        QTableWidgetItem *account = ui->tblTotp->item(i, 0);
        QTableWidgetItem *token_item = ui->tblTotp->item(i, 1);
        QTableWidgetItem *website = ui->tblTotp->item(i, 2);
        TOTP *t = getTotpFromItemRow(account->row());
        int token = t->generateToken();
        QString token_str = QString((token == TOTP_INVALID_KEY) ? "Invalid key" : QString::number(token));
        account->setText(t->name());
        token_item->setText(token_str);
        website->setText(t->website());
    }
    ui->tblTotp->resizeColumnsToContents();
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
    for (int i = 0; i < totpList.count(); i++)
    {
        TOTP *t = totpList.at(i);
        QString nameKey = QString("totp/%1/name").arg(QString::number(i));
        QString keyKey = QString("totp/%1/key").arg(QString::number(i));
        QString websiteKey = QString("totp/%1/website").arg(QString::number(i));
        settings->setValue(nameKey, t->name());
        settings->setValue(keyKey, t->key());
        settings->setValue(websiteKey, t->website());
    }
}

TOTP *QFactorMain::getTotpFromItemRow(int row)
{
    return static_cast<TOTP*>(ui->tblTotp->item(row, 0)->data(Qt::UserRole).value<void*>());
}
