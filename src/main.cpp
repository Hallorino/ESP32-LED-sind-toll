#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <WiFi.h>
#include <secret.h>

#define LED_PIN 2
#define NUM_LEDS 300
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

AsyncWebServer server(80);

typedef struct {
  const String name;
  CRGBPalette16 palette;
} PaletteWithName;

CRGBPalette16 customPalette = CRGBPalette16(CRGB::Red);

PaletteWithName palettes[] = {
    {"Rainbow", CRGBPalette16(RainbowColors_p)},
    {"RainbowStripes", CRGBPalette16(RainbowStripeColors_p)},
    {"Cloud", CRGBPalette16(CloudColors_p)},
    {"Lava", CRGBPalette16(LavaColors_p)},
    {"Ocean", CRGBPalette16(OceanColors_p)},
    {"Forest", CRGBPalette16(ForestColors_p)},
    {"Party", CRGBPalette16(PartyColors_p)},
    {"Heat", CRGBPalette16(HeatColors_p)},
    {"Custom", customPalette}};

uint8_t palettesCount = sizeof(palettes) / sizeof(palettes[0]);

int currentMode = 0;
int currentPalette = 0;
int currentStep = 3;
long currentColor = 16711908;
String currentColorHex = "";
boolean hasBlend = true;
uint8_t brightness = 64;
uint16_t fps = 100;

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette);

String getSettingsAsJson() {
  StaticJsonDocument<384> doc;
  doc["currentMode"] = currentMode;
  doc["currentPalette"] = palettes[currentPalette].name;
  doc["currentColor"] = currentColorHex;
  doc["currentStep"] = currentStep;
  doc["hasBlend"] = hasBlend;
  doc["brightness"] = brightness;
  doc["fps"] = fps;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String getAllpalettesAsJson() {
  StaticJsonDocument<512> doc;
  for (int i = 0; i < palettesCount; i++) {
    JsonObject mode = doc.createNestedObject();
    mode["name"] = palettes[i].name;
  }
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
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

  server.on("/palettes", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("get repuest on /palettes");
    request->send(200, "application/json", getAllpalettesAsJson());
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("get request on /settings");
    request->send(200, "application/json", getSettingsAsJson());
  });

  // PATCH /settings
  AsyncCallbackJsonWebHandler *ledStripPatchHandler =
      new AsyncCallbackJsonWebHandler(
          "/settings", [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (request->method() == HTTP_PATCH) {
              StaticJsonDocument<128> data;
              if (json.is<JsonObject>()) {
                data = json.as<JsonObject>();
                boolean foundMode = true;
                if (data["currentPalette"]) {
                  foundMode = false;
                  String mode = data["currentPalette"];
                  for (int i = 0; i < palettesCount; i++) {
                    if (mode.compareTo(palettes[i].name) == 0) {
                      currentPalette = i;
                      foundMode = true;
                      break;
                    }
                  }
                }

                hasBlend = data["hasBlend"];
                currentStep = data["currentStep"];

                if (data["currentColor"]) {
                  // TODO Input validation
                  currentColorHex = data["currentColor"].as<String>();
                  currentColor = strtol(data["currentColor"], NULL, 16);
                }

                currentMode = data["currentMode"];

                if (data["brightness"]) {
                  // TODO Input validation
                  brightness = data["brightness"];
                }
                if (data["fps"]) {
                  // TODO Input validation
                  fps = data["fps"];
                }

                if (foundMode)
                  request->send(200, "application/json", getSettingsAsJson());
                else
                  request->send(400, "application/json",
                                "{\"message\":\"Bad Request mode not found\"}");
              } else {
                request->send(400, "application/json",
                              "{\"message\":\"Bad Request no Json found\"}");
              }
            } else {
              notFound(request);
            }
          });
  server.addHandler(ledStripPatchHandler);

  // PATCH /modes/custom
  AsyncCallbackJsonWebHandler *customModePatchHandler =
      new AsyncCallbackJsonWebHandler(
          "/palettes/custom", [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (request->method() == HTTP_PATCH) {
              StaticJsonDocument<384> data;
              if (json.is<JsonArray>()) {
                JsonArray array = json.as<JsonArray>();
                long colorArray[16];
                for (int i = 0; i < 16; i++) {
                  String hexColor = array[i].as<String>();
                  colorArray[i] = strtol(hexColor.c_str(), NULL, 16);
                }
                palettes[8].palette = CRGBPalette16(
                    colorArray[0], colorArray[1], colorArray[2], colorArray[3],
                    colorArray[4], colorArray[5], colorArray[6], colorArray[7],
                    colorArray[8], colorArray[9], colorArray[10],
                    colorArray[11], colorArray[12], colorArray[13],
                    colorArray[14], colorArray[15]);

                request->send(200, "application/json", getSettingsAsJson());

              } else {
                request->send(400, "application/json",
                              "{\"message\":\"Bad Request no Json found\"}");
              }

            } else {
              notFound(request);
            }
          });
  server.addHandler(customModePatchHandler);

  // CORS Stuff
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "PUT,POST,PATCH,GET,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "600");
  server.on("/settings", HTTP_OPTIONS,
            [](AsyncWebServerRequest *request) { request->send(204); });
    server.on("/palettes/custom", HTTP_OPTIONS,
            [](AsyncWebServerRequest *request) { request->send(204); });

  server.begin();

  Serial.println("setup completed");
}

void loop() {
  // put your main code here, to run repeatedly:

  switch (currentMode) {
    {
      case 0:
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; /* motion speed */

        FillLEDsFromPaletteColors(startIndex, palettes[currentPalette].palette);
        break;

      case 1:
        // Fill LEDS with a color
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = currentColor;
        }
        FastLED.setBrightness(brightness);
        break;

      default:
        break;
    }
  }

  // Schicke Farben zu LED Strip
  FastLED.show();

  // Warte ein bisschen
  FastLED.delay(1000 / fps);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(palette, colorIndex, brightness,
                               hasBlend ? LINEARBLEND : NOBLEND);
    colorIndex += currentStep;
  }
}
