/*
 * Blaze Lite — STM32F411CEU6 custom PCB
 *
 * Forked from WeAct Black Pill F411CE (pins + peripheral tables). Clock uses
 * HSI+PLL like generic F411CE, not Black Pill HSE (see variant cpp). Differences:
 * - PA0 is not a user button (CS_POWER on Blaze); USER_BTN disabled.
 * - No default USART on PA9/PA10 (RGB LEDs on Blaze). Use USB CDC `Serial` and
 *   add `PIO_FRAMEWORK_ARDUINO_SERIAL_WITHOUT_GENERIC` in platformio.ini, or
 *   instantiate `HardwareSerial` on pins you wire for GPS/modem.
 */
#pragma once

#define PA0                     PIN_A0
#define PA1                     PIN_A1
#define PA2                     PIN_A2
#define PA3                     PIN_A3
#define PA4                     PIN_A4
#define PA5                     PIN_A5
#define PA6                     PIN_A6
#define PA7                     PIN_A7
#define PA8                     8
#define PA9                     9
#define PA10                    10
#define PA11                    11
#define PA12                    12
#define PA13                    13
#define PA14                    14
#define PA15                    15
#define PB0                     PIN_A8
#define PB1                     PIN_A9
#define PB2                     18
#define PB3                     19
#define PB4                     20
#define PB5                     21
#define PB6                     22
#define PB7                     23
#define PB8                     24
#define PB9                     25
#define PB10                    26
#define PB12                    27
#define PB13                    28
#define PB14                    29
#define PB15                    30
#define PC13                    31
#define PC14                    32
#define PC15                    33
#define PH0                     34
#define PH1                     35

#define PA0_ALT1                (PA0  | ALT1)
#define PA1_ALT1                (PA1  | ALT1)
#define PA2_ALT1                (PA2  | ALT1)
#define PA2_ALT2                (PA2  | ALT2)
#define PA3_ALT1                (PA3  | ALT1)
#define PA3_ALT2                (PA3  | ALT2)
#define PA4_ALT1                (PA4  | ALT1)
#define PA7_ALT1                (PA7  | ALT1)
#define PA15_ALT1               (PA15 | ALT1)
#define PB0_ALT1                (PB0  | ALT1)
#define PB1_ALT1                (PB1  | ALT1)
#define PB3_ALT1                (PB3  | ALT1)
#define PB4_ALT1                (PB4  | ALT1)
#define PB5_ALT1                (PB5  | ALT1)
#define PB8_ALT1                (PB8  | ALT1)
#define PB9_ALT1                (PB9  | ALT1)
#define PB12_ALT1               (PB12 | ALT1)
#define PB13_ALT1               (PB13 | ALT1)

#define NUM_DIGITAL_PINS        36
#define NUM_ANALOG_INPUTS       10

#ifndef LED_BUILTIN
  #define LED_BUILTIN           PC13
#endif

/** WeAct-style user LED on PC13: sinks current when pin is LOW (LED on). */
#ifndef LED_BUILTIN_ON
  #define LED_BUILTIN_ON        LOW
#endif
#ifndef LED_BUILTIN_OFF
  #define LED_BUILTIN_OFF       HIGH
#endif

/** RGB on PA8 / PA9 / PA10 — on-level depends on common anode vs cathode. */
#ifndef LED_RGB_R
  #define LED_RGB_R             PA8
#endif
#ifndef LED_RGB_G
  #define LED_RGB_G             PA9
#endif
#ifndef LED_RGB_B
  #define LED_RGB_B             PA10
#endif
#ifndef BLAZE_LED_RGB_ON
  #define BLAZE_LED_RGB_ON      HIGH
#endif
#ifndef BLAZE_LED_RGB_OFF
  #define BLAZE_LED_RGB_OFF     LOW
#endif

#ifndef USER_BTN
  #define USER_BTN              PNUM_NOT_DEFINED
#endif

#ifndef PIN_SPI_SS
  #define PIN_SPI_SS            PA4
#endif
#ifndef PIN_SPI_SS1
  #define PIN_SPI_SS1           PA4
#endif
#ifndef PIN_SPI_SS2
  #define PIN_SPI_SS2           PB12
#endif
#ifndef PIN_SPI_SS3
  #define PIN_SPI_SS3           PA15
#endif
#ifndef PIN_SPI_SS5
  #define PIN_SPI_SS5           PB1
#endif
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI          PA7
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO          PA6
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK           PA5
#endif

#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA          PB7
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL          PB6
#endif

#ifndef TIMER_TONE
  #define TIMER_TONE            TIM10
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO           TIM11
#endif

#ifndef HSE_VALUE
  #define HSE_VALUE             25000000U
#endif

#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif
#endif
