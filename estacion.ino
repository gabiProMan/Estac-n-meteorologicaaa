#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include "esp_sleep.h"

// ===== WIFI =====
const char* ssid = "SigmaBoy";
const char* password = "MarioFeoMan";

// ===== THINGSPEAK =====
unsigned long channelID = 3263364;
const char* writeAPIKey = "GFWXVTHY902ADWK6";
WiFiClient client;

// ===== TIEMPOS =====
#define TIEMPO_ACTIVO 60000
#define TIEMPO_DORMIR 60
#define uS_TO_S_FACTOR 1000000

// ===== SENSORES =====
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP280 bmp;

#define PIN_LUZ 34

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(128, 64, &Wire, -1);

unsigned long tiempoInicio;

void setup() {

  Serial.begin(115200);

  Wire.begin();

  tiempoInicio = millis();

  dht.begin();

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 no encontrado");
  }

  // ===== OLED =====
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED no encontrado");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Iniciando...");
  display.display();

  // ===== WIFI =====
  WiFi.begin(ssid, password);

  Serial.print("Conectando WiFi");

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("OK");

  ThingSpeak.begin(client);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi conectado");
  display.println(WiFi.localIP());
  display.display();
}

void loop() {

  // ===== LEER SENSORES =====
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float tempBMP = bmp.readTemperature();
  float presion = bmp.readPressure() / 100.0;
  float altitud = bmp.readAltitude(1013.25);

  int luzRaw = analogRead(PIN_LUZ);
  int luz = map(luzRaw, 0, 4095, 0, 1000);

  // ===== OLED =====
  display.clearDisplay();
  display.setCursor(0,0);

  display.println("ESTACION METEO");
  display.print("Temp: "); display.println(t);
  display.print("Hum: "); display.println(h);
  display.print("TempBMP: "); display.println(tempBMP);
  display.print("Presion: "); display.println(presion);
  display.print("Luz: "); display.println(luz);

  display.display();

  // ===== THINGSPEAK =====
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, tempBMP);
  ThingSpeak.setField(4, presion);
  ThingSpeak.setField(5, altitud);
  ThingSpeak.setField(6, luz);

  ThingSpeak.writeFields(channelID, writeAPIKey);

  delay(5000);

  // ===== DORMIR =====
  if (millis() - tiempoInicio >= TIEMPO_ACTIVO) {

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Durmiendo 1 min...");
    display.display();

    delay(2000);

    esp_sleep_enable_timer_wakeup(TIEMPO_DORMIR * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }
}