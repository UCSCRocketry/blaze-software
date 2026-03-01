#include "AccelerationState.h"
#include <math.h>
#include <Statistic.h>



float AccelerationState::calculateAcceleration(vector<float> altitudes, float dt) {
    float h1 = altitudes[0];
    float h2 = altitudes[1];
    float h3 = altitudes[2];
    return (h3 - 2 * h2 + h1) / (dt * dt); 
}

void AccelerationState::accelerationStateChangeUpdate(){
   int sampleSize = 3;
    vector<float> altitudes(sampleSize);
    for (int i = 0; i < sampleSize; i++) {
        baroObject.read();
        altitudes.push_back(baroObject.getBaroAltitude());
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