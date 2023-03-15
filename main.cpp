#include "ballisticui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BallisticUI ballisticUI;
    ballisticUI.show();
    return a.exec();
}
