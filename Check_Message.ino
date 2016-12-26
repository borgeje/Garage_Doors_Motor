
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }

  if (message.type == V_LIGHT) {
       state = message.getBool();      // Change relay state
       Servo_Dispenser.attach(SERVO_DIGITAL_PIN);
       Servo_Dispenser.write(Serv);
       Serial.println("Dispensando Local");
       digitalWrite(LED_PIN, HIGH);
       delay(2000);
       a = 90-Serv;
       Servo_Dispenser.write(a+90);
       digitalWrite(LED_PIN, LOW);
       delay(2000);
       Servo_Dispenser.write(90);
       
//     digitalWrite(LED_PIN, state?RELAY_ON:RELAY_OFF);
//     saveState(CHILD_ID_Servo, state);  // Store state in eeprom
    
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
   else if (message.type == V_DIMMER) {
        int Dimme = message.data;
        ServoSpeed = (Dimme/5*100)+1000;
        }
        else {};
   Servo_Dispenser.detach();

}
