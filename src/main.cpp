#include <Arduino.h>
#include "SoftwareSerial.h"
#include "TinyGPS++.h"
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;
SoftwareSerial gpsSerial(5, 4); //RX, TX
SoftwareSerial gsmSerial(9, 10); //RX, TX
String stringVal = "";
String number = "+917609934272"; //-> change with your number
int temp=0;

void gsm_init(){
  boolean at_flag=1;
  while (at_flag)
  {
    Serial.println("ATE0");
    while (Serial.available()>0)
    {
      if (Serial.find((char*)"OK"))
      {
        at_flag=0;
      }
    }
    delay(1000);
  }
}

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
  gsmSerial.begin(GPSBaud);
  Serial.print("Vehicle Tracking");
  delay(2000);
  gsm_init();
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(2000);
  temp=0;
  delay(1000);
  boolean net_flag=1;
  while (net_flag)
  {
    gsmSerial.println("AT+CPIN?");
    while (gsmSerial.find((char*)"+CPIN: READY"))
    {
      net_flag=0;
    }
    delay(1000);
  }
  delay(1000);
}


void init_sms(){
  gsmSerial.println("AT+CMGF=1");
  delay(400);
  gsmSerial.println("AT+CMGS=\""+number+"\"");
  delay(400);
}

//
void send_data(String message){
  gsmSerial.print(message);
  delay(200);
}
void send_sms(){
  gsmSerial.write(26);
  delay(1000);
}
//


void recieveMessage()
{
  while (gsmSerial.available()>0)
  {
    Serial.print("Listen to DATA");
    Serial.write(gsmSerial.read());
    if (gsmSerial.find((char*)"Track"))
    {
      //DO SOMETHING
      temp=1;
      break;
    }else{
      temp=0;
    }

  }
}


void tracking(){
  init_sms();
  send_data("Vehicle location is:");
  gsmSerial.print("Latitude: ");
  gsmSerial.print(gps.location.lat(), 6);
  gsmSerial.print("\n Longitude: ");
  gsmSerial.println(gps.location.lng(), 6);
  send_sms();
}

void loop() {
  recieveMessage();
  // while (temp)  //run the loop if temp==1;
  // {
  //   while (gpsSerial.available()>0)
  //   {
  //     gps.encode(gpsSerial.read());
  //     if (gps.location.isUpdated())
  //     {
  //       temp=0;
  //       tracking();
  //     }
  //   }
  // }
}