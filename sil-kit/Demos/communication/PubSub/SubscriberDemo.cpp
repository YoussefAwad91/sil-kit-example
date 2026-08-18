#include "ApplicationBase.hpp"
#include "PubSubDemoCommon.hpp"
#include "QtPlotBridge.h"

#include <chrono>
#include <mutex>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================================
// NOTE:
// PubSubDemoCommon.hpp (as used in Controller.cpp) already provides
// MakeForceSpec() / MakeAngleSpec() for the "force" / "angle" topics.
// It does not yet have equivalents for "x_current" / "x_desired". Add two
// functions there mirroring whatever MakeForceSpec()/MakeAngleSpec() do
// internally, e.g.:
//
//   inline SilKit::Services::PubSub::PubSubSpec MakeXCurrentSpec()
//   {
//       return SilKit::Services::PubSub::PubSubSpec{
//           "x_current", /* same media type MakeForceSpec() uses */};
//   }
//
//   inline SilKit::Services::PubSub::PubSubSpec MakeXDesiredSpec()
//   {
//       return SilKit::Services::PubSub::PubSubSpec{
//           "x_desired", /* same media type MakeForceSpec() uses */};
//   }
//
// This file assumes those two functions exist under those names.
// =============================================================================

class Plotter : public ApplicationBase
{
public:
    using ApplicationBase::ApplicationBase;

private:
    IDataSubscriber* _forceSub{nullptr};
    IDataSubscriber* _angleSub{nullptr};
    IDataSubscriber* _xCurrentSub{nullptr};
    IDataSubscriber* _xDesiredSub{nullptr};

    // =========================
    // GUI BRIDGE
    // =========================

    QtPlotBridge _plotBridge;

    // =========================
    // LATEST VALUES
    // =========================
    // Each topic updates independently and asynchronously, so we keep the
    // latest known value of each. Unlike a per-signal push, samples are only
    // sent to the plot once per simulation step (see DoWorkSync), mirroring
    // how the VSI-based Plotter pushes once per step inside its main thread.

    std::mutex _stateMutex;
    double _force = 0.0;
    double _angle = 0.0;    // radians, as published by Controller
    double _xCurrent = 0.0;
    double _xDesired = 0.0;

    bool _t0Set = false;
    std::chrono::nanoseconds _t0{0};

    void AddCommandLineArgs() override
    {
    }

    void EvaluateCommandLineArgs() override
    {
    }

    // Converts an absolute SILKit timestamp into seconds relative to the
    // first sample received, since RealTimePlotter plots on a relative
    // time axis.
    double ToRelSeconds(std::chrono::nanoseconds timestamp)
    {
        if (!_t0Set)
        {
            _t0 = timestamp;
            _t0Set = true;
        }

        return std::chrono::duration<double>(timestamp - _t0).count();
    }

    void CreateControllers() override
    {
        // Bring up the Qt window on its own thread before data starts
        // flowing, so early samples aren't dropped.
        _plotBridge.start();

        GetLogger()->Info("Creating ForceSubscriber");

        _forceSub =
            GetParticipant()->CreateDataSubscriber(
                "ForceSubscriber",
                PubSubDemoCommon::MakeForceSpec(),
                [this](IDataSubscriber*, const DataMessageEvent& event)
                {
                    double v =
                        PubSubDemoCommon::DeserializeDouble(
                            SilKit::Util::ToStdVector(event.data));

                    std::lock_guard<std::mutex> lock(_stateMutex);
                    _force = v;
                });

        GetLogger()->Info("Creating AngleSubscriber");

        _angleSub =
            GetParticipant()->CreateDataSubscriber(
                "AngleSubscriber",
                PubSubDemoCommon::MakeAngleSpec(),
                [this](IDataSubscriber*, const DataMessageEvent& event)
                {
                    double v =
                        PubSubDemoCommon::DeserializeDouble(
                            SilKit::Util::ToStdVector(event.data));

                    std::lock_guard<std::mutex> lock(_stateMutex);
                    _angle = v;
                });

        GetLogger()->Info("Creating XCurrentSubscriber");

        _xCurrentSub =
            GetParticipant()->CreateDataSubscriber(
                "XCurrentSubscriber",
                PubSubDemoCommon::MakeXCurrentSpec(),
                [this](IDataSubscriber*, const DataMessageEvent& event)
                {
                    double v =
                        PubSubDemoCommon::DeserializeDouble(
                            SilKit::Util::ToStdVector(event.data));

                    std::lock_guard<std::mutex> lock(_stateMutex);
                    _xCurrent = v;
                });

        GetLogger()->Info("Creating XDesiredSubscriber");

        _xDesiredSub =
            GetParticipant()->CreateDataSubscriber(
                "XDesiredSubscriber",
                PubSubDemoCommon::MakeXDesiredSpec(),
                [this](IDataSubscriber*, const DataMessageEvent& event)
                {
                    double v =
                        PubSubDemoCommon::DeserializeDouble(
                            SilKit::Util::ToStdVector(event.data));

                    std::lock_guard<std::mutex> lock(_stateMutex);
                    _xDesired = v;
                });
    }

    void InitControllers() override
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _force = 0.0;
        _angle = 0.0;
        _xCurrent = 0.0;
        _xDesired = 0.0;
        _t0Set = false;
    }

    // Called once per simulation step. Pushes exactly one combined sample
    // per step using whatever latest values have accumulated from the
    // subscriber callbacks, matching the VSI Plotter's mainThread() cadence.
    // Note: angle is pushed as-is (radians), not converted to degrees, to
    // match the VSI Plotter's behavior.
    void DoWorkSync(
        std::chrono::nanoseconds timestamp) override
    {
        double t = ToRelSeconds(timestamp);

        double force;
        double angle;
        double xCur;
        double xDes;

        {
            std::lock_guard<std::mutex> lock(_stateMutex);
            force = _force;
            angle = _angle;
            xCur = _xCurrent;
            xDes = _xDesired;
        }

        _plotBridge.push(t, xCur, xDes, angle, force);
    }

    void DoWorkAsync() override
    {
    }
};

int main(int argc, char** argv)
{
    Arguments args;
    args.participantName = "Plotter";

    Plotter app(args);

    app.SetupCommandLineArgs(
        argc,
        argv,
        "Pendulum Real-Time Plotter");

    return app.Run();
}