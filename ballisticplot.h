#ifndef BallisticPlot_H
#define BallisticPlot_H

#include "ballisticstructs.h"
#include <QWidget>
namespace Ui {
class BallisticPlot;
}

class BallisticPlot : public QWidget
{
    Q_OBJECT

public:
    explicit BallisticPlot(QWidget *parent = nullptr);
    void draw_plot(const Trajectory &traj);
    ~BallisticPlot();

private:
    double widthScale = 1;
    double heightScale = 1;

    Ui::BallisticPlot *ui;
};

#endif // BallisticPlot_H
