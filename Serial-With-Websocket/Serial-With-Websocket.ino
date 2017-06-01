#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <stdio.h>

WebSocketsServer webSocket = WebSocketsServer(81);

const char *ssid = "Your Wifi Name";
const char *password = "Your Wifi password";

uint8_t lastConnection;
//char *readChar;
char readStringa[] = "1";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    case WStype_DISCONNECTED:{
      //	For Debug, it help you to understand if the user has been kicked from the websocket server
      Serial.printf("Disconnected!\n", num);
      break;
    }

    case WStype_CONNECTED:{
      //	Print To the serial the IP of the User that has been Connected
      IPAddress ip = webSocket.remoteIP(num);
      lastConnection = num;	//Only the last user that connected to the websocket can control the car
      Serial.printf("Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      webSocket.sendTXT(num, "Connected");	// Send the word Connected to the website
      break;
    }

    case WStype_TEXT:{
      String text = String((char *) &payload[0]);
      delay(10);         
      webSocket.sendTXT(num, text);	
      Serial.print(text);
      break;
    }  

    case WStype_BIN:{   
      hexdump(payload, lenght);
      // echo data back to browser
      webSocket.sendBIN(num, payload, lenght);
      break;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.setTimeout(0.05);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
  if (Serial.available()){
    readStringa[0] = Serial.read();
    String text = String(readStringa);
    webSocket.sendTXT(lastConnection, text);
    delay(0.05);   
  }
}
