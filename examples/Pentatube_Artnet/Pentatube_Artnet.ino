
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <ArtnetWifi.h>

ArtnetWifi artnet;

int debug_level = 0; //set above 10 for detailed dmx packets on Serial port

const char* host = "OTA23";
const char* ssid = "ssid";
const char* password = "";

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


Pentatube tube = Pentatube(SRCLR, SRCLK, RCLK, SERIN);

WebServer server(80);

/*
   Server Index Page
*/

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

int previousDataLength = 0;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
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
    int shift = segment * 3;
    if (data[shift] || data[shift + 1] || data[shift + 2]) {
      tube.setPixelColor(segment, data[shift], data[shift + 1], data[shift + 2]);
    } else {
      tube.setPixelColor(segment, 0, 0, 0);
    }
  }

  tube.show(1);
  previousDataLength = length;
  tail = false;
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
  WiFi.begin(ssid, password);
  Serial.println("");


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://ota8.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/rgb", handleRGB);

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
        //Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        //Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        //Update.printError(Serial);
      }
    }
  });
  server.begin();
}

//allows requests like http://ip2tube/rgb?segment=0&r=255&g=128&b=0 for fallback 
void handleRGB() { //Handler
  //server.send(200, "text/plain", "Hello world");
  //server.sendHeader("Connection", "close");
  tube.setPixelColor(server.arg(0).toInt(), tube.Color(server.arg(1).toInt(), server.arg(2).toInt(), server.arg(3).toInt()));

  server.sendHeader("Connection", "close");
  server.send(200, "text/html", "0:" + server.arg(0) + " 1:" + server.arg(1) + " 2:" + server.arg(2) + " 3:" + server.arg(3));
}

int color_counter = 0;

void loop(void) {
  //handle http
  server.handleClient();
  //handle artnet
  artnet.read();

  //if no artnet for 5000 rounds, at least show that it's alive on segment 0
  if (roundcounter > 5000) {
    color_counter++;
    if (color_counter < 1000) {
      tube.setPixelColor(0, tube.Color(255, 0, 0));
    }
    else if (color_counter < 2000) {
      tube.setPixelColor(0, tube.Color(0, 255, 0));
    }
    else if (color_counter < 3000) {
      tube.setPixelColor(0, tube.Color(0, 0, 255));
    }
    else {
      color_counter = 0;
    }
    for (int i = 1; i < 8; i++) { //when in doubt, be blue
      tube.setPixelColor(i, tube.Color(0, 0, 255));
    }
    if (roundcounter > 65000) {
      roundcounter = 5001;
    }
  }
  tube.show(1);
  roundcounter++;
}

