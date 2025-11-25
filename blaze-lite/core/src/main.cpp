#include <Arduino.h>
#include <iostream>
#include <cstring>
#include "MS5611.h"
#include <Wire.h>
#include <Statistic.h>
#include <time.h>
#include <chrono>

using namespace std;

const long baudrate = 115200;

const float TIME_INCREMENT = 0.1;

time_t START_TIME = chrono::system_clock::to_time_t(chrono::system_clock::now());

MS5611 MS5611(0x77);

statistic::Statistic<float, u_int32_t, true> stats(MS5611.read());

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

    stats.clear();

    Serial.println();
    Serial.println("Test Statistics for MS5611");
    Serial.println("Celsius\tmBar\tPascal");
    Serial.print(MS5611.getTemperature());
    Serial.print("\t");
    Serial.print(MS5611.getPressure());
    Serial.print("\t");
    Serial.println(MS5611.getPressurePascal());

    /* Next step here: Start a timer to log time intervals (since launch or at least
     * when the barometer kickstarts)
     **/
    
}

/**
 * Approximates inst. acceleration by calculating 2nd derivative
 * using the 3-point central difference.
 * Later implementation will be with Kalman Filter.
 * @param altitudes List of altitudes at t-h, t, t+h (fixed size of 3)
 * @param dt Time difference in seconds
 * @return Acceleration in m/s²
 */
float calculateAcceleration(vector<float> altitudes, float dt) {
    float h1 = altitudes[0];
    float h2 = altitudes[1];
    float h3 = altitudes[2];
    return (h3 - 2 * h2 + h1) / (dt * dt); 
}



void loop()
{
    Wire.begin();
    MS5611.read();           //  note no error checking => "optimistic".
    
    Serial.println("Temperature (°C)\tPressure (mBar)\t Altitude (m)");
    Serial.print(MS5611.getTemperature(), 2);
    Serial.print('\t');
    Serial.print(MS5611.getPressure(), 2);
    Serial.print('\t');
    Serial.print(MS5611.getAltitude(), 2);
    Serial.println();
    accelerationStateChangeUpdate();
}

void accelerationStateChangeUpdate(){
    /** 
     * New Idea: Collect 3 altitude readings (over 0.1s intervals)
     * and use 2nd derivative from the graph to estimate the acceleration.
     * For Rockets, note: Fnet = (dv/dt)*m + v*(dm/dt)
     *  - This means we get an acceleration reading every ~0.3s
     * Kickstart a running average. Using the z-score, determine if a 
     * new acceleration reading is an outlier. 
     *  - If it is, reset the running average and "notify" that there's been
     * a significant shift in acceleration (for now using Serial.print(),
     * but implement a callback function at a later stage)
     *  - If it isn't, continue updating the running acceleration average.
     * 
    */
   int sampleSize = 3;
    vector<float> altitudes(sampleSize);
    for (int i = 0; i < sampleSize; i++) {
        MS5611.read();
        altitudes.push_back(MS5611.getAltitude());
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