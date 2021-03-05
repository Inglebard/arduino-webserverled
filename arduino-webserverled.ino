// This code was tested with RGB ws2812b and ESP32 (lolin D32).
// It requires the following libraries
//
//Adafruit_NeoPixel :https://github.com/adafruit/Adafruit_NeoPixel
//ESPAsyncWebServer : https://github.com/me-no-dev/ESPAsyncWebServer
//
//and the following hardware features :
//
//SPIFFS
//Wi-Fi


#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include <Adafruit_NeoPixel.h>


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        27

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS        4 

// Replace with your network credentials
const char* ssid = "your ssid";
const char* password = "your password";

// All differents modes
enum Led_Transition { STANDARD, RAINBOW, FADE, FADE_SEQUENTIAL, REVERSE_FADE_SEQUENTIAL,STROBE, STROBE_SEQUENTIAL,REVERSE_STROBE_SEQUENTIAL };
struct ledColor
{
  int r;
  int g;
  int b;
};

// var and initial values
Led_Transition Current_Led_Transition = STANDARD;
ledColor ledColorState[NUMPIXELS];
int loopPass=0;
int delay_value=10;
int brightness_value=255;

void initLedsColorState()
{
  for(int i=0; i<NUMPIXELS; i++) {
    ledColorState[i].r=255;
    ledColorState[i].g=0;
    ledColorState[i].b=0;
  }  
}

//Will cork with RGBW but will not use true white.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// set initial variable inside html file to track actual state
String processor(const String& var){
  int num_pixels = NUMPIXELS;
  if(var == "LED_NUMBER"){
    return String(num_pixels);
  }
  if(var == "LED_ARRAY")
  {
    String output="";    
    for(int i=0; i<NUMPIXELS; i++) {
          
      char buffer_str[256];    
      sprintf(buffer_str, "%d:{r:%d,g:%d,b:%d},", i,ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);
      output +=String(buffer_str);
    }
    return output;
  }  
  if(var == "DELAY_VALUE")
  {
    return String(delay_value);
  }
  if(var == "BRIGHTNESS_VALUE")
  {
    return String(brightness_value);
  }
  if(var == "MODE_VALUE")
  {
    String Current_Led_Transition_str="STANDARD";
    if(Current_Led_Transition == STROBE )
    {
      Current_Led_Transition_str = "STROBE";
    }
    if(Current_Led_Transition == STROBE_SEQUENTIAL )
    {
      Current_Led_Transition_str = "STROBE_SEQUENTIAL";
    }
    if(Current_Led_Transition == REVERSE_STROBE_SEQUENTIAL)
    {
      Current_Led_Transition_str = "REVERSE_STROBE_SEQUENTIAL";
    }
    if(Current_Led_Transition == FADE )
    {
      Current_Led_Transition_str = "FADE";
    }
    if(Current_Led_Transition == FADE_SEQUENTIAL)
    {
      Current_Led_Transition_str = "FADE_SEQUENTIAL";
    }
    if(Current_Led_Transition == REVERSE_FADE_SEQUENTIAL )
    {
      Current_Led_Transition_str = "REVERSE_FADE_SEQUENTIAL";
    }
    if(Current_Led_Transition == RAINBOW)
    {
      Current_Led_Transition_str = "RAINBOW";
    }
    if(Current_Led_Transition == STANDARD)
    {
      Current_Led_Transition_str = "STANDARD";
    }    
    return Current_Led_Transition_str;
  }
  return String();
}
 
