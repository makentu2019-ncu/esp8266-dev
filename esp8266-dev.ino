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
#define STASSID "MakeNTU2019-NCU"
#define STAPSK  "lazy_tech"
#define SERVER "http://makentu2019-test.herokuapp.com"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const String host = SERVER;

String token = "undefined";
boolean b_spaceStatus = false;
int sinalPin[3] = {5,4,0};
int totalSinalLength = 3;
int deviceID = -1;

void setup(){
  Serial.begin(115200);

  for(int i=0;i<totalSinalLength;i++){
    pinMode(sinalPin[i],INPUT);
  }
  
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
  Serial.println("update!");
  updateRequest();
//  getSpaceStatus(totalSinalLength);
  delay(5000);
}

String registered(){
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, host+"/api/registered?name=esp8266&lat=25.045722&lon=121.5315245")){
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
      }else{
        Serial.println("[HTTP] Server error code: "+String(httpCode));
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

void updateRequest(){
  WiFiClient client;
  HTTPClient http;
  StaticJsonDocument<256> spaceStatus;
  spaceStatus["token"] = token;
  spaceStatus = false;

  String path = "/api/update";
  String ssts = (b_spaceStatus?"true":"false");
  String parame = "token="+token+"&status="+ssts+"&space="+getSpaceStatus(totalSinalLength);
  if(http.begin(host+path)){
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int statusCode = http.POST(parame);
    String result = http.getString();
    Serial.println("result: "+result+", code:"+String(statusCode));
  }
}

String getSpaceStatus(int len){
//  const size_t CAPACITY = JSON_ARRAY_SIZE(10);
  String r = "[";
  b_spaceStatus = false;
  for(int i=0;i<len;i++){
    String s = sensorStatus(i);
    if(s=="true")
      b_spaceStatus = true;
    String jsonObj = "{\"pid\":"+String(i)+",\"status\":"+s+",\"floor\":\"B1\"}"+(i+1<len?",":"");
    r+=jsonObj;
  }
  r+="]";
  Serial.println(r);
  return r;
}

String sensorStatus(int p){
  return digitalRead(sinalPin[p])==1?"true":"false";
}
