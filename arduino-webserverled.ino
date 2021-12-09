// This code was tested with RGB ws2812b and ESP32 (lolin D32).
// It requires the following libraries
//
// Adafruit_NeoPixel :https://github.com/adafruit/Adafruit_NeoPixel
// ESPAsyncWebServer : https://github.com/me-no-dev/ESPAsyncWebServer
//
// and the following hardware features :
//
// SPIFFS
// Wi-Fi

#include <WiFi.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 27

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 200

// Replace with your network credentials
const char *ssid = "your ssid";
const char *password = "your password";
const char *espHostname = "your hostname";
const char *ota_password = "your password";

// All differents modes
enum Led_Transition
{
  STANDARD,
  RAINBOW,
  SLIDE,
  FADE,
  FADE_SEQUENTIAL,
  REVERSE_FADE_SEQUENTIAL,
  STROBE,
  STROBE_SEQUENTIAL,
  REVERSE_STROBE_SEQUENTIAL,
  CUSTOM_1,
  CUSTOM_2,
  CUSTOM_3,
  CUSTOM_4,
  CUSTOM_5
};
enum Led_Direction
{
  FORWARD,
  BACKWARD
};
struct ledColor
{
  int r;
  int g;
  int b;
};

// var and initial values
ledColor ledColorState[NUMPIXELS];
Led_Transition Current_Led_Transition = STANDARD;
Led_Direction Current_Led_Direction = FORWARD;
int loopPass = 0;
int delay_value = 10;
int brightness_value = 1;

void initLedsColorState()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    ledColorState[i].r = 255;
    ledColorState[i].g = 0;
    ledColorState[i].b = 0;
  }
}

// Will cork with RGBW but will not use true white.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// set initial variable inside html file to track actual state
String processor(const String &var)
{
  int num_pixels = NUMPIXELS;
  if (var == "LED_NUMBER")
  {
    return String(num_pixels);
  }
  if (var == "LEDS_OBJS")
  {
    return getAllLedsColorJSON();
  }
  if (var == "DELAY_VALUE")
  {
    return String(delay_value);
  }
  if (var == "BRIGHTNESS_VALUE")
  {
    return String(brightness_value);
  }
  if (var == "DIRECTION_VALUE")
  {
    return String(Current_Led_Direction);
  }
  if (var == "MODE_VALUE")
  {
    String Current_Led_Transition_str = "STANDARD";
    if (Current_Led_Transition == STROBE)
    {
      Current_Led_Transition_str = "STROBE";
    }
    if (Current_Led_Transition == STROBE_SEQUENTIAL)
    {
      Current_Led_Transition_str = "STROBE_SEQUENTIAL";
    }
    if (Current_Led_Transition == REVERSE_STROBE_SEQUENTIAL)
    {
      Current_Led_Transition_str = "REVERSE_STROBE_SEQUENTIAL";
    }
    if (Current_Led_Transition == FADE)
    {
      Current_Led_Transition_str = "FADE";
    }
    if (Current_Led_Transition == FADE_SEQUENTIAL)
    {
      Current_Led_Transition_str = "FADE_SEQUENTIAL";
    }
    if (Current_Led_Transition == REVERSE_FADE_SEQUENTIAL)
    {
      Current_Led_Transition_str = "REVERSE_FADE_SEQUENTIAL";
    }
    if (Current_Led_Transition == SLIDE)
    {
      Current_Led_Transition_str = "SLIDE";
    }
    if (Current_Led_Transition == RAINBOW)
    {
      Current_Led_Transition_str = "RAINBOW";
    }
    if (Current_Led_Transition == CUSTOM_1)
    {
      Current_Led_Transition_str = "CUSTOM_1";
    }
    if (Current_Led_Transition == CUSTOM_2)
    {
      Current_Led_Transition_str = "CUSTOM_2";
    }
    if (Current_Led_Transition == CUSTOM_3)
    {
      Current_Led_Transition_str = "CUSTOM_3";
    }
    if (Current_Led_Transition == CUSTOM_4)
    {
      Current_Led_Transition_str = "CUSTOM_4";
    }
    if (Current_Led_Transition == CUSTOM_5)
    {
      Current_Led_Transition_str = "CUSTOM_5";
    }
    if (Current_Led_Transition == STANDARD)
    {
      Current_Led_Transition_str = "STANDARD";
    }
    return Current_Led_Transition_str;
  }
  return String();
}

