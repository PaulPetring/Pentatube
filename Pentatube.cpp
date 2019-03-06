#include "./Pentatube.h"

void Pentatube::show(int duration) {

  unsigned long start = millis();

  while (start + duration > millis())
  {
    byte r = 0;
    byte g = 0;
    byte b = 0;
    uint8_t rgb[3];

    for (int i = 0; i <= 7; i++) {
      r = r << 1;
      g = g << 1;
      b = b << 1;

      //TODO more even distribution
      if (_colors[i][0] > _c) { 
        r++;
      }
      if (_colors[i][1] > _c) {
        g++;
      }
      if (_colors[i][2] > _c) {
        b++;
      }
    }

    rgb[0] = b;
    rgb[1] = g;
    rgb[2] = r;

    SPI.transfer(rgb, 3);

    if (_c++ > 255) {
      _c = 0;
    }
  }
}

Pentatube::Pentatube() {
  Pentatube(4, 18, 5, 23);
}

Pentatube::Pentatube(int SRCLR, int SRCLK, int RCLK, int SERIN) {
  _SRCLR = SRCLR;
  _SRCLK = SRCLK;
  _RCLK = RCLK;
  _SERIN = SERIN;
}

Pentatube::~Pentatube() {
  //bla
}

void Pentatube::begin() {
  pinMode (_SRCLR, OUTPUT);
  //pinMode (RCLK, OUTPUT);

  SPI.begin(_SRCLK, 24, _SERIN, _RCLK);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setHwCs(true);
  SPI.setFrequency(79999999); //80MhZ
  //slower SPI transfer by a devider
  //SPI.setClockDivider(SPI_CLOCK_DIV128);
  digitalWrite (_SRCLR, HIGH);
}

void Pentatube::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  _colors[n][0] = r;
  _colors[n][1] = g;
  _colors[n][2] = b;
}

void Pentatube::setPixelColor(uint16_t n, uint32_t c) {
  _colors[n][0] = (uint8_t)(c >> 16),
                  _colors[n][1] = (uint8_t)(c >>  8),
                                  _colors[n][2] = (uint8_t)c;
}

uint32_t Pentatube::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

int Pentatube::numPixels() {
  return 8;
}
