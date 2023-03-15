#pragma once

#include <ballisticstructs.h>
#include <QMainWindow>
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class BallisticUI; }
QT_END_NAMESPACE

class BallisticUI : public QMainWindow
{
    Q_OBJECT
public:
    BallisticUI(QWidget *parent = nullptr);
    ~BallisticUI();

    void draw_plot(Trajectory &traj);
    enum CalcSetting { range, angle };

private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentIndexChanged(int index);


private:
    Ui::BallisticUI *ui;
    QMainWindow *plotWindow;
    QLineSeries *series;
    QChart *chart;
    QChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;

signals:

};

