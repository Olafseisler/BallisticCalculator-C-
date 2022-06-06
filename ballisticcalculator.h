#ifndef BALLISTICCALCULATOR_H
#define BALLISTICCALCULATOR_H


QT_BEGIN_NAMESPACE
namespace Ui { class BallisticCalculator; }
QT_END_NAMESPACE

struct Trajectory {
    double angle;
    std::vector<std::pair<double, double>> pos_points;
    double time;
};

class BallisticCalculator : public QMainWindow
{
    Q_OBJECT

public:
    BallisticCalculator(QWidget *parent = nullptr);
    ~BallisticCalculator();
    Trajectory calculate_trajectory(double a, double v, double h, double p,
                              double C, double A, double m);

    std::pair<double, double> v_to_components(double v, double angle);
    double components_to_v(double x, double y);
    double get_air_density(double h);
    double get_frontal_area(double d);
    double get_CdG7(double targetVelocity, double base_Cd);
    double get_mach(double velocity);
    void draw_plot(Trajectory traj);
    double find_angle(double r, double try_a, double v, double h, double p,
                      double C, double A, double m, double min,
                      double max, double tolerance, double i);

private slots:
    void on_pushButton_clicked();
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::BallisticCalculator *ui;
};
#endif // BALLISTICCALCULATOR_H
