#include "QFactorMain.h"
#include <QApplication>
#include <QTranslator>

#define LOCALE_DIR "locale"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    QByteArray langEnv = qgetenv("LANG");
    QString localeStr = QString("%1.qm").arg(QString(langEnv.constData()).split(".").at(0));
    translator.load(localeStr, LOCALE_DIR);
    a.installTranslator(&translator);

    QFactorMain w;
    w.show();

    return a.exec();
}
