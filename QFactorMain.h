#ifndef QFACTORMAIN_H
#define QFACTORMAIN_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSettings>
#include <QTimer>
#include "TOTP.h"

#define TOKEN_REFRESH_RATE 30 /* seconds */

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
    void totpDoubleClicked(QModelIndex index);
    void deleteClicked();

private:
    Ui::QFactorMain *ui;
    QList<TOTP*> totpList;
    QTimer *refreshTimer;
    QTimer *clipboardTimer;
    QSettings *settings;

    void addTOTP(QString name, QString key, QString website);
    void refreshTotps();
    void loadSettings();
    void saveSettings();
};

#endif // QFACTORMAIN_H
