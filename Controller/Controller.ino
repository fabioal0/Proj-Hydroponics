// LIBRARIES
  
  // External
    #include <OneWire.h>
    #include <DallasTemperature.h>
  
  // Local
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) 
    #include "pins/pins_arduino_uno.h"
    #endif

// VARIABLES
  
  // Inputs 
    volatile byte potenciometer = 0x00;
    volatile byte btn [3] = {0,0,0};

  // Control
    volatile bool consumed = true;
    byte action = 0x00;
    byte change = 0x00;  

  // Auxiliar
    volatile byte lastIn [3] = {0,0,0};
    volatile byte counter_T2 = 1;
    volatile long last_time = 0;

  // Clocks
    volatile long current_time = 0;

  // Enables
    bool heater_en = false;
    bool light_en = false;
    bool pump_en = false; 
     
  // Outputs
    byte heater_status = 0x00;
    byte light_status = 0x00;
    byte pump_speed = 0x00;

  // Base sensors setup
    OneWire comunication(TEMPERATURE_SENSOR_PIN);   
    DallasTemperature sensors(&comunication);

// CONSTANTS
  

// FUNCTIONS

  // Setup
    void setup() 
    {
      // Start external libraries
      
        Serial.begin(9600);                                                       // Start serial comunication with a baud rate 9600
        sensors.begin();                                                          // Start temperature sensor processing
    
      // Configure pins
      
        pinMode(TEMPERATURE_SENSOR_PIN, INPUT);                                   // Set temp_sensor analogic pin as input
        pinMode(MAIN_POTENCIOMETER_PIN, INPUT);                                   // Set btn1 analogic pin as input
      
        pinMode(ENTER_BUTTON_PIN, INPUT);                                         // Set btn0 digital pin as input
        pinMode(DOWN_BUTTON_PIN, INPUT);                                          // Set btn1 digital pin as input
        pinMode(UP_BUTTON_PIN, INPUT);                                            // Set btn2 digital pin as input
    
        pinMode(WATER_PUMP_SPEED_PIN, OUTPUT);                                    // Set pump_en digital pin as output
    
        pinMode(LIGHT_STATE_PIN, OUTPUT);                                         // Set light digital pin as output
    
        pinMode(WATER_HEATER_STATE_PIN, OUTPUT);                                  // Set heater digital pin as output
       
      // Configure interrupts
      
        cli();                                                                    // Stop interrupts            
      
        // Set timer 1                                   
        TCCR1A = 0;                                                               // Reset TCCR1A register
        TCCR1B = 0;                                                               // Reset TCCR1B register
        TCNT1  = 0;                                                               // Reset timer value
        OCR1A = 25000;                                                            // Set compare register A to 25000 --> (16*10^6) / 64 = 250 khz -> 1 / 250 khz = 4 us -> 100 ms / 4 us =  25000 
        
        TCCR1B |= (1 << WGM12);                                                   // Set mode 4 ( CTC | Top = OCR1A | Update = immediate | Flag on = Max)
        TCCR1B |= (1 << CS10) | (1 << CS11);                                      // Set prescaler 64
        TIMSK1 |= (1 << OCIE1A);                                                  // Enable timer interrupts
      
        // Set timer 2                                   
        TCCR2A = 0;                                                               // Reset TCCR1A register
        TCCR2B = 0;                                                               // Reset TCCR1B register
        TCNT2  = 0;                                                               // Reset timer value
        OCR2A = 250;                                                              // Set compare register A to 250 --> (16*10^6) / 256 = 62.5 khz -> 1 / 62.5 khz = 16 us -> 4 ms / 16 us =  250 
        
        TCCR2B |= (1 << WGM12);                                                   // Set mode 4 ( CTC | Top = OCR1A | Update = immediate | Flag on = Max)
        TCCR2B |= (1 << CS12);                                                    // Set prescaler 256
        TIMSK2 |= (1 << OCIE2A);                                                  // Enable timer interrupts
      
        sei();                                                                    // Enable interrupts
    
      // Initiate variables
      
        analogWrite(WATER_PUMP_SPEED_PIN, pump_speed);                            // Write base pump_speed to pin
        digitalWrite(LIGHT_STATE_PIN, light_status);                              // Write base light_status to pin
        digitalWrite(WATER_HEATER_STATE_PIN, heater_status);                      // Write base heater_status to pin
    }

  // Collect potenciometer and update custom clock
    ISR(TIMER1_COMPA_vect)
    {
      potenciometer = ((byte)(analogRead(MAIN_POTENCIOMETER_PIN) >> 2)) & 0xf0 ;  // Read pin an1 to variable potenciometer, convert to 1 byte and set the value to multiples of 16 
    
      current_time ++;                                                            // Increase time every 100 ms
        
      if(current_time >= 864000)                                                  // Check if the couter reach 24 hours
      {
        current_time = 0;                                                         // Reset current time
        last_time = 0;                                                            // Reset last time
      }
    }

  // Read buttons data
    ISR(TIMER2_COMPA_vect)
    {
      if(counter_T2 == 8)                                                         // Colect buttons data every 16 ms (4 * 4)
      {
        if(consumed)                                                              // Block new inputs if the last inputs weren't consumed
        {
          debounce_button(digitalRead(ENTER_BUTTON_PIN), 0);                      // Read pin btn0 and debounce the value 
          debounce_button(digitalRead(DOWN_BUTTON_PIN), 1);                       // Read pin btn1 and debounce the value 
          debounce_button(digitalRead(UP_BUTTON_PIN), 2);                         // Read pin btn2 and debounce the value 
    
          consumed = (btn[0] | btn[1] | btn[2]) == 0;                             // Set consumed to false if a button was pressed 
        }
        counter_T2 = 1;                                                           // Reset counter_T2 variable
      }
      else
      {
        counter_T2 = counter_T2 << 1;                                             // Count in powers of 2
      }
    }

  // Debounce button
    int debounce_button (int button, int index)
    {
      btn[index] = (button ^ lastIn[index]) & button;                             // Check if the new input frm the button is diferent from the last input from the button and if is 1
      lastIn[index] = button;                                                     // Update last input from the button
    }


  // Print important data to the terminal
    void current_status ()
    {
      Serial.print("Status:");
      Serial.print("\n  Time - ");
      Serial.print(current_time);
      Serial.print("\n  Light:");
      Serial.print(" St - ");
      Serial.print(light_status);
      Serial.print(" En - ");
      Serial.print(light_en);
      Serial.print("\n  Temp - ");
      Serial.print(sensors.getTempCByIndex(0));
      Serial.print("\n  Heater:");
      Serial.print(" St - ");
      Serial.print(heater_status);
      Serial.print(" En - ");
      Serial.print(heater_en);
      Serial.print("\n  Pump:");
      Serial.print(" Sp - ");
      Serial.print(pump_speed);
      Serial.print(" En - ");
      Serial.print(pump_en);
      Serial.print("\n\n");
    }

  // Main program
    void loop() 
    {
      // Collect 
    
        if (last_time + 100 < current_time)                                       // Wait at least 10 seconds
          {
            sensors.requestTemperatures();                                        // Collect the new temperature
            last_time = current_time;                                             // Update last time with new
            action = 1;                                                           // Record the action taken
          }
    
        else if(!consumed)                                                        // Check if a button was pressed
        {
          if((btn[0]) == 1)                                                       // Verify if the button pressed was the 1 
          {
            action = 2;                                                           // Record the action taken
          }
          else if ((btn[1]) == 1)                                                 // Verify if the button pressed was the 2
          {
            action = 3;                                                           // Record the action taken
          }
          else if ((btn[2]) == 1)                                                 // Verify if the button pressed was the 3 
          {
            action = 4;                                                           // Record the action taken
          }
          else
          {
            Serial.print("Error.\n");                                             // Verify if there was an error 
          }
          consumed = true;                                                        // Reset the variable to allow new buttons input
        }    
    
      // Process 
      
        switch(action)                                                            // Execute the action collected
        {
          case 0:                                                                 // Case 0 - No changes
            break;                                                                
          case 1:                                                                 // Case 1 - New twmperature collection
            break;                                                                
          case 2:                                                                 // Case 2 - New pump enable
    
            pump_en = !pump_en;                                                   // Invert pump_en ( true | false )
            change = 0x04;                                                        // Record current change
    
            break;
          case 3:                                                                 // Case 3 - New light enable
    
            light_en = !light_en;                                                 // Invert light_en ( true | false )
            change = 0x05;                                                        // Record current change
    
            break;
          case 4:                                                                 // Case 4 - New heater enable
    
            heater_en = !heater_en;                                               // Invert heater_en ( true | false )
            change = 0x06;                                                        // Record current change
    
            break;
          default:                                                                // Case default - Error
          
            Serial.print("Error.\n");                                             // Verify if there was an error
        }
    
          if(heater_en)                                                           // Verify if heater enable is on
          {
            if (heater_status == 0x00 && sensors.getTempCByIndex(0) < 20)         // Check if the temperature is < 20 and the status is off
            {
              heater_status = 0x01;                                               // Change heater status
              change = 0x01;                                                      // Record current change
            }
            else if (heater_status == 0x01 && sensors.getTempCByIndex(0) > 25)    // Check if the temperature is > 25 and the status is on
            {
              heater_status = 0x00;                                               // Change heater status
              change = 0x01;                                                      // Record current change
            }
          }
          else if (heater_status == 0x01)                                         // Check if the status is on and the enable is off
          {
            heater_status = 0x00;                                                 // Change heater status
            change = 0x01;                                                        // Record current change
          }
    
          if(light_en)                                                            // Verify if light enable is on
          {
            if (current_time >= 288000 && current_time <= 720000)                 // Check if the time is between 8 am and 8 pm
            {
              if (light_status == 0x00)                                           // Check if the status is off
              {
                light_status = 0x01;                                              // Change light status
                change = 0x02;                                                    // Record current change
              }
            }
            else                                                                  // Any other time
            {
              if (light_status == 0x01)                                           // Check if the status is on
              {
                light_status = 0x00;                                              // Change light status
                change = 0x02;                                                    // Record current change
              }
            }
          }
          else if (light_status == 0x01)                                          // Check if the status is on and the enable is off
          {
            light_status = 0x00;                                                  // Change light status
            change = 0x02;                                                        // Record current change
          }
    
          if(pump_en)                                                             // Verify if pump enable is on
          {
            if (potenciometer != pump_speed)                                      // Check if the pump speed changed
            {
              pump_speed = potenciometer;                                         // Set the pump speed to the potenciometer value  
              change = 0x03;                                                      // Record current change
            }
          }
          else if (pump_speed != 0x00)                                            // Check if the pump is not off and if the enable is off
          {   
            pump_speed = 0x00;                                                    // Turn of pump
            change = 0x03;                                                        // Record current change
          }
        
      // Execute 
    
        switch(change)                                                            // Execute the change
        {
          case 1:                                                                 // Case 1 - Change the heater status
          
            digitalWrite(WATER_HEATER_STATE_PIN, heater_status);                  // Update output
    
            break;
          case 2:                                                                 // Case 2 - Change the light status
                
            digitalWrite(LIGHT_STATE_PIN, light_status);                          // Update output
    
            break;
          case 3:                                                                 // Case 3 - Change the pump speed
                
            analogWrite(WATER_PUMP_SPEED_PIN, pump_speed);                        // Update output
    
            break;
        }
    
        if (change != 0x00)                                                       // Check if a change happened
        {
          current_status();                                                       // Show the important data
          change = 0x00;                                                          // Reset change
        }
    
        action = 0x00;                                                            // Reset action
    }
