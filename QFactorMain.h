#ifndef QFACTORMAIN_H
#define QFACTORMAIN_H

#include <QDir>
#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStandardPaths>
#include <QTimer>
#include "TOTP.h"

#define TOKEN_REFRESH_RATE 30 /* seconds */
const QDir SETTINGS_DIR = QDir(QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).absoluteFilePath("QFactor"));
const QString SETTINGS_FILE = SETTINGS_DIR.absoluteFilePath("settings.json");
const QString TOTP_FILE = SETTINGS_DIR.absoluteFilePath("totp.json");

namespace Ui {
class QFactorMain;
}

class QFactorMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit QFactorMain(QWidget *parent = 0);
    ~QFactorMain();

public slots:
    void addClicked();
    void refreshTimerTimeout();
    void clipboardTimerTimeout();
    void totpItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void totpItemClicked(QModelIndex index);
    void totpItemCellChanged(int row, int column);
    void deleteClicked();

private:
    Ui::QFactorMain *ui;
    QList<TOTP*> totpList;
    QTimer *refreshTimer;
    QTimer *clipboardTimer;

    void addTOTP(QString name, QString key, int tokenLength, QString website, bool save = true);
    void refreshTotps();
    void loadSettings();
    void saveSettings();
    TOTP *getTotpFromItemRow(int row);
};

#endif // QFACTORMAIN_H
