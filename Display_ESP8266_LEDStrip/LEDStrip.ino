
#include "FastLED.h"

#define DATA_PIN    4 //GPIO2
#define LED_TYPE    SK6812
#define COLOR_ORDER RGB
#define NUM_LEDS    56
#define BRIGHTNESS  250
#define FRAMES_PER_SECOND 5

uint8_t pps = 4;            // number of Pixels Per Segment
CHSV segON10(96, 255, 255); // color of 10s digit segments NOT TURNED RED
CHSV segON(96, 255, 255);   // color of 1s digit segments

CRGBArray<NUM_LEDS> leds;     /* CRGB leds[NUM_LEDS];  <--not using this.  Using CRGBArray instead. */

// Name segments (based on layout in link above) and define pixel ranges.
CRGBSet segA(  leds(pps * 0,  pps - 1+(pps * 0)  ));
CRGBSet segB(  leds(pps * 1,  pps - 1+(pps * 1)  ));
CRGBSet segC(  leds(pps * 2,  pps - 1+(pps * 2)  ));
CRGBSet segD(  leds(pps * 3,  pps - 1+(pps * 3)  ));
CRGBSet segE(  leds(pps * 4,  pps - 1+(pps * 4)  ));
CRGBSet segF(  leds(pps * 5,  pps - 1+(pps * 5)  ));
CRGBSet segG(  leds(pps * 6,  pps - 1+(pps * 6)  ));
//-----------------------------------------------------------------------------------------------------------

#define PORT  "1234"
String ssid = "Thesis";
String IPADDRESS  = "192.168.1.4";
String PASSWORD = "12345678";
int Na_received_na_Time = 24;
int count = 24;

void LEDStrip_setup() 
{
  pinMode(DATA_PIN, OUTPUT);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
}

void LEDStrip_loop()
{
  FastLED.delay(1);
}

void setSegments(uint8_t count) {
  // Based on the current count set number segments on or off
  uint8_t c1 = 0;                  // Variable to store 1s digit
  uint8_t c10 = 0;                 // Variable to store 10s digit
  uint8_t c;
  CHSV segCOLOR(0, 0, 0);

  if (count > 9) {                 // Split out 1s and 10s digits if count is greater then 9
    c1 = count % 10;
    c10 = count / 10;
  } else {
    c1 = count;
    c10 = 0;
  }
  
  // Operate on 1s digit segments first, shift them over, and then do the 10s digit segments
  for (uint8_t i = 0; i < 2; i++) {
    if (i == 0) {
      c = c1;
      segCOLOR = segON;
    } else {
      c = c10;
      segCOLOR = segON10;
    }

    segA = segB = segC = segD = segE = segF = segG = CRGB::Black;  // Initially set segments off

    if (c == 0) {
      segB = segC = segD = segE = segF = segG = segCOLOR;
    }
    if (c == 1) {
      segB = segG = segCOLOR;
    }
    if (c == 2) {
      segA = segB = segC = segE = segF = segCOLOR;
    }
    if (c == 3) {
      segA = segB = segC = segF = segG = segCOLOR;
    }
    if (c == 4) {
      segA = segB = segD = segG = segCOLOR;
    }
    if (c == 5) {
      segA = segC = segD = segF = segG = segCOLOR;
    }
    if (c == 6) {
      segA = segC = segD = segE = segF = segG = segCOLOR;
    }
    if (c == 7) {
      segB = segC = segG = segCOLOR;
    }
    if (c == 8) {
      segA = segB = segC = segD = segE = segF = segG = segCOLOR;
    }
    if (c == 9) {
      segA = segB = segC = segD = segF = segG = segCOLOR;
    }

    if (i == 0) {  // Shift segments over to 1s digit display area
      for (uint8_t p = 0; p < (7 * pps); p++) {
        leds[p + (7 * pps)] = leds[p];
      }
    }
  }
}//end setSegments

