#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "MakeNTU2019-A-2.4G"
#define STAPSK  "lazy_tech"
#define SERVER "http://1d0bcac3.ngrok.io"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const String host = SERVER;
const String configFilePath = "/config.json";

String token = "undefined";
int deviceID = -1;

void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting.");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n------------------------");
  Serial.print("SSID:\t\t");
  Serial.println(ssid);
  Serial.print("Password:\t");
  Serial.println(password);
  Serial.println("------------------------\n");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("geting token...");
  while(token=="undefined"){
    Serial.print("Status: ");
    token = registered();
    delay(500);
  }
  Serial.println("token: "+token);
}

void loop(){
  int time = 10000;//random(1000, 5000);
  String id = String(deviceID);
  Serial.println("your device id:    "+id);
  Serial.println("your access token: "+token);
  Serial.println("random update time: "+String(time));
  sendPost();
  delay(time);
}

String registered(){
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, host+"/api/registered?name=esp8266&lat=25.4444&lon=121.5555")){
    int httpCode = http.GET();
    if(httpCode > 0){
      //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
        StaticJsonDocument<256> doc;
        String data = http.getString();
        DeserializationError error = deserializeJson(doc, data);
        Serial.println("success!!");
        String token = doc["token"];
        deviceID = doc["id"];
        return token;
      }
    }else{
      Serial.println("[HTTP] GET... failed");
    }
    http.end();
  }else{
    Serial.println("[HTTP] Unable to connect");
  }
  return "undefined";
}

void sendPost(){
  WiFiClient client;
  HTTPClient http;
  StaticJsonDocument<256> spaceStatus;
  spaceStatus["token"] = token;
  spaceStatus["status"] = true;
  String jsonToken = spaceStatus["token"];
  String statusOfSpace = (spaceStatus["status"]==true?"true":"false");
  String path = "/api/update";
  String parame = "token="+jsonToken+"&status="+statusOfSpace+"&space="+createPackage(16);
  if(http.begin(host+path)){
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int statusCode = http.POST(parame);
    String result = http.getString();
    Serial.println(parame);
    Serial.println("status: "+String(statusCode));
    Serial.println("result: "+result);
    http.end();
  }
}

String createPackage(const int len){
  String result = "[";
  const String space_template[] = "{\"pid\":1,\"status\":false,\"floor\":\"unknow\"}";
  for(int i=0;i<len;i++){
    String r;
    StaticJsonDocument<256> space;
    space["pid"] = i;
    space["status"] = sensorStatus(i);
    space["floor"] = String(i/4+1);
    serializeJson(space, r);
    String b = ",";
    if(i+1==len)
      b = "]";
    result+= (r.c_str()+b);
  }
  return result;
}

boolean sensorStatus(int i){
  return random(1)==(i%2);
}
