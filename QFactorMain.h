#ifndef QFACTORMAIN_H
#define QFACTORMAIN_H

#include <QMainWindow>
#include <QList>
#include <QListWidgetItem>
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

private:
    Ui::QFactorMain *ui;
    QList<TOTP*> totpList;
    QTimer *refreshTimer;
    QSettings *settings;

    void addTOTP(QString name, QString key);
    void refreshTotps();
    void loadSettings();
    void saveSettings();
};

#endif // QFACTORMAIN_H