void setup()
{
  Serial.begin(115200);

  initLedsColorState();

  // Initialize rgb leds
  pixels.setBrightness(brightness_value);
  pixels.begin();
  pixels.clear();

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.setHostname(espHostname);
  WiFi.mode(WIFI_STA);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // OTA
  ArduinoOTA.setHostname(espHostname);
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // static files
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.serveStatic("/jquery-3.6.0.min.js", SPIFFS, "/jquery-3.6.0.min.js");

  // main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
     Serial.println("Request : /");
    request->send(SPIFFS, "/index.html", String(), false, processor); });

  // AJAX change specific led color
  server.on("/led/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
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
    request->send(200, "text/json", "OK"); });

  // AJAX change all led color
  server.on("/allleds/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        
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
    request->send(200, "text/json", "OK"); });

  // AJAX change parameters (delay or brightness or direction)
  server.on("/parameters/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        
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
      
      if(p_name->value() == "direction")
      {
        int direction_value = p_value->value().toInt();        
        Current_Led_Direction = (Led_Direction)direction_value;
      }   
      
    }      
    request->send(200, "text/json", "OK"); });

  // AJAX change leds modes
  server.on("/modes/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                
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
      if(modestr == "SLIDE")
      {
        Current_Led_Transition = SLIDE;
      }
      if(modestr == "RAINBOW")
      {
        Current_Led_Transition = RAINBOW;
      }
      if(modestr == "CUSTOM_1")
      {
        Current_Led_Transition = CUSTOM_1;
      }
      if(modestr == "CUSTOM_2")
      {
        Current_Led_Transition = CUSTOM_2;
      }
      if(modestr == "CUSTOM_3")
      {
        Current_Led_Transition = CUSTOM_3;
      }
      if(modestr == "CUSTOM_4")
      {
        Current_Led_Transition = CUSTOM_4;
      }
      if(modestr == "CUSTOM_5")
      {
        Current_Led_Transition = CUSTOM_5;
      }
      if(modestr == "STANDARD")
      {
        Current_Led_Transition = STANDARD;
      }       
    }      
    request->send(200, "text/json", "OK"); });

  // AJAX change leds templates, return all leds values
  server.on("/templates/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                
    AsyncWebParameter* p_template = nullptr;    
      
    if(request->hasParam("template")) 
    {
      p_template = request->getParam("template");
      String templatestr=p_template->value();
      
      if(templatestr == "CHRISTMAS_RW" )
      {
         for(int i=0; i<NUMPIXELS; i++) {
          if(i%2 == 0)
          { 
            ledColorState[i].r=255;
            ledColorState[i].g=0;
            ledColorState[i].b=0;    
          }
          else
          {
            ledColorState[i].r=255;
            ledColorState[i].g=255;
            ledColorState[i].b=255;     
          }             
        }        
      }
      if(templatestr == "CHRISTMAS_GW" )
      {
         for(int i=0; i<NUMPIXELS; i++) {
          if(i%2 == 0)
          {
            ledColorState[i].r=179;
            ledColorState[i].g=163;
            ledColorState[i].b=4;    
          }
          else
          {
            ledColorState[i].r=255;
            ledColorState[i].g=255;
            ledColorState[i].b=255;     
          }             
        }  
      }
      if(templatestr == "WB")
      {
         for(int i=0; i<NUMPIXELS; i++) {
          if(i%2 == 0)
          {
            ledColorState[i].r=0;
            ledColorState[i].g=0;
            ledColorState[i].b=0;    
          }
          else
          {
            ledColorState[i].r=255;
            ledColorState[i].g=255;
            ledColorState[i].b=255;     
          }
        }
      }
      if(templatestr == "RANDOM")
      {
        for(int i=0; i<NUMPIXELS; i++) {
            int randNumber_r = random(255);
            int randNumber_g = random(255);
            int randNumber_b = random(255);
           ledColorState[i].r=randNumber_r;
           ledColorState[i].g=randNumber_g;
           ledColorState[i].b=randNumber_b;     
        }
        
      }
      if(templatestr == "BWR")
      {
         for(int i=0; i<NUMPIXELS; i++) {
          if(i%3 == 0)
          {
            ledColorState[i].r=0;
            ledColorState[i].g=0;
            ledColorState[i].b=255;    
          }
          if(i%3 == 1)
          {
            ledColorState[i].r=255;
            ledColorState[i].g=255;
            ledColorState[i].b=255;     
          }
          if(i%3 == 2)
          {
            ledColorState[i].r=255;
            ledColorState[i].g=0;
            ledColorState[i].b=0;     
          }
        }  
      }     
    }

    
    String output = getAllLedsColorJSON();
    
    request->send(200, "text/json", output); });

  // Start server
  server.begin();
}

