#include <ThingSpeak.h>
#include <WiFi.h>
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <EEPROM.h>
#include "GravityTDS.h"
#include <Adafruit_ADS1X15.h>
#include "DFRobot_PH.h"






OneWire ourWire(4);
DallasTemperature sensors(&ourWire);
#define SensorTUR 3
#define TdsSensorPin 36
#define PH_PIN 34
GravityTDS gravityTds;
DFRobot_PH ph;
//----Definicion de la red en la que se conectara---
const char* ssid = "Man in the Mirror";             
const char* password = "wingfers";     

unsigned long channelID = 2479480;
const char* WriteAPIKey = "AENOOWOK9MCKS36O";

const int FieldNumber1 = 1;
const int FieldNumber2 = 2;
const int FieldNumber3 = 3;
const int FieldNumber4 = 4;

float tdsValue = 0;
float voltage,phValue;

//---------------------------------------------------

WiFiClient Client;




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){

    delay(500);
    Serial.print(".");

  }

  Serial.println("WiFi Conectado!");

  ThingSpeak.begin(Client);

sensors.begin();
delay(100);

 gravityTds.setPin(TdsSensorPin);
    gravityTds.setAref(3.3);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds.begin();  //initialization

ph.begin();
}

void loop() {
  // put your main code here, to run repeatedly:


int x = 60;
int y = 100;

//
sensors.requestTemperatures();  
  float temp= sensors.getTempCByIndex(0); 
  Serial.print("Temperatura= ");
  Serial.print(temp);
  Serial.println(" C");
  delay(100);           

//
int sensorValue = analogRead(SensorTUR);
  float Medicion = sensorValue * (100.0 / 1024.0); 
  Serial.println(Medicion); 
  delay(500);

//
 gravityTds.setTemperature(temp);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate 
    tdsValue = gravityTds.getTdsValue();  // then get the value
    Serial.print(tdsValue,0);
    Serial.println("ppm");
    delay(1000);
 //   
static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U){                  //time interval: 1s
        timepoint = millis();
        //temperature = readTemperature();         // read your temperature sensor to execute temperature compensation
        voltage = analogRead(PH_PIN)/1024.0*5000;  // read the voltage
        phValue = ph.readPH(voltage,temp);  // convert voltage to pH with temperature compensation
        Serial.print("pH:");
        Serial.println(phValue,2);
    }
    ph.calibration(voltage,temp);           // calibration process by Serail CMD
 //
ThingSpeak.setField (1,Medicion);
ThingSpeak.setField (2,temp);
ThingSpeak.setField (3,tdsValue);
ThingSpeak.setField (4,phValue);

//
Serial.print ("Dato 1: ");
Serial.println (x);
delay(250);
Serial.print ("Dato 2: ");
Serial.println (y);
delay(250);

ThingSpeak.writeFields(channelID, WriteAPIKey);
Serial.println("Datos enviados a ThingSpeak!");
delay(10000);








}