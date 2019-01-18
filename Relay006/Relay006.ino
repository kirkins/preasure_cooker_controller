/*To Do List:
 * run additional tests to check that all stages work properly
 * add pins for LEDs for various stages
 * change timer values and temp values to suit actual use
 * fix button interference issue
 * fix relay interference issue
 * mount LEDs to PCB
 * reassemble circuit box
 * test on actual burner
 * build 3 more units
 */


#include <PID_v1.h>
#include <Countimer.h>

#define buzzer         4
#define startBtn       6
#define tempSensor     A0
#define heatDial       A1
#define rco            8

int warmupTemp       = 24;
int defaultTemp      = 28;
int preheatMinutes   = 0;      // 10 minutes
int alarmTime        = 1500;   // 60000 miliseconds (1 minute)
int heatingHours     = 0;       // 5 hours
int processPhase     = 0;
int dutyON           = 0;
int dutyOFF          = 0;
int counter          = 0;     //allows entry to if statement when == 1
double currentTemp, pidOutput, targetTemp;
int burnerState      = 0;
PID heatingPID(&currentTemp, &pidOutput, &targetTemp,5,15,0, DIRECT);

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
    Serial.println(processPhase);
    Serial.print("  currentTemp:");
    Serial.print(currentTemp);
    


  // Check process phase and run associated loop

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
  }
}

void startcycle() {
  tone(buzzer, 1000, 1000);
  processPhase = 1;
}

// Process phases and alarms
void preHeatStart() {
  //Phase 1
  // Before reaching target temperature
  currentTemp = getTemp();
  digitalWrite(rco, HIGH);
  if(currentTemp > warmupTemp) {
   preheatTimer.setCounter(0, preheatMinutes, 10, preheatTimer.COUNT_DOWN, preHeatAlarm);

    processPhase = 2;
  }
}

void preHeat() {
  //Phase 2
  // Start heating to 212 for 10 minutes
  digitalWrite(rco, HIGH);

}

void preHeatAlarm() {
  // preheat done, switch phase and sound alarm
  processPhase = 3;
  tone(buzzer, 2000, alarmTime);
}

void mainHeatStart() {
  //Phase 3
  currentTemp = getTemp();
  // Before reaching target temperature
  // Target should add 0-20 based on heatDial
  targetTemp = defaultTemp + getHeatDial();
  digitalWrite(rco, HIGH);
  if(currentTemp > targetTemp) {
    mainTimer.setCounter(heatingHours, 1, 0, mainTimer.COUNT_DOWN, heatingDone);
    processPhase = 4;
  }
}

void mainHeat() {
  //Phase 4
  // Run heating code add mainTemp with heatDial
  targetTemp = defaultTemp + getHeatDial();
  currentTemp = getTemp();
  heatingPID.Compute();
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
    dutyON = duty*4;
    dutyOFF = (4-dutyON);
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
  digitalWrite(13, HIGH);
  
}

void OFFfunction(){
  burnerState = 2;
  OFF.run();
  OFF.start();
  digitalWrite(13, LOW);
}

void RESETfunction(){
  burnerState = 0;
}

void heatingDone() {
  processPhase = 5;
  tone(buzzer, 3000, alarmTime);
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

float getHeatDial() {
  float tempAdjustment = (((analogRead(heatDial)-1023)*(-1))/1023) * 20;
  return tempAdjustment;
}
