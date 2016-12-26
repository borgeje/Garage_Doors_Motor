
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }
  state = message.getBool();      // Change relay state
  if (message.type == V_LIGHT && message.sensor == CHILD_ID_Garage_Motor_1 && state == true) {
       Serial.println("Drive Motor 1");
       digitalWrite(Garage_Motor_1, ON);
       wait(DoorActivationPeriod);
       digitalWrite(Garage_Motor_1, OFF);
       send(msgMotor1.set(false), true);    // After activating the door, move indicator to FALSE again

       // Write some debug info
       Serial.print("Incoming change for sensor:");
       Serial.print(message.sensor);
       Serial.print(", New status: ");
       Serial.println(state);
   } 
   else if (message.type == V_LIGHT && message.sensor == CHILD_ID_Garage_Motor_2 && state == true) {
       Serial.println("Drive Motor 2");
       digitalWrite(Garage_Motor_2, ON);
       wait(DoorActivationPeriod);
       digitalWrite(Garage_Motor_2, OFF);
       send(msgMotor2.set(false), true);    // After activating the door, move indicator to FALSE again
       // Write some debug info
       Serial.print("Incoming change for sensor:");
       Serial.print(message.sensor);
       Serial.print(", New status: ");
       Serial.println(state);
        }
     else if (message.type == V_DIMMER && message.sensor == CHILD_ID_PWM) {
          PWMvar = message.getInt();
           }
        else {};

}
