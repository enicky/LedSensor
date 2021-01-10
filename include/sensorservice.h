#ifndef SENSORSERVICE_H
#define SENSORSERVICE_H

#include <BME280I2C.h>
#include "sensorresult.h"

class SensorService
{
private:
    BME280I2C::Settings settings;
    BME280I2C bme;
    bool started = false;

public:
    SensorService();
    SensorResult readBmeValues(void);
    bool bmeNotFound(void);
};

#endif