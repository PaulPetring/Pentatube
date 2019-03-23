#include <Arduino.h>
#include <SPI.h>

class Pentatube
{
  public:
    Pentatube(int SRCLR, int SRCLK, int RCLK, int SERIN);
    Pentatube(void);
    ~Pentatube();

    void show(int duration);
    void begin(),
         clear(),
         setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
         setPixelColor(uint16_t n, uint32_t c);
    int numPixels();
    static uint32_t
    Color(uint8_t r, uint8_t g, uint8_t b);
  private:
    int _SRCLR;
    int _SRCLK;
    int _RCLK;
    int _SERIN;
    byte _c=0;
    byte _colors[8][3] = {  {128, 128, 128},
      {255, 128, 255},
      {64, 128, 0},
      {128, 128, 0},
      {255, 255, 255},
      {0, 0, 255},
      {0, 255, 255},
      {32, 32, 32}
    };



};
