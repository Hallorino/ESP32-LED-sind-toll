#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <WiFi.h>
#include <secret.h>

#define LED_PIN 2
#define NUM_LEDS 8
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

AsyncWebServer server(80);

typedef struct {
  const String name;
  const CRGBPalette16 palette;
} Mode;

Mode modes[] = {{"Rainbow", CRGBPalette16(RainbowColors_p)},
                {"RainbowStripes", CRGBPalette16(RainbowStripeColors_p)},
                {"Cloud", CRGBPalette16(CloudColors_p)},
                {"Lava", CRGBPalette16(LavaColors_p)},
                {"Ocean", CRGBPalette16(OceanColors_p)},
                {"Forest", CRGBPalette16(ForestColors_p)},
                {"Party", CRGBPalette16(PartyColors_p)},
                {"Heat", CRGBPalette16(HeatColors_p)}};

uint8_t modesCount = sizeof(modes) / sizeof(modes[0]);

int curentMode = 0;
boolean hasBlend = true;
uint8_t brightness = 64;
uint8_t fps = 100;

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette);

String getSettingsAsJson() {
  StaticJsonDocument<96> doc;
  doc["mode"] = modes[curentMode].name;
  doc["hasBlend"] = hasBlend;
  doc["brightness"] = brightness;
  doc["fps"] = fps;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String getAllModesAsJson() {
  StaticJsonDocument<512> doc;
  for (int i = 0; i < modesCount; i++) {
    JsonObject mode = doc.createNestedObject();
    mode["name"] = modes[i].name;
  }
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  WiFi.mode(WIFI_STA);  // Optional
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/modes", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("get repuest on /modes");
    request->send(200, "application/json", getAllModesAsJson());
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("get request on /settings");
    request->send(200, "application/json", getSettingsAsJson());
  });
  server.begin();

  Serial.println("setup completed");
}

void loop() {
  // put your main code here, to run repeatedly:

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */

  FillLEDsFromPaletteColors(startIndex, modes[curentMode].palette);

  // Schicke Farben zu LED Strip
  FastLED.show();

  // Warte ein bisschen
  FastLED.delay(1000 / fps);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette) {
  uint8_t brightness = 255;

  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(palette, colorIndex, brightness, hasBlend?LINEARBLEND:NOBLEND);
    colorIndex += 3;
  }
}
