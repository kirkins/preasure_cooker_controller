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

double currentTemp, pidOutput, targetTemp;
PID heatingPID(&currentTemp, &pidOutput, &targetTemp,5,15,0, DIRECT);

Countimer timer;

void setup() {
  heatingPID.SetMode(AUTOMATIC);
  Serial.begin(9600);
  pinMode(rco, OUTPUT);
}

void loop() {
  // Check process phase and run associated loop
  switch (processPhase) {
  case 0:
    if (digitalRead(startBtn) == HIGH) {
      start();
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

void start() {
  tone(buzzer, 1000, 1000);
  processPhase = 1;
}

// Process phases and alarms
void preHeatStart() {
  // Before reaching target temperature
  currentTemp = getTemp();
  digitalWrite(rco, HIGH);
  if(currentTemp > warmupTemp) {
    timer.setCounter(0, preheatMinutes, 0, timer.COUNT_DOWN, preHeatAlarm);
    timer.run();
    processPhase = 2;
  }
}

void preHeat() {
  digitalWrite(rco, HIGH);
}

void preHeatAlarm() {
  // preheat done, switch phase and sound alarm
  processPhase = 3;
  tone(buzzer, 2000, alarmTime);
}

void mainHeatStart() {
  // Before reaching target temperature
  float temp = getTemp();
  // Target should add 0-20 based on heatDial
  targetTemp = defaultTemp + getHeatDial();
  digitalWrite(rco, HIGH);
  if(temp > targetTemp) {
    timer.setCounter(heatingHours, 0, 0, timer.COUNT_DOWN, heatingDone);
    processPhase = 4;
  }
}

void mainHeat() {
  // Run heating code add mainTemp with heatDial
  targetTemp = defaultTemp + getHeatDial();
  float temp = getTemp();
  heatingPID.Compute();
}

void heatingDone() {
  processPhase = 5;
  tone(buzzer, 3000, alarmTime);
}

// Helper functions
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
