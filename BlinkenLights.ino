/**
 * BlinkenLights
 * 
 * Code by Mitchell Pomery [BG3] with help from Andrew Adamson [BOB]
 * Hardware by Andrew Adamson [BOB] with help from Mitchell Pomery [BG3]
 * 
 * Get the dates from the git commits.
 * 
 * The LED strip should be wired from the top left corner and zig zag down
 * 
 */

///TODO: Vary light pattern randomly
///TODO: Make run when no network is present
///TODO: Recover when network reappears
///TODO: Make everything use posts. maybe
///TODO: Sanity checks. Everywhere. Until I'm Insane.
///TODO: Figure out rough memory usage. If we max memory we will see weird behaviour.

#include <Ethernet.h> // Needed For Reasons
#include <WebServer.h> // Lets us easily do web requests
#include <SPI.h> // Needed to control Lights
#include <Adafruit_NeoPixel.h> // Controls the Lights

#define PIN 6 // Pin that the LED strip is attached to
#define PREFIX "" // Document root for our pages
#define PORT 80 // Web Server Port
#define NAMELEN 8 // Max variable length in request
#define VALUELEN 255 // Max value from request
#define WIDTH 7 // How many LEDs wide our array is
#define HEIGHT 6 // How many LEDs high our array is
#define STRIPLENGTH 42 // Number of LED's in light strip
#define NUMSEQUENCES 8 // Number predefined sequences

// Used to store the LED's values
struct led {
  char red;
  char green;
  char blue;
};

// Used for our default light sequences
char red = (int) 255;
char green = (int) 255;
char blue = (int) 255;

// For controlling the lights - Should only be changed by the functions
int position = 0; // How far through the cycle we are
int lightOption = 1; // Which predefined light sequence we are running
char brightness = (char) 128; //Brightness of the LEDs
// We seem to have issues at the moment putting this up to the maximum (255)
// Most likely due to the fact the LED strip is underpowered

// Network Settings
static uint8_t mac[] = { 
  0xC0, 0xCA, 0xC0, 0x1A, 0x19, 0x82 }; // C0CA C01A 1982
IPAddress ip(130,95,13,96); // 130.95.13.96 (Can we forcefully take .82?
// If we remove the ip address, we will automatically use DHCP

// Set up our light strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN,
NEO_GRB + NEO_KHZ800);
// Set up our webserver
WebServer webserver(PREFIX, 80);

// =========================== char-hex conversions ===========================

char hexToChar(char* c) {
  return (char) strtol(c, NULL, 16);
}

char* charToHex(char c) {
  char base_digits[16] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  int converted_number[64];
  int base = 16;
  int next_digit, index=0;
  int number_to_convert = (int) c + 128;
  while (number_to_convert != 0)
  {
    converted_number[index] = number_to_convert % base;
    number_to_convert = number_to_convert / base;
    index++;
  }
  index--; // back up to last entry in the array
  char ret[3];
  int i = 0;
  if (index < 0) {
    ret[i] = base_digits[0];
    i++;
  }
  if (index < 1) {
    ret[i] = base_digits[0];
    i++;
  }
  
  for(; index >= 0 && i < 2; index--) // go backward through array 
  {
    ret[i] = base_digits[converted_number[index]];
    i++;
  }
  
  ret[2] = '\0';
  Serial.print(ret); // NEVER REMOVE THIS LINE. OR DIE
  // WITHOUT IT WE GET GARBLED OUTPUT HERE
  // YOU HAVE BEEN WARNED
  return ret;
}

// ============================ LIGHT MANIPULATION ============================

/**
 * Change the lights to our updated array
 * We should probably get rid of this soonish
 */
void updateLights() {
  strip.show();
}

/**
 * go from (x,y) to position in the array
 * @param xpos  x coordinate
 * @param ypos  y cordinate
 * @return      position in the array, -1 if out of array
 */
