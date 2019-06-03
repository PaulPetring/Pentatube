#include "./Pentatube.h"


/*
#if defined(ESP8266)
// ESP8266 show() is external to enforce ICACHE_RAM_ATTR execution
extern "C" void ICACHE_RAM_ATTR espShow(
  uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t type);
#elif defined(ESP32)
extern "C" void espShow(
  uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t type);
#endif // ESP8266
*/

void Pentatube::setPin(uint8_t p) {
  pin = p;
  pinMode(p, OUTPUT);
  digitalWrite(p, LOW);
}

void Pentatube::show(int duration) {


  //place for efficent bit banging - disabled for now
  #if defined(ESP8266) || defined(ESP32)
  //espShow(pin, pixels, 3, true); //TODO remove last two params
  #endif

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

      if (brightness) { // See notes in setBrightness()
        r = (r * brightness) >> 8;
        g = (g * brightness) >> 8;
        b = (b * brightness) >> 8;
      }

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

    SPI.writeBytes(rgb,3);

    if (_c++ > 255) {
      _c = 0;
    }
  }
}

void Pentatube::setBrightness(uint8_t b) {
  // 1 = min brightness (off), 255 = just below max brightness.
  brightness = b;
}

Pentatube::Pentatube() {
  Pentatube(4, 18, 5, 23);
  if (pixels) free(pixels);
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
  //SPI.setFrequency(79999999); //80MhZ (gets rounded anyways)
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

void Pentatube::clear() {
  for (uint16_t i = 0; i < Pentatube::numPixels(); i++) {
    _colors[i][0] = 0;
    _colors[i][1] = 0;
    _colors[i][2] = 0;
  }
}
