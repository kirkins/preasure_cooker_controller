#include <PID_v1.h>
#include <Countimer.h>

#define buzzer         4
#define startBtn       6
#define tempSensor     A0
#define heatDial       A1
#define rco            8

int warmupTemp       = 99;
int defaultTemp      = 120;
int preheatMinutes   = 10;      // 10 minutes
int alarmTime        = 60000;   // 60000 miliseconds (1 minute)
int heatingHours     = 5;       // 5 hours
int processPhase     = 0;

Countimer timer;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // Check process phase
  switch (processPhase) {
  case 0:
    if (digitalRead(startBtn) == HIGH) {
      start();
    }
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

void start() {
  tone(buzzer, 1000);
  processPhase = 1;
}

void preHeatStart() {
  // Before reaching target temperature
  float temp = getTemp();
  if(temp > warmupTemp) {
    processPhase = 2;
  }
}

void preHeat() {
  // Start heating to 212 for 10 minutes
  // Only start timer below once temp is 212
  timer.setCounter(0, preheatMinutes, 0, timer.COUNT_DOWN, mainHeat);
}

void mainHeatStart() {
  // Before reaching target temperature
  float temp = getTemp();
  // Target should add 0-20 based on heatDial
  float targetTemp = defaultTemp + getHeatDial();
  if(temp > targetTemp) {
    processPhase = 4;
  }
}

void mainHeat() {
  processPhase = 2;
  // preheat done, start main heating function
  tone(buzzer, 2000, alarmTime);
  // Run heating code add mainTemp with heatDial
  timer.setCounter(heatingHours, 0, 0, timer.COUNT_DOWN, heatingDone);
}

void heatingDone() {
  processPhase = 3;
  // process is finished
  tone(buzzer, 3000, alarmTime);
}

float getTemp() {
  float rawvoltage= analogRead(tempSensor);
  float millivolts= (rawvoltage/1024.0) * 5000;
  float celsius= millivolts/10;
  return celsius;
}

float getHeatDial() {
  float tempAdjustment = (analogRead(heatDial)/1023) * 20;
  return tempAdjustment;
}
