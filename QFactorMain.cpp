#include "QFactorMain.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QAction>
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
                                           tr("Account") <<
                                           tr("Token") <<
                                           tr("Website") <<
                                           tr("Action"));

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
    connect(ui->tblTotp, SIGNAL(clicked(QModelIndex)), this, SLOT(totpItemClicked(QModelIndex)));
    connect(ui->tblTotp, SIGNAL(cellChanged(int,int)), this, SLOT(totpItemCellChanged(int,int)));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(deleteClicked()));

    /* Keyboard shortcuts */
    QAction *quit = new QAction(this);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quit, SIGNAL(triggered()), this, SLOT(close()));
    this->addAction(quit);

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
    int tokenLength;
    if (NewTOTPDialog(this, &name, &key, &tokenLength, &website).exec() == QDialog::Accepted)
        addTOTP(name, key, tokenLength, website);
}

void QFactorMain::refreshTimerTimeout()
{
    static qint64 time = 0;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
    int untilRefresh = TOKEN_REFRESH_RATE - (currentTime % TOKEN_REFRESH_RATE);
    ui->lblRefresh->setText(tr("Seconds until refresh: %1").arg(QString::number(untilRefresh)));
    ui->pbSeconds->setValue(TOKEN_REFRESH_RATE - untilRefresh);
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
    QApplication::clipboard()->clear();
}

void QFactorMain::totpItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous);
    bool enabled = (current && (ui->tblTotp->rowCount() > 0)  ? true : false);
    ui->btnDelete->setEnabled(enabled);
}

void QFactorMain::totpItemClicked(QModelIndex index)
{
    TOTP *t = getTotpFromItemRow(index.row());
    /* copy to clipboard only if the token column was clicked */
    if (index.column() == 1)
    {
        QClipboard *clipboard = QApplication::clipboard();
        if (!t || !clipboard)
            return;
        int ret;
        QString token = t->generateToken(&ret);
        if (ret != TOTP_SUCCESS)
            return;
        clipboard->setText(token);
        clipboardTimer->stop();
        clipboardTimer->start();
        ui->lblStatus->setText(tr("Copied %1 to clipboard").arg(token));
    }
    /* if website was clicked, open in default web browser */
    else if (index.column() == 3)
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
    else
        refreshTotps();
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

void QFactorMain::addTOTP(QString name, QString key, int tokenLength, QString website, bool save)
{
    TOTP *totp = new TOTP(name, key, tokenLength, website);
    totpList.append(totp);
    int rowCount = ui->tblTotp->rowCount();
    ui->tblTotp->insertRow(rowCount);
    QTableWidgetItem *item = new QTableWidgetItem(totp->name());
    QTableWidgetItem *tokenItem = new QTableWidgetItem();
    QTableWidgetItem *actionItem = new QTableWidgetItem();
    item->setData(Qt::UserRole, qVariantFromValue((void *) totp));
    tokenItem->setFlags(tokenItem->flags() ^ Qt::ItemIsEditable);
    actionItem->setFlags(actionItem->flags() ^ Qt::ItemIsEditable);
    ui->tblTotp->setItem(rowCount, 0, item);
    ui->tblTotp->setItem(rowCount, 1, tokenItem);
    ui->tblTotp->setItem(rowCount, 2, new QTableWidgetItem());
    ui->tblTotp->setItem(rowCount, 3, actionItem);
    refreshTotps();
    if (save)
        saveSettings();
}

void QFactorMain::refreshTotps()
{
    for (int i = 0; i < ui->tblTotp->rowCount(); i++)
    {
        QTableWidgetItem *account = ui->tblTotp->item(i, 0);
        QTableWidgetItem *token_item = ui->tblTotp->item(i, 1);
        QTableWidgetItem *website = ui->tblTotp->item(i, 2);
        QTableWidgetItem *action = ui->tblTotp->item(i, 3);
        TOTP *t = getTotpFromItemRow(account->row());
        /* TODO: Figure out why this sometimes happens */
        if (!(account && token_item && website && action))
            return;
        int ret;
        QString token = t->generateToken(&ret);
        QString token_str = QString((ret == TOTP_INVALID_KEY) ? tr("Invalid key") : token);
        account->setText(t->name());
        token_item->setText(token_str);
        website->setText(t->website());
        action->setText(tr("Open"));
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
        QString tokenLengthKey = QString("totp/%1/tokenlength").arg(QString::number(i));
        QString websiteKey = QString("totp/%1/website").arg(QString::number(i));
        QString name = settings->value(nameKey, QString()).toString();
        QString key = settings->value(keyKey, QString()).toString();
        int tokenLength = settings->value(tokenLengthKey, DEFAULT_TOTP_KEY_LEN).toInt();
        QString website = settings->value(websiteKey, QString()).toString();
        addTOTP(name, key, tokenLength, website, false);
    }
    refreshTotps();
}

void QFactorMain::saveSettings()
{
    settings->clear();
    settings->setValue("ui/location", this->pos());
    settings->setValue("ui/size", this->size());
    settings->setValue("ui/maximized", this->isMaximized());
    settings->setValue("totp/count", totpList.count());
    for (int i = 0; i < totpList.count(); i++)
    {
        TOTP *t = totpList.at(i);
        QString nameKey = QString("totp/%1/name").arg(QString::number(i));
        QString keyKey = QString("totp/%1/key").arg(QString::number(i));
        QString tokenLengthKey = QString("totp/%1/tokenlength").arg(QString::number(i));
        QString websiteKey = QString("totp/%1/website").arg(QString::number(i));
        settings->setValue(nameKey, t->name());
        settings->setValue(keyKey, t->key());
        settings->setValue(tokenLengthKey, t->tokenLength());
        settings->setValue(websiteKey, t->website());
    }
}

TOTP *QFactorMain::getTotpFromItemRow(int row)
{
    return static_cast<TOTP*>(ui->tblTotp->item(row, 0)->data(Qt::UserRole).value<void*>());
}
