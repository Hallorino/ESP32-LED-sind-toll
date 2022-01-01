#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <WiFi.h>

#include "ledEffects.h"
#include "secret.h"

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

typedef struct {
  const String name;
  bool hasCustomColor;
  void (*effect)();
} EffectWithName;

EffectWithName effects[] = {
    {"FadeInOut", false, &FadeInOutEffect},
    {"Strobe", true, &StrobeEffect},
    {"CylonBounce", true, &CylonBounceEffect},
    {"NewKITT", true, &NewKITTEffect},
    {"Twinkle", true, &TwinkleEffect},
    {"TwinkleRandom", false, &TwinkleRandomEffect},
    {"Sparkle", true, &SparkleEffect},
    {"SnowSparkle", false, &SnowSparkleEffect},
    {"RunningLights", false, &RunningLightsEffect},
    {"colorWipe", true, &colorWipeEffect},
    {"theaterChase", true, &theaterChaseEffect},
    {"theaterChaseRainbow", false, &theaterChaseRainbowEffect},
    {"meteorRain", true, &meteorRainEffect}};

uint8_t effectsCount = sizeof(effects) / sizeof(effects[0]);


int currentMode = 0;
int currentPalette = 0;
int currentStep = 3;
int currentEffect = 0;
long currentColor = 16711908;
String currentColorHex = "0xFF00E4";
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
  doc["currentEffect"] = effects[currentEffect].name;
  doc["hasBlend"] = hasBlend;
  doc["brightness"] = brightness;
  doc["fps"] = fps;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String getAllPalettesAsJson() {
  StaticJsonDocument<512> doc;
  for (int i = 0; i < palettesCount; i++) {
    JsonObject mode = doc.createNestedObject();
    mode["name"] = palettes[i].name;
  }
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String getAllEffectsAsJson() {
  StaticJsonDocument<1024> doc;
  for (int i = 0; i < effectsCount; i++) {
    JsonObject effect = doc.createNestedObject();
    effect["name"] = effects[i].name;
    effect["hasCustomColor"] = effects[i].hasCustomColor;
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
    request->send(200, "application/json", getAllPalettesAsJson());
  });

    server.on("/effects", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("get repuest on /palettes");
    request->send(200, "application/json", getAllEffectsAsJson());
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

                // Search Mode
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

                // Search Effect
                boolean foundEffect = true;
                if (data["currentEffect"]) {
                  foundEffect = false;
                  String effect = data["currentEffect"];
                  for (int i = 0; i < effectsCount; i++) {
                    if (effect.compareTo(effects[i].name) == 0) {
                      currentEffect = i;
                      foundEffect = true;
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

                if (foundMode && foundEffect)
                  request->send(200, "application/json", getSettingsAsJson());
                else
                  request->send(400, "application/json",
                                "{\"message\":\"Bad Request mode or effect not found\"}");
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
          "/palettes/custom",
          [](AsyncWebServerRequest *request, JsonVariant &json) {
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
        // Schicke Farben zu LED Strip
        FastLED.show();

        // Warte ein bisschen
        FastLED.delay(1000 / fps);
        break;

      case 1:
        // Fill LEDS with a color
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = currentColor;
        }
        FastLED.setBrightness(brightness);
        // Schicke Farben zu LED Strip
        FastLED.show();

        // Warte ein bisschen
        FastLED.delay(1000 / fps);
        break;
      case 2:
        effects[currentEffect].effect();
        break;

      default:
        break;
    }
  }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette(palette, colorIndex, brightness,
                               hasBlend ? LINEARBLEND : NOBLEND);
    colorIndex += currentStep;
  }
}

// *************************
// ** LEDEffect Starter Functions **
// *************************

void FadeInOutEffect() {
  // FadeInOut - Color (red, green. blue)
  FadeInOut(CRGB(0xff, 0x00, 0x00));  // red
  FadeInOut(CRGB(0xff, 0xff, 0xff));  // white
  FadeInOut(CRGB(0x00, 0x00, 0xff));  // blue
}

void StrobeEffect() {
  // Strobe - Color (red, green, blue), number of flashes, flash speed, end
  // pause
  Strobe(CRGB(currentColor), 10, 50, 1000);
}

void CylonBounceEffect() {
  // CylonBounce - Color (red, green, blue), eye size, speed delay, end
  // pause
  CylonBounce(CRGB(currentColor), 4, 10, 50);
}

void NewKITTEffect() {
  // NewKITT - Color (red, green, blue), eye size, speed delay, end pause
  NewKITT(CRGB(currentColor), 8, 10, 50);
}

void TwinkleEffect() {
  // Twinkle - Color (red, green, blue), count, speed delay, only one
  // twinkle (true/false)
  Twinkle(CRGB(currentColor), 10, 100, false);
}

void TwinkleRandomEffect() {
  // TwinkleRandom - twinkle count, speed delay, only one (true/false)
  TwinkleRandom(20, 100, false);
}

void SparkleEffect() {
  // Sparkle - Color (red, green, blue), speed delay
  Sparkle(CRGB(currentColor), 0);
}

void SnowSparkleEffect() {
  // SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
  SnowSparkle(CRGB(0x10, 0x10, 0x10), 20, random(100, 1000));
}

void RunningLightsEffect() {
  // Running Lights - Color (red, green, blue), wave dealy
  RunningLights(CRGB(0xff, 0x00, 0x00), 50);  // red
  RunningLights(CRGB(0xff, 0xff, 0xff), 50);  // white
  RunningLights(CRGB(0x00, 0x00, 0xff), 50);  // blue
}

void colorWipeEffect() {
  // colorWipe - Color (red, green, blue), speed delay
  colorWipe(CRGB(currentColor), 50);
  colorWipe(CRGB(0x00, 0x00, 0x00), 50);
}

void theaterChaseEffect() {
  // theatherChase - Color (red, green, blue), speed delay
  theaterChase(CRGB(currentColor), 50);
}

void theaterChaseRainbowEffect() {
  // theaterChaseRainbow - Speed delay
  theaterChaseRainbow(50);
}

void meteorRainEffect() {
  // meteorRain - Color (red, green, blue), meteor size, trail decay, random
  // trail decay (true/false), speed delay
  meteorRain(CRGB(currentColor), 10, 64, true, 30);
}

// *************************
// ** LEDEffect Functions **
// *************************
void FadeInOut(CRGB color) {
  float r, g, b;

  for (int k = 0; k < 256; k = k + 1) {
    r = (k / 256.0) * color.red;
    g = (k / 256.0) * color.green;
    b = (k / 256.0) * color.blue;
    setAll(CRGB(r, g, b));
    showStrip();
  }

  for (int k = 255; k >= 0; k = k - 2) {
    r = (k / 256.0) * color.red;
    g = (k / 256.0) * color.green;
    b = (k / 256.0) * color.blue;
    setAll(CRGB(r, g, b));
    showStrip();
  }
}

void Strobe(CRGB color, int StrobeCount, int FlashDelay, int EndPause) {
  for (int j = 0; j < StrobeCount; j++) {
    setAll(color);
    showStrip();
    delay(FlashDelay);
    setAll(CRGB(0, 0, 0));

    showStrip();
    delay(FlashDelay);
  }

  delay(EndPause);
}

void CylonBounce(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
    setAll(CRGB(0, 0, 0));

    setPixel(i, CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, color);
    }
    setPixel(i + EyeSize + 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
    setAll(CRGB(0, 0, 0));

    setPixel(i, CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, color);
    }
    setPixel(i + EyeSize + 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);
}

