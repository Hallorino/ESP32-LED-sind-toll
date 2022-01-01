#include <Arduino.h>


// *************************
// ** LEDEffect Starter Functions **
// *************************
void FadeInOutEffect();
void StrobeEffect();
void CylonBounceEffect();
void NewKITTEffect();
void TwinkleEffect();
void TwinkleRandomEffect();
void SparkleEffect();
void SnowSparkleEffect();
void RunningLightsEffect();
void colorWipeEffect();
void theaterChaseEffect();
void theaterChaseRainbowEffect();
void meteorRainEffect();

// *************************
// ** LEDEffect Functions **
// *************************

void FadeInOut(CRGB color);

void Strobe(CRGB color, int StrobeCount, int FlashDelay, int EndPause);

void CylonBounce(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay);

void NewKITT(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void CenterToOutside(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void OutsideToCenter(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void LeftToRight(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void RightToLeft(CRGB color, int EyeSize, int SpeedDelay, int ReturnDelay);

void Twinkle(CRGB color, int Count, int SpeedDelay, boolean OnlyOne);

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne);

void Sparkle(CRGB color, int SpeedDelay);

void SnowSparkle(CRGB color, int SparkleDelay, int SpeedDelay);

void RunningLights(CRGB color, int WaveDelay);

void colorWipe(CRGB color, int SpeedDelay);

// used by rainbowCycle and theaterChaseRainbow
byte * Wheel(byte WheelPos);

void theaterChase(CRGB color, int SpeedDelay);

void theaterChaseRainbow(int SpeedDelay);

void meteorRain(CRGB color, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay);

// used by meteorrain
void fadeToBlack(int ledNo, byte fadeValue);


// ***************************************
// ** FastLed/NeoPixel Common Functions **
// ***************************************

// Apply LED color changes
void showStrip();

// Set a LED color (not yet visible)
void setPixel(int Pixel, CRGB color);

// Set all LEDs to a given color and apply it (visible)
void setAll(CRGB color);