String getAllLedsColorJSON()
{
  String output = "{";
  for (int i = 0; i < NUMPIXELS; i++)
  {
    char buffer_str[256];
    if (i < NUMPIXELS - 1)
    {
      sprintf(buffer_str, "\"%d\":{\"r\":%d,\"g\":%d,\"b\":%d},", i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
    else
    {
      sprintf(buffer_str, "\"%d\":{\"r\":%d,\"g\":%d,\"b\":%d}", i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
    output += String(buffer_str);
  }
  output += "}";
  return output;
}

// leds with colors speficied
void standard()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {
    pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
  }
  pixels.show();
}

// based on adafruit code
// leds with rainbow colors
// selected colors are not used in this mode
void rainbow()
{
  int wait = delay_value / 2;
  int firstPixelHue = 255 * loopPass;
  for (int i = 0; i < pixels.numPixels(); i++)
  {
    uint32_t pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
    pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue, 255)));
  }
  pixels.show();
  delay(delay_value);
}

// strobe all leds
void strobe()
{
  int blackLeds = loopPass % 2;

  for (int i = 0; i < pixels.numPixels(); i++)
  {
    if (blackLeds == 0)
    {
      pixels.setPixelColor(i, 0, 0, 0);
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
  }
  pixels.show();
  delay(delay_value);
}

// strobe one led sequentially
void strobe_sequential()
{
  int blackLedId = loopPass % pixels.numPixels();

  for (int i = 0; i < pixels.numPixels(); i++)
  {
    if (i == blackLedId)
    {
      pixels.setPixelColor(i, 0, 0, 0);
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
  }
  pixels.show();
  delay(delay_value);
}

// strobe all leds execpt one sequentially
void reverse_strobe_sequential()
{
  int blackLedId = loopPass % pixels.numPixels();

  for (int i = 0; i < pixels.numPixels(); i++)
  {
    if (i == blackLedId)
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
    else
    {
      pixels.setPixelColor(i, 0, 0, 0);
    }
  }
  pixels.show();
  delay(delay_value);
}
// fade all leds
void fade()
{
  int fadePower = 128;
  // linear fade
  int fadeValue = (loopPass % fadePower) - (fadePower / 2);

  for (int i = 0; i < pixels.numPixels(); i++)
  {
    int roundedR = round(ledColorState[i].r / (abs(fadeValue) + 1));
    int roundedG = round(ledColorState[i].g / (abs(fadeValue) + 1));
    int roundedB = round(ledColorState[i].b / (abs(fadeValue) + 1));
    pixels.setPixelColor(i, roundedR, roundedG, roundedB);
  }
  pixels.show();
  delay(delay_value);
}

// fade one led sequentially
void fade_sequential()
{
  int fadePower = 128;
  // linear fade
  int fadeValue = (loopPass % fadePower) - ((fadePower - 1) / 2);
  // selected led
  int ledFaded = ((loopPass - (fadePower / 2)) / fadePower) % (pixels.numPixels());

  for (int i = 0; i < pixels.numPixels(); i++)
  {
    if (i == ledFaded)
    {
      int roundedR = round(ledColorState[i].r / (abs(fadeValue) + 1));
      int roundedG = round(ledColorState[i].g / (abs(fadeValue) + 1));
      int roundedB = round(ledColorState[i].b / (abs(fadeValue) + 1));
      pixels.setPixelColor(i, roundedR, roundedG, roundedB);
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
  }
  pixels.show();
  delay(delay_value);
}

// fade all leds exectp one sequentially
void reverse_fade_sequential()
{
  int fadePower = 128;
  // linear fade
  int fadeValue = (loopPass % fadePower) - ((fadePower - 1) / 2);
  // selected led
  int ledFaded = ((loopPass - (fadePower / 2)) / fadePower) % (pixels.numPixels());

  for (int i = 0; i < pixels.numPixels(); i++)
  {
    if (i == ledFaded)
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
    else
    {
      int roundedR = round(ledColorState[i].r / (abs(fadeValue) + 1));
      int roundedG = round(ledColorState[i].g / (abs(fadeValue) + 1));
      int roundedB = round(ledColorState[i].b / (abs(fadeValue) + 1));
      pixels.setPixelColor(i, roundedR, roundedG, roundedB);
    }
  }
  pixels.show();
  delay(delay_value);
}

// slide leds
void slide()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {
    int mapped_led = (i + loopPass) % pixels.numPixels();
    pixels.setPixelColor(i, ledColorState[mapped_led].r, ledColorState[mapped_led].g, ledColorState[mapped_led].b);
  }
  pixels.show();
  delay(delay_value);
}

void custom_1()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {
    if ((i+loopPass)%2 == 0)
    {
      pixels.setPixelColor(i, 0, 0, 0);
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
  }
  pixels.show();
  delay(delay_value);
}

void custom_2()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {
    int lastLed=loopPass%pixels.numPixels();
    if (i < lastLed)
    {
      pixels.setPixelColor(i, 0, 0, 0);
    }
    else
    {
      pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
    }
  }
  pixels.show();
  delay(delay_value);
}

void custom_3()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {
    int lastLed=loopPass%(pixels.numPixels()*2);
    if(lastLed < pixels.numPixels())
    {
      if (i < lastLed)
      {
        pixels.setPixelColor(i, 0, 0, 0);
      }
      else
      {
        pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
      }
    }
    else
    {
      int newlastLed= pixels.numPixels() - (lastLed - pixels.numPixels());
      if (i < newlastLed)
      {
        pixels.setPixelColor(i, 0, 0, 0);
      }
      else
      {
        pixels.setPixelColor(i, ledColorState[i].r, ledColorState[i].g, ledColorState[i].b);
      }
      
    }
  }
  pixels.show();
  delay(delay_value);
}


