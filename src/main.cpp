#include <Arduino.h>
#include "SoftwareSerial.h"
#include "TinyGPS++.h"
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 5); //RX, TX
SoftwareSerial gsmSerial(9, 10); //RX, TX
String number = ""; //-> change with your number
boolean allowGPSSearching=1;
String payLoad="";

void init_receive_sms(){
  gsmSerial.listen();
  Serial.println("Init receive");
  gsmSerial.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(500);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(400);
  return;
}

void init_sms(){
  Serial.println("Init Send");
  gsmSerial.println("AT+CMGF=1");
  delay(400);
  gsmSerial.println("AT+CMGS=\""+number+"\"");
  number="";  //Clean the number.
  delay(400);
  return;
}

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
  gsmSerial.begin(GPSBaud);
  Serial.println("Vehicle Tracking");
  delay(2000);
  init_receive_sms();
}



//
void send_data(String message){
  gsmSerial.print(message);
  delay(200);
  return;
}

void send_sms(String message){
  init_sms();
  gsmSerial.print(message);
  gsmSerial.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module
  delay(500);
  init_receive_sms();
  return;
}


void tracking()
{
  while (allowGPSSearching)
  {
    gpsSerial.listen();
   while (gpsSerial.available()>0)
    {
      gps.encode(gpsSerial.read());
     if (gps.location.isUpdated())
     {
       init_sms();
       gsmSerial.print("your vehicle location is ");
       gsmSerial.print("latitude: ");
       gsmSerial.print(gps.location.lat(), 6);
       gsmSerial.print(", longitude: ");
       gsmSerial.print(gps.location.lng(), 6);
       gsmSerial.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module
       delay(100);
       //
       init_receive_sms();
      allowGPSSearching=0;
     }
     break;
    }
  }
  return;
}

//
bool isContain(String payload, String find){
  int payloadLength=payload.length();
  int findPayloadLength=find.length();
  if (findPayloadLength>payloadLength)
  {
    return false;
  }
  else
  {
    int index=0;
    int found=0;
    while (index<payloadLength)
    {
      if (find[found] == payload[index])
      {
        found++;
        if (found==findPayloadLength)
        {
          return true;
        }
      }
      else
      {
        found=0;
      }
      index++;
    }
    return false;
  }
}
//

void takeAction(){
  if(isContain(payLoad,"start"))
  {
    Serial.println("This is start");
  }
   else if(isContain(payLoad,"stop"))
  {
    Serial.println("This is stop");
  }
   else if(isContain(payLoad,"checkstatus"))
  {
    Serial.println("This is checkstatus");
    send_sms("Sabu bhala");
  }
   else if(isContain(payLoad,"fetchlocation"))
  {
    Serial.println("fetchlocation");
    tracking();
  }
  return;
}

void recieveMessage()
{
    if (gsmSerial.available() > 0)
    {
      payLoad = gsmSerial.readString(); 
      number=payLoad.substring(9,22); //Extract the sender number.
      Serial.println(payLoad);  
      delay(1000);
    }
    takeAction();
}




void loop() {
  recieveMessage();
}