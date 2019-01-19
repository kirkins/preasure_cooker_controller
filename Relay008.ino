#include <PID_v1.h>
#include <Countimer.h>

#define buzzer         4
#define startBtn       6
#define tempSensor     A0
#define heatDial       A1
#define rco            9
#define redLED         10
#define greenLED       11

int warmupTemp       = 96;// set to 96
int defaultTemp      = 118;// set to 118
int preheatMinutes   = 10;      // 10 minutes
int alarmTime        = 40000;   // 60000 miliseconds (1 minute)
int heatingHours     = 5;       // 5 hours
int processPhase     = 0;
int dutyON           = 0;
int dutyOFF          = 0;
int burnerState      = 0;
double currentTemp, pidOutput, targetTemp;

PID heatingPID(&currentTemp, &pidOutput, &targetTemp, 2, 6, 0, DIRECT);

Countimer preheatTimer;
Countimer mainTimer;
Countimer ON;
Countimer OFF;

void setup() {
  heatingPID.SetMode(AUTOMATIC);
  Serial.begin(9600);
  preheatTimer.setInterval(refreshPreheat, 1000);
  mainTimer.setInterval(refreshMain, 1000);
  ON.setInterval(refreshON, 1000);
  OFF.setInterval(refreshOFF, 1000);
  pinMode(rco, OUTPUT);
}

void loop() {
   preheatTimer.run();
   if(!preheatTimer.isCounterCompleted()) { preheatTimer.start(); }
   mainTimer.run();
   if(!mainTimer.isCounterCompleted()) { mainTimer.start(); }
   ON.run();
   if(!ON.isCounterCompleted()) { ON.start(); }
   OFF.run();
   if(!OFF.isCounterCompleted()) { OFF.start(); }

   currentTemp = getTemp();
    Serial.print("  Phase:");
    Serial.print(processPhase);
    Serial.print("  PotentiometerReading: ");
    Serial.print(analogRead(heatDial));
    Serial.print("  currentTemp:");
    Serial.println(currentTemp);
    
//Check process phase and run associated loop
  switch (processPhase) {
  case 0:
    if (digitalRead(startBtn) == HIGH) {
      startcycle();
    }
    break;
  case 1:
    preHeatStart();
    break;
  case 2:
    preHeat();
    break;
  case 3:
    mainHeatStart();
    break;
  case 4:
    mainHeat();
    break;
  case 5:
    analogWrite(greenLED, 255);
    digitalWrite(rco, LOW);
    digitalWrite(redLED, LOW);
  }
}

void startcycle() {
  tone(buzzer, 1400, 1000);
  processPhase = 1;
}

// Process phases and alarms
void preHeatStart() {
  //Phase 1
  // Before reaching target temperature
  currentTemp = getTemp();
  digitalWrite(rco, HIGH);
  analogWrite(redLED, 255);
  if(currentTemp > warmupTemp) {
   preheatTimer.setCounter(0, preheatMinutes, 10, preheatTimer.COUNT_DOWN, preHeatAlarm);
   processPhase = 2;
  }
}

void preHeat() {
  //Phase 2
  // Start heating to 212 for 10 minutes
  digitalWrite(rco, HIGH);
  analogWrite(redLED, 255);

}

void preHeatAlarm() {
  // preheat done, switch phase and sound alarm
  processPhase = 3;
  tone(buzzer, 2400, alarmTime);
}

void mainHeatStart() {
  //Phase 3
  currentTemp = getTemp();
  // Target should add 0-20 based on heatDial
  //targetTemp = defaultTemp + getHeatDial();    **saved in case debug does not work.  See next line
  //targetTemp = defaultTemp + (((analogRead(heatDial)-1023)*(-1))/1023) * 20;  //analogRead was originally placed in separate function but attempting to debug by placing here. 
  targetTemp = defaultTemp + getHeatDial(); 
  digitalWrite(rco, HIGH);
  analogWrite(redLED, 255);
  if(currentTemp > targetTemp) {
    mainTimer.setCounter(heatingHours, 0, 0, mainTimer.COUNT_DOWN, heatingDone);
    processPhase = 4;
  }
}

void mainHeat() {
  //Phase 4
  // Run heating code add mainTemp with heatDial
  targetTemp = defaultTemp + getHeatDial();
  currentTemp = getTemp();
  heatingPID.Compute();
  digitalWrite(redLED, HIGH);
  Serial.print("  PID output:");
  Serial.print(pidOutput);
  Serial.print("  targetTemp:");
  Serial.print(targetTemp);

switch (burnerState) {
  case 0:
    getDuty();
    ONfunction();
    break;
  case 1:
    ONfunction();
    break;
  case 2:
    OFFfunction();
    break;
}
  
}

void getDuty(){
    double duty = pidOutput/255;
    dutyON = duty*10;
    dutyOFF = (10-dutyON);
  }

  void ONfunction(){
  if(burnerState == 0){
    ON.setCounter(0, 0, dutyON, ON.COUNT_DOWN, OFFfunction);
    OFF.setCounter(0, 0, dutyOFF, OFF.COUNT_DOWN,  RESETfunction);
  }
  burnerState = 1;
  ON.run();
  ON.start();
  digitalWrite(rco, HIGH);
}

void OFFfunction(){
  burnerState = 2;
  OFF.run();
  OFF.start();
  digitalWrite(rco, LOW);
}

void RESETfunction(){
  burnerState = 0;
}

void heatingDone() {
  processPhase = 5;
  tone(buzzer, 3400, 40000);
}

// Helper functions
void refreshPreheat() {
    Serial.print("   preheat time: ");
    Serial.print(preheatTimer.getCurrentTime());
}

void refreshMain(){
    Serial.print("   main time:  ");
    Serial.print(mainTimer.getCurrentTime());
}

void refreshON(){
  Serial.print("   ON time:  ");
  Serial.print(ON.getCurrentTime());
}

void refreshOFF(){
  Serial.print("   OFF time:  ");
  Serial.println(OFF.getCurrentTime());
}

float getTemp() {
  float rawvoltage= analogRead(tempSensor);
  float millivolts= (rawvoltage/1024.0) * 5000;
  float celsius= millivolts/10;
  return celsius;
}

double getHeatDial() {
 //double tempAdjustment = (((analogRead(heatDial)-1023)*(-1))/1023) * 20;    **This line doesn't work
 double potValue = analogRead(heatDial);  
 double tempAdjustment = (((potValue-1023)*(-1))/1023)*20;
 Serial.print(" TEST TempAdustment:  ");
 Serial.print(tempAdjustment);
 return tempAdjustment;
}
