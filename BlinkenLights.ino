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
///TODO: Sanity checks. Everywhere. Until I'm Insane.
///TODO: Figure out memory usage. If we max memory we will see weird behaviour.

#include <Ethernet.h> // Needed For Reasons
#include <WebServer.h> // Lets us easily do web requests
#include <SPI.h> // Needed to control Lights
#include <Adafruit_NeoPixel.h> // Controls the Lights

#define PIN 6 // Pin that the LED strip is attached to
#define PREFIX "" // Document root for our pages
#define PORT 80 // Web Server Port
#define NAMELEN 8 // Max variable length in request
#define VALUELEN 256 // Max value from request
#define WIDTH 7 // How many LEDs wide our array is
#define HEIGHT 6 // How many LEDs high our array is
#define STRIPLENGTH 42 // Number of LED's in light strip

// Used to store the LED's values
struct led {
  char red;
  char green;
  char blue;
};

// For controlling the lights - Should only be changed by the functions
int position = 0; // How far through the cycle we are
struct led ledArray[STRIPLENGTH]; // led array
int lightOption = 0; // Which predefined light sequence we are running
char brightness = (char) 128; //Brightness of the LEDs
  // We seem to have issues at the moment putting this up to the maximum (255)
  // Most likely due to the fact the LED strip is underpowered

// Network Settings
static uint8_t mac[] = { 0xC0, 0xCA, 0xC0, 0x1A, 0x19, 0x82 }; // C0CA C01A 1982
IPAddress ip(130,95,13,96); // 130.95.13.96 (Can we forcefully take .82?
// If we remove the ip address, we will automatically use DHCP

// Set up our light strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN, NEO_GRB + NEO_KHZ800);
// Set up our webserver
WebServer webserver(PREFIX, 80);

// ============================ LIGHT MANIPULATION ============================

/**
 * Change the lights to our updated array
 * We should probably get rid of this soonish
 */
void showArray() {
  ///TODO: Rename this to something better?
  for (int i = 0; i < STRIPLENGTH; i ++) {
    strip.setPixelColor(i, (int) ledArray[i].red, (int) ledArray[i].green, (int) ledArray[i].blue);
  }
  strip.show();
}

/**
 * Set the LED at pos in the strip to the defined color
 * @param xpos  x coordinate
 * @param ypos  y cordinate
 * @return      position in the array
 */
int coordToPos(int xpos, int ypos) {
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
  return pos;
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
 * Set the LED at (xpos, ypos) in the strip to the defined color
 * @param xpos  x coordinate
 * @param ypos  y coordinate
 * @param red   red portion of color
 * @param green green portion of color
 * @param blue  blue portion of color
 */
void setLED(int xpos, int ypos, int red, int green, int blue) {
  int pos = coordToPos(xpos, ypos);
  setLED(pos, red, green, blue);
}

/**
 * Get the colour of the LED in pos position in the strip
 * @param pos   position in the strip
 * @return      color of LED at pos
 */
struct led getLED(int pos) {
  return ledArray[pos];
}

/**
 * Get the colour of the LED at (xpos, ypos)
 * @param xpos  x coordinate
 * @param ypos  y coordinate
 * @return      color of LED at (xpos, ypos)
 */
struct led getLED(int xpos, int ypos) {
  int pos = coordToPos(xpos, ypos);
  return ledArray[pos];
}

// ================================ WEB PAGES ================================

///TODO: Comment this stuff
///TODO: Reduce code reuse in the web page functions
///TODO: Start using post requests
///TODO: Make a function to set all the lights at once

// Sets the light sequence to one that is predefined
void webSetSequence(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  
  server.httpSuccess();
  // Kill the connection before doing anything if all they want is head
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
  // Kill the connection before doing anything if all they want is head
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
  if (xPos != -1 && yPos != -1 && r != -1 && g != -1 && b != -1) {
    setLED(xPos, r, g, b);
  }
}

void webSetBrightness(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  
  server.httpSuccess();
  // Kill the connection before doing anything if all they want is head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) { //WebServer::POST
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
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

// ============================== LIGHT DISPLAYS ==============================

///TODO: Comment this stuff
///TODO: Make sure these functions don't block. Otherwise web requests are slow

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

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) { // do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c); // turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0); // turn every third pixel off
      }
    }
  }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) { // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          // turn every third pixel on
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0); // turn every third pixel off
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

// =============================== MAIN PROGRAM ===============================

void setup() {
  // Start Ethernet
  Ethernet.begin(mac, ip);
  // Set up webpages
  webserver.setDefaultCommand(&webSetSequence);
  webserver.addCommand("custom", &webSetSequence);
  webserver.addCommand("individual", &webSetLED);
  webserver.addCommand("brightness", &webSetBrightness);
  // Start Webserver
  webserver.begin();
  // Turn our lights on
  for (int i = 0; i < STRIPLENGTH; i ++) {
    ledArray[i].red = (char) 128;
    ledArray[i].green = (char) 128;
    ledArray[i].blue = (char) 128;
  }
  
  // Start Lights
  strip.begin();
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
  // Show our lights
  showArray();
}
