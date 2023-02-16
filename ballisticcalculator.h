#ifndef BALLISTICCALCULATOR_H
#define BALLISTICCALCULATOR_H
#include "ballisticstructs.h"

class BallisticCalculator
{

public:
    static Trajectory calculate_trajectory(double a, double v, double h, double p,
                              double C, double A, double m);

    static std::pair<double, double> v_to_components(double v, double angle);
    static double components_to_v(double x, double y);
    static double get_air_density(double h);
    static double get_frontal_area(double d);
    static double get_CdG7(double targetVelocity, double base_Cd);
    static double get_mach(double velocity);
    static double find_angle(double r, double try_a, double v, double h, double p,
                      double C, double A, double m, double min,
                      double max, double tolerance, double i);

};
#endif // BALLISTICCALCULATOR_H
