#include <FastLED.h>
#define FASTLED_ESP8266_D1_PIN_ORDER

#include <Arduino.h>
#include "config.h"

#include <EEPROM.h>
#include <ArduinoOTA.h>

#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#include <Wire.h>

#include "main.h"
#include "sensorresult.h"
#include "mqttservice.h"

#define USE_LEDS
#define DEBUG
//#define BME_ENABLED

#ifdef BME_ENABLED
#include "sensorservice.h"
SensorService sensorService;

#endif

WiFiManager wifiManager;
WiFiClient client;
String nameString;
ESP8266HTTPUpdateServer httpUpdateServer;
#define DEBUG_ESP_HTTP_SERVER
ESP8266WebServer webServer(80);

MqttService mqttService(client);

void setup()
{
  Serial.begin(115200);
  //disable builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

// put your setup code here, to run once:
#ifdef USE_LEDS
  FastLED.addLeds<WS2812B, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(125);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
#endif

  EEPROM.begin(512);
  loadSettings();

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  Wire.begin();

#ifdef DEBUG
  Serial.println();
  Serial.print(F("Heap: "));
  Serial.println(system_get_free_heap_size());
  Serial.print(F("Boot Vers: "));
  Serial.println(system_get_boot_version());
  Serial.print(F("CPU: "));
  Serial.println(system_get_cpu_freq());
  Serial.print(F("SDK: "));
  Serial.println(system_get_sdk_version());
  Serial.print(F("Chip ID: "));
  Serial.println(system_get_chip_id());
  Serial.print(F("Flash ID: "));
  Serial.println(spi_flash_get_id());
  Serial.print(F("Flash Size: "));
  Serial.println(ESP.getFlashChipRealSize());
  Serial.print(F("Vcc: "));
  Serial.println(ESP.getVcc());
  Serial.println();
#endif

#ifdef BME_ENABLED
  Serial.println("Start configuring BME stuff");

  while (!sensorService.bmeNotFound())
  {
    Serial.println("BME280 not found ...");

    FastLED.showColor(CRGB::Red);
    delay(1000);
  }
#endif

  prepareWifi();
  prepareWebServer(nameChar);

  prepareOTA(nameString);

  rst_info *rstinfo = ESP.getResetInfoPtr();
  Serial.println(rstinfo->reason);
}

void loop()
{
  //Serial.println("[main] Start Handle Client ... ");
  webServer.handleClient();
  MDNS.update();
  ArduinoOTA.handle();
  if (power == 0)
  {
    //if no power => do not handle the rest
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(1000 / FRAMES_PER_SECOND);
    return;
  }

#ifdef BME_ENABLED
  EVERY_N_SECONDS(60)
  {

    SensorResult data = sensorService.readBmeValues();
#ifdef DEBUG
    Serial.print("[main] BME DATA : ");
    Serial.println(data.toString());

#endif
    /*if(!mqttService.isConnected()){
      #ifdef DEBUG
        Serial.println("[main] Not connected to MQTT => try to connect");
      #endif
      
    }
    */
    mqttService.publish(data);
  }
#endif
  EVERY_N_MILLISECONDS(40)
  {
    // slowly blend the current palette to the next
    //nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 8);
    gHue++; // slowly cycle the "base color" through the rainbow
  }

  //Serial.print("[main] Calling pattern on currentPatternIndex: ");
  //Serial.println(currentPatternIndex);
  patterns[currentPatternIndex].pattern();
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void prepareWifi(void)
{
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) + String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();

  nameString = "ESP8266-" + macID;
  //nameChar = char[nameString.length() + 1];

  //char nameChar[nameString.length() + 1];
  memset(nameChar, 0, nameString.length() + 1);

  for (uint8_t i = 0; i < nameString.length(); i++)
    nameChar[i] = nameString.charAt(i);

  Serial.printf("Name: %s\n", nameChar);
  if (wifiManager.autoConnect(nameChar))
  {
    Serial.println("Wi-Fi connected");
  }
  else
  {
    Serial.println("Wi-Fi manager portal running");
  }
  mqttService.connect();
}

void prepareWebServer(char nameChar[])
{
  Serial.printf("nameChar : %s\n", nameChar);
  httpUpdateServer.setup(&webServer);

  webServer.on("/all", HTTP_GET, []() {
    Serial.println("All has been triggered");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", "{'success' : true}");
  });

  webServer.on("/on", HTTP_GET, []() {
    Serial.println("Turn Lights On");
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", "{'success' : true, \"color\": \"white\"}");
  });

  webServer.on("/off", HTTP_GET, []() {
    Serial.println("Turn Lights OFF");

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "application/json", "{'success' : true, \"color\": \"black\"}");
  });

  webServer.on("/brightness", HTTP_POST, []() {
    String value = webServer.arg("value");
    int intValue = atoi(value.c_str());
    if (intValue < 0)
      intValue = 0;
    if (intValue > 255)
      intValue = 255;
    FastLED.setBrightness(intValue);
    FastLED.show();
  });

  webServer.on("/power", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPower(atoi(value.c_str()));
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    sendInt(power);
  });

  webServer.on("/solidColor", HTTP_POST, []() {
    String r = webServer.arg("r");
    String g = webServer.arg("g");
    String b = webServer.arg("b");
    setSolidColor(r.toInt(), g.toInt(), b.toInt());
    sendString(String(solidColor.r) + "," + String(solidColor.g) + "," + String(solidColor.b));
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
  });

  webServer.on("/pattern", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPattern(value.toInt());
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    sendInt(currentPatternIndex);
  });

  MDNS.begin(nameChar);
  MDNS.setHostname(nameChar);

  webServer.begin();
  Serial.println("HTTP web server started");
};

void prepareOTA(String espName)
{
  ArduinoOTA.setHostname(espName.c_str());
  ArduinoOTA.setPassword("Aveve2008");

  ArduinoOTA.onStart([] {
    Serial.println("[OTA] onStart");
  });

  ArduinoOTA.onEnd([] {
    Serial.println("[OTA] onEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
};

void sendInt(uint8_t value)
{
  sendString(String(value));
};

void sendString(String value)
{
  webServer.send(200, "text/plain", value);
}

void setPower(uint8_t value)
{
  power = value == 0 ? 0 : 1;
  EEPROM.write(5, power);
  EEPROM.commit();
}

void loadSettings(void)
{
  brightness = EEPROM.read(0);

  currentPatternIndex = EEPROM.read(1);
  if (currentPatternIndex < 0)
    currentPatternIndex = 0;
  else if (currentPatternIndex >= patternCount)
    currentPatternIndex = patternCount - 1;

  byte r = EEPROM.read(2);
  byte g = EEPROM.read(3);
  byte b = EEPROM.read(4);

  if (r == 0 && g == 0 && b == 0)
  {
  }
  else
  {
    solidColor = CRGB(r, g, b);
  }
  power = EEPROM.read(5);
};

void setSolidColor(CRGB color)
{
  setSolidColor(color.r, color.g, color.b);
};

void setPattern(uint8_t value)
{
  if (value >= patternCount)
    value = patternCount - 1;

  currentPatternIndex = value;

  //if (autoplay == 0)
  //{
  EEPROM.write(1, currentPatternIndex);
  EEPROM.commit();
  //}
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  solidColor = CRGB(r, g, b);

  EEPROM.write(2, r);
  EEPROM.write(3, g);
  EEPROM.write(4, b);
  EEPROM.commit();

  setPattern(0);
}

void showSolidColor()
{
  //Serial.println("[main] showSolidColorCalled");
  fill_solid(leds, NUM_LEDS, solidColor);
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 255 / NUM_LEDS);
}