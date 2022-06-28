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
char gsmState ='i';

void reset_gsm_state(){
  gsmState ='i';
}

void init_receive_sms(){
  Serial.println("Init receive");
  gsmSerial.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(500);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(400);
  reset_gsm_state();
}

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
  gsmSerial.begin(GPSBaud);
  Serial.print("Vehicle Tracking");
  delay(2000);
  init_receive_sms();
  temp=0;
}


void init_sms(){
  gsmSerial.println("AT+CMGF=1");
  delay(400);
  gsmSerial.println("AT+CMGS=\""+number+"\"");
  delay(400);
  reset_gsm_state();
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
    
    if (gsmSerial.available() > 0)
    {
      char msg = gsmSerial.read();
      Serial.print(msg);
      if (msg==(char*)"start")
      {
        Serial.println(" this is start");
      }
    
    
      // Serial.write(gsmSerial.read());

    //   if (gsmSerial.find((char*)"start"))
    //   {
    //     Serial.println("start");
    //   }
    //   else if (gsmSerial.find((char*)"stop"))
    //   {
    //     Serial.println("stop");
    //   }
    //   else if (gsmSerial.find((char*)"checkstatus"))
    //   {
    //     Serial.println("checkstatus");
    //   }
    //   else if (gsmSerial.find((char*)"fetchlocation"))
    //   {
    //     Serial.println("fetchlocation");
    //   } else{
    //     Serial.println("invalid");

    //   }     
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
  init_receive_sms();
}

void loop() {
  switch (gsmState)
    {
      case 's':
        init_sms();
        break;
      case 'r':
        init_receive_sms();
        break;
      default:
      break;
    }

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