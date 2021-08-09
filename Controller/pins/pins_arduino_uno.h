#pragma once

/**
 *  Arduino Uno pin assignments
 */

#if !defined(__AVR_ATmega328P__) || !defined(__AVR_ATmega168__) 
#error "Select Arduino Uno in 'Tools -> Board'."
#endif

//
// Buttons
//
#define ENTER_BUTTON_PIN                      13
#define DOWN_BUTTON_PIN                       12
#define UP_BUTTON_PIN                         11

//
// Pumps
//
#define NUTRIENTS_PUMP_SPEED_PIN              A0  // Analog Output
#define WATER_PUMP_SPEED_PIN                  A1  // Analog Output

//
// Sensors
//
#define TEMPERATURE_SENSOR_PIN                A2  // Analog Input

//
// Heater
//
#define WATER_HEATER_PIN                      10  

//
// Light
//
#define LIGHT_STATE_PIN                       9

//
// Display
//
#define E0_DISPLAY_PIN                        8
#define E1_DISPLAY_PIN                        7
#define E2_DISPLAY_PIN                        6
#define E3_DISPLAY_PIN                        5

#define LATCH_DISPLAY_PIN                     4
#define DATA_DISPLAY_PIN                      3
#define CLK_DISPLAY_PIN                       2

//
// Wifi
//
#define TX_PIN                                1 
#define RX_PIN                                0