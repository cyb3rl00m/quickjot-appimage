#include <QApplication>
#include <QFile>
#include <QString>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName(QStringLiteral("QuickJot"));
    QApplication::setApplicationDisplayName(QStringLiteral("QuickJot"));
    QApplication::setOrganizationName(QStringLiteral("QuickJot"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }

    MainWindow window;
    window.show();

    return app.exec();
}
