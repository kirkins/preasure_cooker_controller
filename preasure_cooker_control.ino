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
bool isRunning       = false;

Countimer timer;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  if (digitalRead(startBtn)== HIGH && isRunning == false) {
    isRunning = true;
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
}

void mainHeat() {
  // preheat done, start main heating function
  tone(buzzer, 2000, alarmTime);
  // Run heating code add mainTemp with heatDial
  timer.setCounter(heatingHours, 0, 0, timer.COUNT_DOWN, heatingDone);
}

void heatingDone() {
  // process is finished
  tone(buzzer, 3000, alarmTime);
}
