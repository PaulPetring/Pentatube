#include "./Pentatube.h"

const int SRCLR   = 4; //ATMEL 15
const int SRCLK   = 18; //ATMEL 16
const int RCLK    = 5; //2; //ATMEL 14
const int SERIN   = 23; //ATMEL 19 MOSI
//const int MISO = 24; //ATMEL 19 MOSI

Pentatube tube = Pentatube(SRCLR, SRCLK, RCLK, SERIN);

byte c = 0;

void setup() {
  tube.begin();

  for (int i = 0; i < 8; i++) {
    tube.setPixelColor(i, tube.Color(255, 128, 0));
  }
  tube.show(1000);
}


void loop() {
  colorWipe(tube.Color(255, 0, 0), 250); // Red
  colorWipe(tube.Color(0, 255, 0), 250); // Green
  colorWipe(tube.Color(0, 0, 255), 250); // Blue
  colorWipe(tube.Color(0, 255, 255), 250);  //cyan
  colorWipe(tube.Color(255, 0, 255), 250);  //pink
  colorWipe(tube.Color(255, 255, 0), 250);  //yellow
  theaterChase(tube.Color(255, 255, 255), 25); // White
  theaterChase(tube.Color(255, 0, 0), 25); // Red
  theaterChase(tube.Color(0, 0, 255), 25); // Blue
  theaterChase(tube.Color(0, 255, 255), 25);  //cyan
  theaterChase(tube.Color(255, 0, 255), 25);  //pink
  theaterChase(tube.Color(255, 255, 0), 25);  //yellow
  rainbowCycle(100);
  rainbow(100);
  theaterChaseRainbow(25);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < 8; i++) {
      tube.setPixelColor(i, Wheel((i + j) & 255));
    }
    tube.show(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < tube.numPixels(); i++) {
      tube.setPixelColor(i, Wheel(((i * 256 / tube.numPixels()) + j) & 255));
    }
    tube.show(wait);
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < tube.numPixels(); i++) {
    tube.setPixelColor(i, c);
    tube.show(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return tube.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return tube.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return tube.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<50; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < tube.numPixels(); i=i+3) {
        tube.setPixelColor(i+q, c);    //turn every third pixel on
      }
      tube.show(wait);
      for (uint16_t i=0; i < tube.numPixels(); i=i+3) {
        tube.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < tube.numPixels(); i=i+3) {
        tube.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      tube.show(wait);

      for (uint16_t i=0; i < tube.numPixels(); i=i+3) {
        tube.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}
