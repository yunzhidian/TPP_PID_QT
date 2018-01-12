#include "mainwindow.h"
#include <QApplication>
#include <QFile>

class CommonHelper
{
public:
    static void setStyle(const QString &style) {
        QFile qss(style);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载QSS样式:qss并未完善，初始版本挺丑的，如果有艺术感很好的，可以把qss修改的美观一点
//    CommonHelper::setStyle(":/qss/tpp.qss");

    MainWindow w;
    w.show();

    return a.exec();
}
