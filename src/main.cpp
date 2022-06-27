#include <Arduino.h>
#include "SoftwareSerial.h"
#include "TinyGPS++.h"
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
SoftwareSerial gpsSerial(5, 4); //RX, TX
String stringVal = "";
String number = "+917609934272"; //-> change with your number
int temp=0;


void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
  Serial.print("Vehicle Tracking");
  delay(2000);
  gsm_init();
  Serial.println("AT+CNMI=2,2,0,0,0");
  delay(2000);
  temp=0;
}

void gsm_init(){
  boolean at_flag=1;
  while (at_flag)
  {
    Serial.println("ATE0");
    while (Serial.available()>0)
    {
      if (Serial.find("OK"))
      {
        at_flag=0;
      }
    }
    delay(1000);
  }
  delay(1000);
  boolean net_flag=1;
  while (net_flag)
  {
    Serial.println("AT+CPIN?");
    while (Serial.find("+CPIN: READY"))
    {
      net_flag=0;
    }
    delay(1000);
  }
  delay(1000);
}


void init_sms(){
  Serial.println("AT+CMGF=1");
  delay(400);
  Serial.println("AT+CMGS=\""+number+"\"");
  delay(400);
}

//
void send_data(String message){
  Serial.print(message);
  delay(200);
}
void send_sms(){
  Serial.write(26);
}
//


void recieveMessage()
{
  while (Serial.available()>0)
  {
    if (Serial.find("Track"))
    {
      //DO SOMETHING
      temp=1;
      break;
    }else{
      temp=0;
    }
    Serial.println("Reading SMS");
    Serial.println(Serial.readString());
  }
}


void tracking(){
  init_sms();
  send_data("Vehicle location is:");
  Serial.print("Latitude: ");
  Serial.print(gps.location.lat(), 6);
  Serial.print("\n Longitude: ");
  Serial.println(gps.location.lng(), 6);
  send_sms();
  delay(2000);
}

void loop() {
  recieveMessage();
  while (temp)  //run the loop if temp==1;
  {
    while (gpsSerial.available()>0)
    {
      gps.encode(gpsSerial.read());
      if (gps.location.isUpdated())
      {
        temp=0;
        tracking();
      }
    }
  }
}