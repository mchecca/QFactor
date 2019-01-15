#include "QFactorMain.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QClipboard>
#include <QDesktopServices>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QUrl>
#include <QAction>
#include "NewTOTPDialog.h"
#include "ui_QFactorMain.h"

QFactorMain::QFactorMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QFactorMain)
{
    ui->setupUi(this);
    ui->lblStatus->setText(QString());;

    /* center the window */
    QRect screenFrame = frameGeometry();
    screenFrame.moveCenter(QDesktopWidget().availableGeometry().center());
    move(screenFrame.topLeft());

    /* set up TOTP table */
    ui->tblTotp->setColumnCount(5);
    ui->tblTotp->setHorizontalHeaderLabels(QStringList() <<
                                           tr("Account") <<
                                           tr("Token") <<
                                           tr("Website") <<
                                           tr("Action") <<
                                           tr("QR Code"));

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

    QTimer::singleShot(0, this, SLOT(refreshTimerTimeout()));
}

QFactorMain::~QFactorMain()
{
    saveSettings();
    QApplication::clipboard()->clear();
    delete ui;
    foreach (TOTP *t, totpList)
        delete t;
    refreshTimer->stop();
    clipboardTimer->stop();
    delete refreshTimer;
    delete clipboardTimer;
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
    /* if show QR was clicked, pop up the image in a new window */
    else if (index.column() == 4)
    {
        QString cmd = "qrencode";
        QString otpUri = QString("otpauth://totp/%1?secret=%2").arg(t->name(), t->key());
        QStringList args = QStringList() << "-t" << "png" << otpUri << "-o" << "-";
        QProcess p(this);
        p.start(cmd, args, QProcess::ReadOnly);
        if (p.waitForFinished() && p.exitStatus() == QProcess::NormalExit)
        {
            QByteArray output = p.readAllStandardOutput();
            QPixmap qrPixmap = QPixmap::fromImage(QImage::fromData(output));
            QDialog *dialog = new QDialog(this);
            QLayout *layout = new QGridLayout(dialog);
            QLabel *nameLabel = new QLabel(dialog);
            QLabel *label = new QLabel(dialog);
            QPushButton *close = new QPushButton(dialog);
            connect(close, &QPushButton::clicked, dialog, &QDialog::accept);
            layout->addWidget(nameLabel);
            layout->addWidget(label);
            layout->addWidget(close);
            nameLabel->setText(t->name());
            nameLabel->setAlignment(Qt::AlignCenter);
            nameLabel->setFont(QFont(nameLabel->font().family(), 18));
            label->setPixmap(qrPixmap.scaledToWidth(this->width() / 2));
            close->setText("Close");
            dialog->resize(this->width() / 2, this->height() / 2);
            dialog->setWindowFlag(Qt::FramelessWindowHint);
            dialog->setWindowModality(Qt::WindowModal);
            dialog->setLayout(layout);
            label->show();
            dialog->show();
        }
        else
        {
            QString errorMsg = p.errorString();
            if (p.error() == QProcess::FailedToStart)
            {
                errorMsg = "Unable to find qrencode binary";
            }
            QMessageBox::critical(this, "QFactor", errorMsg);
        }
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
    QTableWidgetItem *qrItem = new QTableWidgetItem();
    item->setData(Qt::UserRole, qVariantFromValue((void *) totp));
    tokenItem->setFlags(tokenItem->flags() ^ Qt::ItemIsEditable);
    actionItem->setFlags(actionItem->flags() ^ Qt::ItemIsEditable);
    qrItem->setFlags(qrItem->flags() ^ Qt::ItemIsEditable);
    ui->tblTotp->setItem(rowCount, 0, item);
    ui->tblTotp->setItem(rowCount, 1, tokenItem);
    ui->tblTotp->setItem(rowCount, 2, new QTableWidgetItem());
    ui->tblTotp->setItem(rowCount, 3, actionItem);
    ui->tblTotp->setItem(rowCount, 4, qrItem);
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
        QTableWidgetItem *qr = ui->tblTotp->item(i, 4);
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
        qr->setText(tr("Show"));

    }
    ui->tblTotp->resizeColumnsToContents();
}

