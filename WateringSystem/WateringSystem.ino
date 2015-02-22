/* Title: AUTOMATED WATERING SYSTEM
   Engineer: Alain Grandjean
   Date Created: 1-10-15
   Date Last Modified: 2-22-15
   Version: 4   

Description: Watering system for 1 - 3 motor(s).
Scalable to larger size if Arduino with more pins and better processor
and a bigger motor driver is used. An external power source is also recommended.
*/
#include <LiquidCrystal.h>
#include <Wire.h>
#include "RTClib.h"

#define plantNum 1   //Total number of plants (edit as needed)
#define pumpNum 1   //Total number of pumps (up to 3)

#define highLED 6   //Pin for green LED
#define lowLED 5    //Pin for yellow LED
#define lowWaterLED 4  //Pin for red low water LED
#define waterLevel 3 //Pin for water level sensor probe
 
RTC_DS1307 RTC;
                //BS,E, D4,D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
int pNum;  //Initialize global plant number variable for loops and functions
int moisture; //Initialize moisture variable
char* plantNames[] = {"Snake Plant."}; //Edit as needed
int pump[4] = {13,1,0};  //Initialize pump pin array
boolean isEmpty = false; //No water cutoff variable
volatile int interruptIt = 0; //Interrupt variable
int sensorRead;


void setup () {
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();         //Start the clock
    attachInterrupt(0, interruptMe, RISING); //Button interrupt on pin 2
    pinMode(waterLevel, INPUT); //Water level probe
    pinMode(lowWaterLED, OUTPUT); //LED warning of low water 
    pinMode(lowLED, OUTPUT); //Plant1 low moisture alert (Red)
    pinMode(highLED, OUTPUT); //Plant1 good moisture alert (Green)
    //pinMode(A0, INPUT); //Moisture sensor
    for(int i=0;i<pumpNum;i++){
      pinMode(pump[i], OUTPUT); //Motor pin
    }
    lcd.begin(16,2); //Begin LCD Display
    
    if (! RTC.isrunning()) {
      Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      RTC.adjust(DateTime(__DATE__, __TIME__));
    }
}
 
void loop () {
    lcd.clear();
    checkLevel();
}

void checkLevel(){
  if(digitalRead(waterLevel) == LOW){  //If there is no more signal to the probe
      isEmpty = true;    //Activate pump cutoff variable
      interruptIt = 0;  //In case interrupt was triggered
      emptyNotify();    //Print message and blink LED
  }
  else {
   isEmpty = false;    //Deactivate pump cutoff variable
   lcd.clear();
   if(interruptIt == 1){
     delay(500);
     executeAll();
     interruptIt = 0;
   }
   else {
     checkTime();
   }
  }
}

void emptyNotify(){      //Notify when water level is low
  lcd.display();
  lcd.setCursor(0,0);
  lcd.print("Water Level Low.");
  lcd.setCursor(0,1);
  lcd.print("Replace At Once.");
     for(int i=0; i<5; i++){
        digitalWrite(lowWaterLED, HIGH);
        delay(500);
        digitalWrite(lowWaterLED, LOW);
        delay(500);
     }
}

void checkTime(){
    DateTime now = RTC.now();      //Get date and time from when sketch was compiled
 
  /*  Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();                // Print out the time to serial monitor
    */
      if ((now.hour() == 8) && (now.minute() == 0)){ // First check at 08:00 A.M.
          executeAll();
      }
      if ((now.hour() == 19) && (now.minute() == 0)){ // First check at 07:00 P.M.
          executeAll();
      }
}

int checkMoisture(int pNum){
  int moist = 0;
  sensorRead = analogRead(pNum);
  moist = map(sensorRead,1022,22,0,100); //Map readings from 0 - 100
  constrain(moist,0,100);    //Constrain readings from 0 - 100
  return moist;
}

void printLCD(int pNum){
    lcd.display();
    lcd.clear();
    moisture = checkMoisture(pNum);
    lcd.setCursor(2,0);  //First line of LCD
    lcd.print(plantNames[pNum]); //Print name of current plant
    lcd.setCursor(0,1); //Second line of LCD
    lcd.print("Moisture:    %.");
    lcd.setCursor(10,1); //Second line of LCD
    lcd.print(moisture); //Print moisture level to LCD display
    delay(1000);
}

void lowAlert(){            //Blink the red light
  for(int i=0; i<5; i++){
    digitalWrite(lowLED, HIGH);
    delay(500);            //.5 second delay
    digitalWrite(lowLED,LOW);
    delay(500);
  }
}

void highAlert(){            //Blink the green light
   for(int i=0; i<5; i++){
     digitalWrite(highLED, HIGH);
     delay(500);            //.5 second delay
     digitalWrite(highLED,LOW);
     delay(500);
  }
}

void moistEval(int pNum){
  moisture = checkMoisture(pNum);
    if(moisture < 50){
          lowAlert();                //Blink low light
          digitalWrite(pump[pNum], HIGH); //Turn on pump[pNum]
          moistEval(pNum);       //Check the soil moisture
    } 
    else {
          highAlert();
          digitalWrite(pump[pNum], LOW);  //Turn off pump[pNum]
    }
}

void executeAll(){         //Perform all tasks
  if(isEmpty == false){ //While there is water in the tank
      for(pNum=0;pNum<(plantNum);pNum++){
         printLCD(pNum);      //Print plant name and moisture level
         moistEval(pNum);     //Pump water based on moisture
         
      }
    interruptIt = 0;  //Deactivate the interrupt variable
  }
  else {
    emptyNotify();
  }
}


void interruptMe(){
 interruptIt = 1; //Set interrupt variable to 1. This will execute all
}
