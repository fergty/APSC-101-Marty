#include <AFMotor.h>
#include <Wire.h>

// Motor shield outputs (1–4 only)
AF_DCMotor Dpump(2);      // M2
AF_DCMotor Ppump(1);      // M1
AF_DCMotor Impmotor(4);   // M4
AF_DCMotor Pressmotor(3); // M3

// Relay on digital pin 8
const byte RELAY_PIN = 53;  // Relay input pin (You can choose any in your design)

void setup() {
  Serial.begin(9600);
  Serial.println("Hi! I'm MARTY, hope you're thirsty");

  //======DIAPHRAGM PUMP=====//
  Dpump.setSpeed(255);
  Dpump.run(FORWARD);
  delay(10000);
  Dpump.run(RELEASE);

  delay(2000);
  //======PERI PUMP=======//
  Ppump.setSpeed(255);
  Ppump.run(FORWARD);
  Serial.println("Peri Pump Pumping!");
  delay(5000);
  Ppump.run(RELEASE);

  delay(2000);
  //=====IMP MOTOR FAST=====//
  Impmotor.setSpeed(180);
  Impmotor.run(FORWARD);
  Serial.println("Starting IMP motor FAST.");
  delay(10000);

  //=====IMP MOTOR SLOW=====//
  Impmotor.setSpeed(100);
  Serial.println("Starting IMP motor SLOW.");
  delay(10000);
  Impmotor.run(RELEASE);

  delay(2000);
  //=====FRENCH PRESS=====//
  Serial.println("Starting Press motor.");
  Pressmotor.setSpeed(180);
  Pressmotor.run(FORWARD);
  delay(20000);
  Pressmotor.run(RELEASE);

  delay(2000);
  //=====HORIZONTAL SUB PUMP (relay)=====//
  //digitalWrite(HSubpumpRelayPin, HIGH);  // or LOW, depending on relay module
  //delay(8000);
  //digitalWrite(HSubpumpRelayPin, LOW);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // set relay OFF (assuming active-LOW) LOW = OFF, HIGH = ON

  // Turn relay ON
  digitalWrite(RELAY_PIN, HIGH);
  delay(8000);                   // 8 seconds
  // Turn relay OFF
  digitalWrite(RELAY_PIN, LOW);  // active-LOW relay


  Serial.println("All done! Drink up :)");
}

void loop() {
  // nothing here; everything runs once in setup()
}
