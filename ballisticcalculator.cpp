#include "ballisticcalculator.h"
#include "ui_ballisticcalculator.h"
//#include "ballisticplot.h"
#include "QDoubleValidator"

#include <cmath>
#include <vector>
#include <map>
#include <iostream>


// Constants
#define PI 3.14159265 // pi
#define g 9.8067 // acceleration due to gravity
#define TIMESTEP 0.001 // simulation granularity
#define p0 101325 // base air density at sea level
#define T0 288.15 // base temp in Kelvin at sea level
#define M 0.0289652 // molar mass of dry air
#define R 8.31446 // ideal gas constant
#define L 0.0065 // temperature lapse rate


// Which parameter to calculate
enum CalcSetting { range, angle };
static CalcSetting setting;
static std::map<double, double> cd_G7 = { {0, 0.9061488673139159},
                                        {0.4, 0.9022090896299423},
                                        {0.5, 0.7879555367947094},
                                        {0.6, 0.6737019839594766},
                                        {0.7, 0.6461235401716617},
                                        {0.8, 0.5673279864921907},
                                        {0.825, 0.5555086534402701},
                                        {0.85, 0.539749542704376},
                                        {0.875, 0.539749542704376},
                                        {0.9, 0.5594484311242436},
                                        {0.925, 0.6067257633319262},
                                        {0.95, 0.6973406500633178},
                                        {0.975, 0.929787533417757},
                                        {1, 1.2055719712959054},
                                        {1.025, 1.3158857464471647},
                                        {1.05, 1.3434641902349795},
                                        {1.075, 1.3592233009708736},
                                        {1.1, 1.3671028563388208},
                                        {1.15, 1.3710426340227944},
                                        {1.2, 1.3710426340227944},
                                        {1.3, 1.3513437456029267},
                                        {1.4, 1.3237653018151119},
                                        {1.5, 1.2922470803433235},
                                        {1.6, 1.2646686365555087},
                                        {1.8, 1.1976924159279583},
                                        {2, 1.1504150837202756},
                                        {2.2, 1.1110173068805402},
                                        {2.4, 1.0637399746728577}};

// Custom validator class for parameter fields
class MyValidator : public QDoubleValidator
{
public:
    MyValidator(double bottom, double top, int decimals, QObject * parent) :
        QDoubleValidator(bottom, top, decimals, parent)
    {
    }

    QValidator::State validate(QString &s, int &i) const
    {
        if (s.isEmpty() || s == '-') {
            return QValidator::Intermediate;
        }

        bool ok;
        double d = s.toDouble(&ok);

        if(s.indexOf('.') != -1) {
            int charsAfterPoint = s.length() - s.indexOf('.') - 1;

            if (charsAfterPoint > decimals()) {
                return QValidator::Invalid;
            }
        }

        if (ok && d > bottom() && d < top()) {
            return QValidator::Acceptable;
        } else {
            return QValidator::Invalid;
        }
    }
};



// Set up UI
BallisticCalculator::BallisticCalculator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BallisticCalculator)
{
    ui->setupUi(this);

//    plot = new BallisticPlot();
//    plot->show();
    plotWindow = nullptr;


    // Set up numerical only fields
    ui->initialVelocityBox->setValidator( new MyValidator(-1, 2000, 2, this) );
    ui->initialHeightBox->setValidator( new MyValidator(-1, 10000, 2, this) );
    ui->angleBox->setValidator( new MyValidator(-91.0, 90, 3, this) );
    ui->projectileMassBox->setValidator( new MyValidator(-1, 1000, 5, this) );
    ui->projectileDiameterBox->setValidator( new MyValidator(-1, 10, 5, this) );
    ui->cdBox->setValidator( new MyValidator(-1, 3, 3, this) );

}

// Destructor
BallisticCalculator::~BallisticCalculator()
{
    delete ui;
    delete plotWindow;
    delete chart;
    delete axisX;
    delete axisY;
}

