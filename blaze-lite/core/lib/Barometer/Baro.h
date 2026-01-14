
#include <Arduino.h>
#include <SPI.h>
#include <MS5611_SPI.h>

class Baro {
    public:
        //constructor
        Baro(uint8_t csPin);
        //destructor
        ~Baro();

        bool startUp();

        //read functions
        int read();
        uint32_t getDeviceID();
        uint16_t getProm(uint8_t index);
        uint16_t getCRC();

        //configuration functions
        void setOverSamplingRate(uint8_t osr);

        void setPressureOffset(float offset);

        //get methods
        float getSeaLevelPressure();

        //set methods
        void setSeaLevelPressure(float pressure);

        //sensor reading functions
        float getTemperature();
        float getPressure();
        float getPressurePascal();
        float getBaroAltitude(float seaLevelPressure);


    private:
        MS5611_SPI baroObject;
        uint8_t CS_PIN;
        float seaLevelPressure; // in mbar
};