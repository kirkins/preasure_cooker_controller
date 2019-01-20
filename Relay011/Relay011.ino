//#include <ArduinoSTL.h>
#include <PID_v1.h>
#include <Countimer.h>

#define buzzer         4
#define startBtn       6
#define tempSensor     A0
#define heatDial       A1
#define rco            9
#define redLED         10
#define greenLED       11

int warmupTemp       = 82;// set to 82
int defaultTemp      = 101;// set to 101
int preheatMinutes   = 10;      // 10 minutes
int alarmTime        = 4000;   // 4000 miliseconds
int heatingHours     = 5;       // 5 hours
int processPhase     = 0;
int dutyON           = 0;
int dutyOFF          = 0;
int burnerState      = 0;
int count            = 0;
int i                = 0;
double currentTemp, pidOutput, targetTemp;
int sampleSize = 50;
float avgTemp[50];
//std::vector<double>avgTemp(30);

PID heatingPID(&currentTemp, &pidOutput, &targetTemp, 0.5, 7, 1, DIRECT);

Countimer preheatTimer;
Countimer mainTimer;
Countimer ON;
Countimer OFF;
Countimer Failsafe;
Countimer Failsafe2;

void setup() {
  heatingPID.SetMode(AUTOMATIC);
  Serial.begin(9600);
  preheatTimer.setInterval(refreshPreheat, 1000);
  mainTimer.setInterval(refreshMain, 1000);
  ON.setInterval(refreshON, 1000);
  OFF.setInterval(refreshOFF, 1000);
  pinMode(rco, OUTPUT);
  Failsafe.setCounter(0, 40, 0, Failsafe.COUNT_DOWN, checkSafe);
  Failsafe2.setCounter(0, 80, 0, Failsafe2.COUNT_DOWN, checkSafe2);
  for ( i=0; i<sampleSize; i++) {

  avgTemp[i] = 23;

}
  
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
   if(count == 9600){
    count = 0;
   
    Serial.print(108);
    Serial.print("  ");
    Serial.print(94);
    //Serial.print("  Phase:");
    //Serial.print(processPhase);
    //Serial.print("  PotentiometerReading: ");
    //Serial.print(analogRead(heatDial));
    //Serial.print("  currentTemp:");
    Serial.print("  ");
    Serial.print(currentTemp);
    Serial.print("  ");
    Serial.println();
    }else{count++;}
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
  // Brings temp to boil, and starts 10 minute timer to purge air.  
  Failsafe2.run();
  Failsafe2.start();
  currentTemp = getTemp();
  digitalWrite(rco, HIGH);
  analogWrite(redLED, 255);
  if(currentTemp > warmupTemp) {
   preheatTimer.setCounter(0, preheatMinutes, 0, preheatTimer.COUNT_DOWN, preHeatAlarm);
   processPhase = 2;
  }
}

void preHeat() {
  //Phase 2
  // Start heating to 212 for 10 minutes
  //runs during countdown to alarm. air is purging
  digitalWrite(rco, HIGH);
  analogWrite(redLED, 255);

}

void preHeatAlarm() {
  // preheat done, switch phase and sound alarm
  //alarm is sounding, stopcock should be closed during this time.  
  processPhase = 3;
  tone(buzzer, 2400, 60000);
}

void mainHeatStart() {
  //Phase 3
  //stopcock is closed and pressure is climbing
  Failsafe.run();
  Failsafe.start();
  currentTemp = getTemp(); 
  targetTemp = defaultTemp + getHeatDial(); 
  digitalWrite(rco, HIGH);
  analogWrite(redLED, 255);
  analogWrite(greenLED, 255);
  if(currentTemp > targetTemp) {
    mainTimer.setCounter(heatingHours, 0, 0, mainTimer.COUNT_DOWN, heatingDone);
    processPhase = 4;
  }
}

void mainHeat() {
  //Phase 4
  // Run heating code add mainTemp with heatDial
  Failsafe.run();
  Failsafe.start();
  targetTemp = defaultTemp + getHeatDial();
  currentTemp = getTemp();
  heatingPID.SetOutputLimits(50, 150);
  heatingPID.Compute();
  analogWrite(redLED, 255);
  analogWrite(greenLED, 255);
  /*Serial.print("  PID output:");
  Serial.print(pidOutput);*/
  //Serial.print("  targetTemp:");
  //Serial.print(targetTemp);

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

void checkSafe(){
  currentTemp = getTemp(); 
  targetTemp = defaultTemp + getHeatDial(); 
  float difference = abs(currentTemp - targetTemp);
  if(difference>15){
    processPhase = 5;
  }
}

void checkSafe2(){
  if(processPhase == 1){
    processPhase = 5;
  }
}

void getDuty(){
    double duty = pidOutput/255;
    dutyON = duty*20;
    dutyOFF = (20-dutyON);
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
    /*Serial.print("   preheat time: ");
    Serial.print(preheatTimer.getCurrentTime());  */
}

void refreshMain(){
    /*Serial.print("   main time:  ");
    Serial.print(mainTimer.getCurrentTime());*/
}

void refreshON(){
 /* Serial.print("   ON time:  ");
  Serial.print(ON.getCurrentTime());*/
}

void refreshOFF(){
 /* Serial.print("   OFF time:  ");
  Serial.println(OFF.getCurrentTime());*/
}

double getTemp() {
  int i = 0;
  double total = 0;
  double rawvoltage= analogRead(tempSensor);
  double millivolts= (rawvoltage/1024.0) * 5000;
  double celsius= millivolts/10;
  for ( i=0; i<sampleSize-1; i++) {
avgTemp[i] = avgTemp[i+1];
}
avgTemp[sampleSize-1] = celsius;
for ( i=0; i<sampleSize; i++) {

  total = total + avgTemp[i];

}
double average = total / sampleSize;
  return average;
}

/*double getTemp(){
  double rawvoltage= analogRead(tempSensor);
  double millivolts= (rawvoltage/1024.0) * 5000;
  double celsius= millivolts/10;
  avgTemp.push_back(celsius);
  double average = accumulate( avgTemp.begin(), avgTemp.end(), 0.0)/avgTemp.size();
  return average;
}*/

double getHeatDial() {
 double potValue = analogRead(heatDial);  
 double tempAdjustment = (((potValue-1023)*(-1))/1023)*4;
 /*Serial.print(" TEST TempAdustment:  ");
 Serial.print(tempAdjustment);*/
 return tempAdjustment;
}
