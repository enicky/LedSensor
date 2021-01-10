#include "sensorservice.h"
#include "sensorresult.h"
#include <Arduino.h>
SensorService::SensorService()
{
    settings = BME280I2C::Settings(BME280::OSR_X1, BME280::OSR_X1, BME280::OSR_X1, BME280::Mode_Forced,
                                   BME280::StandbyTime_1000ms, BME280::Filter_Off, BME280::SpiEnable_False,
                                   BME280I2C::I2CAddr_0x76);
    bme = BME280I2C();
}

bool SensorService::bmeNotFound(void)
{
    while (!bme.begin())
    {
        Serial.println("BME280 not found ...");
        return false;
    }
    started = true;
    return started;
}

SensorResult SensorService::readBmeValues(void)
{
    float temp(NAN), hum(NAN), pres(NAN);
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_hPa);

    bme.read(pres, temp, hum, tempUnit, presUnit);  
    SensorResult sensorResult(temp, hum, pres);
    return sensorResult;
   // String data = String(temp) + "," + String(hum) + "," + String(pres) + "";
}
