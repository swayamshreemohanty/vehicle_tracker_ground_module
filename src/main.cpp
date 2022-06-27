#include <Arduino.h>
#include "SoftwareSerial.h"
#include "TinyGPS++.h"
SoftwareSerial sim(5, 4);
int _timeout;
String _buffer;
String number = "+917609934272"; //-> change with your number
void setup() {
  delay(7000); //delay for 7 seconds to make sure the modules get the signal
  Serial.begin(9600);
  // _buffer.reserve(50);
  Serial.println("System Started...");
  sim.begin(9600);
  delay(1000);
  Serial.println("Type s to send an SMS, r to receive an SMS, and c to make a call");
}

String readSerial() {
  _timeout = 0;
  while  (!sim.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}

void SendMessage()
{
  Serial.println ("Sending Message");
  sim.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(200);
  //Serial.println ("Set SMS Number");
  sim.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
  delay(200);
  String SMS = "Hello, how are you? greetings from miliohm.com admin";
  sim.println(SMS);
  delay(100);
  sim.println((char)26);// ASCII code of CTRL+Z
  delay(200);
  _buffer = readSerial();
}

void RecieveMessage()
{
  Serial.println ("SIM800L Read an SMS");
  sim.println("AT+CMGF=1");
  delay (200);
  sim.println("AT+CNMI=1,2,0,0,0"); // AT Command to receive a live SMS
  delay(200);
  Serial.write ("Unread Message done");
}


void loop() {
  if (sim.available () > 0)
    switch (Serial.read())
    {
      case 's':
        SendMessage();
        break;
      case 'r':
        RecieveMessage();
    }
  // if (sim.available() > 0)
  //   Serial.write(sim.read());
}