# Preasure Cooker Controller

A sketch to control preasure cooker.

## Pin Layout

    #define buzzer         4
    #define startBtn       6
    #define tempSensor     A0
    #define heatDial       A1
    #define rco            8

## Adjustable Variables

    int warmupTemp       = 212;
    int defaultTemp      = 240;
    int preheatMinutes   = 10;      // 10 minutes
    int alarmTime        = 60000;   // 60000 miliseconds (1 minute)
    int heatingHours     = 5;       // 5 hours
    bool isRunning       = false;
    
## Libraries Used

- [Timer library](https://github.com/inflop/Countimer)
- [PID library](https://github.com/br3ttb/Arduino-PID-Library)

## Initial Instructions

program does not execute until a start button is pushed.  A short beep from buzzer can help indicate that code has begun execution.  
exact numbers do not matter as they can be changed later.   

brings temp to 212 for 10 minutes and sounds alarm for 60 seconds. (this purges the air from the pressure cooker and alarm allows me to flip the toggle valve on cooker)

after alarm sounds code then brings temp to 240 and holds for 5 hours.  Timer begins only after 240 is reached.  

potentiometer allows set point of 240 to be adjusted up by 0 to 20 degrees.  

after 5 hours timer is up the cycle is finished and no more power is sent to heating element.  An alarm goes off for 60 seconds.  

power to heating element is controlled by a mechanical relay.  Mechanical relay can only be switched 100000 times before it needs to be replaced do to wear, so it is important that code by implemented in a way that does not force the relay to switch too frequently.  If code causes rapid switching, the relay could be ruined in a matter of minutes.    Best solution is to determine what fraction of each 20 second timer interval the relay should be switched on.  This way the relay only switches on and off 3 times per minute.  So if for example max heat is needed, relay will just be on entire 20 seconds.  If 50% power is needed, relay will be on 10 seconds and then off 10 seconds.  There might be a way to adjust the frequency of PWM outputs or a special bit of code may need to be written to achieve this.  

once cycle is complete the unit should need to be unplugged and plugged back in to begin again.   

Pins are:
Buzzer/Alarm:  pin 4
Start Button:  pin 6
Temp Sensor:  A0
Potentiometer:  A1
Relay control output:  pin 8
