
#include <Arduino.h>
#include <SPI.h>
#include <driver_ms5611.h>

class Baro {
    public:
        //constructor
        // Baro(uint8_t csPin);
        Baro(ms5611_interface_t interface, 	uint8_t addr_pin);
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
        float getTemperature() const {
            return temperature;
        }
        float getPressure() const {
            return seaLevelPressure;
        }
        float getPressurePascal();
        float getBaroAltitude(float seaLevelPressure);


    private:
        ms5611_handle_t baroObject;
        ms5611_interface_t interface;
        uint8_t CS_PIN; //changed from uint8
        float seaLevelPressure; // in mbar
        float temperature;
        float altitude;
        
};