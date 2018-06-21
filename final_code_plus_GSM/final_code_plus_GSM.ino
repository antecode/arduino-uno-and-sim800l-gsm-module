//reset system
#include <SoftReset.h>
//gsm
#include <SoftwareSerial.h>
SoftwareSerial mySerial(9,10);
char incomingByte;
String inputString;
#define GSM_LIGHTS 8
#define GSM_PUMP 13

//Temperature & Humidity variable, Libraries and pins declaration
#include <dht.h>
dht DHT;
#define DHT11_PIN 11
#define heatup_alarm_pin 7
#define alarm_indicator 12

//Soil moisture variable and pins declaration
#define water_pump_pin 6
#define moisture_sensor A0  
int moisture_value; 
int moisture_percentage = 0;  

//LDR variable and pins declaration
#define uv_lighting 4 
#define floodlight 5 
float light_value;
int light_intensity;
int LDR = A1; 
int sensorValue = 0; 
//variables
int i = 0; 
void setup(){     
//GSM bypass pins
    pinMode (GSM_LIGHTS, OUTPUT);
      pinMode (GSM_PUMP, OUTPUT);
//DHT-11
  pinMode (heatup_alarm_pin, OUTPUT);
//Soil Moisture Sensor
  pinMode (water_pump_pin, OUTPUT);
  pinMode (moisture_sensor, INPUT);
//LDR
  pinMode (uv_lighting, OUTPUT);
  pinMode (LDR, INPUT); 
  pinMode (floodlight, OUTPUT);   

//START SERIAL MONITOR FOR ALL
  Serial.begin(9600);
   //GSM connection serial begin
         mySerial.begin(9600);
      // to check connection to gsm module
      while(!mySerial.available()){
        mySerial.println("AT");
        delay(1000);
        Serial.println("mkulima.com is connecting to network, please wait...");
        }
      Serial.println("Connected!");
      mySerial.println("AT+CMGF=1");  //Set SMS Text Mode mode
      delay(1000);
      mySerial.println("AT+CNMI=1,2,0,0,0");  //procedure, how receiving of new messages from the network
      delay(1000);
      mySerial.println("AT+CMGL=\"REC UNREAD\""); //soma texts
      mySerial.println("AT+CMGD=1,4");
 //GSM connection serial end here
  Serial.println("\t\t\t\t\t\t\t\"Mkulima.com\" e-Agriculture Project ");
  Serial.println("\t\t\t\t\t\t\tBy: Antony M. Munyao & Josphine Samuel");
  Serial.println("\t\t\t\t\t\t\t  Computer Science, Moi University");
  Serial.print("\t\t\t\t\t\t\t    PROJECT VERSION: ");
  Serial.print(DHT_LIB_VERSION);
  Serial.println();
  Serial.println();
  Serial.println("\t Feed \t\t Status \tHumidity (%) \t  Temperature(C)\tLight Intensity(%) \t  Moisture (%)");
  Serial.println("\t ---------------------------------------------------------------------------------------------------------");
  Serial.println();
}
 
