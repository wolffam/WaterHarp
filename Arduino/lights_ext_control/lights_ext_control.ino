// Upload to Metro Mini clone as Arduino Uno

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(204, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(13, OUTPUT); // LED Output
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

int last = 0; // only update the strip on a transition - this keeps track of last state

void loop() 
{
  if((last == 0) && digitalRead(2))
  {
    strip.fill(strip.Color(255, 255, 255), 0, 204);
    strip.show();
    last = 1;
  }

  if((last == 1) && !digitalRead(2))
  {
    strip.fill(strip.Color(0, 0, 0), 0, 204);
    strip.show();
    last = 0;
  }

  // Update the LED for debug purposes
  if(digitalRead(2))
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13, LOW);
  }
}