// Calculate projectile trajectory given angle, velocity and other parameters
Trajectory BallisticCalculator::calculate_trajectory(double a, double v, double h,
                                               double p, double C, double A, double m){
    std::pair<double, double> pair = v_to_components(v, a);
    double vx = pair.first;
    double vy = pair.second;
    double x = 0;
    double y = h;
    double time = 0;

    Trajectory trajectory;
    trajectory.pos_points.clear();
    std::pair<double, double> point;
    point.first = x;
    point.second = y;
    trajectory.pos_points.push_back(point);
    trajectory.angle = a;

    // Calculate path to trajectory apex
    while (vy > 0){
        x += vx * TIMESTEP;
        y += vy * TIMESTEP;
        std::pair<double, double> new_pos;
        new_pos.first = x;
        new_pos.second = y;
        p = get_air_density(y);
        double Cd = get_CdG7(get_mach(components_to_v(vx, vy)), C);
        vx -= ((p * Cd * A * vx * vx) / (2 * m)) * TIMESTEP;
        vy -= ((p * Cd * A * vy * vy) / (2 * m)) * TIMESTEP + (g * TIMESTEP);
        time += TIMESTEP;
        trajectory.pos_points.push_back(new_pos);
    }

    trajectory.apex = std::pair(x, y);

    // Calculate path to ground
    while (y > 0){
        x += vx * TIMESTEP;
        y += vy * TIMESTEP;
        std::pair<double, double> new_pos;
        new_pos.first = x;
        new_pos.second = y;
        p = get_air_density(y);
        double Cd = get_CdG7(get_mach(components_to_v(vx, vy)), C);
        vx -= ((p * Cd * A * vx * vx) / (2 * m)) * TIMESTEP;
        vy -= -((p * Cd * A * vy * vy) / (2 * m)) * TIMESTEP + (g * TIMESTEP);
        time += TIMESTEP;
        trajectory.pos_points.push_back(new_pos);
    }

    trajectory.time = time;
    return trajectory;
}

void BallisticCalculator::draw_plot(Trajectory &traj){
    if (plotWindow == nullptr){
        plotWindow = new QMainWindow(this);
        plotWindow->resize(420, 300);
        plotWindow->setWindowTitle("Trajectory Plot");

        series = new QLineSeries(plotWindow);
        chart = new QChart();

        axisX = new QValueAxis();
        axisY = new QValueAxis();

        chart->addSeries(series);
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        series->attachAxis(axisX);
        series->attachAxis(axisY);

        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
    }

    series->clear();

    for (size_t i = 0; i < traj.pos_points.size(); i += 10){
        series->append(traj.pos_points[i].first, traj.pos_points[i].second);
    }

    double xScaleMax = traj.pos_points[traj.pos_points.size() - 1].first * 1.02;
    axisX->setRange(traj.pos_points[0].first, xScaleMax);
    axisX->setMin(traj.pos_points[0].first);
    axisX->setMax(xScaleMax);
    axisX->setTickCount(5);

    double yScaleMax = traj.apex.second * 1.02;
    axisY->setRange(traj.pos_points[0].second, yScaleMax);
    axisY->setMin(traj.pos_points[0].second);
    axisY->setMax(yScaleMax);
    axisY->setTickCount(5);

    plotWindow->setCentralWidget(chartView);
    chartView->repaint();
    plotWindow->show();

}

// Trigger for "Calculate" button
void BallisticCalculator::on_pushButton_clicked()
{
    double v = ui->initialVelocityBox->text().toDouble();
    double h = ui->initialHeightBox->text().toDouble();
    double p = get_air_density(h);
    double C = ui->cdBox->text().toDouble();
    double diameter = ui->projectileDiameterBox->text().toDouble();
    double A = get_frontal_area(diameter);
    double m = ui->projectileMassBox->text().toDouble();
    double a = ui->angleBox->text().toDouble();

    if (v <= 0) return;

    if (setting == CalcSetting::range){
        Trajectory traj = calculate_trajectory(a, v, h, p, C, A, m);
        std::pair<double, double> last_point = traj.pos_points.back();
        ui->resultBox->setText(QString::number(last_point.first, 'f', 2));
        draw_plot(traj);

    }
    else if (setting == CalcSetting::angle){
        double desired_range = ui->angleBox->text().toDouble();
        double angle = find_angle(desired_range, 27.5, v, h, p, C, A, m, -10, 45, 0.01, 20);
        if (angle == -100){
            ui->resultBox->setText("OUT OF MAX RANGE");
        } else {
            ui->resultBox->setText(QString::number(angle, 'f', 3));
        }
    }

}

