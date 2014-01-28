/*
 BlinkenLights
 
 Code by Mitchell Pomery [BG3] with help from Andrew Adamson [BOB]
 Hardware by Andrew Adamson [BOB] with help from Mitchell Pomery [BG3]
 
 Get the dates from the git commits.
 
 The LED strip should be wired from the top left corner and zig zag down
 
 */

///TODO: Vary light pattern randomly
///TODO: Make run when no network is present
///TODO: Recover when network reappears
///TODO: Make everything use posts. maybe

#include <Ethernet.h> // Needed For Reasons
#include <WebServer.h> // Lets us easily do web requests
#include <SPI.h> // Needed to control Lights
#include <Adafruit_NeoPixel.h> // Controls the Lights

#define STRIPLENGTH 42 // Number of LED's in light strip
#define PIN 6 // Pin that the LED strip is attached to
#define PREFIX "" // Document root for our pages
#define PORT 80 // Web Server Port
#define NAMELEN 8 // Max variable length in request
#define VALUELEN 256 // Max value from request
#define WIDTH // How many LEDs wide our array is
#define HEIGHT // How many LEDs high our array is

// Used to store the LED's values
struct led {
  char red;
  char green;
  char blue;
};

// For controlling the lights
int position = 0; // How far through the cycle we are
struct led ledArray[STRIPLENGTH]; // led array
char brightness = (char) 128; //Brightness of the LEDs
  // We seem to have issues at the moment putting this up to the maximum (255)
  // Most likely due to the fact the LED strip is underpowered
int lightOption = 0; // Which predefined light sequence we are running

static uint8_t mac[] = { 0xC0, 0xCA, 0xC0, 0x1A, 0x19, 0x82 }; // C0CA C01A 1982
IPAddress ip(130,95,13,96); // 130.95.13.96 (Can we forcefully take .82?


// Set up our light strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN, NEO_GRB + NEO_KHZ800);
// Set up our webserver
WebServer webserver(PREFIX, 80);

/**
 * Change the lights to our updated array
 */
void showArray() {
  for (int i = 0; i < STRIPLENGTH; i ++) {
    strip.setPixelColor(i, (int) ledArray[i].red, (int) ledArray[i].green, (int) ledArray[i].blue);
  }
  strip.show();
}

/**
 * Set the LED at pos in the strip to the defined color
 * @param pos   position in the array
 * @param red   red portion of color
 * @param green green portion of color
 * @param blue  blue portion of color
 */
void setLED(int pos, int red, int green, int blue) {
  ledArray[pos].red = red;
  ledArray[pos].green = green;
  ledArray[pos].blue = blue;
}

/**
 * Set the LED at pos in the strip to the defined color
 * @param pos   position in the array
 * @param red   red portion of color
 * @param green green portion of color
 * @param blue  blue portion of color
 */
void setLED(int xpos, int yxpos, int red, int green, int blue) {
  ///TODO: Test this function
  int pos = 0;
  if (ypos % 2 == 0) { // if we are on an even line, add y
    pos = ypos * WIDTH + xpos;
  }
  else { // on an odd line, take y
    if (WIDTH % 2 == 0) { // even width
      pos = ypos * WIDTH + HEIGHT - xpos;
    }
    else { // odd width
      pos = ypos * WIDTH + HEIGHT - xpos -1;
    }
  }
  setLED(pos, red, green, blue);
}

/**
 * Get the color of the LED at pos in the strip
 * @param pos   position in the array
 * @return      color of LED at pos
 */
struct led getLED(int pos) {
  return ledArray[pos];
}

// Sets the light sequence to one that is predefined
void webSetSequence(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  
  server.httpSuccess();
  // Kill the connection before doing anything if all they want is the head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) {
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
          if (String(name).equals("seq")) {
            lightOption = atoi(value);
            server.print(lightOption);
          }
        }
      }
    }
  }
  else {
    server.print("Unknown Request");
  }
}

void webSetLED(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  int xPos = -1;
  int yPos = -1;
  int r = -1;
  int g = -1;
  int b = -1;
  
  server.httpSuccess();
  // Kill the connection before doing anything if all they want is the head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) {
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
          if (String(name).equals("x")) {
            xPos = atoi(value);
          }
          else if (String(name).equals("y")) {
            yPos = atoi(value);
          }
          else if (String(name).equals("r")) {
            r = atoi(value);
          }
          else if (String(name).equals("g")) {
            g = atoi(value);
          }
          else if (String(name).equals("b")) {
            b = atoi(value);
          }
        }
      }
    }
  }
  else {
    server.print("Unknown");
  }
  
  //server.print(xPos);
  //server.print(yPos);
  //server.print(r);
  //server.print(g);
  //server.print(b);
  
  if (xPos != -1 && yPos != -1 && r != -1 && g != -1 && b != -1) {
    setLED(xPos, r, g, b);
  }
}

void webSetBrightness(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  
  server.httpSuccess();
  // Kill the connection before doing anything if all they want is the head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) { //WebServer::POST
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
          //Serial.println(name);
          //Serial.println(value);
          if (String(name).equals("bright")) {
            strip.setBrightness(atoi(value));
            server.print(atoi(value));
            strip.show();
          }
        }
      }
    }
  }
  else {
    server.print("Unknown");
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+position) & 255));
  }
  strip.show();
  delay(wait);
  position++;
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) { //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c); //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0); //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255)); //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0); //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void setup() {
  // Open //Serial communications and wait for port to open:
  //Serial.begin(9600);
   //while (!//Serial) {
    //; // wait for //Serial port to connect. Needed for Leonardo only
  //}
  //Serial.println("//Serial Port Opened");
  
  // start the Ethernet connection and the server:
  //Serial.println("Starting Ethernet");
  Ethernet.begin(mac, ip);
  
  //Serial.println("Setting Up Server");
  webserver.setDefaultCommand(&webSetSequence);
  webserver.addCommand("custom", &webSetSequence);
  webserver.addCommand("individual", &webSetLED);
  webserver.addCommand("brightness", &webSetBrightness);
  
  //Serial.println("Starting Server");
  webserver.begin();
  //Serial.print("server is at ");
  //Serial.println(Ethernet.localIP());
  
  // Initialize our LED Array
  
  for (int i = 0; i < STRIPLENGTH; i ++) {
    ledArray[i].red = (char) 128;
    ledArray[i].green = (char) 128;
    ledArray[i].blue = (char) 128;
  }
  
  // Start Lights
  strip.begin();
  //strip.show();
  strip.setBrightness(128);
  showArray();
}

void loop()
{
  // process incoming connections one at a time forever
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
  position = position % 256;
  //rainbow(10);
  // Run our light sequence after checking for we requests
  switch (lightOption) {
    case 0: // Don't change the lights at all
      break;
    case 1: // Wipe the LED's to Red
      colorWipe(strip.Color(255, 0, 0), 50); // Red
      break;
    default:
      lightOption = 1;
      break;
  }
  
  showArray();
}
