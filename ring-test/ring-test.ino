#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
 
#define PIN D7
#define STRIPSIZE 12

bool mode = 1;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPSIZE, PIN, NEO_GRB + NEO_KHZ800);
 
void setup() {
  strip.begin();
  strip.setBrightness(25);  // Lower brightness and save eyeballs!
  strip.show(); // Initialize all pixels to 'off'

  if(random(0, 1000) < 500 ) {
      mode = ! mode;
  }
}

int r = 0;
int g = 0;
int b = 0;

void loop() { 
  setColor(255, 0, 0);    // red
  setColor(0, 255, 0);    // green
  setColor(0, 0, 255);    // blue
  setColor(255, 255, 0);  // yellow
  setColor(80, 0, 80);    // purple
  setColor(0, 255, 255);  // aqua
}


void setColor(int red, int green, int blue) {
  while ( r != red || g != green || b != blue ) {
    if ( r < red ) r += 1;
    if ( r > red ) r -= 1;

    if ( g < green ) g += 1;
    if ( g > green ) g -= 1;

    if ( b < blue ) b += 1;
    if ( b > blue ) b -= 1;

    _setColor();

    if( mode ) {
      delay(10);
    } else {
      delay(30);
    }
  }
}
int i = 0;
#define PREV(x) ( (x)==0)?STRIPSIZE-1: (x)-1
void _setColor() {
  // for each pixel set color to red. Wait 100ms between each individual pixel.
  
      int p = PREV(i);
      int pp = PREV(p);
      strip.setPixelColor(i, strip.Color(g, r, b));
      if( ! mode ) {
        strip.setPixelColor(p, strip.Color(g/2, r/2, b/2));
      
        strip.setPixelColor(pp, strip.Color(0, 0, 0));
      }
      strip.show();
  if(++i ==  STRIPSIZE ) {
    i = 0;   
  }

}
