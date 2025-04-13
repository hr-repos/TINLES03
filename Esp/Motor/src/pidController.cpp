// pidController.cpp
#include "pidController.h"

// Constructor implementation
PIDController::PIDController(double p, double i, double d, double minOutput, double maxOutput) 
    : Kp(p), Ki(i), Kd(d), setpoint(0), prevInput(0), integral(0),
      outputMin(minOutput), outputMax(maxOutput), output(0), lastTime(millis()) {}

double PIDController::compute(double input) {
    unsigned long now = millis();
    double timeChange = (now - lastTime) / 1000.0;
    
    if(timeChange <= 0.001) {
        output = constrain((Kp * (setpoint - input)), outputMin, outputMax);
        return output;
    }
    
    lastTime = now;

    double error = setpoint - input;
    double Pout = Kp * error;

    integral += error * timeChange;
    integral = constrain(integral, outputMin/Ki, outputMax/Ki);
    double Iout = Ki * integral;

    double derivative = (input - prevInput) / timeChange;
    double Dout = -Kd * derivative;

    output = constrain(Pout + Iout + Dout, outputMin, outputMax);
    prevInput = input;

    return output;
}

void PIDController::setTarget(double target) {
    setpoint = target;
}

void PIDController::setTunings(double p, double i, double d) {
    Kp = p;
    Ki = i;
    Kd = d;
}

void PIDController::reset() {
    integral = 0;
    prevInput = 0;
}