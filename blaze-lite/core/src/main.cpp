#include <Arduino.h>
#include <iostream>
#include <cstring>
#include "MS5611.h"
#include <Wire.h>

using namespace std;

const long baudrate = 115200;

MS5611 MS5611(0x77);

void setup()
{
    Serial.begin(baudrate); // From the arduino library
    while (!Serial)
    {
        Serial.println();
        Serial.println(__FILE__);
        Serial.print("MS5611_LIB_VERSION: ");
        Serial.println(MS5611_LIB_VERSION);
        Serial.println();
        delay(100);
    }
    Serial.println("This is on the BlackPill board");

    Wire.begin();

    if (MS5611.begin() == true)
    {
    Serial.print("MS5611 found: ");
    Serial.println(MS5611.getAddress());
    }
    else
    {
    Serial.println("MS5611 not found. halt.");
    // while (1);
    }

    Serial.println();
    Serial.println("Test Statistics for MS5611");
    Serial.println("Celsius\tmBar\tPascal");
    Serial.println(MS5611.getTemperature().toString() + "\t" + MS5611.getPressure() + "\t" + MS5611.getPressurePascal() + "\n");
}



void loop()
{
    Wire.begin();
    MS5611.read();           //  note no error checking => "optimistic".
    Serial.print(MS5611.getTemperature(), 2);
    Serial.print('\t');
    Serial.print(MS5611.getPressure(), 2);
    Serial.print('\t');
    Serial.print(MS5611.getPressurePascal(), 2);
    Serial.println();
    delay(1000);
    accelerationStateChangeUpdate();
}

void accelerationStateChangeUpdate(){
    // Keep reading the data 
    // change in altitude / time = v_average
    // use kin equation 1: x = 0.5*a*t^2 => a = 2x/t^2

    // measure altitude between 2 time intervals
    while (1)
    {

        // Won't work because acceleration is NOT constant in rocketry

        float altitude1 = MS5611.getAltitude();
        delay(1000);
        float altitude2 = MS5611.getAltitude(); 
        float v_avg = (altitude2 - altitude1) / 1.0;
        float a_avg = 2 * (altitude2 - altitude1) / (1.0 * 1.0); // over 1 sec

        if (!isnan(altitude1) && !isnan(altitude2)) {
            Serial.print("Velocity (m/s): ");
            Serial.print(v_avg, 2);
        }
    }
    
}