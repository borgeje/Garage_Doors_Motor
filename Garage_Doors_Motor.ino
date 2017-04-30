/**
 *******************************
 *
 * REVISION HISTORY
 * Version 1.2 - Joao E Borges
 * 
 * DESCRIPTION
 *    Garage Sensor, planning this to be the single board in the garage 
 *    (need to see how wiring will be)
 *    I/O
 *     3x Input Garage Door Switches on D2, D3 and D4
 *     1x Output DC or PWM output (generic) on D5
 *     1x Input  Presence sensor for Lights on D6
 *     2x Input Relay Output on D7 for 1 Garage Door Control on D8 and A0
 *     1x TH sensor DHT22 on pin D7
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
#define ON 0
#define OFF 1
#define DETACH_DELAY 900          // Tune this to let your movement finish before detaching the servo
#define SENSOR_TEMP_OFFSET 0      // Set this offset if the sensor has a permanent small offset to the real temperatures

// Some additional definitons and INSTANCES creation
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgMotion(CHILD_ID_Presence, V_TRIPPED);
MyMessage msgPWM(CHILD_ID_PWM, V_DIMMER);
MyMessage msgDoor1(CHILD_ID_Door_Sensor_1, V_TRIPPED);
MyMessage msgDoor2(CHILD_ID_Door_Sensor_2, V_TRIPPED);
MyMessage msgDoor3(CHILD_ID_Door_Sensor_3, V_TRIPPED);
MyMessage msgMotor1(CHILD_ID_Garage_Motor_1, V_LIGHT);
MyMessage msgMotor2(CHILD_ID_Garage_Motor_2, V_LIGHT);

Bounce Debounce_Door1 = Bounce();  // create instance of debounced button
Bounce Debounce_Door2 = Bounce();  // create instance of debounced button
Bounce Debounce_Door3 = Bounce();  // create instance of debounced button
Bounce Debounce_PRES = Bounce();
DHT dht;                          // Creating instance of DHT

//VARIABLES
int oldDoorValue1=0;
int oldDoorValue2=0;
int oldDoorValue3=0;
int a = 0;                        //intermediate variable for servo backandforth math // Servo moves around 90                       
static const uint64_t UPDATE_INTERVAL = 15000; // Sleep time between sensor updates (in milliseconds) // Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint8_t FORCE_UPDATE_N_READS = 3; // Force sending an update of the temperature after n sensor reads
float lastTemp;                   // variable to hold last read temperature
float lastHum;                    // variable to hold last read humidity
uint8_t nNoUpdatesTemp;           // keeping track of # of reqdings 
uint8_t nNoUpdatesHum;
bool metric = true;               // metric or imperial?
bool state;
const uint8_t DoorActivationPeriod = 600; // [ms]
int PWMvar = 100;
bool oldPresence;






void setup()  
{  
  Serial.println("Running Setup");
  // Doors setup input pins and debounce function
  pinMode(Door_Sensor_1,INPUT);                 // Setup Doors as INputs
  digitalWrite(Door_Sensor_1,HIGH);             // Activate internal pull-up
  Debounce_Door1.attach(Door_Sensor_1);           // After setting up the button, setup debouncer
  Debounce_Door1.interval(1);
  pinMode(Door_Sensor_2,INPUT);                 // Setup Doors as INputs
  digitalWrite(Door_Sensor_2,HIGH);             // Activate internal pull-up
  Debounce_Door2.attach(Door_Sensor_2);           // After setting up the button, setup debouncer
  Debounce_Door2.interval(1);
  pinMode(Door_Sensor_3,INPUT);                 // Setup Doors as INputs
  digitalWrite(Door_Sensor_3,HIGH);             // Activate internal pull-up
  Debounce_Door3.attach(Door_Sensor_3);           // After setting up the button, setup debouncer
  Debounce_Door3.interval(1);
  pinMode(Presence_Input, INPUT);
//  digitalWrite(Presence_Input, HIGH);            //disabled internal inputs in version 1.2
//  Debounce_PRES.attach(Presence_Input);          // disabled using debouce function in version 1.2r
//  Debounce_PRES.interval(1);
  
  // Digital outputs pin
  digitalWrite(Garage_Motor_1, OFF);               // Make sure Motor is off at startup
  pinMode(Garage_Motor_1, OUTPUT);                 // Then set Motor pins in output mode
  digitalWrite(Garage_Motor_2, OFF);               // Make sure Motor is off at startup
  pinMode(Garage_Motor_2, OUTPUT);                 // Then set Motor pins in output mode
  digitalWrite(PWM, OFF);                         // Make sure Motor is off at startup
  analogWrite(PWM, 0);
  pinMode(PWM, OUTPUT);                 // Then set Motor pins in output mode
  digitalWrite(A1, ON);
  pinMode(A1, OUTPUT);           // This pin is just to turn an LED to show we have been able to enter SETUP 
  dht.setup(DHT_DATA_PIN); // set data pin of DHT sensos
  wait(2000);
  digitalWrite(A1, OFF);
}







void presentation()  
{
  sendSketchInfo("Joao_Garage_Sensors", "1.2");   // Send the sketch version information to the gateway and Controller
  Serial.println("Presentation function..");
  present(CHILD_ID_TEMP, S_TEMP);         // Registar Temperature to gw
  present(CHILD_ID_HUM, S_HUM);           // Register Humidity to gw
  present(CHILD_ID_Door_Sensor_1, S_DOOR);      // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_Door_Sensor_2, S_DOOR);      // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_Door_Sensor_3, S_DOOR);      // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_PWM, S_DIMMER);              // Register PWM output
  present(CHILD_ID_Presence, S_MOTION);      // Register Motor doors
  present(CHILD_ID_Garage_Motor_1, S_BINARY);      // Register Motor doors
  present(CHILD_ID_Garage_Motor_2, S_BINARY);      // Register Motor doors  
//  metric = getConfig().isMetric;          // get configuration from the controller on UNIT system

}






/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop() 
{

  Serial.print("Main loop at node: ");
  Serial.println(getNodeId());
  Debounce_Door1.update();
  Debounce_Door2.update();
  Debounce_Door3.update();
//  Debounce_PRES.update();

  int value = Debounce_Door1.read();   //Get the update value
  if (value != oldDoorValue1) {
     send(msgDoor1.set(value?true:false), true); // Send new state and request ack back
     Serial.print("Door 1:");
     Serial.println(value);
    }
  oldDoorValue1 = value;

  int value2 = Debounce_Door2.read();   //Get the update value
  if (value2 != oldDoorValue2) {
     send(msgDoor2.set(value2?true:false), true); // Send new state and request ack back
     Serial.print("Door 2:");
     Serial.println(value2);
    }
  oldDoorValue2 = value2;

  int value3 = Debounce_Door3.read();   //Get the update value
  if (value3 != oldDoorValue3) {
     send(msgDoor3.set(value3?true:false), true); // Send new state and request ack back
     Serial.print("Door 3:");
     Serial.println(value3);
    }
  oldDoorValue3 = value3;

 bool Tripped = digitalRead(Presence_Input);   //Get the update value
 // if (value4 != oldPres && value4 == true) {
     send(msgMotion.set((Tripped?"1":"0"))); // Send new state and request ack back
     Serial.print("Motion Detected in Garage: ");
     Serial.println(Tripped);
 //    analogWrite(PWM, PWMvar);
  //  }
  //  else if (value4 != oldPres && value4 == false) {
  //    send(msgMotion.set(false)); // Send new state and request ack back
 //     analogWrite(PWM, 0);
//    } else{};
//    oldPresence = value4;
  ReadTemp(); 
  wait(2000);
} 