void setup(){
  Serial.begin(115200);
  
  initLedsColorState();

  // Initialize rgb leds
  pixels.setBrightness(brightness_value);
  pixels.begin();
  pixels.clear();
  
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


  // static files
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.serveStatic("/jquery-3.6.0.min.js", SPIFFS, "/jquery-3.6.0.min.js");


  // main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // AJAX change specific led color
  server.on("/led/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebParameter* p_id = nullptr;      
    AsyncWebParameter* p_r = nullptr;         
    AsyncWebParameter* p_g = nullptr;    
    AsyncWebParameter* p_b = nullptr;   
      
    
    if(request->hasParam("id"))
      p_id = request->getParam("id");
      
    if(request->hasParam("r"))
      p_r = request->getParam("r");
      
    if(request->hasParam("g"))
      p_g = request->getParam("g");
      
    if(request->hasParam("b"))
      p_b = request->getParam("b");
      
    int letId=p_id->value().toInt();
    ledColorState[letId].r=p_r->value().toInt();
    ledColorState[letId].g=p_g->value().toInt();
    ledColorState[letId].b=p_b->value().toInt();      
    request->send(200, "text/json", "OK");
  });
  
  // AJAX change all led color
  server.on("/allleds/", HTTP_GET, [](AsyncWebServerRequest *request){
        
    AsyncWebParameter* p_r = nullptr;         
    AsyncWebParameter* p_g = nullptr;    
    AsyncWebParameter* p_b = nullptr;   
      
    if(request->hasParam("r"))
      p_r = request->getParam("r");
      
    if(request->hasParam("g"))
      p_g = request->getParam("g");
      
    if(request->hasParam("b"))
      p_b = request->getParam("b");
      
          
    for(int i=0; i<NUMPIXELS; i++) {
      ledColorState[i].r=p_r->value().toInt();
      ledColorState[i].g=p_g->value().toInt();
      ledColorState[i].b=p_b->value().toInt();
    }
    request->send(200, "text/json", "OK");
  });
  
  // AJAX change parameters (delay or brightness)
  server.on("/parameters/", HTTP_GET, [](AsyncWebServerRequest *request){
        
    AsyncWebParameter* p_name = nullptr;         
    AsyncWebParameter* p_value = nullptr;    
      
    if(request->hasParam("name") && request->hasParam("value")) 
    {
      p_name = request->getParam("name");
      p_value = request->getParam("value");

      if(p_name->value() == "delay")
      {
        delay_value = p_value->value().toInt();
      }
      
      if(p_name->value() == "brightness")
      {
        brightness_value = p_value->value().toInt();
        pixels.setBrightness(brightness_value);
      }      
      
    }      
    request->send(200, "text/json", "OK");
  });
  
  // AJAX change leds modes 
  server.on("/modes/", HTTP_GET, [](AsyncWebServerRequest *request){
                
    AsyncWebParameter* p_mode = nullptr;    
      
    if(request->hasParam("mode")) 
    {
      p_mode = request->getParam("mode");
      String modestr=p_mode->value();
      
      if(modestr == "STROBE" )
      {
        Current_Led_Transition = STROBE;
      }
      if(modestr == "STROBE_SEQUENTIAL" )
      {
        Current_Led_Transition = STROBE_SEQUENTIAL;
      }
      if(modestr == "REVERSE_STROBE_SEQUENTIAL")
      {
        Current_Led_Transition = REVERSE_STROBE_SEQUENTIAL;
      }
      if(modestr == "FADE" )
      {
        Current_Led_Transition = FADE;
      }
      if(modestr == "FADE_SEQUENTIAL")
      {
        Current_Led_Transition = FADE_SEQUENTIAL;
      }
      if(modestr == "REVERSE_FADE_SEQUENTIAL" )
      {
        Current_Led_Transition = REVERSE_FADE_SEQUENTIAL;
      }
      if(modestr == "RAINBOW")
      {
        Current_Led_Transition = RAINBOW;
      }
      if(modestr == "STANDARD")
      {
        Current_Led_Transition = STANDARD;
      }       
    }      
    request->send(200, "text/json", "OK");
  });

  // Start server
  server.begin();
}

// leds with colors speficied
void standard()
{
  for(int i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);       
  }
  pixels.show();
}

// based on adafruit code
// leds with rainbow colors
// selected colors are not used in this mode
void rainbow()
{
  int wait = delay_value/2;  
  int firstPixelHue = 255*loopPass;  
  for(int i=0; i<pixels.numPixels(); i++) {
    uint32_t pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
    pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue, 255)));
  }
  pixels.show();
  delay(delay_value);
}


// strobe all leds
void strobe()
{
  int blackLeds=loopPass%2;
  
  for(int i=0; i<pixels.numPixels(); i++) {
    if(blackLeds==0)
    {
      pixels.setPixelColor(i, 0,0,0);      
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);       
    }
    
  }
  pixels.show();
  delay(delay_value);
}

// strobe one led sequentially
void strobe_sequential()
{
  int blackLedId=loopPass%pixels.numPixels();
  
  for(int i=0; i<pixels.numPixels(); i++) {
    if(i==blackLedId)
    {
      pixels.setPixelColor(i, 0,0,0);      
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);       
    }
    
  }
  pixels.show();
  delay(delay_value);
}

// strobe all leds execpt one sequentially
void reverse_strobe_sequential()
{
  int blackLedId=loopPass%pixels.numPixels();
  
  for(int i=0; i<pixels.numPixels(); i++) {
    if(i==blackLedId)
    {
      pixels.setPixelColor(i, ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);     
    }
    else
    {
      pixels.setPixelColor(i, 0,0,0);          
    }
    
  }
  pixels.show();
  delay(delay_value);
}
// fade all leds
void fade()
{ 
  int fadePower=128;
  //linear fade 
  int fadeValue=(loopPass%fadePower)-(fadePower/2);  

  for(int i=0; i<pixels.numPixels(); i++) {
    int roundedR=round(ledColorState[i].r/(abs(fadeValue)+1));
    int roundedG=round(ledColorState[i].g/(abs(fadeValue)+1));
    int roundedB=round(ledColorState[i].b/(abs(fadeValue)+1));
    pixels.setPixelColor(i, roundedR,roundedG,roundedB);    
  }
  pixels.show();
  delay(delay_value);
}

// fade one led sequentially
void fade_sequential()
{
  int fadePower=128;
  //linear fade
  int fadeValue=(loopPass%fadePower)-((fadePower-1)/2);  
  //selected led
  int ledFaded=((loopPass-(fadePower/2))/fadePower)%(pixels.numPixels());

  for(int i=0; i<pixels.numPixels(); i++) {
    if(i==ledFaded)
    {
      int roundedR=round(ledColorState[i].r/(abs(fadeValue)+1));
      int roundedG=round(ledColorState[i].g/(abs(fadeValue)+1));
      int roundedB=round(ledColorState[i].b/(abs(fadeValue)+1));
      pixels.setPixelColor(i, roundedR,roundedG,roundedB);      
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);      
    }
  }
  pixels.show();
  delay(delay_value);
}

// fade all leds exectp one sequentially
void reverse_fade_sequential()
{
  int fadePower=128;
  //linear fade
  int fadeValue=(loopPass%fadePower)-((fadePower-1)/2);  
  //selected led
  int ledFaded=((loopPass-(fadePower/2))/fadePower)%(pixels.numPixels());

  for(int i=0; i<pixels.numPixels(); i++) {
    if(i==ledFaded)
    {    
      pixels.setPixelColor(i, ledColorState[i].r,ledColorState[i].g,ledColorState[i].b);  
    }
    else
    {
      int roundedR=round(ledColorState[i].r/(abs(fadeValue)+1));
      int roundedG=round(ledColorState[i].g/(abs(fadeValue)+1));
      int roundedB=round(ledColorState[i].b/(abs(fadeValue)+1));
      pixels.setPixelColor(i, roundedR,roundedG,roundedB);      
    }
  }
  pixels.show();
  delay(delay_value);
}

// main loop
// everything is based on number of cycles
void loop(){
  switch(Current_Led_Transition) {
    case STROBE:
      strobe();    
      break;
    case STROBE_SEQUENTIAL:
      strobe_sequential();
      break;
    case REVERSE_STROBE_SEQUENTIAL:
      reverse_strobe_sequential();
      break;
    case FADE:
      fade();    
      break;
    case FADE_SEQUENTIAL:
      fade_sequential();
      break;
    case REVERSE_FADE_SEQUENTIAL:
      reverse_fade_sequential();
      break;
    case RAINBOW:
      rainbow();
      break;
    case STANDARD:
    default:
      standard();
  }
  loopPass++;
}
