#include <Arduino.h>
#include "SoftwareSerial.h"
#include "TinyGPS++.h"

#define accy A0
#define tempc A1
#define relay 11

static const uint32_t baudRate =9600 ;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 5); //RX, TX
SoftwareSerial gsmSerial(9, 10); //RX, TX
String number = ""; //-> change with your number
boolean allowGPSSearching=1;
boolean allowCheckStatus=1;
String payLoad="";
unsigned int loopInterval=20000;
unsigned long started_waiting_time = 0;

void init_receive_sms(){
  gsmSerial.listen();
  // Serial.println("Init receive");
  gsmSerial.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(500);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(400);
  return;
}

void init_sms(){
  // Serial.println("Init Send");
  gsmSerial.println("AT+CMGF=1");
  delay(400);
  gsmSerial.println("AT+CMGS=\""+number+"\"");
  number="";  //Clean the number.
  delay(400);
  return;
}

void setup() {
  Serial.begin(baudRate);
  gpsSerial.begin(baudRate);
  gsmSerial.begin(baudRate);

  pinMode(accy, INPUT);
  pinMode(tempc, INPUT);
  pinMode(relay, OUTPUT);


  // Serial.println("Vehicle Tracking");
  delay(2000);
  init_receive_sms();
}
//
void resetData(){
  allowGPSSearching=1;
  allowCheckStatus=1;
  payLoad="";
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

void engine_on(){
  digitalWrite(relay, HIGH);
  send_sms("Ignation ON");
}

void engine_off(){
  digitalWrite(relay, LOW);
  send_sms("Ignation OFF");
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
    engine_on();
  }
   else if(isContain(payLoad,"stop"))
  {
    engine_off();
  }
   else if(isContain(payLoad,"checkstatus"))
  {
    // Serial.println("This is checkstatus");
    if(allowCheckStatus){
      send_sms("Sabu bhala");
      allowCheckStatus=0;
    }
  }
   else if(isContain(payLoad,"fetchlocation"))
  {
    tracking();
  }
  return;
}

void listenToSMS( unsigned long *endTime,  unsigned long *startTime){
 if (gsmSerial.available() > 0)
    {
      payLoad = gsmSerial.readString(); 
      number=payLoad.substring(9,22); //Extract the sender number.
      Serial.println(payLoad);  
      *startTime=*endTime;
      delay(1000);
    }
}

void recieveMessage()
{
    unsigned long endTime = millis();
    unsigned long startTime = 0;
    listenToSMS(&endTime,&startTime);
    while ((endTime - startTime) <= loopInterval)
    { 
      listenToSMS(&endTime,&startTime); //while in the loop listen to the SMS
      takeAction();
      endTime= millis();
    }
    resetData();
}

void loop() {
  recieveMessage();
}