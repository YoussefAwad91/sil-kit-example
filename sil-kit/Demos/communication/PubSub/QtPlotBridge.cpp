#include "QtPlotBridge.h"

// Qt
#include <QApplication>
#include <QTimer>
#include <QCoreApplication>
#include <QMetaObject>

#include "RealTimePlotter.h"

QtPlotBridge::QtPlotBridge() {}

QtPlotBridge::~QtPlotBridge()
{
    stop();
}

void QtPlotBridge::start()
{
    if (running) return;

    stopRequested = false;
    running = true;

    guiThread = std::thread(&QtPlotBridge::guiThreadMain, this);
}

void QtPlotBridge::stop()
{
    if (!running) return;

    stopRequested = true;

    // If the Qt loop exists, ask it to quit (thread-safe)
    if (QCoreApplication::instance()) {
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
    }

    if (guiThread.joinable()) {
        guiThread.join();
    }

    running = false;
    ready = false;
}

void QtPlotBridge::push(double t, double x_current, double x_desired, double angle_deg, double force)
{
    if (!ready) return; // drop samples until GUI is up

    std::lock_guard<std::mutex> lock(mtx);

    if (queue.size() >= maxQueueSize) {
        queue.pop_front();
    }

    queue.push_back(PlotSample{t, x_current, x_desired, angle_deg, force});
}

void QtPlotBridge::guiThreadMain()
{
    int argc = 0;
    char** argv = nullptr;

    QApplication app(argc, argv);

    RealTimePlotter plotter(/*bufferSize=*/1000, /*updateFrequency=*/1);
    plotter.resize(1200, 800);
    plotter.show();

    ready = true;

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        // Drain queue
        std::deque<PlotSample> local;
        {
            std::lock_guard<std::mutex> lock(mtx);
            local.swap(queue);
        }

        for (const auto& s : local) {
            plotter.update_data(s.t, s.x_current, s.x_desired, s.angle_deg, s.force);
        }

        // Optional: if someone asked us to stop but quit wasn't invoked for some reason
        if (stopRequested) {
            QCoreApplication::quit();
        }
    });

    timer.start(16); // ~60 Hz

    app.exec();

    ready = false;
    running = false;
}
