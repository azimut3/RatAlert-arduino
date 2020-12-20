#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "MQ135.h"
#include "DHT.h"
 
#define DHTPin D5        // define the digital I/O pin 
#define DHTTYPE DHT11 
#define RZERO 1 

DHT dht11(DHTPin, DHTTYPE);  
WiFiClient client;

String apiKey = "14K8UL2QEK8BTHN6"; // Enter your Write API key from ThingSpeak
const char *ssid = "zigor";     
const char *pass = "zigor725";
const char *accessPointName = "EspMeteo";
const char *accessPointPass = "zigor725";

IPAddress local_ip(192,168,4,4);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

float air_quality_ppm;    
String relativeAirQuality;
float temperature;
float humidity;
 
ESP8266WebServer server(80);


String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
 
String html_1 = R"=====(
<!DOCTYPE html>
<html>
 <head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <meta charset='utf-8'>
  <style>
    body {font-size:100%;} 
    #main {display: table; margin: auto;  padding: 0 10px 0 10px; } 
    h2 {text-align:center; } 
    p { text-align:center; }
  </style>
  <script>
   function refresh(refreshPeriod) 
   {
      setTimeout("location.reload(true);", refreshPeriod);
   } 
   window.onload = refresh(5000);
  </script>
  <title>Home meteo station</title>
 </head>
 
 <body>
   <div id='main'>
     <h2>Current condition:</h2>
     <div id='airQlt'> 
       <p>Air Quality - %airPpmLevel% (%relativeAirQuality%)</p>
       <p>Temperature - %temperature%*C</p>
       <p>Humidity - %humidity%%</p>
     </div>
   </div> 
 </body>
</html>
)====="; 

void setup(){
  Serial.begin(115200);
  delay(100);
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
 
  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(accessPointName, accessPointPass) ? "AP initialized" : "AP initialization failed!");
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  //server.on("/", handleRoot);
  server.on("/", handleUpdate);
  server.begin();
  Serial.println("HTTP server started");

  pinMode(DHTPin, INPUT);
  dht11.begin();
}
 
 
  void loop(){
    server.handleClient();
}

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

void handleRoot() {
  server.send(200, "text/html", html_1);
}

void handleUpdate(){
    Serial.println(); 
    Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum()); 
    MQ135 gasSensor = MQ135(A0);
    float zero = gasSensor.getRZero(); 
    Serial.print ("rzero: "); 
    Serial.println (zero); 
    
    air_quality_ppm = gasSensor.getPPM();    
    relativeAirQuality = getAirQualityLevel(air_quality_ppm); 
    
    Serial.println(String(air_quality_ppm) + " - " + relativeAirQuality);    
    
    temperature = dht11.readTemperature(); // Gets the values of the temperature
    humidity = dht11.readHumidity();

    Serial.println(String(temperature) + "*C - " + humidity + "%");
  
    //value is passed by an URL argument
    String tmpString = html_1;
    tmpString.replace("%airPpmLevel%", String(air_quality_ppm) );
    tmpString.replace("%relativeAirQuality%", relativeAirQuality );
    
    tmpString.replace("%temperature%", String(temperature) );
    tmpString.replace("%humidity%", String(humidity) );    
    
    server.send(200,"text/html", tmpString);     
}
