#ifndef BALLISTICSTRUCTS_H
#define BALLISTICSTRUCTS_H

#include <vector>

struct Trajectory {
    double angle;
    std::pair<double, double> apex;
    std::vector<std::pair<double, double>> pos_points;
    double time;
};
#endif // BALLISTICSTRUCTS_H