void QFactorMain::loadSettings()
{
    QFile settingsFile(SETTINGS_FILE);
    if (settingsFile.exists() && settingsFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = settingsFile.readAll();
        QJsonParseError error;
        QJsonDocument settingsDoc = QJsonDocument::fromJson(data, &error);
        if (error.error == QJsonParseError::NoError)
        {
            QJsonObject settingsObj, locationObj, sizeObj;
            QJsonValue locationValue, sizeValue;
            settingsObj = settingsDoc.object();
            locationValue = settingsObj.value("location");
            if (!locationValue.isNull())
            {
                locationObj = locationValue.toObject();
                if (!locationObj.isEmpty())
                {
                    QPoint location(locationObj.value("x").toInt(this->pos().x()),
                                    locationObj.value("y").toInt(this->pos().y()));
                    this->move(location);
                }
            }
            sizeValue = settingsObj.value("size");
            if (!sizeValue.isNull())
            {
                sizeObj = sizeValue.toObject();
                if (!sizeObj.isEmpty())
                {
                    QSize size(sizeObj.value("width").toInt(this->size().width()),
                               sizeObj.value("height").toInt(this->size().height()));
                    this->resize(size);
                }
            }
            bool isMaximized = settingsObj.value("maximized").toBool(false);
            if (isMaximized)
            {
                this->showMaximized();
            }
        }
    }

    QFile totpFile(TOTP_FILE);
    if (totpFile.exists() && totpFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = totpFile.readAll();
        QJsonParseError error;
        QJsonDocument totpDoc = QJsonDocument::fromJson(data, &error);
        if (error.error == QJsonParseError::NoError)
        {
            QJsonArray totps = totpDoc.array();
            for (QJsonValue t : totps)
            {
                QJsonObject tObj = t.toObject();
                if (tObj.isEmpty())
                {
                    continue;
                }
                QString secret, label, website;
                int digits;
                secret = tObj.value("secret").toString();
                label = tObj.value("label").toString();
                website = tObj.value("website").toString();
                digits = tObj.value("digits").toInt();
                addTOTP(label, secret, digits, website, false);
            }
        }
    }
    refreshTotps();
}

void QFactorMain::saveSettings()
{
    QJsonDocument settingsDoc;
    QJsonObject settingsObj, locationObj, sizeObj;
    locationObj.insert("x", this->pos().x());
    locationObj.insert("y", this->pos().y());
    settingsObj.insert("location", locationObj);
    sizeObj.insert("width", this->size().width());
    sizeObj.insert("height", this->size().height());
    settingsObj.insert("size", sizeObj);
    settingsObj.insert("maximized", this->isMaximized());
    settingsDoc.setObject(settingsObj);
    QFile settingsFile(SETTINGS_FILE);
    if (SETTINGS_DIR.mkpath(".") && settingsFile.open(QIODevice::WriteOnly))
    {
        settingsFile.write(settingsDoc.toJson(QJsonDocument::Indented));
    }
    QJsonArray totps;
    for (TOTP *t : totpList)
    {
        QJsonObject totpObj;
        totpObj.insert("secret", t->key());
        totpObj.insert("label", t->name());
        totpObj.insert("website", t->website());
        totpObj.insert("digits", t->tokenLength());
        totps.append(totpObj);
    }
    QJsonDocument totpDoc;
    totpDoc.setArray(totps);
    QFile totpFile(TOTP_FILE);
    if (SETTINGS_DIR.mkpath(".") && totpFile.open(QIODevice::WriteOnly))
    {
        totpFile.write(totpDoc.toJson(QJsonDocument::Indented));
    }
}

TOTP *QFactorMain::getTotpFromItemRow(int row)
{
    return static_cast<TOTP*>(ui->tblTotp->item(row, 0)->data(Qt::UserRole).value<void*>());
}
