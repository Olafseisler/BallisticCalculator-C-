#include "ballisticcalculator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BallisticCalculator w;
    w.show();
    return a.exec();
}
