#include <FastLED.h>

#define LED_PIN     D5
#define BUTTON_PIN  D7
#define NUM_LEDS    64
#define BRIGHTNESS  128
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define DEBUG 0
#define CIRCLES 0
#define RANDOM_FILL 0

#define UPDATES_PER_SECOND 20

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

CRGB leds[NUM_LEDS];

int DA = A0;
const int sampleWindow = 20; // mS. Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
unsigned int peakToPeakMax = 0;
unsigned int peakToPeakMin = 1024;
int mode = 1;

int buttonState = 0;         // variable for reading the pushbutton status

int circles[4][28] = {
  {27, 28, 35, 36, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {18, 19, 20, 21, 29, 37, 45, 44, 43, 42, 34, 26, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  {9, 10, 11, 12, 13, 14, 22, 30, 38, 46, 54, 53, 52, 51, 50, 49, 41, 33, 25, 17, -1, -1, -1, -1, -1, -1, -1, -1},
  {0, 1, 2, 3, 4, 5, 6, 7, 15, 23, 31, 39, 47, 55, 63, 62, 61, 60, 59, 58, 57, 56, 48, 40, 32, 24, 16, 8}
};

int brightness_map[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

int random_leds[64];

CRGBPalette16 myPal;
int iter = 120;
bool to_center = 0;
int animation;

CRGBPalette16 get_pal() {
  return CRGBPalette16(CRGB::Black, Wheel(iter));
}

void setup() {
  for (int i = 0; i < 64; i++) {
    random_leds[i] = i;
  }
  int tmp;
  int random_i;
  for (int i = 0; i < 64; i++) {
    random_i = random(0, 64 - i);
    tmp = random_leds[i];
    random_leds[i] = random_leds[random_i];
    random_leds[random_i] = tmp;
  }

  myPal = get_pal();

  delay(3000);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

  Serial.begin(115200);
}

void loop() {
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    mode++;
    if (mode == 5) {
      mode = 1;
    }
    Serial.println(mode);
    delay(1000);
  }

  if (DEBUG) {
    //    for (int heatindex = 0; heatindex < 256; heatindex++) {
    //      for (int j = 0; j < NUM_LEDS; j++) {
    ////        leds[j] = ColorFromPalette(myPal, heatindex);
    //        leds[j] = Wheel(heatindex);
    //      }
    //      FastLED.show();
    //      Serial.println(heatindex);
    //      Serial.println(Wheel(heatindex));
    //      delay(20);
    //    }
    for (int heatindex = 0; heatindex < 256; heatindex++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        //            leds[j] = ColorFromPalette(myPal, heatindex);
        //            leds[j] = ColorFromPalette(myPal, brightness_map[heatindex]);
        leds[j] = Wheel(heatindex);
      }
      FastLED.show();
      Serial.println(heatindex);
      delay(20);
    }
  } else if (CIRCLES) {
    for (int circle_num = 3; circle_num >= 0; circle_num--) {
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Black;
      }
      for (int j = 0; j < 28; j++) {
        if (circles[circle_num][j] != -1) {
          leds[circles[circle_num][j]] = CRGB::Red;
        }
      }
      FastLED.show();
      delay(100);
    }
  } else if (RANDOM_FILL) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j] = CRGB::Black;
    }
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[random_leds[j]] = CRGB::Red;
      FastLED.show();
      delay(10);
    }
  } else {
    if (mode == 1) {
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      
      FillLEDsFromPaletteColors( startIndex);
      
      FastLED.show();
      FastLED.delay(1000 / UPDATES_PER_SECOND);
    } else if (mode == 3) {
      CRGB color = CRGB(255, 0, 255);
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
      }
      FastLED.show();
    } else if (mode == 2) {
      unsigned long startMillis = millis(); // Start of sample window
      unsigned int peakToPeak = 0;   // peak-to-peak level

      unsigned int signalMax = 0;
      unsigned int signalMin = 1024;

      //     myPal = CRGBPalette16(CRGB::Black, Wheel(iter), Wheel((iter + 50) % 256));

      while (millis() - startMillis < sampleWindow)
      {
        sample = analogRead(DA);
        if ((sample < 1024) and (sample > 200))  // toss out spurious readings
        {
          if (sample > signalMax)
          {
            signalMax = sample;  // save just the max levels
          }

          if (sample < signalMin)
          {
            signalMin = sample;  // save just the min levels
          }
        }
      }
      peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

      if (peakToPeak > 1024) {
        peakToPeak = 1024;
      } else {
        if (peakToPeak > peakToPeakMax) {
          peakToPeakMax = peakToPeak;
        }
        if (peakToPeak < peakToPeakMin) {
          peakToPeakMin = peakToPeak;
        }
      }
      int heatindex = int((float(peakToPeak) / float(peakToPeakMax)) * 255.0);

      heatindex = brightness_map[heatindex];
      Serial.println(heatindex);
      //    Serial.print(' ');
      //    Serial.print(peakToPeakMin);
      //    Serial.print(' ');
      //    Serial.println(peakToPeakMax);

      if (heatindex > 200)  {
        iter += 10;
        iter = iter % 256;
        myPal = get_pal();

        animation = random(0, 7);

        if (animation == 0) {
          for (int j = 0; j < NUM_LEDS; j++) {
            leds[j] = CRGB::Black;
          }
          for (int j = 0; j < NUM_LEDS; j++) {
            leds[random_leds[j]] = Wheel(heatindex);
            FastLED.show();
            delay(7);
          }
        } else if (animation == 1) {
          if (to_center) {
            for (int circle_num = 3; circle_num >= 0; circle_num--) {
              for (int j = 0; j < NUM_LEDS; j++) {
                leds[j] = CRGB::Black;
              }
              for (int j = 0; j < 28; j++) {
                if (circles[circle_num][j] != -1) {
                  leds[circles[circle_num][j]] = Wheel(heatindex);
                }
              }
              FastLED.show();
              delay(80);
            }

            for (int circle_num = 0; circle_num < 4; circle_num++) {
              for (int j = 0; j < NUM_LEDS; j++) {
                leds[j] = CRGB::Black;
              }
              for (int j = 0; j < 28; j++) {
                if (circles[circle_num][j] != -1) {
                  leds[circles[circle_num][j]] = Wheel(heatindex);
                }
              }
              FastLED.show();
              delay(80);
            }

            to_center = false;
          } else {
            for (int circle_num = 0; circle_num < 4; circle_num++) {
              for (int j = 0; j < NUM_LEDS; j++) {
                leds[j] = Wheel((iter + 100) % 256);
              }
              for (int j = 0; j < 28; j++) {
                if (circles[circle_num][j] != -1) {
                  leds[circles[circle_num][j]] = Wheel(heatindex);
                }
              }
              FastLED.show();
              delay(80);
            }

            for (int circle_num = 3; circle_num >= 0; circle_num--) {
              for (int j = 0; j < NUM_LEDS; j++) {
                leds[j] = Wheel((iter + 50) % 256);
              }
              for (int j = 0; j < 28; j++) {
                if (circles[circle_num][j] != -1) {
                  leds[circles[circle_num][j]] = Wheel(heatindex);
                }
              }
              FastLED.show();
              delay(80);
            }
            to_center = true;
          }
        }
      }

      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = ColorFromPalette(myPal, heatindex);
      }
      FastLED.show();

      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Black;
      }
    } else if (mode == 1) {
      unsigned long startMillis = millis(); // Start of sample window
      unsigned int peakToPeak = 0;   // peak-to-peak level

      unsigned int signalMax = 0;
      unsigned int signalMin = 1024;

      //     myPal = CRGBPalette16(CRGB::Black, Wheel(iter), Wheel((iter + 50) % 256));

      while (millis() - startMillis < sampleWindow)
      {
        sample = analogRead(DA);
        if ((sample < 1024) and (sample > 200))  // toss out spurious readings
        {
          if (sample > signalMax)
          {
            signalMax = sample;  // save just the max levels
          }

          if (sample < signalMin)
          {
            signalMin = sample;  // save just the min levels
          }
        }
      }
      peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

      if (peakToPeak > 1024) {
        peakToPeak = 1024;
      } else {
        if (peakToPeak > peakToPeakMax) {
          peakToPeakMax = peakToPeak;
        }
        if (peakToPeak < peakToPeakMin) {
          peakToPeakMin = peakToPeak;
        }
      }
      int heatindex = int((float(peakToPeak) / float(peakToPeakMax)) * 255.0);

      heatindex = brightness_map[heatindex];
      Serial.println(heatindex);

      leds[63] = Wheel(heatindex);
      leds[62] = Wheel(heatindex);
      leds[55] = Wheel(heatindex);
      leds[54] = Wheel(heatindex);

      for (int j = 0; j < 8; j++) {
        leds[j] = ColorFromPalette(myPal, heatindex);
        leds[j * 8] = ColorFromPalette(myPal, heatindex);
      }
      FastLED.show();

      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Black;
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
CRGB Wheel(int WheelPos) {
  if (WheelPos < 85) {
    return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 50;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
