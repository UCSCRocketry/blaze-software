#include <Arduino.h>
#include <iostream>
#include <cstring>
#include "MS5611.h"
#include <Wire.h>
#include <Statistic.h>
#include <time.h>
#include <chrono>
#include "accelerationState.h"
// #include "telemetry_logger.h"
// extern TelemetryLogger telemetryLogger;

using namespace std;

const long baudrate = 115200;

const float TIME_INCREMENT = 0.1;

time_t START_TIME = chrono::system_clock::to_time_t(chrono::system_clock::now());

// MS5611 MS5611(0x77);
MS5611 baroObject(0x77);

statistic::Statistic<float, u_int32_t, true> stats(baroObject.read());

float calculateAcceleration(vector<float> altitudes, float dt) {
    float h1 = altitudes[0];
    float h2 = altitudes[1];
    float h3 = altitudes[2];
    return (h3 - 2 * h2 + h1) / (dt * dt); 
}

void accelerationStateChangeUpdate(){
   int sampleSize = 3;
    vector<float> altitudes(sampleSize);
    for (int i = 0; i < sampleSize; i++) {
        baroObject.read();
        altitudes.push_back(baroObject.getAltitude());
        delay(100); //0.1s
    }
    float accel = calculateAcceleration(altitudes, TIME_INCREMENT);

    // Next step here: Time-plot of ALL acceleration values
    
    Serial.print("Acceleration (m/s²):");
    Serial.println(accel);

    stats.add(accel);
    float z_accel = (accel - stats.average()) / stats.pop_stdev();

    if (fabs(z_accel) >= 2.0) {
        Serial.print("Significant acceleration change 𝜟: From ");
        Serial.print(stats.middle());
        Serial.print("m/s² to ");
        Serial.print(accel);
        Serial.println("m/s²");

        stats.clear(); // reset to baseline - don't want to do calculations based on old state
        stats.add(accel); // new baseline
    }
    
}
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

    

    if (baroObject.begin() == true)
    {
    Serial.print("baroObject found: ");
    Serial.println(baroObject.getAddress());
    }
    else
    {
    Serial.println("baroObject not found. halt.");
    // while (1);
    }

    stats.clear();

    Serial.println();
    Serial.println("Test Statistics for baroObject");
    Serial.println("Celsius\tmBar\tPascal");
    Serial.print(baroObject.getTemperature());
    Serial.print("\t");
    Serial.print(baroObject.getPressure());
    Serial.print("\t");
    Serial.println(baroObject.getPressurePascal());

    /* Next step here: Start a timer to log time intervals (since launch or at least
     * when the barometer kickstarts)
     **/
    
}

void loop()
{
    Wire.begin();
    baroObject.read();           //  note no error checking => "optimistic".
    
    Serial.println("Temperature (°C)\tPressure (mBar)\t Altitude (m)");
    Serial.print(baroObject.getTemperature(), 2);
    Serial.print('\t');
    Serial.print(baroObject.getPressure(), 2);
    Serial.print('\t');
    Serial.print(baroObject.getAltitude(), 2);
    Serial.println();
    accelerationStateChangeUpdate();
}

