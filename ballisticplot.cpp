#include "ballisticplot.h"
#include <QPainter>
#include <cmath>

#define upper_left_x 40;
#define upper_left_y 80;
#define lower_right_x 560;
#define lower_right_y 320;

BallisticPlot::BallisticPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BallisticPlot)
{
    ui->setupUi(this);
}

void BallisticPlot::draw_plot(const Trajectory& traj){
    QVector<double> qTrajX;
    QVector<double> qTrajY;
    QString s = "e";

    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(qTrajX, qTrajY);
    int vec_size = traj.pos_points.size();
    int vec_increment = vec_size / 100;
    for (int i = 0, j = 0; i < vec_size - vec_increment; i += vec_increment, j++){
        qTrajX[j] = traj.pos_points[i].first;
        qTrajY[j] = traj.pos_points[i].second;
    }

    ui->customPlot->replot();
}

BallisticPlot::~BallisticPlot()
{
    delete ui;
}
