#include "ApplicationBase.hpp"
#include "PubSubDemoCommon.hpp"

#include <chrono>
#include <string>

class Controller : public ApplicationBase
{
public:
    using ApplicationBase::ApplicationBase;

private:
    IDataSubscriber* _angleSub{nullptr};
    IDataPublisher* _forcePub{nullptr};

    // Latest angle received from the plant
    double _angle{0.0};
    bool _angleReceived{false};
	
	std::chrono::nanoseconds _lastControllerUpdate{0};

    // PID Parameters
    double _kp = 0.4;
    double _ki = 0.3;
    double _kd = 0.8;

    // Desired angle [rad]
    double _setpoint = 0.0;

    // Saturation limits
    double _outputMin = -100.0;
    double _outputMax = 100.0;

    // PID State
    double _integral = 0.0;
    double _previousError = 0.0;

    bool _firstUpdate = true;

    std::chrono::nanoseconds _previousTimestamp{0};

    void AddCommandLineArgs() override
    {
    }

    void EvaluateCommandLineArgs() override
    {
    }

    void CreateControllers() override
    {
        // Force publisher
        _forcePub =
            GetParticipant()->CreateDataPublisher(
                "ForcePublisher",
                PubSubDemoCommon::MakeForceSpec(),
                0);

        GetLogger()->Info("Creating AngleSubscriber");

        // Angle subscriber
        _angleSub =
            GetParticipant()->CreateDataSubscriber(
                "AngleSubscriber",
                PubSubDemoCommon::MakeAngleSpec(),
                [this](IDataSubscriber*, const DataMessageEvent& event)
                {
                    // Only receive and store the latest angle.
                    // PID calculation is NOT done here.
                    _angle =
                        PubSubDemoCommon::DeserializeDouble(
                            SilKit::Util::ToStdVector(event.data));

                    _angleReceived = true;
                });
    }

    void InitControllers() override
    {
        _angle = 0.0;
        _angleReceived = false;

        _integral = 0.0;
        _previousError = 0.0;

        _firstUpdate = true;
        _previousTimestamp = std::chrono::nanoseconds{0};
    }

    void DoWorkSync(std::chrono::nanoseconds now) override
    {


        double error = (_setpoint - _angle);

        double dt = 0.0;

        if (!_firstUpdate)
        {
            dt = std::chrono::duration<double>(now - _previousTimestamp).count();
        }

        _previousTimestamp = now;


        if (dt > 0.0)
        {
            _integral += error * dt;
        }

        double derivative = 0.0;

        if (!_firstUpdate && dt > 0.0)
        {
            derivative = (error - _previousError) / dt;
        }


        double force = (_kp * error) + (_ki * _integral) + (_kd * derivative);


        if (force > _outputMax)
        {
            force = _outputMax;
        }
        else if (force < _outputMin)
        {
            force = _outputMin;
        }

        _previousError = error;
        _firstUpdate = false;

        GetLogger()->Info(
            "Time=" + std::to_string( std::chrono::duration<double>(now).count()) +
            " Angle=" + std::to_string(_angle) +
            " Error=" + std::to_string(error) +
            " Integral=" + std::to_string(_integral) +
            " Derivative=" + std::to_string(derivative) +
            " Force=" + std::to_string(force));


        _forcePub->Publish(
            PubSubDemoCommon::SerializeDouble(force));
    }

    void DoWorkAsync() override
    {
        // Nothing needed here.
    }
};


int main(int argc, char** argv)
{
    Arguments args;

    args.participantName = "Controller";

    Controller app(args);

    app.SetupCommandLineArgs(
        argc,
        argv,
        "Pendulum PID Controller");

    return app.Run();
}