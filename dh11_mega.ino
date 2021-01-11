
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>

#include <LiquidCrystal.h>

#define DHTTYPE DHT11
#define DHTPIN A0

DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

uint32_t delayMS;

String _temp;
String _hum;
String esp_data;
String electro_current = "1";
String _tmp;
int electro_expected = 1;



const size_t capacity = JSON_OBJECT_SIZE(3) + 120;

void setup() {
  Serial.begin(9600);
  Serial3.begin(115200);
  // Initialize device.
  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
 
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

  lcd.begin(16, 2);
  lcd.print("Hello Plants");

  pinMode (2, OUTPUT);
  digitalWrite(2, HIGH);

}


void loop() {
  // Delay between measurements.
  delay(5123);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    _temp = String(event.temperature);
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    _hum = String(event.relative_humidity);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(_temp);
  lcd.print(" *C, ");
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(_hum);
  lcd.print(" % ");


  //send data to esp
  DynamicJsonDocument output_doc(2048);
  output_doc["tem"] = _temp;
  output_doc["hum"] = _hum;
  output_doc["electro_current"] = electro_current;
  serializeJson(output_doc, Serial3);

  DynamicJsonDocument esp_doc(2048);
  
  //get data from esp
  if (Serial3.available() > 0){
    _tmp = Serial3.readString();
    Serial.println(_tmp);
    DeserializationError error = deserializeJson(esp_doc, Serial3);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
  } else {
    electro_expected = esp_doc["electro_expected"].as<int>();
    // data from esp
    Serial.println(electro_expected);
    }
  }

  if (electro_current == "1" && electro_expected == 0){
    digitalWrite(2, LOW);
    electro_current = "0";
    }

  if (electro_current == "0" && electro_expected == 1){
    digitalWrite(2, HIGH);
    electro_current = "1";
    }
  //delay(2000);
  //digitalWrite(2, LOW);
}
