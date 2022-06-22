
#define temp_sensor A0
#define an1 A1

#define btn0 8
#define btn1 12
#define btn2 13

#define pump_en 5
#define pump_h 2
#define pump_l 4

#define light 3

#define heater 6

// // Include local libraries
// #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) 
// #include "pins/pins_arduino_uno.h"
// #endif

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire comunication(temp_sensor);
DallasTemperature sensors(&comunication);


// Input variables
volatile int potenciometer;
volatile byte btn [3] = {0,0,0};

// Auxiliar variables
volatile byte lastIn [3] = {0,0,0};
volatile long current_time = 0;
volatile byte counter_T2 = 0x1;
volatile bool consumed = true;

// Output variables


void setup() 
{
  Serial.begin(9600);
  sensors.begin();                                    

  // Configure pins
  
    pinMode(temp_sensor, INPUT);                      // Set temp_sensor analogic pin as input
    pinMode(an1, INPUT);                              // Set btn1 analogic pin as input
  
    pinMode(btn0, INPUT);                             // Set btn0 digital pin as input
    pinMode(btn1, INPUT);                             // Set btn1 digital pin as input
    pinMode(btn2, INPUT);                             // Set btn2 digital pin as input

    pinMode(pump_en, OUTPUT);                         // Set pump_en digital pin as output
    pinMode(pump_h, OUTPUT);                          // Set pump_h digital pin as output
    pinMode(pump_l, OUTPUT);                          // Set pump_l digital pin as output

    pinMode(light, OUTPUT);                           // Set light digital pin as output

    pinMode(heater, OUTPUT);                          // Set heater digital pin as output
   
  // Configure interrupts
  
    cli();                                            // Stop interrupts            
  
    // Set timer 1                                   
    TCCR1A = 0;                                       // Reset TCCR1A register
    TCCR1B = 0;                                       // Reset TCCR1B register
    TCNT1  = 0;                                       // Reset timer value
    OCR1A = 25000;                                    // Set compare register A to 25000 --> (16*10^6) / 64 = 250 khz -> 1 / 250 khz = 4 us -> 100 ms / 4 us =  25000 
    
    TCCR1B |= (1 << WGM12);                           // Set mode 4 ( CTC | Top = OCR1A | Update = immediate | Flag on = Max)
    TCCR1B |= (1 << CS10) | (1 << CS11);              // Set prescaler 64
    TIMSK1 |= (1 << OCIE1A);                          // Enable timer interrupts
  
    // Set timer 2                                   
    TCCR2A = 0;                                       // Reset TCCR1A register
    TCCR2B = 0;                                       // Reset TCCR1B register
    TCNT2  = 0;                                       // Reset timer value
    OCR2A = 250;                                      // Set compare register A to 250 --> (16*10^6) / 256 = 62.5 khz -> 1 / 62.5 khz = 16 us -> 4 ms / 16 us =  250 
    
    TCCR2B |= (1 << WGM12);                           // Set mode 4 ( CTC | Top = OCR1A | Update = immediate | Flag on = Max)
    TCCR2B |= (1 << CS12);                            // Set prescaler 256
    TIMSK2 |= (1 << OCIE2A);                          // Enable timer interrupts
  
    sei();                                            // Enable interrupts

  // Initiate variables

    digitalWrite(pump_h,HIGH);
    digitalWrite(pump_l,LOW);
    analogWrite(pump_en, 180);
  
    digitalWrite(light,HIGH);
  
    digitalWrite(heater,HIGH);

}


ISR(TIMER1_COMPA_vect)
{
  potenciometer = analogRead(an1);                    // Read pin an1 to variable potenciometer

  if (current_time >> 5 & 0x1 == 0x1)                 
  {  
    sensors.requestTemperatures();                    // Read pin an0 to variable temp_sensor
  }
  
  current_time ++;                                    // Increase time in 1 unit each 10 ms
  
  if(current_time == 864000)                          // Check if the couter reach 24 hours
  {
    current_time = 0;                                 // Reset current time
  }
}


ISR(TIMER2_COMPA_vect)
{
  if(counter_T2 == 0x8)                                // Colect buttons data every 16 ms (4 * 2^(4 - 1))
  {
    if(consumed)                                      // Block new inputs if the last inputs weren't consumed
    {
      debounce_button(digitalRead(btn0), 0);          // Read pin btn0 and debounce the value 
      debounce_button(digitalRead(btn1), 1);          // Read pin btn1 and debounce the value 
      debounce_button(digitalRead(btn2), 2);          // Read pin btn2 and debounce the value 

      consumed = false;                               // Update consumed flag
    }
    counter_T2 = 0x1;                                 // Reset counter_T2 variable
  }
  else
  {
    counter_T2 = counter_T2 << 1;                     // Count in powers of 2
  }
}


int debounce_button (int button, int index)
{
  btn[index] = (button ^ lastIn[0]) & button;         // Check if the new input frm the button is diferent from the last input from the button and if is 1
  lastIn[index] = button;                             // Update last input from the button
}


void loop() 
{
  
  if(!consumed)
  {
    if((btn[0] | btn[1] | btn[2]) == 1)
    {
    Serial.print(btn[0]);
    Serial.print("\n");
    Serial.print(btn[1]);
    Serial.print("\n");
    Serial.print(btn[2]);
    Serial.print("\n\n");
    

    consumed= true;
    }   
        Serial.print("\n");

  Serial.print(sensors.getTempCByIndex(0));
      Serial.print("\n");

  }
}
