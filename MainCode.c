// SafetyCage Code
//Troy Williams
//12-3-2018
//Include Necessary Libaries
#include <Controllino.h>

//Define Necessary Variables
volatile long CurrentEncoder = 0;
long CurrentStateLength;
long PreviousEncoderCheck;
int CPSStatus;
int OPSStatus;
int HPSStatus;
int SetupSpeed = 400;
int Left = LOW;
int Right = HIGH;
int PulseWidthMicros = 10;
long StepCount;
long CurrentTime;
long PreviousEncoder;
int OperationSpeed = 150;
long CurrentPosition;
long PreviousEncoderTimeCheck = 0;
long PreviousStepTime;
int SSStatus;
int LastSSStatus;
long LastDebounceTime;
int DebounceDelay = 50;
//Define Necessary Pins

//Motor Setup
int MotorDirectionPin = CONTROLLINO_DO5;
int MotorStepPin = CONTROLLINO_DO4;
int MotorEnablePin = CONTROLLINO_DO6;

//Proximity Switches
int ClosedProximitySensor = CONTROLLINO_AI0;
int OpenedProximitySensor = CONTROLLINO_AI1;
int HandleProximitySensor = CONTROLLINO_AI2;
int SetupSwitch = CONTROLLINO_AI3;

//Encoder Setup
// Red - 24V // Black - GND
const int encoder_a = CONTROLLINO_IN0;// Green - pin IN0 - Digital 4.7K PULLUP RESISTORS
const int encoder_b = CONTROLLINO_IN1; // White - pin IN1 - Digital 4.7K PULLL UP RESISTORS

void setup() {
  Serial.begin(9600);
  pinMode(MotorDirectionPin, OUTPUT);
  pinMode(MotorStepPin, OUTPUT);
  pinMode(MotorEnablePin, OUTPUT);
  pinMode(ClosedProximitySensor, INPUT);
  pinMode(OpenedProximitySensor, INPUT);
  pinMode(HandleProximitySensor, INPUT);
  pinMode(SetupSwitch, INPUT);
  pinMode(encoder_a, INPUT);
  pinMode(encoder_b, INPUT);
  attachInterrupt(digitalPinToInterrupt(encoder_a), encoderPinChangeA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder_b), encoderPinChangeB, CHANGE);
  digitalWrite(MotorEnablePin, HIGH);
  SetupDistances();


}
//WHEN IT TURNS ON, CLOSE THE SHIELD.  AFTER HITTING SENSOR, BEGIN OPENING SLOWLY TO ADJUSTED POSTIION WHILE COUNTING

void loop() {

  CurrentTime = micros();
  SSStatus = digitalRead(SetupSwitch);
  if (SSStatus == HIGH) {
    delay(30);
    while (SSStatus == HIGH) {
      SSStatus = digitalRead(SetupSwitch);
      delay(30);
      Serial.println("HI2");
    }
    SetupDistances();
    PreviousEncoder = CurrentEncoder;
  }

  if (abs(CurrentTime - PreviousEncoderTimeCheck) >= 10000) {
    if ((CurrentEncoder - PreviousEncoder) > 10) {
      MoveGuard(OperationSpeed, Left);
      Serial.println("Closing");
    }
    if ((PreviousEncoder - CurrentEncoder) > 10 ) {
      MoveGuard(OperationSpeed, Right);
      Serial.println("Opening");
    }
    PreviousEncoder = CurrentEncoder;
    PreviousEncoderTimeCheck = micros();
  }
  HPSStatus = digitalRead(HandleProximitySensor);
  if (HPSStatus == HIGH) {
    while (HPSStatus == HIGH) {
      MoveGuard(SetupSpeed, Left);
      HPSStatus = digitalRead(HandleProximitySensor);
    }
    SetupDistances();
  }
}
/*
  void DebounceSwitch(int Status) {
  if (Status != LastSSStatus) {
    LastDebounceTime = millis();
  }
  if ((millis() - LastDebounceTime) > DebounceDelay) {
    if (Status != LastSSStatus) {
      LastSSStatus = Status;
    }
  }
  }

*/

void SetupDistances() {
  CPSStatus = digitalRead(ClosedProximitySensor);
  Serial.println("SetupDistances");

  while (CPSStatus == HIGH) {
    CurrentTime = micros();
    TakeStep(SetupSpeed, Left);
    CPSStatus = digitalRead(ClosedProximitySensor);
  }
  OPSStatus = digitalRead(OpenedProximitySensor);
  HPSStatus = digitalRead(HandleProximitySensor);
  while (HPSStatus == HIGH) {
    HPSStatus = digitalRead(HandleProximitySensor);
  }
  StepCount = 0;
  while (OPSStatus == HIGH) {
    CurrentTime = micros();
    TakeStep(SetupSpeed, Right);
    OPSStatus = digitalRead(OpenedProximitySensor);
  }
  CurrentStateLength = StepCount;
  CurrentPosition = 0;
}


void MoveGuard(int MotorDelay, int Direction) {
  digitalWrite(MotorDirectionPin, Direction);
  delay(10);
  Serial.println("MoveGuard");
  if (Direction == LOW) {
    Serial.println("MoveGuardLeft");
    while (CurrentPosition < CurrentStateLength) {
      digitalWrite(MotorStepPin, HIGH);
      delayMicroseconds(PulseWidthMicros); //Send StepSignal
      digitalWrite(MotorStepPin, LOW);
      delayMicroseconds(MotorDelay);
      CurrentPosition = CurrentPosition + 1;
    }
  }
  if (Direction == HIGH) {
    Serial.println("MoveGuardRight");
    while (CurrentPosition > 0) {
      digitalWrite(MotorStepPin, HIGH);
      delayMicroseconds(PulseWidthMicros); //Send StepSignal
      digitalWrite(MotorStepPin, LOW);
      delayMicroseconds(MotorDelay);
      CurrentPosition = CurrentPosition - 1;
    }
  }
}


void TakeStep(int MotorDelay, int Direction) {
  digitalWrite(MotorDirectionPin, Direction);
  if (abs(CurrentTime - PreviousStepTime) >= MotorDelay) {
    digitalWrite(MotorStepPin, HIGH);
    delayMicroseconds(PulseWidthMicros); //Send StepSignal
    digitalWrite(MotorStepPin, LOW);
    PreviousStepTime = micros();
    StepCount = StepCount + 1;
  }
}
// GOOD STUFF BELOW

void encoderPinChangeA() {
  CurrentEncoder += digitalRead(encoder_a) == digitalRead(encoder_b) ? 1 : -1;

}

void encoderPinChangeB() {
  CurrentEncoder += digitalRead(encoder_a) != digitalRead(encoder_b) ? 1 : -1;
}
