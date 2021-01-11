#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESPDateTime.h>
#define EspSerial Serial
#define SERIAL_BUFFER_SIZE 256


ESP8266WiFiMulti wifiMulti;


const char* ssid = "putin_huylo2014"; // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "123454321"; // The password of the Wi-Fi network
const char* host = "plants0mon.herokuapp.com"; // The data receiver host name (not URL)
const int port = 443; // The data receiver host port
const size_t capacity = JSON_OBJECT_SIZE(3) + 120;
void handleGet();
void handlePost();
void handleNotFound();

String message = "NULL";
String ip_addr;
int electro_expected = 1;
String electro_current = "NULL";
String tem = "NULL";
String hum = "NULL";
const byte interruptPin = 5; // Pin to set interrupt to
int interruptCounter = 0; // counter of interrupt executions

ESP8266WebServer server(80); // set WiFiServer on port 80

void setupDateTime() {
  // setup this after wifi connected
  // you can use custom timeZone,server and timeout
  // DateTime.setTimeZone(-4);
  //   DateTime.setServer("asia.pool.ntp.org");
  //   DateTime.begin(15 * 1000);
  DateTime.setTimeZone(3);
  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
}

void setup() {

  EspSerial.begin(115200);// Start the Serial communication to send messages to the computer
  Serial1.begin(115200);
  delay(1000);
  Serial.println('\n');

  WiFi.hostname("ESPboard-counter");
  WiFi.begin(ssid, password); // Connect to the network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println(" ...");
  ip_addr[3] = 0;
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.println(++i);
    Serial.println(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  setupDateTime();
}

void loop(){
  delay(5000);
  WiFiClient client;
  if (!client.connect(host, port)) { 
    Serial.println("connection failed"); 
    return; 
  } 

  // send data to at mega
  DynamicJsonDocument doc(2048);
  DynamicJsonDocument mega_doc(2048);
  mega_doc["electro_expected"] = electro_expected;
  String data_string;
  serializeJson(mega_doc, data_string);
  Serial.println(data_string);
  
  //geting data from at mega
  if (EspSerial.available() > 0){
    DynamicJsonDocument esp_doc(2048);
    String esp_response = EspSerial.readString();
    DeserializationError error = deserializeJson(esp_doc, esp_response);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    } else {
        tem = esp_doc["tem"].as<char*>();
        hum = esp_doc["hum"].as<char*>();
        electro_current = esp_doc["electro_current"].as<char*>();
      }
  }

  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    HTTPClient http;    //Declare object of class HTTPClient
    http.begin("http://plants0mon.herokuapp.com/api/watch");      //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
    String json_str;
    doc["tem"] = tem;
    doc["hum"] = hum;
    doc["dt"] = DateTime.toString();
    doc["electro_current"] = electro_current;
    serializeJson(doc, json_str);
    //Serial.println(json_str);
    auto httpCode = http.POST(json_str);   //Send the request
    String payload = http.getString();                  //Get the response payload
    //Serial.println("### httpcode, payload");
    //Serial.println(httpCode);   //Print HTTP return code
    //Serial.println(payload);    //Print request response payload
    http.end();  //Close connection

    
    http.begin("http://plants0mon.herokuapp.com/api/electro");
    auto httpCodeGet = http.GET();   //Send the request
    String payloadGet = http.getString();       
    //Serial.println("### httpcodeGet, payload");
    //Serial.println(httpCodeGet);   //Print HTTP return code
    //Serial.println(payloadGet);    //Print request response payload
    http.end();  //Close connection  

    if (httpCodeGet == 200){
      DynamicJsonDocument http_doc(2048);
      DeserializationError error = deserializeJson(http_doc, payloadGet);
    if (error) {
      Serial.print(F("deserializeJson() failed after http response: "));
      Serial.println(error.c_str());
    } else {
        electro_expected = http_doc["electro"].as<int>();
      }
  }
    
  }else{
    Serial.println("Error in WiFi connection");
  }
}
