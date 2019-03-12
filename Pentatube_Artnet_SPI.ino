#include "./Pentatube.h"
#include <WiFi.h>
#include <ArtnetWifi.h>

const char* ssid = "ssid";
const char* password = "passwd";

ArtnetWifi artnet;
const int numLeds = 8; // change for your setup
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)
const int startUniverse = 1; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

const int SRCLR   = 4; //ATMEL 15
const int SRCLK   = 18; //ATMEL 16
const int RCLK    = 5; //2; //ATMEL 14
const int SERIN   = 23; //ATMEL 19 MOSI
//const int MISO = 24; //ATMEL 19 MOSI

Pentatube tube = Pentatube(SRCLR, SRCLK, RCLK, SERIN);

byte c = 0;

void setup() {
  Serial.begin(115200);
  tube.begin();

  for (int i = 0; i < 8; i++) {
    tube.setPixelColor(i, tube.Color(255, 0, 0));
  }
  tube.show(1000);

}



void loop() {

  //colorWipe(tube.Color(255, 0, 0), 25); // Red
  //colorWipe(tube.Color(0, 255, 0), 25); // Green
  //colorWipe(tube.Color(0, 0, 255), 25); // Blue
  //colorWipe(tube.Color(0, 255, 255), 25);  //cyan
  //colorWipe(tube.Color(255, 0, 255), 25);  //pink
  //colorWipe(tube.Color(255, 255, 0), 25);  //yellow
  //theaterChase(tube.Color(255, 255, 255), 25); // White
  //theaterChase(tube.Color(255, 0, 0), 25); // Red
  //theaterChase(tube.Color(0, 0, 255), 25); // Blue
  //theaterChase(tube.Color(0, 255, 255), 25);  //cyan
  //theaterChase(tube.Color(255, 0, 255), 25);  //pink
  //theaterChase(tube.Color(255, 255, 0), 25);  //yellow
  rainbowCycle(10);
  //rainbow(10);
  //theaterChaseRainbow(25);
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
  for (int j = 0; j < 50; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < tube.numPixels(); i = i + 3) {
        tube.setPixelColor(i + q, c);  //turn every third pixel on
      }
      tube.show(wait);
      for (uint16_t i = 0; i < tube.numPixels(); i = i + 3) {
        tube.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < tube.numPixels(); i = i + 3) {
        tube.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      tube.show(wait);

      for (uint16_t i = 0; i < tube.numPixels(); i = i + 3) {
        tube.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
