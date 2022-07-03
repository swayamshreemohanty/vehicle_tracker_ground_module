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
boolean allowGPSSearching=1;
boolean allowCheckStatus=1;
String number = "";
String payLoad="";
String ownerPhoneNumber= "+917205308022"; 
unsigned int loopInterval=20000;
unsigned long started_waiting_time = 0;
int fallAngle=280; 
int flipAngle=400; 

void init_receive_sms(){
  gsmSerial.listen();
  gsmSerial.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(500);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(500);
  return;
}

void init_sms(){
  gsmSerial.println("AT+CMGF=1");
  delay(400);
  gsmSerial.println("AT+CMGS=\""+number+"\"");
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

  delay(2000);
  init_receive_sms();
}
//
void resetData(){
  allowGPSSearching=1;
  allowCheckStatus=1;
  payLoad="";
  number="";  //Clean the number.
}
//


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
  delay(100);
}

void engine_off(){
  digitalWrite(relay, LOW);
  send_sms("Ignation OFF");
  delay(100);
}


void emergency_engine_off(){
  digitalWrite(relay, LOW);
  delay(100);
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

String vehicleStatus(String condition, int temp){
  return "your vehicle status is condition: "+condition+", temperature: "+temp;
}

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
       if (analogRead(accy)<= fallAngle || analogRead(accy) >=flipAngle) //reading<=280, flipAngle>=400
      {
      send_sms(vehicleStatus("Danger",60));
      }
      else{
      send_sms(vehicleStatus("Normal",40));
      }
      allowCheckStatus=0;
    }
  }
   else if(isContain(payLoad,"fetchlocation"))
  {
    tracking();
  }
  return;
}

void listenToSMS(unsigned long *endTime,  unsigned long *startTime){
 if (gsmSerial.available() > 0)
    {
      payLoad = gsmSerial.readString(); 
      number=payLoad.substring(9,22); //Extract the sender number.
      *startTime=*endTime;
      delay(500);
    }
}

void listenToAccySensor(unsigned long *endTime,  unsigned long *startTime){
  if (analogRead(accy)<= fallAngle || analogRead(accy) >=flipAngle) //reading<=280, flipAngle>=400
  {
    *startTime=*endTime;//Reset the while loop
    number=ownerPhoneNumber;
    emergency_engine_off();
    send_sms(vehicleStatus("Danger",90));
    tracking();
  }
}

void recieveMessage()
{
  unsigned long endTime = millis();
  unsigned long startTime = 0;
  listenToSMS(&endTime,&startTime);
  listenToAccySensor(&endTime,&startTime);
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