void NewKITT(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay) {
  RightToLeft(color, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(color, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(color, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(color, EyeSize, SpeedDelay, ReturnDelay);
  LeftToRight(color, EyeSize, SpeedDelay, ReturnDelay);
  RightToLeft(color, EyeSize, SpeedDelay, ReturnDelay);
  OutsideToCenter(color, EyeSize, SpeedDelay, ReturnDelay);
  CenterToOutside(color, EyeSize, SpeedDelay, ReturnDelay);
}

// used by NewKITT
void CenterToOutside(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for (int i = ((NUM_LEDS - EyeSize) / 2); i >= 0; i--) {
    setAll(CRGB(0, 0, 0));

    setPixel(i, CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, color);
    }
    setPixel(i + EyeSize + 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));

    setPixel(NUM_LEDS - i,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(NUM_LEDS - i - j, color);
    }
    setPixel(NUM_LEDS - i - EyeSize - 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));

    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void OutsideToCenter(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for (int i = 0; i <= ((NUM_LEDS - EyeSize) / 2); i++) {
    setAll(CRGB(0, 0, 0));

    setPixel(i, CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, color);
    }
    setPixel(i + EyeSize + 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));

    setPixel(NUM_LEDS - i,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(NUM_LEDS - i - j,
               CRGB(color.red / 10, color.green / 10, color.blue / 10));
    }
    setPixel(NUM_LEDS - i - EyeSize - 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));

    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void LeftToRight(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
    setAll(CRGB(0, 0, 0));

    setPixel(i, CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, color);
    }
    setPixel(i + EyeSize + 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void RightToLeft(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
    setAll(CRGB(0, 0, 0));

    setPixel(i, CRGB(color.red / 10, color.green / 10, color.blue / 10));
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, color);
    }
    setPixel(i + EyeSize + 1,
             CRGB(color.red / 10, color.green / 10, color.blue / 10));
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

void Twinkle(CRGB color, int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(CRGB(0, 0, 0));

  for (int i = 0; i < Count; i++) {
    setPixel(random(NUM_LEDS), color);
    showStrip();
    delay(SpeedDelay);
    if (OnlyOne) {
      setAll(CRGB(0, 0, 0));
    }
  }

  delay(SpeedDelay);
}

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(CRGB(0, 0, 0));

  for (int i = 0; i < Count; i++) {
    setPixel(random(NUM_LEDS),
             CRGB(random(0, 255), random(0, 255), random(0, 255)));
    showStrip();
    delay(SpeedDelay);
    if (OnlyOne) {
      setAll(CRGB(0, 0, 0));
    }
  }

  delay(SpeedDelay);
}

