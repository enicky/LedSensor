#ifndef SENSORRESULT_H
#define SENSORRESULT_H

#include <Arduino.h>

class SensorResult{
    private : 
        float temp;
        float hum;
        float press;
    public:
        SensorResult();
        SensorResult(float temp, float hum, float press);
        String toString();
        float humidity();
        float temperature();
        float pressure();
};


#endif