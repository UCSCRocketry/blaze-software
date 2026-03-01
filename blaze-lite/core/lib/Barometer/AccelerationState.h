#include <vector>
#include <Baro.h>

using namespace std;

class AccelerationState
{
private:
    Baro baroObject;
    Statistic accelStats;
public:
    AccelerationState(/* args */);
    ~AccelerationState();
    /**
     * Approximates inst. acceleration by calculating 2nd derivative
     * using the 3-point central difference.
     * @param altitudes List of altitudes at t-h, t, t+h (fixed size of 3)
     * @param dt Time difference in seconds
     * @return Acceleration in m/s²
     */
    float calculateAcceleration(vector<float> altitudes, float dt);

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
    void accelerationStateChangeUpdate();
};

AccelerationState::AccelerationState(/* args */)
{
}

AccelerationState::~AccelerationState()
{
}