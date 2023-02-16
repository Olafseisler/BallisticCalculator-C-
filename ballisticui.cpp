#include "ballisticui.h"
#include "ui_ballisticcalculator.h"
#include "QDoubleValidator"


static BallisticUI::CalcSetting setting;

// Custom validator class for parameter fields
class MyValidator : public QDoubleValidator
{
public:
    MyValidator(double bottom, double top, int decimals, QObject *parent) :
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

BallisticUI::BallisticUI(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::BallisticUI)
{
    ui->setupUi(this);
    plotWindow = nullptr;

    // Set up numerical only fields
    ui->initialVelocityBox->setValidator( new MyValidator(-1, 2000, 2, this) );
    ui->initialHeightBox->setValidator( new MyValidator(-1, 10000, 2, this) );
    ui->angleBox->setValidator( new MyValidator(-91.0, 90, 3, this) );
    ui->projectileMassBox->setValidator( new MyValidator(-1, 1000, 5, this) );
    ui->projectileDiameterBox->setValidator( new MyValidator(-1, 10, 5, this) );
    ui->cdBox->setValidator( new MyValidator(-1, 3, 3, this) );

}

BallisticUI::~BallisticUI()
{
    delete ui;
    delete chart;
    delete axisX;
    delete axisY;
    delete plotWindow;
}



void BallisticUI::draw_plot(Trajectory &traj){
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
void BallisticUI::on_pushButton_clicked()
{
    double v = ui->initialVelocityBox->text().toDouble();
    double h = ui->initialHeightBox->text().toDouble();
    double p = BallisticCalculator::get_air_density(h);
    double C = ui->cdBox->text().toDouble();
    double diameter = ui->projectileDiameterBox->text().toDouble();
    double A = BallisticCalculator::get_frontal_area(diameter);
    double m = ui->projectileMassBox->text().toDouble();
    double a = ui->angleBox->text().toDouble();

    if (v <= 0) return;

    if (setting == CalcSetting::range){
        Trajectory traj = BallisticCalculator::calculate_trajectory(a, v, h, p, C, A, m);
        std::pair<double, double> last_point = traj.pos_points.back();
        ui->resultBox->setText(QString::number(last_point.first, 'f', 2));
        draw_plot(traj);

    }
    else if (setting == CalcSetting::angle){
        double desired_range = ui->angleBox->text().toDouble();
        double angle = BallisticCalculator::find_angle(desired_range, 27.5, v, h, p, C, A, m, -10, 45, 0.01, 20);
        if (angle == -100){
            ui->resultBox->setText("OUT OF MAX RANGE");
        } else {
            ui->resultBox->setText(QString::number(angle, 'f', 3));
        }
    }

}

// Trigger activates on combo box parameter changed
void BallisticUI::on_comboBox_currentIndexChanged(int index)
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


