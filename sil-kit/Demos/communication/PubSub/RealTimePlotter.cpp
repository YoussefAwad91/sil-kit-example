#include "RealTimePlotter.h"

#include "SimplePlotWidget.h"
#include "PendulumWidget.h"

#include <QPen>
#include <QVector>

RealTimePlotter::RealTimePlotter(int bufferSize, int updateFrequency, QWidget *parent)
    : QMainWindow(parent),
      buffer_size(bufferSize),
      update_frequency(updateFrequency),
      update_counter(0),
      centralWidget(nullptr),
      gridLayout(nullptr),
      pos_plot(nullptr),
      angle_plot(nullptr),
      force_plot(nullptr),
      anim_plot(nullptr)
{
    setupUI();
}

RealTimePlotter::~RealTimePlotter()
{
    // Qt parent-child cleanup
}

void RealTimePlotter::setupUI()
{
    setWindowTitle("Real-Time Inverted Pendulum");
    resize(1200, 800);

    centralWidget = new QWidget(this);
    gridLayout = new QGridLayout(centralWidget);
    setCentralWidget(centralWidget);

    // 1) Position vs Time
    pos_plot = new SimplePlotWidget(this);
    pos_plot->setTitle("Position vs Time");
    pos_plot->setAxisLabels("Time [s]", "Position [m]");
    pos_plot->setLegendVisible(true);
    pos_plot->setGridVisible(true);
    pos_plot->setPanZoomEnabled(true);

    QPen blue(Qt::blue);
    QPen green(Qt::green);
    green.setStyle(Qt::DashLine);

    pos_plot->addSeries("x_current", blue);
    pos_plot->addSeries("x_desired", green);

    gridLayout->addWidget(pos_plot, 0, 0);

    // 2) Angle vs Time
    angle_plot = new SimplePlotWidget(this);
    angle_plot->setTitle("Angle vs Time");
    angle_plot->setAxisLabels("Time [s]", "Angle [deg]");
    angle_plot->setLegendVisible(false);
    angle_plot->setGridVisible(true);
    angle_plot->setPanZoomEnabled(true);

    angle_plot->addSeries("angle", QPen(Qt::red));

    gridLayout->addWidget(angle_plot, 0, 1);

    // 3) Force vs Time
    force_plot = new SimplePlotWidget(this);
    force_plot->setTitle("Force vs Time");
    force_plot->setAxisLabels("Time [s]", "Force");
    force_plot->setLegendVisible(false);
    force_plot->setGridVisible(true);
    force_plot->setPanZoomEnabled(true);

    force_plot->addSeries("force", QPen(Qt::magenta));

    gridLayout->addWidget(force_plot, 1, 0);

    // 4) Pendulum Animation
    anim_plot = new PendulumWidget(this);
    anim_plot->setTitle("Inverted Pendulum");
    anim_plot->setXLabel("Cart position [m]");
    anim_plot->setYRange(-0.6, 0.6);
    anim_plot->setRodLength(0.5);
    anim_plot->setAutoExpandX(true);
    anim_plot->setXTicks(6); 

    gridLayout->addWidget(anim_plot, 1, 1);
}

void RealTimePlotter::update_data(double t, double x_current, double x_desired,
                                  double angle_deg, double force)
{
    t_buf.push_back(t);
    x_current_buf.push_back(x_current);
    x_desired_buf.push_back(x_desired);
    angle_buf.push_back(angle_deg);
    force_buf.push_back(force);

    if ((int)t_buf.size() > buffer_size) {
        t_buf.pop_front();
        x_current_buf.pop_front();
        x_desired_buf.pop_front();
        angle_buf.pop_front();
        force_buf.pop_front();
    }

    update_counter++;
    if (update_counter >= update_frequency) {
        update_counter = 0;
        _redraw();
    }
}

std::vector<double> RealTimePlotter::_deque_to_vector(const std::deque<double>& dq)
{
    return std::vector<double>(dq.begin(), dq.end());
}

void RealTimePlotter::_redraw()
{
    std::vector<double> t_arr = _deque_to_vector(t_buf);
    std::vector<double> x_cur_arr = _deque_to_vector(x_current_buf);
    std::vector<double> x_des_arr = _deque_to_vector(x_desired_buf);
    std::vector<double> angle_arr = _deque_to_vector(angle_buf);
    std::vector<double> force_arr = _deque_to_vector(force_buf);

    if (t_arr.empty()) return;

    QVector<double> t_qvec(t_arr.begin(), t_arr.end());
    QVector<double> x_cur_q(x_cur_arr.begin(), x_cur_arr.end());
    QVector<double> x_des_q(x_des_arr.begin(), x_des_arr.end());
    QVector<double> ang_q(angle_arr.begin(), angle_arr.end());
    QVector<double> force_q(force_arr.begin(), force_arr.end());

    // Position plot
    pos_plot->setSeriesData("x_current", t_qvec, x_cur_q);
    pos_plot->setSeriesData("x_desired", t_qvec, x_des_q);
    pos_plot->setXRange(t_qvec.first(), t_qvec.last());
    pos_plot->autoScaleYFromData();

    // Angle plot
    angle_plot->setSeriesData("angle", t_qvec, ang_q);
    angle_plot->setXRange(t_qvec.first(), t_qvec.last());
    angle_plot->autoScaleYFromData();

    // Force plot
    force_plot->setSeriesData("force", t_qvec, force_q);
    force_plot->setXRange(t_qvec.first(), t_qvec.last());
    force_plot->autoScaleYFromData();

    // Pendulum animation (last sample)
    const double x = x_cur_arr.back();
    const double theta = angle_arr.back() * M_PI / 180.0;
    anim_plot->setState(x, theta);
}

void RealTimePlotter::close()
{
    QMainWindow::close();
}
