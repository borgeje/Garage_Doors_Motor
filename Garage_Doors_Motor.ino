/**
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Joao E Borges
 * 
 * DESCRIPTION
 *    Garage Sensor, planning this to be the single board in the garage 
 *    (need to see how wiring will be)
 *    I/O
 *     3x Input Garage Door Switches on D2, D3 and D4
 *     1x Output DC or PWM output (generic) on D5
 *     1x Input  Presence sensor for Lights on D6
 *     2x Input Relay Output on D7 for 1 Garage Door Control on D8 and A0
 *     1x TH sensor DHT22 on pin D6
 *     Radio normally connected as per Mysensors.org
 *     
 *     Optionally (not implemented right away here) an LCD on A4 and A5
 *     
 *     
 *    
 ********************
 *
 *     The MySensors Armduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
************************************************************
 */ 

// Basic Configuration - alwyas keep
#define MY_DEBUG                  // Enable debug prints to serial monitor
#define MY_RADIO_NRF24            // Enable and select radio type attached
#define MY_REPEATER_FEATURE       // Enabled repeater feature for this node


#ifndef MY_RF24_PA_LEVEL
 #define MY_RF24_PA_LEVEL RF24_PA_HIGH
#endif

//Libraries, some are project specific
#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>
#include <Servo.h> 
#include <DHT.h>                  // library for temp and humidity sensor


// Project Pins
#define Door_Sensor_1 2           // Doors on D2, D3 and D4
#define Door_Sensor_2 3
#define Door_Sensor_3 4
#define PWM 5
#define Presence_Input 6
#define DHT_DATA_PIN 7            // Set this to the pin you connected the DHT's data pin to
#define Garage_Motor_1 8
#define Garage_Motor_2 A0

// Node Childs (all individual iDs for each IO)
#define CHILD_ID_TEMP 1            // iD for temperature reporting
#define CHILD_ID_HUM 2             // iD for humidity reporting
#define CHILD_ID_Presence 3
#define CHILD_ID_PWM 4
#define CHILD_ID_Door_Sensor_1 5
#define CHILD_ID_Door_Sensor_2 6
#define CHILD_ID_Door_Sensor_3 7
#define CHILD_ID_Garage_Motor_1 8
#define CHILD_ID_Garage_Motor_2 9

// Other Defines
#define ON 1
#define OFF 0
#define DETACH_DELAY 900          // Tune this to let your movement finish before detaching the servo
#define SENSOR_TEMP_OFFSET 0      // Set this offset if the sensor has a permanent small offset to the real temperatures

// Some additional definitons and INSTANCES creation
MyMessage msgServo(CHILD_ID_Servo, V_LIGHT);
MyMessage msgServo_Dur(CHILD_ID_Servo, V_DIMMER);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
Servo Servo_Dispenser;            // create servo object to control a servo   
Bounce Local_dispense_button = Bounce();  // create instance of debounced button
DHT dht;                          // Creating instance of DHT

//VARIABLES
const int analogInPin = A0
int oldValue=0;
int a = 0;                        //intermediate variable for servo backandforth math // Servo moves around 90                       
// Sleep time between sensor updates (in milliseconds) // Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t UPDATE_INTERVAL = 15000;
// Force sending an update of the temperature after n sensor reads
// i.e. the sensor would force sending an update every UPDATE_INTERVAL*FORCE_UPDATE_N_READS [ms]
static const uint8_t FORCE_UPDATE_N_READS = 10;
float lastTemp;                   // variable to hold last read temperature
float lastHum;                    // variable to hold last read humidity
uint8_t nNoUpdatesTemp;           // keeping track of # of reqdings 
uint8_t nNoUpdatesHum;
bool metric = true;               // metric or imperial?
int ServoSpeed = 2000;
bool state;







void setup()  
{  
  Serial.println("Running Setup");
  pinMode(BUTTON_PIN,INPUT);                 // Setup the button
  digitalWrite(BUTTON_PIN,HIGH);             // Activate internal pull-up
  Local_dispense_button.attach(BUTTON_PIN);  // After setting up the button, setup debouncer
  Local_dispense_button.interval(5);
  digitalWrite(LED_PIN, OFF);               // Make sure LED is off at startup
  pinMode(LED_PIN, OUTPUT);                 // Then set LED pins in output mode

 // NOT USED CODE - Set relay to last known state (using eeprom storage) 
 // state = loadState(CHILD_ID_Servo);
 // digitalWrite(LED_PIN, state?RELAY_ON:RELAY_OFF);

  Servo_Dispenser.attach(SERVO_DIGITAL_PIN);  // attaches the servo on pin  to the servo object 
  Servo_Dispenser.detach();
  dht.setup(DHT_DATA_PIN); // set data pin of DHT sensos
  wait(1500);
}







void presentation()  
{
  sendSketchInfo("Joao_Feeder", "1.0");   // Send the sketch version information to the gateway and Controller
  Serial.println("Presentation function..");
  present(CHILD_ID_Servo, S_DIMMER);      // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_TEMP, S_TEMP);         // Registar Temperature to gw
  present(CHILD_ID_HUM, S_HUM);           // Register Humidity to gw
  metric = getConfig().isMetric;          // get configuration from the controller on UNIT system
}




/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop() 
{

  Local_dispense_button.update();
  Serial.println("Main loop");
  int value = Local_dispense_button.read();   //Get the update value
  if (value != oldValue && value==0) {
     send(msgServo.set(value?false:true), true); // Send new state and request ack back
     Serial.println("Button Pressed - Servo");
    }
  oldValue = value;

  ReadTemp_n_Humidity;
  
  wait(10000);
 //sleep(UPDATE_INTERVAL);  // Sleep for a while to save energy
 // sleep(BUTTON_PIN, CHANGE, UPDATE_INTERVAL);

} 
 