void loop()
{      // READ DATA FOR THE DHT11
Serial.print("\t Sensors\t");
  int chk = DHT.read11(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
    Serial.print("Connected!\t"); 
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    Serial.print("Checksum error,\t"); 
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    Serial.print("Time out error,\t"); 
    break;
    case DHTLIB_ERROR_CONNECT:
        Serial.print("Connect error,\t");
        break;
    case DHTLIB_ERROR_ACK_L:
        Serial.print("Ack Low error,\t");
        break;
    case DHTLIB_ERROR_ACK_H:
        Serial.print("Ack High error,\t");
        break;
    default: 
    Serial.print("\t Unknown error,\t"); 
    break;
  }
  
  // read the value from the soil moisture sensor:
  moisture_value = analogRead(moisture_sensor);
  moisture_percentage = moisture_value * (100 / 1023.0);
  
  //read data from the LDR and convert to %
  light_value=analogRead(LDR);
  light_intensity=light_value* (100 / 1023.0);
  
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print("\t\t\t");
  Serial.print(DHT.temperature, 1);
  Serial.print("\t\t\t");  
  Serial.print(light_intensity);
  Serial.print("\t\t\t");
  Serial.print(moisture_percentage);
  Serial . println();

  
  /*conditions controlling the actuators*/

  //Actuator 1 (fire alert sms and fire alarm)

if(DHT.temperature > 33)
{
  alarm();
  alertsms();
  delay(100);
}

else{
  digitalWrite(heatup_alarm_pin, LOW);
  }

//Actuator 2 (Water Pump)
  if (moisture_percentage < 31)
{
  //switch off other appliances
  //electric pump uses an induction motor and can cause a short circuit
  digitalWrite(heatup_alarm_pin, LOW);
  digitalWrite(floodlight, LOW);
  digitalWrite(uv_lighting, LOW);
  digitalWrite(GSM_LIGHTS, LOW);

  digitalWrite(water_pump_pin, HIGH);
}else{
digitalWrite(water_pump_pin, LOW);}

//Actuator 3 (U.V Lighting / Flood light)
  if (light_intensity < 80)
{
  digitalWrite(floodlight, HIGH);
}
  if (light_intensity < 75)
{
  digitalWrite(uv_lighting, HIGH);
}
  else
{
  digitalWrite(floodlight, LOW);
  digitalWrite(uv_lighting, LOW);
}
delay(5000); //re-run whole loop after 5 sec(s)

//START GSM
  if(mySerial.available()){
      delay(100);
      // Serial buffer
      while(mySerial.available()){
        incomingByte = mySerial.read();
        inputString += incomingByte;
        }
        delay(10);
        inputString.toUpperCase(); // uppercase the message received
        //turn relay ON or OFF
         if (inputString.indexOf("LIGHTS_ON") > -1){
          digitalWrite(GSM_LIGHTS, HIGH);
           digitalWrite(uv_lighting, HIGH);
          }
         if (inputString.indexOf("LIGHTS_OFF") > -1){
          digitalWrite(GSM_LIGHTS, LOW);
          digitalWrite(uv_lighting, LOW);
          delay(1000);
          soft_restart();
          }
         if (inputString.indexOf("PUMP_ON") > -1){
          digitalWrite(GSM_PUMP, HIGH);
          }
         if (inputString.indexOf("PUMP_OFF") > -1){
          digitalWrite(GSM_PUMP, LOW);
          delay(1000);
          soft_restart();
          }
          if (inputString.indexOf("ALARM_ON") > -1){
          alarm();
          }
         if (inputString.indexOf("ALARM_OFF") > -1){
          digitalWrite(heatup_alarm_pin, LOW);
          digitalWrite(alarm_indicator, LOW);
          delay(1000);
          soft_restart();
          }
         if (inputString.indexOf("RESTART") > -1){
          delay(1000);
          soft_restart();
          delay(1000);
          }
         if (inputString.indexOf("STATUS") > -1){
         SendMessage();
         delay(1000);
         soft_restart();
          }
        delay(50);
        }
//end gsm   
}
//FUNCTIONS START HERE
//this is a function to send a reply sms
  void SendMessage()
{
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+254706434259\"\r"); // Replace x with mobile number
  delay(1000);
   mySerial.println("real-time status");
   mySerial.println("----------------------------");
     mySerial.print("Sensors: : ");
  mySerial.println("Connected"); 
   mySerial.print("Humidity: ");
  mySerial.println(DHT.humidity, 1);
    mySerial.print("Temperature: ");
  mySerial.println(DHT.temperature, 1);
    mySerial.print("Soil moisture: ");
  mySerial.println(moisture_percentage);
    mySerial.print("Light intensity: ");
  mySerial.println(light_intensity);
  mySerial.println("");
  mySerial.println("System SMS");
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  
  delay(1000);
}
//alert SMS function
  void alertsms()
{
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+254706434259\"\r"); // Receiver mobile number
  delay(1000);
  mySerial.println("Hi, system heatup detected, urgent attention required!");// The SMS text to send
  mySerial.println("");
  mySerial.print("System SMS");
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}
//play alarm function
void alarm() { 
//heatup_alarm_pin
        for(i = 1; i <= 10; i++)
        {
            analogWrite(alarm_indicator, 255);
            analogWrite(heatup_alarm_pin, 200);
            delay(100);
            analogWrite(alarm_indicator, 0);
            analogWrite(heatup_alarm_pin, 25);
            delay(100);
            
            analogWrite(alarm_indicator, 255);
            analogWrite(heatup_alarm_pin, 200);
            delay(100);
            analogWrite(alarm_indicator, 0);
            analogWrite(heatup_alarm_pin, 25);
            delay(100);
                        analogWrite(alarm_indicator, 255);
            analogWrite(heatup_alarm_pin, 200);
            delay(100);
            analogWrite(alarm_indicator, 0);
            analogWrite(heatup_alarm_pin, 25);
            delay(100);
            
            analogWrite(alarm_indicator, 255);
            analogWrite(heatup_alarm_pin, 200);
            delay(100);
            analogWrite(alarm_indicator, 0);
            analogWrite(heatup_alarm_pin, 25);
            delay(100);
        }
            analogWrite(heatup_alarm_pin, 0);
            delay(100);
//heatup_alarm_pin
  }
//EoF
//Glory be to God for His unmerrited grace and support
