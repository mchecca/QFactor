#ifndef QFACTORMAIN_H
#define QFACTORMAIN_H

#include <QMainWindow>
#include <QList>
#include <QListWidgetItem>
#include "TOTP.h"

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
    void totpItemChanged(QListWidgetItem *item);

private:
    Ui::QFactorMain *ui;
    QList<TOTP*> totpList;
};

#endif // QFACTORMAIN_H