int coordToPos(int xpos, int ypos) {
  ///TODO: Test this function
  ///TODO: Make this smaller
  ///TODO: Turn this into a define
  int pos = 0;
  if ((xpos < 0 || xpos >= WIDTH) || (ypos < 0 || ypos >= HEIGHT)) {
    return -1;
  }
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
 * Set the LED at pos in the strip to the defined colour
 * @param pos   position in the array
 * @param red   red portion of colour
 * @param green green portion of colour
 * @param blue  blue portion of colour
 */
void setLED(int pos, int red, int green, int blue) {
  if (pos > 0 && pos < STRIPLENGTH) {
    strip.setPixelColor(pos, (int) red, (int) green, (int) blue);
  }
}

/**
 * Set the LED at (xpos, ypos) in the strip to the defined colour
 * @param xpos  x coordinate
 * @param ypos  y coordinate
 * @param red   red portion of colour
 * @param green green portion of colour
 * @param blue  blue portion of colour
 */
void setLED(int xpos, int ypos, int red, int green, int blue) {
  int pos = coordToPos(xpos, ypos);
  setLED(pos, red, green, blue);
}

/**
 * Get the colour of the LED in pos position in the strip
 * @param pos   position in the strip
 * @return      colour of LED at pos
 */
struct led getLED(int pos) {
  uint32_t light = strip.getPixelColor(pos);
  //Serial.print(light);
  led ret;
  ret.red = light >> 16;
  ret.green = light >> 8;
  ret.blue = light;
  return ret;
}

/**
 * Get the colour of the LED at (xpos, ypos)
 * @param xpos  x coordinate
 * @param ypos  y coordinate
 * @return      colour of LED at (xpos, ypos)
 */
struct led getLED(int xpos, int ypos) {
  int pos = coordToPos(xpos, ypos);
  return getLED(pos);
}

// ================================ WEB PAGES ================================

///TODO: Comment this stuff
///TODO: Reduce code reuse in the web page functions
///TODO: Make a function to set all the lights at once

/**
 * Sets the light sequence to one that is predefined.
 * Set it to 0 to disable the cycling of lights.
 * @param server        
 * @param type          
 * @param url_tail      
 * @param tail_complete 
 */
void webSetSequence(WebServer &server, WebServer::ConnectionType type,
char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];

  server.httpSuccess();
  // Kill the connection before doing anything if all they want is head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) {
    while (strlen(url_tail)) {
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
      if (rc != URLPARAM_EOS) {
        if (String(name).equals("seq")) {
          lightOption = atoi(value);
          position = 0;
        }
      }
    }
    server.print(lightOption);
  }
  else {
    server.print("Unknown Request");
  }
}

void webGetArray(WebServer &server, WebServer::ConnectionType type,
char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  
  server.httpSuccess("application/json");
  // Kill the connection before doing anything if all they want is head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) {
    //server.print("{"+'"'+"+lights"+'"'+":"+'"');
    server.print("a({");
    server.print('"');
    server.print("lights");
    server.print('"');
    server.print(':');
    server.print('"');
    for (int i = 0; i < WIDTH; i++) {
      for (int j = 0; j < HEIGHT; j++) {
        led light = getLED(i, j);
        //Serial.print(i);
        //Serial.print(", ");
        //Serial.println(j);
        server.print(charToHex(light.red));
        //Serial.print(light.red);
        server.print(charToHex(light.green));
        //Serial.print(light.green);
        server.print(charToHex(light.blue));
        //Serial.println(light.blue);
      }
    }
    server.print('"');
    server.print("});");
    //server.print(array);
  }
  else {
    server.print("Unknown Request");
  }
}

void webSetArray(WebServer &server, WebServer::ConnectionType type,
char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];

  server.httpSuccess();
  // Kill the connection before doing anything if all they want is head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) {
    ///TODO: Get rid of these magic numbers
    //if (strlen(url_tail) >= 252) {
      for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
          char red[3];
          char green[3];
          char blue[3];
          red[2] = '\0';
          green[2] = '\0';
          blue[2] = '\0';
          int start = HEIGHT * i + j;
          Serial.print(start);
          Serial.print(" ");
          red[0] = url_tail[6 * start];
          red[1] = url_tail[(6 * start) + 1];
          green[0] = url_tail[(6 * start) + 2];
          green[1] = url_tail[(6 * start) + 3];
          blue[0] = url_tail[(6 * start) + 4];
          blue[1] = url_tail[(6 * start) + 5];
          Serial.print(red);
          Serial.print(green);
          Serial.print(blue);
          Serial.println("");
          setLED(i, j, hexToChar(red), hexToChar(green), hexToChar(blue));
        }
      }
    //}
  }
  else {
    server.print("Unknown Request");
  }
}

/**
 * Set the brightness to a specific magnitude
 * @param server        
 * @param type          
 * @param url_tail      
 * @param tail_complete 
 */
void webSetBrightness(WebServer &server, WebServer::ConnectionType type,
char *url_tail, bool tail_complete) {
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];

  //server.print("Access-Control-Allow-Origin: *");
  server.httpSuccess("application/json", "Access-Control-Allow-Origin: *");
  // Kill the connection before doing anything if all they want is head
  if (type == WebServer::HEAD) {
    return;
  }
  else if (type == WebServer::GET) { //WebServer::POST
    while (strlen(url_tail)) {
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
      if (rc != URLPARAM_EOS) {
        if (String(name).equals("bright")) {
          strip.setBrightness(atoi(value));
          strip.show();
        }
      }
    }
    server.print((int) brightness);
    Serial.println((int) brightness);
  }
  else {
    server.print("Unknown");
  }
}