// Returns horizontal and vertical components of velocity given angle to ground
std::pair<double, double> BallisticCalculator::v_to_components(double v, double angle){
    double vx = v * cos ( angle * PI / 180.0 );
    double vy = v * sin ( angle * PI / 180.0 );
    std::pair<double, double> components;
    components.first = vx;
    components.second = vy;
    return components;
}

double BallisticCalculator::components_to_v(double vx, double vy){
    return sqrt(vx * vx + vy * vy);
}

// Return air density at given altitude
double BallisticCalculator::get_air_density(double h){
    return (p0 * M)/(R*T0) * pow((1 - L*h/T0), ((g*M)/(R*L) - 1));
}

// Return frontal area assuming projectile has a circular cross section
double BallisticCalculator::get_frontal_area(double d){
    return PI * 0.25 * d * d;
}

// Return the drag coefficient for a given airspeed
double BallisticCalculator::get_CdG7(double target_velocity, double base_Cd){
    std::map<double, double>::iterator low, prev;
    low = cd_G7.lower_bound(target_velocity);
    if (low == cd_G7.end()) {
        return cd_G7[cd_G7.rbegin()->first] * base_Cd;
    } else if (low == cd_G7.begin()) {
        return cd_G7[low->first] * base_Cd;
    } else {
        prev = std::prev(low);
        if ((target_velocity - prev->first) < (low->first - target_velocity))
            return cd_G7[prev->first] * base_Cd;
        else
            return cd_G7[low->first] * base_Cd;
    }
}

// Return mach number based on velocity
double BallisticCalculator::get_mach(double velocity){
    return velocity / 343.0;
}

// Find angle given a desired range for projectile using an over-under bracketing method
double BallisticCalculator::find_angle(double r, double try_a, double v, double h,
                                       double p, double C, double A,
                                       double m, double min, double max,
                                       double tolerance, double i){

    // Calculate trajectory using try angle
    Trajectory traj = calculate_trajectory(try_a, v, h, p, C, A, m);
    qDebug() << "Angle" << try_a << " Range: " << traj.pos_points.back().first;

    if (i <= 0 && traj.pos_points.back().first < r){
        return -100;
    }

    // If trajectory range is within tolerance or counter reaches zero, return used angle;
    if (i <= 0 || abs(r - traj.pos_points.back().first) < r * tolerance){
        return traj.angle;
    }

    // If resulting range is less than desired, increase try angle by half of min-max difference
    if (traj.pos_points.back().first < r){
        min = try_a;
        return find_angle(r, try_a+(max-min)/2, v, h, p, C, A, m, min, max, tolerance, i - 1);
    }
    // If resulting range is more than desired, decrease try angle by half of min-max difference
    else {
        max = try_a;
        return find_angle(r, try_a-(max-min)/2, v, h, p, C, A, m, min, max, tolerance, i - 1);
    }
}

// Trigger activates on combo box parameter changed
void BallisticCalculator::on_comboBox_currentIndexChanged(int index)
{
    ui->angleBox->setText("");
    ui->resultBox->setText("");

    if (index == 0){
        ui->resultLabel->setText("Range (m):");
        setting = CalcSetting::range;
        ui->angleLabel->setText("Angle (degrees)");
        ui->angleBox->setValidator( new MyValidator(-91.0, 90, 3, this) );
    }
    else if (index == 1){
        ui->resultLabel->setText("Angle (degrees):");
        setting = CalcSetting::angle;
        ui->angleLabel->setText("Range (m)");
        ui->angleBox->setValidator( new MyValidator(-1, 10000, 1, this) );
    }
}

