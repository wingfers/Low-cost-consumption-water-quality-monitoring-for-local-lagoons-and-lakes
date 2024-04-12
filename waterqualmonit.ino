#include <ThingSpeak.h>
#include <WiFi.h>
#include <OneWire.h>                
#include <DallasTemperature.h>

OneWire ourWire(32); 
DallasTemperature sensors(&ourWire); 
#define SensorTUR 33 
#define TdsSensorPin 39
#define PH_PIN 34



float voltage,phValue;


//------------------------ NETWORK ---------------------------
const char* ssid = "Familia_2002";             
const char* password = "owen2002";     

unsigned long channelID = 2479480;
const char* WriteAPIKey = "AENOOWOK9MCKS36O";

const int FieldNumber1 = 1;
const int FieldNumber2 = 2;
const int FieldNumber3 = 3;
const int FieldNumber4 = 4;
const int FieldNumber5 = 5;

WiFiClient Client;


//---------------------ORP--------------------------
int VOLTAJE = 3.30;    //system voltage
int OFFSET = 0;        //zero drift voltage

double orpValue;

#define ArrayLenth 40    //times of collection
#define orpPin 35          //orp meter output,connect to Arduino controller ADC pin

int orpArray[ArrayLenth];
int orpArrayIndex=0;

double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    printf("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}


 
void setup()
{
    Serial.begin(9600);
        sensors.begin();
    delay(500);
WiFi.begin(ssid, password);// Se inicia la conexion WiFi
  while(WiFi.status() != WL_CONNECTED){

    delay(500);
    Serial.print(".");

  }
 
  Serial.println("WiFi Conectado!");

  ThingSpeak.begin(Client); // Se inicia conexion con ThingSpeak
}
 
void loop()

{
  //Temperatura
  sensors.requestTemperatures();  // Envia la peticion para lectura de temperatura
  float temp= sensors.getTempCByIndex(0); // Obtencion del dato
  Serial.print("Temperatura= ");
  Serial.print(temp);
  Serial.println(" C");
  delay(100);   

//Turbidez
int sensorValue = analogRead(SensorTUR); //Lectura del valor analogico 
  float NTU;
  float Medicion = sensorValue * (5.0 / 4096.0); //Convierte el valor analogico leido (el cual tiene valores entre 0-4096), en porcentaje (0-100%) 
  if (Medicion < 2.5) {
NTU = 3000;
}
else if (Medicion > 4.2) {
NTU = 0;
}
else {
NTU = -1120.4 * pow(Medicion,2) + 5742.3 * Medicion - 4353.8;
} 
  Serial.print("Valor de Turbidez: ");
  Serial.print(Medicion); 
  Serial.println("V");
  Serial.print(NTU);
  Serial.println("NTU");
  delay(1000);

//TDS
  int ADC_value;
unsigned long int avgval_ADC;
int buffer_tds[10],temp1;
  for(int i=0;i<10;i++) 
  { 
    buffer_tds[i]=analogRead(TdsSensorPin);
    delay(30);
  }
 
    for(int i=0;i<9;i++)
      {
        for(int j=i+1;j<10;j++)
          {
            if(buffer_tds[i]>buffer_tds[j])
              {
                temp1=buffer_tds[i];
                buffer_tds[i]=buffer_tds[j];
                buffer_tds[j]=temp1;
               
              }
            }
       }
 
  Serial.println (temp1);
                delay(500);
      avgval_ADC=0;
      for(int i=2;i<8;i++)
      avgval_ADC+=buffer_tds[i];
      
      float voltage_value = (float)avgval_ADC*3.3/4096.0/6;
      float TDS = (133.42/voltage_value*voltage_value-255.86*voltage_value*voltage_value + 857.39*voltage_value)*0.5;
      Serial.print("TDS Value =");
      Serial.print(TDS);
      Serial.println("PPM");
      delay(1000);


//pH
voltage = analogRead(PH_PIN) * (3.3/4096.0);
phValue = 3.3*voltage;
Serial.print("Voltaje: ");
Serial.print(voltage);
Serial.println("V");
delay(1000);
Serial.print("pH: ");
Serial.println(phValue);
delay(1000);

//ORP
static unsigned long orpTimer=millis();   //analog sampling interval
  static unsigned long printTime=millis();
  if(millis() >= orpTimer)
  {
    orpTimer=millis()+20;
    orpArray[orpArrayIndex++]=analogRead(orpPin);    //read an analog value every 20ms
    if (orpArrayIndex==ArrayLenth) {
      orpArrayIndex=0;
    }   
    orpValue=(((30*(double)VOLTAJE*1000)-(75*avergearray(orpArray, ArrayLenth)*VOLTAJE*1000/4096))/75-OFFSET);   //convert the analog value to orp according the circuit
  }
  if(millis() >= printTime)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
	printTime=millis()+800;
	Serial.print("ORP: ");
	Serial.print((int)orpValue);
        Serial.println("mV");
        Serial.println(orpPin);
  }


ThingSpeak.setField (1,phValue);
ThingSpeak.setField (2,TDS);
ThingSpeak.setField (3,NTU);
ThingSpeak.setField (4,temp);
ThingSpeak.setField (5,(int)orpValue);

ThingSpeak.writeFields(channelID, WriteAPIKey);
Serial.println("Datos enviados a ThingSpeak!");
delay(10000);
}