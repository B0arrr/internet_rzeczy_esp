#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

#define HTTP_REST_PORT 8080
ESP8266WebServer server(HTTP_REST_PORT);

const char* ssid = "AndroidAP8915";
const char* password = "6846c39d8534";
// const char* ssid = "akademik2";
// const char* password = "AkademiK2";

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;

sensors_event_t humidity, temp;

void restServerRouting();
void handleNotFound();

void setup() {
  Serial.begin(115200);
  Serial.println("\nAHT20+BMP280 test");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf("Connected to %s\n", ssid);
    Serial.println(WiFi.localIP());
  }

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  while (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }

  Serial.println("AHT10 or AHT20 found");
  
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1) delay(10);
  }

  Serial.println("BMP280 found");

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  restServerRouting();
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  aht.getEvent(&humidity, &temp);
  server.handleClient();
  MDNS.update();

  // Serial.printf("Temperature AHT: %.02f *C\n", temp.temperature);
  // Serial.printf("Temperature BMP: %.02f *C\n", bmp.readTemperature());
  // Serial.printf("Humidity: %.02f %RH\n", humidity.relative_humidity);
  // Serial.printf("Pressure: %.02f hPa\n", bmp.readPressure());
  // Serial.printf("Approx altitude: %.02f m\n", bmp.readAltitude(1013.25));

  // delay(1000);
}

void restServerRouting() {
  server.on("/", HTTP_GET, getAllData);
}

void getAllData() {
  StaticJsonDocument<300> JSONData;
  JSONData["temperature"] = temp.temperature;
  JSONData["humidity"] = humidity.relative_humidity;
  JSONData["pressure"] = bmp.readPressure();
  JSONData["altitude"] = bmp.readAltitude(1013.25);
  char data[300];
  serializeJson(JSONData,data);
  server.send(200,"application/json",data);
}

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