void custom_4()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {    
    float multiplicator= abs(sin((loopPass+i)*0.05));
    pixels.setPixelColor(i, ledColorState[i].r*multiplicator, ledColorState[i].g*multiplicator, ledColorState[i].b*multiplicator);    
  }
  pixels.show();
  delay(delay_value);
}


void custom_5()
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {        
    int temp = ((i+loopPass) % 10) + 1;
    float multiplicator= 1.f / temp;
    pixels.setPixelColor(i, ledColorState[i].r*multiplicator, ledColorState[i].g*multiplicator, ledColorState[i].b*multiplicator);    
  }
  pixels.show();
  delay(delay_value);
}

// main loop
// everything is based on number of cycles
void loop()
{
  ArduinoOTA.handle();
  switch (Current_Led_Transition)
  {
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
  case SLIDE:
    slide();
    break;
  case RAINBOW:
    rainbow();
    break;
  case CUSTOM_1:
    custom_1();
    break;
  case CUSTOM_2:
    custom_2();
    break;
  case CUSTOM_3:
    custom_3();
    break;
  case CUSTOM_4:
    custom_4();
    break;
  case CUSTOM_5:
    custom_5();
    break;
  case STANDARD:
  default:
    standard();
  }

  if (Current_Led_Direction == FORWARD)
  {
    loopPass++;
  }
  else
  {
    loopPass--;
  }
}
