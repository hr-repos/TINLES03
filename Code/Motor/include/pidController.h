// pidController.h
#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PIDController {
public:
    PIDController(double p, double i, double d, double minOutput, double maxOutput);
    
    double compute(double input);  // Changed from 'update' to 'compute' for consistency
    void setTarget(double target);
    void setTunings(double p, double i, double d);
    void reset();
    
    // Add getter for output
    double getOutput() const { return output; }  // New member function

private:
    double Kp, Ki, Kd;
    double setpoint;
    double prevInput;
    double integral;
    double outputMin, outputMax;
    double output;         // Added to store last output
    unsigned long lastTime;
};

#endif