#pragma once

#include <deque>
#include <mutex>
#include <thread>
#include <atomic>

class QtPlotBridge
{
public:
    QtPlotBridge();
    ~QtPlotBridge();

    QtPlotBridge(const QtPlotBridge&) = delete;
    QtPlotBridge& operator=(const QtPlotBridge&) = delete;

    // Start/stop GUI thread
    void start();
    void stop();

    
    void push(double t, double x_current, double x_desired, double angle_deg, double force);

private:
    struct PlotSample {
        double t;
        double x_current;
        double x_desired;
        double angle_deg;
        double force;
    };

    void guiThreadMain();

    std::thread guiThread;
    std::mutex mtx;
    std::deque<PlotSample> queue;

    std::atomic<bool> running{false};
    std::atomic<bool> ready{false};
    std::atomic<bool> stopRequested{false};

    size_t maxQueueSize = 5000;
};
