
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>

#include <Update.h>
#include <WebServer.h>

#include <ArtnetWifi.h>
ArtnetWifi artnet;



const char* version = __DATE__ " / " __TIME__;

int debug_level = 0; //set above 10 for detailed dmx packets on Serial port

const char* host = "OTA23";


int tube_universe = 1;
int tube_channel_offset=0;

int roundcounter = 0;

#include <Pentatube.h>


//using vspi default ports following - srlcr wired high
//https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/examples/SPI_Multiple_Buses/SPI_Multiple_Buses.ino

// JST1 GND //PIN/
const int SRCLR   = 22; //3; //ATMEL 15 //green //HIGH //TODO REMOVE
const int SRCLK   = 18; //ATMEL 16 //lila //JST2  // P30 (Pin 9)
const int RCLK    = 5; //2; //ATMEL 14 // brown //JST3  // P29 (Pin10)
const int SERIN   = 23; //ATMEL 19 MOSI //blau //JST4 //P37 (Pin2)
//JST 5 VCC

WiFiMulti wifiMulti;

Pentatube tube = Pentatube(SRCLR, SRCLK, RCLK, SERIN);

WebServer server(80);


/*
   Server Index Page
*/

const char* serverIndex =
  "Pentatube 1 Labore<br>" 
  "Version: " __DATE__ " / " __TIME__ "<br>" 
  //"Universe:" tube_universe " tube_channel_offset: " tube_channel_offset "<br>"
  "<form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "done.";


int previousDataLength = 0;

boolean onDmxFrameRunning = false;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  //if (tube_universe!=universe) { return; }
  
  roundcounter = 0;

  boolean tail = false;
  if (debug_level > 5) {
    Serial.print("DMX: Univ: ");
    Serial.print(universe, DEC);
    Serial.print(", Seq: ");
    Serial.print(sequence, DEC);
    Serial.print(", Data (");
    Serial.print(length, DEC);
    Serial.print("): ");

    if (length > 16) {
      length = 16;
      tail = true;
    }
    // send out the buffer
    for (int i = 0; i < length; i++)
    {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    if (tail) {
      Serial.print("...");
    }
    Serial.println();
  }


  //set tube per segment starting at channel zero
  //TODO set offset accordingly in multi tube setup
  for (int segment = 0; segment < 8; segment++)
  {
    int shift = tube_channel_offset + segment * 3;
    if (data[shift] || data[shift + 1] || data[shift + 2]) {
      tube.setPixelColor(segment, data[shift], data[shift + 1], data[shift + 2]);
    } else {
      tube.setPixelColor(segment, 0, 0, 0);
    }
  }

  //tube.show(1);
  //previousDataLength = length;
  //tail = false;
  //onDmxFrameRunning = false;
}


TaskHandle_t Task1, Task2;

void Task1code( void * pvParameters ) {
  //Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());

  for (;;) {
    tube.show(1);
  }
}

void Task2code( void * pvParameters ) {
  //Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());

  for (;;) {
    //handle artnet
    artnet.read();
    //tube.show(1);
  }
}

/*
   setup function
*/
void setup(void) {

  Serial.begin(115200);

  //grace start to fill up condensators
  delay(3000);

  //init tube
  tube.begin();

  //set tube ready color
  for (int i = 0; i < 8; i++) {
    tube.setPixelColor(i, tube.Color(0, 0, 0));
  }
  tube.show(1000);

  // Connect to WiFi network
  WiFi.setSleep(false);
  Serial.println("");

  wifiMulti.addAP("Pentatube","");
 
  Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        
        Serial.println(WiFi.localIP());
    }
    
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);

  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/rgb", handleRGB);
  server.on("/artnet", handleArtnetSettings);


  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    10000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    1,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */


}

//allows requests like http://ip2tube/rgb?segment=0&r=255&g=128&b=0 for fallback
void handleRGB() { //Handler
  //server.send(200, "text/plain", "Hello world");
  //server.sendHeader("Connection", "close");
  tube.setPixelColor(server.arg(0).toInt(), tube.Color(server.arg(1).toInt(), server.arg(2).toInt(), server.arg(3).toInt()));
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", "0:" + server.arg(0) + " 1:" + server.arg(1) + " 2:" + server.arg(2) + " 3:" + server.arg(3));
}

void handleArtnetSettings() { 
  tube_universe = server.arg(1).toInt();
  tube_channel_offset = server.arg(2).toInt(); 
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", "Set to universe:" + server.arg(0) + " channel_offset:" + server.arg(1));
}



int color_counter = 0;


void loop(void) {

  artnet.read();

  //if no artnet for 5000 rounds, at least show that it's alive on segment 0
  if (roundcounter > 50000) {

    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
    }    
    color_counter++;
    if (color_counter < 15000) {
      tube.setPixelColor(0, tube.Color(0, 0, 255));
    }
    else if (color_counter < 35000) {
      tube.setPixelColor(0, tube.Color(255, 196, 00));
    }
    else {
      color_counter = 0;
    }
    for (int i = 1; i < 8; i++) { //when in doubt, be blue
      tube.setPixelColor(i, tube.Color(0, 0, 255));
    }
    if (roundcounter > 65000) {
      roundcounter = 50001;
    }
    //handle http (only handle http if there is no artnet
    server.handleClient();
  }

  roundcounter++;

}


