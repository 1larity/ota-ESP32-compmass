/*ESP 32 web based inteface for QMC5883L digital compass 
Uses Webseral to create webpage on ESP's IP address to display output from QMC5883L. 
Includes OTA firmware update.


R Beech 2021
*/
//compass support
#include <QMC5883LCompass.h>
//platformIO arduino lib
#include <Arduino.h>
//Wifi support
#include <WiFi.h>
#include <AsyncTCP.h>
//web based serial console
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
//ESP wifi OTA firmware updater
#include <ArduinoOTA.h>
//I2C support
#include <Wire.h>
//Wifi credentials
#include <secrets.h>
//create webserver
AsyncWebServer server(80);
QMC5883LCompass compass;

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
}

//setup OTA
void setupOTA() {

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}


void setupWifI (){
//WIFI setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSID_PW);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // WebSerial setup. Server is accessible at "<IP Address>/webserial" in browser
    WebSerial.begin(&server);
    /* Attach Message Callback */
    WebSerial.msgCallback(recvMsg);
    server.begin();
}

void setup() {
  // put your setup code here, to run once:
  delay (1000);
  Serial.begin(115200);
  setupWifI();
  setupOTA();
  Wire.begin();
  compass.init();
  compass.setCalibration(-1122, 1852, -1481, 1468, -915, 1703);
  compass.setSmoothing(5, false);
  Serial.println("Ready");
  }


void loop() {
 ArduinoOTA.handle();
 compass.read();
 int x, y, z;
  x = compass.getX();
  y = compass.getY();
  z = compass.getZ();
  WebSerial.print("X: ");
  WebSerial.print(x);
  WebSerial.print(" Y: ");
  WebSerial.print(y);
  WebSerial.print(" Z: ");
  WebSerial.print(z);

  int azimuth = compass.getAzimuth();
  WebSerial.print(" AZ: ");
  WebSerial.print(azimuth);
  char compassArray[3];
  compass.getDirection(compassArray, azimuth);
  WebSerial.print(" dir1: ");
  WebSerial.print(char(compassArray[0]));
  WebSerial.print(" dir2: ");
  WebSerial.print(compassArray[1]);
  WebSerial.print(" dir2: ");
  WebSerial.print(compassArray[2]);
  WebSerial.println();
 delay(5000);   

}