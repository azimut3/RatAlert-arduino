#include <SPI.h>
#include <Wire.h>

#include <ArduinoJson.h>

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "MQ135.h"
#include "DHT.h"
 
#define DHTPin D5        // define the digital I/O pin 
#define DHTTYPE DHT11 
#define RZERO 676.139

const char* ssid = "zigor";     
const char* pass = "zigor725";
MQ135 gasSensor;

class LifeQuality{
  public: 
  float airQualityPpm;
  float temperature;
  float humidity;
  float rzero;
  LifeQuality(){    
  } 
  
  String toJson(){
    String jsonString;
      
      StaticJsonDocument<200> doc;
      doc["airQualityPpm"] = airQualityPpm;
      doc["temperature"] = temperature;
      doc["humidity"] = humidity;
      doc["rzero"] = rzero;

      serializeJson(doc, jsonString);
      
      return jsonString;
  }
}; 
  
String relativeAirQuality;

DHT dht11(DHTPin, DHTTYPE);  
ESP8266WebServer server(80);

String getAirQualityLevel(int ppm){
  String airQuality = "";
  if(ppm <= 50){
    airQuality += "GOOD ";
  } else if(ppm <= 100){
    airQuality += "Moderate ";
  } else if(ppm <= 150){
    airQuality += "Unhealthy for SG ";
  } else if(ppm <= 200){
    airQuality += "Unhealthy ";
  }else if(ppm <= 300){
    airQuality += "Very Unhealthy ";
  } else {
    airQuality += "Hazardous ";
  }
  
  return airQuality;
}

void getLifeQualityData() {
    
    float zero = gasSensor.getRZero(); 
    Serial.print ("rzero: "); 
    Serial.println (zero); 
    LifeQuality qualityObject;
    //  sensorValue = analogRead(A0);
    Serial.print("Analog reads: ");
    Serial.println(analogRead(A0)/1024*3.3);
    
    qualityObject.airQualityPpm = gasSensor.getPPM();    
    relativeAirQuality = getAirQualityLevel(qualityObject.airQualityPpm);
    qualityObject.rzero = gasSensor.getRZero();
    
    qualityObject.temperature = dht11.readTemperature(); // Gets the values of the temperature
    qualityObject.humidity = dht11.readHumidity();

    String requestJson = qualityObject.toJson();
    Serial.println(requestJson); 
    server.send(200, "application/json", requestJson);
}
 
// Define routing
void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
            F("Success"));
    });
    server.on(F("/lifeQuality"), HTTP_GET, getLifeQualityData);
}
 
// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(){
  Serial.begin(115200);  
 
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting...");
  }
  
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");

  pinMode(DHTPin, INPUT);
  dht11.begin();   
  gasSensor = MQ135(A0);
} 
 
void loop(){
    server.handleClient();
}
