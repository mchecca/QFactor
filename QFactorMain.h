#ifndef QFACTORMAIN_H
#define QFACTORMAIN_H

#include <QMainWindow>

namespace Ui {
class QFactorMain;
}

class QFactorMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit QFactorMain(QWidget *parent = 0);
    ~QFactorMain();

private:
    Ui::QFactorMain *ui;
};

#endif // QFACTORMAIN_H
