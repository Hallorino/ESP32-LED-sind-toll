#include <Arduino.h>
#include <fastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define LED_PIN 2
#define NUM_LEDS 8
#define BRIGHTNESS 64
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

CRGB leds[NUM_LEDS];

const char *ssid = "KEKSE";
const char *password = "huch";
AsyncWebServer server(80);

int modus;

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette);
void FillLEDswithColor(CRGB color);
void get_network_info();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, INPUT);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  modus = 0;

  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  get_network_info();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get repuest on /");
              modus++;
              request->send(200, "text/plain", "ok cool");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /rainbowstripe");
              modus = 0;
              request->send(200, "text/plain", "ok cool regebogen in cool");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /cloudcolors");
              modus = 1;
              request->send(200, "text/plain", "ok cool wolken");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /Lava");
              modus = 2;
              request->send(200, "text/plain", "Lava heiß");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /Ocean");
              modus = 3;
              request->send(200, "text/plain", "nicer Ocean Bruder");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /Forest");
              modus = 4;
              request->send(200, "text/plain", "coole Bäume");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /rainbow");
              modus = 5;
              request->send(200, "text/plain", "ok cool regebogen");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /Party");
              modus = 6;
              request->send(200, "text/plain", "machst du heute Parteyyyy?");
            });

  server.on("/rainbow", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("get request on /Heat");
              modus = 7;
              request->send(200, "text/plain", "huch ganz schön heiß hier");
            });

  server.begin();

  Serial.println("setup completed");
}

void loop()
{
  // put your main code here, to run repeatedly:

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */

  switch (modus)
  {
  case 0:
    FillLEDsFromPaletteColors(startIndex, RainbowStripeColors_p);
    break;

  case 1:
    FillLEDsFromPaletteColors(startIndex, CloudColors_p);
    break;

  case 2:
    FillLEDsFromPaletteColors(startIndex, LavaColors_p);
    break;

  case 3:
    FillLEDsFromPaletteColors(startIndex, OceanColors_p);
    break;

  case 4:
    FillLEDsFromPaletteColors(startIndex, ForestColors_p);
    break;

  case 5:
    FillLEDsFromPaletteColors(startIndex, RainbowColors_p);
    break;

  case 6:
    FillLEDsFromPaletteColors(startIndex, PartyColors_p);
    break;

  case 7:
    FillLEDsFromPaletteColors(startIndex, HeatColors_p);
    break;

  default:
    modus = 0;
    break;
  }

  //Schicke Farben zu LED Strip
  FastLED.show();

  //Warte ein bisschen
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors(uint8_t colorIndex, CRGBPalette16 palette)
{
  uint8_t brightness = 255;

  for (int i = 0; i < NUM_LEDS; ++i)
  {
    leds[i] = ColorFromPalette(palette, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
  }
}

void FillLEDswithColor(CRGB color)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

void get_network_info()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("[*] Network information for ");
    Serial.println(ssid);

    Serial.println("[+] BSSID : " + WiFi.BSSIDstr());
    Serial.print("[+] Gateway IP : ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("[+] Subnet Mask : ");
    Serial.println(WiFi.subnetMask());
    Serial.println((String) "[+] RSSI : " + WiFi.RSSI() + " dB");
    Serial.print("[+] ESP32 IP : ");
    Serial.println(WiFi.localIP());
  }
}
