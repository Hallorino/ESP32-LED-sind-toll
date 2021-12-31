#include <Arduino.h>
// *************************
// ** LEDEffect Functions **
// *************************

void RGBLoop();

void FadeInOut(byte red, byte green, byte blue);

void Strobe(byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int EndPause);

void HalloweenEyes(byte red, byte green, byte blue,
                   int EyeWidth, int EyeSpace,
                   boolean Fade, int Steps, int FadeDelay,
                   int EndPause);


void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

void NewKITT(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

void Twinkle(byte red, byte green, byte blue, int Count, int SpeedDelay, boolean OnlyOne);

void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne);

void Sparkle(byte red, byte green, byte blue, int SpeedDelay);

void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay);

void RunningLights(byte red, byte green, byte blue, int WaveDelay);

void colorWipe(byte red, byte green, byte blue, int SpeedDelay);

void rainbowCycle(int SpeedDelay);

// used by rainbowCycle and theaterChaseRainbow
byte * Wheel(byte WheelPos);

void theaterChase(byte red, byte green, byte blue, int SpeedDelay);

void theaterChaseRainbow(int SpeedDelay);

void Fire(int Cooling, int Sparking, int SpeedDelay);

void setPixelHeatColor (int Pixel, byte temperature);

void BouncingColoredBalls(int BallCount, byte colors[][3], boolean continuous);

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay);

// used by meteorrain
void fadeToBlack(int ledNo, byte fadeValue);


// ***************************************
// ** FastLed/NeoPixel Common Functions **
// ***************************************

// Apply LED color changes
void showStrip();

// Set a LED color (not yet visible)
void setPixel(int Pixel, byte red, byte green, byte blue);

// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue);