/*
 BlinkenLights
 
 by Mitchell Pomery [BG3] with help from Andrew Adamson [BOB]
 modified 20 Jan 2014
 
 */

///TODO: Vary light pattern randomly
///TODO: Make run when no network is present
///TODO: Recover when network reappears

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

int position = 0;

// led array
struct led ledArray[STRIPLENGTH];
char brightness = (char) 128;


static uint8_t mac[] = { 0xC0, 0xCA, 0xC0, 0x1A, 0x19, 0x82 }; // C0CA C01A 1982
IPAddress ip(130,95,13,96);

int lightOption = 0;

// Set up our light strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN, NEO_GRB + NEO_KHZ800);

// Set up our webserver
WebServer webserver(PREFIX, 80);

// showArray
void showArray() {
  for (int i = 0; i < STRIPLENGTH; i ++) {
    strip.setPixelColor(i, (int) ledArray[i].red, (int) ledArray[i].green, (int) ledArray[i].blue);
  }
  strip.show();
}

// setLED
void setLED(int pos, int red, int green, int blue) {
  ledArray[pos].red = red;
  ledArray[pos].green = green;
  ledArray[pos].blue = blue;
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
  else if (type == WebServer::POST) {
    while (server.readPOSTparam(name, NAMELEN, value, VALUELEN))
    {
      //Serial.print(name);
      //Serial.print(" = ");
      //Serial.println(value);
      //if (name == "seq") {
      //  lightOption = value;
      //}
    }
  }
  else if (type == WebServer::GET) { //WebServer::POST
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
          //Serial.println(name);
          //Serial.println(value);
          if (String(name).equals("seq")) {
            lightOption = atoi(value);
            server.print(lightOption);
          }
        }
      }
    }
    
    ///TODO: Get ?seq=X and set lightOption to X
  }
  else {
    server.print("Unknown");
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
  else if (type == WebServer::POST) {
    while (server.readPOSTparam(name, NAMELEN, value, VALUELEN))
    {
      //Serial.print(name);
      //Serial.print(" = ");
      //Serial.println(value);
      //if (name == "seq") {
      //  lightOption = value;
      //}
    }
  }
  else if (type == WebServer::GET) { //WebServer::POST
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
          //Serial.println(name);
          //Serial.println(value);
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
  
  //Serial.println(xPos);
  //Serial.println(yPos);
  //Serial.println(r);
  //Serial.println(g);
  //Serial.println(b);
  
  if (xPos != -1 && yPos != -1 && r != -1 && g != -1 && b != -1) {
    setLED(xPos, r, g, b);
  }
}

void webSetBrightness(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete) {
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
  else if (type == WebServer::POST) {
    while (server.readPOSTparam(name, NAMELEN, value, VALUELEN))
    {
      //Serial.print(name);
      //Serial.print(" = ");
      //Serial.println(value);
      //if (name == "seq") {
      //  lightOption = value;
      //}
    }
  }
  else if (type == WebServer::GET) { //WebServer::POST
    if (strlen(url_tail)) {
      while (strlen(url_tail)) {
        rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
        if (rc != URLPARAM_EOS) {
          //Serial.println(name);
          //Serial.println(value);
          if (String(name).equals("brightness")) {
            strip.setBrightness(atoi(value));
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
  strip.setBrightness(32);
  showArray();
}

void loop()
{
  // process incoming connections one at a time forever
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
  position = position % 256;
  rainbow(10);
  // Run our light sequence after checking for we requests
  /*switch (lightOption) {
    case 0:
      break;
    case 1:
      colorWipe(strip.Color(255, 0, 0), 50); // Red
      break;
    case 2:
      colorWipe(strip.Color(0, 255, 0), 50); // Green
      break;
    case 3:
      colorWipe(strip.Color(0, 0, 255), 50); // Blue
      break;
    case 4:
      theaterChase(strip.Color(127, 127, 127), 50); // White
      break;
    case 5:
      theaterChase(strip.Color(127,   0,   0), 50); // Red
      break;
    case 6:
      theaterChase(strip.Color(  0,   0, 127), 50); // Blue
      break;
    case 7:
      rainbow(100);
      break;
    case 8:
      rainbowCycle(20);
      break;
    case 9:
      theaterChaseRainbow(50);
      break;
    default:
      lightOption = 7;
      break;
  }*/
  
  showArray();
}
