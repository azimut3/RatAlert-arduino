#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include "MQ135.h"

#define RZERO 1
 
String apiKey = "14K8UL2QEK8BTHN6"; // Enter your Write API key from ThingSpeak
const char *ssid = "zigor";     // replace with your wifi ssid and wpa2 key
const char *pass = "zigor725";
 
WiFiClient client; 
 
void setup(){
  Serial.begin(115200);
  delay(10);
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected");       
}
 
 
  void loop(){
    MQ135 gasSensor = MQ135(A0);
    float zero = gasSensor.getRZero(); 
    Serial.print ("rzero: "); 
    Serial.println (zero); 
    float air_quality = gasSensor.getPPM();
    Serial.println(getAirCondition(air_quality));
    /*  
    if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(air_quality);
    postStr += "r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    
    Serial.println("Data Send to Thingspeak");
  }
    client.stop();*/   
 
    delay(2000);      // thingspeak needs minimum 15 sec delay between updates.
}

String getAirCondition(int ppm){
  String airQuality = "Air Quality: ";
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
  airQuality += "(";
 airQuality += ppm;
 airQuality += " PPM)";
  
  return airQuality;
}
