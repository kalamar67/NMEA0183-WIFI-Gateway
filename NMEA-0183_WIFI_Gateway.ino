#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define MAX_NMEA0183_MSG_BUF_LEN 81  //According to NMEA 3.01. Can not contain multi message as in AIS
#define UART_BAUD 38400 //NMEA0183 HS (High Speed)

const char *ssid = "SSID_NAME"; //Replace with WIFI name.
IPAddress broadcast(192, 168, 4, 255); //ESP-8266 default run's on 192.168.4.1/255.255.255.0, so broadcast on 192.168.4.255
const int port = 9876;
WiFiUDP udp;

//Variables to keep track on received message
size_t MsgCheckSumStartPos;
char MsgInBuf[MAX_NMEA0183_MSG_BUF_LEN];
size_t MsgInPos;
bool MsgInStarted;
//////

void setup() {

  delay(1000);  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  Serial.begin(UART_BAUD);
}

void loop() {

  while(getMessage()) {
    udp.beginPacketMulticast(broadcast, port, WiFi.localIP());
    udp.println(MsgInBuf);
    udp.endPacket();    
  }
}

bool getMessage() {
  
  bool result=false;
  while (Serial.available() > 0 && !result) {
    int NewByte=Serial.read();
      if (NewByte=='$' || NewByte=='!') { // Message start
        MsgInStarted=true;
        MsgInPos=0;
        MsgInBuf[MsgInPos]=NewByte;
        MsgInPos++;
      } else if (MsgInStarted) {
        MsgInBuf[MsgInPos]=NewByte;
        if (NewByte=='*') MsgCheckSumStartPos=MsgInPos;
        MsgInPos++;
        if (MsgCheckSumStartPos!=SIZE_MAX and MsgCheckSumStartPos+3==MsgInPos) { // We have full checksum and so full message
            MsgInBuf[MsgInPos]=0; // add null termination
            result=true;
            MsgInStarted=false;
            MsgInPos=0;
            MsgCheckSumStartPos=SIZE_MAX;  
        }
        if (MsgInPos>=MAX_NMEA0183_MSG_BUF_LEN) { // Too may chars in message. Start from beginning
          MsgInStarted=false;
          MsgInPos=0;
          MsgCheckSumStartPos=SIZE_MAX;  
        }
      }
  }  
  return result;
}
