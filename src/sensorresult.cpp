#include "sensorresult.h"

SensorResult::SensorResult(){
    temp = 0.0f;
    hum = 0.0f;
    press = 0.0f;
}

SensorResult::SensorResult(float a, float b, float c){
    temp = a;
    hum = b;
    press = c;
};


String SensorResult::toString(){
    String data = String(temp) + "," + String(hum) + "," + String(press) + "";
    return data;
};

float SensorResult::temperature(){
    return temp;
}

float SensorResult::humidity(){
    return hum;
}

float SensorResult::pressure(){
    return press;
}