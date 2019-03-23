/*
#include "./Pentatube.h"
#include <WiFi.h>
#include <ArtnetWifi.h>

const char* ssid = "ssid";
const char* password = "pass";

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

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

void initTest()
{
  for (int i = 0 ; i < numLeds ; i++)
    tube.setPixelColor(i, 127, 0, 0);
  tube.show(1000);
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    tube.setPixelColor(i, 0, 127, 0);
  tube.show(1000);
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    tube.setPixelColor(i, 0, 0, 127);
  tube.show(1000);
  delay(500);
  for (int i = 0 ; i < numLeds ; i++)
    tube.setPixelColor(i, 0, 0, 0);
  tube.show(1000);
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{

  for (int led = 0; led < 8; led++)
  {
    tube.setPixelColor(led, data[0], data[1], data[2]);
  }
  tube.show(1);

  previousDataLength = length;

  boolean tail = false;


}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  artnet.begin();
  tube.begin();

  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
*/
