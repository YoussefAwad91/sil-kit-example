#ifndef REALTIMEPLOTTER_H
#define REALTIMEPLOTTER_H

#include <QMainWindow>
#include <QGridLayout>
#include <deque>
#include <vector>
#include <cmath>

class SimplePlotWidget;
class PendulumWidget;

class RealTimePlotter : public QMainWindow {
    Q_OBJECT

public:
    RealTimePlotter(int bufferSize = 1000, int updateFrequency = 5, QWidget *parent = nullptr);
    ~RealTimePlotter();

    void update_data(double t, double x_current, double x_desired,
                     double angle_deg, double force);

    void close();

private:
    // Configuration
    int buffer_size;
    int update_frequency;
    int update_counter;

    // Fixed-size FIFO buffers
    std::deque<double> t_buf;
    std::deque<double> x_current_buf;
    std::deque<double> x_desired_buf;
    std::deque<double> angle_buf;
    std::deque<double> force_buf;

    // UI elements
    QWidget *centralWidget;
    QGridLayout *gridLayout;

    // Plots 
    SimplePlotWidget *pos_plot;
    SimplePlotWidget *angle_plot;
    SimplePlotWidget *force_plot;
    PendulumWidget  *anim_plot;

    // Helper methods
    void setupUI();
    void _redraw();
    std::vector<double> _deque_to_vector(const std::deque<double>& dq);
};

#endif // REALTIMEPLOTTER_H
