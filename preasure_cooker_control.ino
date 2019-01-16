#include <PID_v1.h>
#include <Countimer.h>

#define buzzer         4
#define startBtn       6
#define tempSensor     A0
#define heatDial       A1
#define rco            8

int warmupTemp       = 212;
int defaultTemp      = 240;
int preheatMinutes   = 10;      // 10 minutes
int alarmTime        = 60000;   // 60000 miliseconds (1 minute)
int heatingHours     = 5;       // 5 hours
int processPhase     = 0;

Countimer timer;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  if (digitalRead(startBtn)== HIGH && processPhase == 0) {
    processPhase = 1;
    start();
  }
}

void start() {
  tone(buzzer, 1000);
  preHeat();
}

void preHeat() {
  // Start heating to 212 for 10 minutes
  // Only start timer below once temp is 212
  timer.setCounter(0, preheatMinutes, 0, timer.COUNT_DOWN, mainHeat);
  // If still in preHeat phase loop
  if(processPhase == 1) {
    preHeat();
  }
}

void mainHeat() {
  processPhase = 2;
  // preheat done, start main heating function
  tone(buzzer, 2000, alarmTime);
  // Run heating code add mainTemp with heatDial
  timer.setCounter(heatingHours, 0, 0, timer.COUNT_DOWN, heatingDone);
  // If still in mainHeat phase loop
  if(processPhase == 2) {
    mainHeat();
  }
}

void heatingDone() {
  processPhase = 3;
  // process is finished
  tone(buzzer, 3000, alarmTime);
}