void Sparkle(CRGB color, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, color);
  showStrip();
  delay(SpeedDelay);
  setPixel(Pixel, CRGB(0, 0, 0));
}

void SnowSparkle(CRGB color, int SparkleDelay, int SpeedDelay) {
  setAll(color);

  int Pixel = random(NUM_LEDS);
  setPixel(Pixel, CRGB(0xff, 0xff, 0xff));
  showStrip();
  delay(SparkleDelay);
  setPixel(Pixel, color);
  showStrip();
  delay(SpeedDelay);
}

void RunningLights(CRGB color, int WaveDelay) {
  int Position = 0;

  for (int i = 0; i < NUM_LEDS * 2; i++) {
    Position++;  // = 0; //Position + Rate;
    for (int i = 0; i < NUM_LEDS; i++) {
      // sine wave, 3 offset waves make a rainbow!
      // float level = sin(i+Position) * 127 + 128;
      // setPixel(i,level,0,0);
      // float level = sin(i+Position) * 127 + 128;
      setPixel(i, CRGB(((sin(i + Position) * 127 + 128) / 255) * color.red,
                       ((sin(i + Position) * 127 + 128) / 255) * color.green,
                       ((sin(i + Position) * 127 + 128) / 255) * color.blue));
    }

    showStrip();
    delay(WaveDelay);
  }
}

void colorWipe(CRGB color, int SpeedDelay) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    setPixel(i, color);
    showStrip();
    delay(SpeedDelay);
  }
}

// used by rainbowCycle and theaterChaseRainbow
byte *Wheel(byte WheelPos) {
  static byte c[3];

  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;
}

void theaterChase(CRGB color, int SpeedDelay) {
  for (int j = 0; j < 10; j++) {  // do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < NUM_LEDS; i = i + 3) {
        setPixel(i + q, color);  // turn every third pixel on
      }
      showStrip();

      delay(SpeedDelay);

      for (int i = 0; i < NUM_LEDS; i = i + 3) {
        setPixel(i + q, CRGB(0, 0, 0));  // turn every third pixel off
      }
    }
  }
}

void theaterChaseRainbow(int SpeedDelay) {
  byte *c;

  for (int j = 0; j < 256; j++) {  // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < NUM_LEDS; i = i + 3) {
        c = Wheel((i + j) % 255);
        setPixel(i + q,
                 CRGB(*c, *(c + 1), *(c + 2)));  // turn every third pixel on
      }
      showStrip();

      delay(SpeedDelay);

      for (int i = 0; i < NUM_LEDS; i = i + 3) {
        setPixel(i + q, CRGB(0, 0, 0));  // turn every third pixel off
      }
    }
  }
}

void meteorRain(CRGB color, byte meteorSize, byte meteorTrailDecay,
                boolean meteorRandomDecay, int SpeedDelay) {
  setAll(CRGB(0, 0, 0));

  for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++) {
    // fade brightness all LEDs one step
    for (int j = 0; j < NUM_LEDS; j++) {
      if ((!meteorRandomDecay) || (random(10) > 5)) {
        leds[j].fadeToBlackBy(meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++) {
      if ((i - j < NUM_LEDS) && (i - j >= 0)) {
        setPixel(i - j, color);
      }
    }

    showStrip();
    delay(SpeedDelay);
  }
}

// ***************************************
// ** FastLed Common Functions **
// ***************************************

// Apply LED color changes
void showStrip() {
  // FastLED
  FastLED.show();
}

// Set a LED color (not yet visible)
void setPixel(int Pixel, CRGB color) {
  // FastLED
  leds[Pixel] = color;
}

// Set all LEDs to a given color and apply it (visible)
void setAll(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, color);
  }
  showStrip();
}