// ============================== LIGHT DISPLAYS ==============================

/**
 * Fill the dots one after the other with a colour
 * @param colour colour to fill the array with
 * @param wait  delay between colour changes
 */
void colourWipe(uint32_t c, uint8_t wait) {
  strip.setPixelColor(position, c);
  strip.show();
  delay(wait);
  position++;
  if (position >= STRIPLENGTH) {
    position = 256;
  }
}

/**
 * Turn the light strip into a rainbow
 * @param wait  delay between colour changes
 */
void rainbow(uint8_t wait) {
  int outOf = 256;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+position) & 255));
  }
  strip.show();
  delay(wait);
  position++;
  if (position > outOf) {
    position = 0;
  }
}

/**
 * Slightly different, this makes the rainbow equally distributed throughout
 * @param wait  delay between colour changes
 */
void rainbowCycle(uint8_t wait) {
  int outOf = 256 * 5;
  uint16_t i;
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + position) & 255));
  }
  strip.show();
  delay(wait);
  position++;
  if (position >= outOf) {
    position = 0;
  }
}

/**
 * Theatre-style crawling lights.
 * @param colour colour to fill the array with
 * @param wait  delay between colour changes
 */
void theaterChase(uint32_t c, uint8_t wait) {
  ///TODO: stop this blocking. Somehow
  int outOf = 10;
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
  position++;
  if (position >= outOf) {
    position = 0;
  }
}

/**
 * Theatre-style crawling lights with rainbow effect
 * @param wait  delay between colour changes
 */
void theaterChaseRainbow(uint8_t wait) {
  ///TODO: stop this blocking. Somehow
  int outOf = 256;
  for (int q=0; q < 3; q++) {
    for (int i=0; i < strip.numPixels(); i=i+3) {
      // turn every third pixel on
      strip.setPixelColor(i+q, Wheel( (i+position) % 255));
    }
    strip.show();

    delay(wait);

    for (int i=0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, 0); // turn every third pixel off
    }
  }

  position++;
  if (position >= outOf) {
    position = 0;
  }
}

/**
 * Input a value 0 to 255 to get a colour value.
 * The colours are a transition r - g - b - back to r.
 * @param WheelPos  Where we currently are in the cycle
 * @return          A new colour
 */
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// =============================== MAIN PROGRAM ===============================

///TODO: Comment this stuff

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Starting Ethernet");
  // Start Ethernet
  Ethernet.begin(mac, ip);
  Serial.println("Ethernet Started");
  // Set up webpages
  webserver.setDefaultCommand(&webSetSequence);
  webserver.addCommand("custom", &webSetSequence);
  //webserver.addCommand("individual", &webSetLED);
  webserver.addCommand("brightness", &webSetBrightness);
  webserver.addCommand("get", &webGetArray);
  webserver.addCommand("set", &webSetArray);
  // Start Webserver
  webserver.begin();
  
  /*for (int i = -128; i < 128; i++) {
   Serial.print(i);
   Serial.print(" ");
   Serial.print((char) i);
   Serial.print(" ");
   char *c = charToHex((char) i);
   char d = hexToChar((char*) (int) c - 128);
   Serial.print(c);
   Serial.print(" ");
   Serial.print(d);
   Serial.println("");
  }*/

  // Start Lights
  strip.begin();
  strip.setBrightness(128);
  updateLights();
}

void loop()
{
  // process incoming connections one at a time forever
  char buff[256];
  int len = 256;
  webserver.processConnection(buff, &len);
  if (lightOption > NUMSEQUENCES) {
    //lightOption = 1;
  }

  // Run our light sequence after checking for we requests
  ///TODO: Make these switches nicer
  switch (lightOption) {
  case 0: // Don't change the lights at all
    break;
  case 1: // RainbowCycle
    rainbowCycle(20);
    break;
    // Cycling through LED Sequences
  case 11:
    colourWipe(strip.Color(red, green, blue), 50);
    Serial.println("ColourWipe");
    break;
  case 12:
    rainbow(20);
    break;
  case 13:
    rainbowCycle(20);
    break;
  case 14:
    theaterChaseRainbow(20);
    break;
  default: // Go back to cycling
    lightOption = 11;
    break;
  }
  
  updateLights();
  
  // Show our lights
  if (position == 0 && lightOption > 10) { // if we have completed a sequence, move to the next one
    lightOption++;
  }
}


