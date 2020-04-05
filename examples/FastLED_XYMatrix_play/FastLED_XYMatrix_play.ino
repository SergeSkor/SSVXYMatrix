//
// By Serge Skorodinsky, 11/15/2019
//

#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include "SSVXYMatrix.h"
#include "SSVRGBGradientCalc.h"
#include "SSVXYMatrixText.h"

FASTLED_USING_NAMESPACE

#include <SSVTimer.h>
SSVTimer Tmr_effects; 
SSVTimer Tmr_effectChange; 
SSVTimer Tmr_OnBoardLED; 

#include <ESP8266WebServer.h>   //WiFi object declared here. Also declared in <ESP8266WiFi.h>
ESP8266WebServer webserver (80);
//home
const char *ssid = "Serge S. home (2.4GHz)";
const char *password = "dell_latitude";

//.33 router
//const char* ssid     = "SLEWIFITEST9";
//const char* password = "TESTENG109";


#include "FS.h" //file system
#include <ArduinoOTA.h>

//HostName is for three things - WiFi.hostname, mDNS hostname(.local) and OTA.setHostname
const char* HostNameTemplate = "Matrix-%06x";
char HostName[15];

#define onboard_led 2 //D4, onboard LED, see FASTLED_ESP8266_RAW_PIN_ORDER above!
#define LED_PIN  D8 //15 //ESP8266 Lolin ESP-12E module

#define COLOR_ORDER GRB
#define CHIPSET     WS2811

//#define BRIGHTNESS 32

// Params for width and height
const uint8_t MatrixWidth = 16;
const uint8_t MatrixHeight = 16;

#define NUM_LEDS (MatrixWidth * MatrixHeight)

CRGB leds[NUM_LEDS];

XYMatrix matrix(leds, MatrixWidth, MatrixHeight, true);
XYMatrixText matrixText(&matrix);

//ahead declarations
void TimerEffect1Func();
void TimerEffect2Func();
void TimerEffect3Func();
void TimerEffect4Func();
void TimerEffect5Func();
void TimerEffect6Func();
void TimerEffect7Func();
void TimerEffect8Func();
void TimerEffect9Func();
void TimerEffect10Func();
void TimerEffect11Func();
void TimerEffect12Func();
void TimerEffect13Func();
void TimerEffect14Func();
void TimerEffect15Func();

typedef struct EffectsStruct 
  {timer_callback EffectFunc; const char* EffectName; };

EffectsStruct Effects[] = 
  {
  {TimerEffect1Func,  "#1: 2D Rainbow, 4 Frames"},
  {TimerEffect2Func,  "#2: 2D Rainbow, Full Matrix"},
  {TimerEffect3Func,  "#3: Lissajous figure"},
  {TimerEffect4Func,  "#4: Kaleidoscope"},
  {TimerEffect5Func,  "#5: Fade Rectangles"},
  {TimerEffect6Func,  "#6: Fade Circles"},
  {TimerEffect7Func,  "#7: Fade Lines"},
  {TimerEffect8Func,  "#8: Circling Ball"},
  {TimerEffect9Func,  "#9: Circling Lines"},
  {TimerEffect10Func, "#10: Pattern Shift"},
  {TimerEffect11Func, "#11: Colors Shift"},
  {TimerEffect12Func, "#12: Colors Shift 2(to debug)"},
  {TimerEffect13Func, "#13: 2D Plasma"},
  {TimerEffect14Func, "#14: 2D Plasma (Random Palette)"},
  {TimerEffect15Func, "#15: 2D Plasma (Fade to Random Palette)"},
  };

String HTMLOptionList;
char HTMLOptionTpl[] = "<option value='?Effect=%Effect#%' %EffSel%>%EffName%</option>\n";

uint8_t effectnum=0;
uint8_t TotalEffects;//uint8_t arrsize = (sizeof(Effects))/(sizeof(Effects[0]));

char effect_init_str[] = "...effect #%d init.\n";

enum  TEffectChangeType {ECT_RANDOM=0, ECT_SEQUENTIAL=1, ECT_NO_CHANGE=2};

//control variables
boolean IsON=false;
unsigned long OFFtimestamp;
TEffectChangeType EffChangeType = ECT_RANDOM;
unsigned long EffChangeInterval = 60; //default effect change interval, seconds
uint8_t Brightness = 100; //default brightness, 0...255
CRGBPalette16 Palette1, Palette2;

void WaitForSerial(const char *serial_message)
{
Serial.println(serial_message);

//Serial.println("\r    Press <Send> in serial terminal...\n");
//while (Serial.available()) {Serial.read();} //to clear incoming buffer
//while (! (Serial.available() ) ) {}; //wait for any char
//while (Serial.available()) {Serial.read();} //to clear incoming buffer

delay(1000);
}

void CombineHTMLOptionList()
{
HTMLOptionList = ""; //global
String S;
for (uint8_t i=0; i<TotalEffects; i++) 
  {
  S = HTMLOptionTpl;
  S.replace("%Effect#%", String(i+1));
  S.replace("%EffName%", Effects[i].EffectName);
  S.replace("%EffSel%", (i==effectnum)?"selected":"");
  HTMLOptionList = HTMLOptionList + S;
  }
}

void Tmr_EffectChangeCB()
{
if (!IsON) return; //not active

switch (EffChangeType)
  {
    case ECT_RANDOM: 
      effectnum=random8(TotalEffects);
      break;
    
    case ECT_SEQUENTIAL: 
      effectnum++; 
      if (effectnum >= TotalEffects) effectnum=0; 
      break;  
    
    case ECT_NO_CHANGE: 
      break;
    
    default: 
      effectnum=random8(TotalEffects); //default random
      break;
  }



Tmr_effects.SetOnTimer(Effects[effectnum].EffectFunc);
Tmr_effects.ResetCounter();
//Combine HTMLOptionList to use in HTTP resp, only when effect changed for optimization
CombineHTMLOptionList();
Tmr_effectChange.SetEnabled(false); Tmr_effectChange.SetEnabled(true); //to restart timer
//Serial.print("Change effect to #"); Serial.println(effectnum+1);
Serial.printf("Change effect to #%d, ", effectnum+1);
Serial.print("Name: "); Serial.println(Effects[effectnum].EffectName);
}

void setup() 
{
  TotalEffects = (sizeof(Effects))/(sizeof(Effects[0]));
  CombineHTMLOptionList();
  Serial.begin(115200);
  pinMode(onboard_led, OUTPUT ); //led
  digitalWrite(onboard_led, HIGH); //off
  Tmr_effects.SetInterval(20);
  Tmr_effects.SetOnTimer(Effects[0].EffectFunc);
  Tmr_effects.SetEnabled(true);
  Tmr_effectChange.SetOnTimer(Tmr_EffectChangeCB);
  Tmr_effectChange.SetInterval(EffChangeInterval * 1000); //default is 60 - change once in a minute
  Tmr_effectChange.SetEnabled(false);
  Tmr_OnBoardLED.SetInterval(20);
  Tmr_OnBoardLED.SetOnTimer(Tmr_OnBrdLED);
  Tmr_OnBoardLED.SetEnabled(false);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( Brightness );
  FastLED.clear ();
  FastLED.show();
  delay(100); //for stability
  Serial.println();Serial.println();
  //HostName
  sprintf(HostName, HostNameTemplate, ESP.getChipId());
  WiFi.mode(WIFI_STA);  //WiFi.mode(WIFI_AP_STA);  
  WiFi.hostname(HostName);
  WiFi.begin(ssid, password);
  //connect to wifi
  unsigned long t=0;
  while (WiFi.status() != WL_CONNECTED) 
    {
    t++;
    //fill_solid (struct CRGB *leds, int numToFill, const struct CRGB &color)
    //fill_gradient_RGB(leds, NUM_LEDS, CRGB::Green, CRGB::Red);
    //fill_rainbow (leds, t+1, 0, 32);
    FastLED.clear();
    leds[t % NUM_LEDS]=CRGB::LightPink;
    FastLED.show();  
    Serial.print(".");
    digitalWrite(onboard_led, !digitalRead(onboard_led) );
    delay(100);
    }
  Serial.println();
  Serial.print("WiFi connected with ip ");  
  Serial.println(WiFi.localIP());
  digitalWrite(onboard_led, HIGH); //off
  Serial.printf("Host Name: %s\r\n", HostName);
  //web-server
  webserver.on ("/", webSrvHandleRoot);
  webserver.onNotFound (webSrvHandleNotFound);
  webserver.on ("/diag", handleDiagInfo);
  webserver.begin();
  Serial.println ("HTTP server started");
  SPIFFS.begin();  
  //     ArduinoOTA
  //ArduinoOTA.setPort(8266);            // Port defaults to 8266
  ArduinoOTA.setHostname(HostName); // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setPassword("admin");    // No authentication by default
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3"); //not working (S.S.)!!
  ArduinoOTA.onStart(OTA_onStart);
  ArduinoOTA.onEnd(OTA_onEnd);
  ArduinoOTA.onProgress(OTA_onProgress);
  ArduinoOTA.onError(OTA_onError);
  ArduinoOTA.begin();


  StaticFramesDemo(); ////////////////////////////////////////////////////////////////

  
  
  //done, prepare to start main loop
  if (IsON)
    {
    IsON=true;
    Tmr_effects.SetOnTimer(Effects[effectnum].EffectFunc);
    Tmr_effectChange.SetEnabled(true);
    Tmr_effects.ResetCounter();
    Tmr_effects.SetEnabled(false);
    Tmr_OnBoardLED.SetEnabled(false);
    Serial.printf("Init ON\r\n");
    }
    else 
    {
    IsON=false;
    Tmr_effects.SetOnTimer(TimerOFFFunc1);
    Tmr_effectChange.SetEnabled(false);
    Tmr_effects.ResetCounter();
    Tmr_OnBoardLED.SetEnabled(true);
    Serial.printf("Init OFF\r\n");
    OFFtimestamp=millis();
    }
  Serial.print("Effect #1, Name: ");
  Serial.println(Effects[0].EffectName);
} //end of setup()

void loop()
{
  Tmr_effects.RefreshIt();
  Tmr_effectChange.RefreshIt();
  Tmr_OnBoardLED.RefreshIt();
  webserver.handleClient(); //check httpserver
  ArduinoOTA.handle();
}

void Tmr_OnBrdLED()
{
  //digitalWrite(onboard_led, !digitalRead(onboard_led) );
  static uint32_t phaseshift; //to start always from zero brightness
  if ( (millis() - OFFtimestamp)>5000 ) //older than 5 seconds ago, better to check if all leds are black?
    {
    if (Tmr_effects.GetEnabled()) 
      {//first time when started
      phaseshift=millis()-500; //quoter cycle shift. If beat-rate is 30 (2 Hz) - full cycle is 2000, quoter is 500
      Tmr_effects.SetEnabled(false);
      }
    analogWrite(onboard_led, beatsin16(30/*beat-rate*/, 0, 1023, phaseshift ) ); //sin, conflicting with FastLED.show()!!! 
    }
}

void TimerEffect1Func()
{
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  //FastLED.clear (); //not needed, all matrix updated anyway
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / MatrixWidth));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / MatrixHeight));
    
  RectangleWH R1 = {Point(0,0), 7, 7};
  matrix.draw_2DRainbow(R1, ms/65536*1000, Point(3,3), yHueDelta32/32768, xHueDelta32/32768);
    
  RectangleWH R2 = {Point(8,0), 7, 7};
  matrix.draw_2DRainbow(R2, ms/65536*100, Point(11,3), xHueDelta32/32768*2, yHueDelta32/32768*2);
    
  RectangleWH R3 = {Point(0,8), 7, 7};
  matrix.draw_2DRainbow(R3, ms/65536*500, Point(3,11), yHueDelta32/32768*2, xHueDelta32/32768*2);
    
  RectangleWH R4 = {Point(8,8), 7, 7};
  matrix.draw_2DRainbow(R4, ms/65536*2000, Point(11,11), xHueDelta32/32768, yHueDelta32/32768);
  FastLED.show();
}

void TimerEffect2Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  //FastLED.clear (); //not needed, all matrix updated anyway
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / MatrixWidth));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / MatrixHeight));
    
  RectangleWH R1 = {Point(0,0), 15, 15};
  matrix.draw_2DRainbow(R1, ms/65536*1000, Point(7,7), yHueDelta32/32768, xHueDelta32/32768);
  FastLED.show();
}

void TimerEffect3Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  uint8_t posX = triwave8(beat8(37)) /16;
  uint8_t posY = triwave8(beat8(23)) /16;
  uint8_t hue = beat8(5);
  matrix.setPixelColor(posX,posY, CHSV( hue, 255, 255) );
  FastLED.show();
}

void TimerEffect4Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
/*  colors layout: 
  3  6  9
  2  5  8
  1  4  7
 */
  CRGB cCorners =  CHSV( triwave8(beat8(13)), 255, triwave8(beat8(7)));
  CRGB cMidSides = CHSV( triwave8(beat8(11)), 255, triwave8(beat8(3)));
  CRGB cCenter =   CHSV( triwave8(beat8(9)), 255, triwave8(beat8(2)));
  
  CRGB c1 = cCorners;
  CRGB c3 = cCorners;
  CRGB c7 = cCorners;
  CRGB c9 = cCorners;

  CRGB c2 = cMidSides;
  CRGB c4 = cMidSides;
  CRGB c6 = cMidSides;
  CRGB c8 = cMidSides;

  CRGB c5 = cCenter;
  
  RectangleWH R1 = {Point(0,0), 7, 7};
  RectangleWH R2 = {Point(8,0), 7, 7};
  RectangleWH R3 = {Point(0,8), 7, 7};
  RectangleWH R4 = {Point(8,8), 7, 7};

  matrix.draw_2DGradient(R1, c1, c2, c4, c5); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
  matrix.draw_2DGradient(R2, c4, c5, c7, c8); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
  matrix.draw_2DGradient(R3, c2, c3, c5, c6); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
  matrix.draw_2DGradient(R4, c5, c6, c8, c9); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
 
  FastLED.show();
}

void TimerEffect5Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  long how_often=250; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     uint8_t s=random(3,10); //square size
     RectangleWH R = {Point(random(15-s), random(15-s)), s, s};
     if (random(255) > 128) 
       matrix.draw_rect(R, CHSV( random(255), 255, 255));
      else 
       matrix.draw_fillrect(R, CHSV( random(255), 255, 255));
    }
  FastLED.show();
}

void TimerEffect6Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  long how_often=250; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     uint8_t r=random(2,6); //radius
     uint8_t cx = random (r, MatrixWidth-r);
     uint8_t cy = random (r, MatrixHeight-r);
     if (random(255) > 128) 
       matrix.draw_circle(cx, cy, r, CHSV( random(255), 255, 255));
      else 
       matrix.draw_fillCircle(cx, cy, r, CHSV( random(255), 255, 255));
    }
  FastLED.show();
}

void TimerEffect7Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  long how_often=250; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     uint8_t x1 = random (MatrixWidth-1);
     uint8_t y1 = random (MatrixHeight-1);
     uint8_t x2 = random (MatrixWidth-1);
     uint8_t y2 = random (MatrixHeight-1);
     matrix.draw_line_gradient(x1, y1, x2, y2, CHSV( random(255), 255, 255), CHSV( random(255), 255, 255));
    }
  FastLED.show();
}

void TimerEffect8Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int16_t r = 2;
  uint8_t posCX = beatsin8(20, r, MatrixWidth-r, 0, 0 );
  uint8_t posCY = beatsin8(20, r, MatrixHeight-r, 0, 64 ); //90 deg phase shift
  uint8_t hue = beat8(5);
  matrix.draw_circle(posCX, posCY, r, CHSV(hue, 255, 255) );
  FastLED.show();
}

void TimerEffect9Func()
{
    if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf(effect_init_str, effectnum+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  //external circle
  int16_t r = 1;
  uint8_t f = 15; //frequency
  uint8_t CX1 = beatsin8(f, r, MatrixWidth-r,  0, 0 );
  uint8_t CY1 = beatsin8(f, r, MatrixHeight-r, 0, -64 ); //90 deg phase shift
  CRGB clr1 = CRGB::Red; //beat8(5);

  uint8_t CX2 = beatsin8(f, r, MatrixWidth-r,  0, 0+85 );
  uint8_t CY2 = beatsin8(f, r, MatrixHeight-r, 0, -64+85 ); //90 deg phase shift
  CRGB clr2 = CRGB::Green; //beat8(5);

  uint8_t CX3 = beatsin8(f, r, MatrixWidth-r,  0, 0+170 );
  uint8_t CY3 = beatsin8(f, r, MatrixHeight-r, 0, -64+170 ); //90 deg phase shift
  CRGB clr3 = CRGB::Blue; //beat8(5);

  matrix.setPixelColor(CX1, CY1, clr1);
  matrix.setPixelColor(CX2, CY2, clr2);
  matrix.setPixelColor(CX3, CY3, clr3);


  //internal circle
  r = 4;
  //f = 10; //frequency
  uint8_t CX4 = beatsin8(f, r, MatrixWidth-r,  0, 0 );
  uint8_t CY4 = beatsin8(f, r, MatrixHeight-r, 0, 64 ); //90 deg phase shift
  CRGB clr4 = CRGB::Yellow; //beat8(5);

  uint8_t CX5 = beatsin8(f, r, MatrixWidth-r,  0, 0+85 );
  uint8_t CY5 = beatsin8(f, r, MatrixHeight-r, 0, 64+85 ); //90 deg phase shift
  CRGB clr5 = CRGB::Pink; //beat8(5);

  uint8_t CX6 = beatsin8(f, r, MatrixWidth-r,  0, 0+170 );
  uint8_t CY6 = beatsin8(f, r, MatrixHeight-r, 0, 64+170 ); //90 deg phase shift
  CRGB clr6 = CRGB::Magenta; //beat8(5);

  matrix.setPixelColor(CX4, CY4, clr4);
  matrix.setPixelColor(CX5, CY5, clr5);
  matrix.setPixelColor(CX6, CY6, clr6);

  //center
  CRGB clr7 = CHSV(beat8(f), 255, 255);
  matrix.setPixelColor(8, 8, clr7);
  
  FastLED.show();
}

void TimerEffect10Func()
{
  static uint8_t dir;
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      FastLED.clear();
      matrix.draw_rect(2, 2, 13, 13,  CRGB::Green);
      matrix.draw_fillrect(5, 5, 10, 10,  CRGB::Yellow);
      matrix.draw_line(0, 0,  15, 15, CRGB::Blue,true);
      matrix.draw_line(0, 15, 15, 0,  CRGB::Red, true);
      dir=0;
      Serial.printf(effect_init_str, effectnum+1);
    }

  long how_often=5000; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     dir = random (8);
    }
    
  how_often=100; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     matrix.Shift_RectRoundDir(dir);
    }
  FastLED.show();  
}

void TimerEffect11Func()
{
  static uint8_t dir;
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      FastLED.clear();
      dir=0;
      Serial.printf(effect_init_str, effectnum+1);
    }

  long how_often=500; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     dir = random (8);
    }
    
  how_often=75; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {

     CRGB c1 =  CHSV( triwave8(beat8(13)), 255, 255); //13
     CRGB c2 =  CHSV( triwave8(beat8(8)),  255, 255); //8
     //fadeToBlackBy( leds, NUM_LEDS, 40);
     matrix.Shift_RectDir(dir, c1, c2);
    }
  FastLED.show();  
}

void TimerEffect12Func()
{
  static uint8_t dir;
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      FastLED.clear();
      dir=0;
      Serial.printf(effect_init_str, effectnum+1);
    }

  long how_often=500; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {
     dir = random (8);
    }
    
  how_often=75; //mS
  if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
    {

//beatsin8(f, r, MatrixHeight-r, 0, 64+170 ); //90 deg phase shift

     CRGB c1[16]; for( byte i=0; i <= 15; i++) {c1[i]=CHSV(beatsin8(5, i), 255, 255);} //not done yet
     CRGB c2[16]; for( byte i=0; i <= 15; i++) {c2[i]=CHSV(beatsin8(7, i), 255, 255);} //not done yet
     //fadeToBlackBy( leds, NUM_LEDS, 40);
     matrix.Shift_RectDir(dir, c1, c2);
    }
  FastLED.show();  
}

void TimerEffect13Func()
{
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      FastLED.clear();
      Serial.printf(effect_init_str, effectnum+1);
    }
  matrix.draw_2DPlasma(0, 0, Tmr_effects.GetCounter()*2 , 30, true, true);
  FastLED.show();  
}

void TimerEffect14Func()
{
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Palette1 = GetRandomPalette();
      FastLED.clear();
      Serial.printf(effect_init_str, effectnum+1);
    }
  matrix.draw_2DPlasmaPal(Palette1, 0, 0, Tmr_effects.GetCounter()*2 , 30, true, true, LINEARBLEND);
  FastLED.show();  
}

void TimerEffect15Func()
{
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Palette1 = GetRandomPalette();
      Palette2 = GetRandomPalette(); //target palette
      FastLED.clear();
      Serial.printf(effect_init_str, effectnum+1);
    }

if (Tmr_effects.GetCounter() % 50 == 0)
  {
  //check periodically, if palettes are the same - get new random
  if (ArePalettesEqual(Palette1, Palette2))
    { Palette2 = GetRandomPalette();} //change target palette
  }
  nblendPaletteTowardPalette( Palette1, Palette2, 24);  //fade Palette1 to Palette2.
  matrix.draw_2DPlasmaPal(Palette1, 0, 0, Tmr_effects.GetCounter()*2 , 30, true, true, LINEARBLEND);
  FastLED.show();  
}

void webSrvHandleNotFound()
{
  Serial.print("handleNotFound(): "); Serial.println( webserver.uri() );
  webserver.send(404, "text/plain", "404: Not found");
}

void webSrvHandleRoot()
{
  Serial.println("handleRoot()");
  processHTTPReq();  //process request parameters
  //webserver.send(200, "text/html", "OK" );
  if (SPIFFS.exists("/index.html")) 
    {// If the file exists
      File file = SPIFFS.open("/index.html", "r");   
      //size_t sent = webserver.streamFile(file, "text/html");

      String Str1 = file.readString();
      //  replacements
      Str1.replace("%bONen%",  IsON ? "disabled" : "");
      Str1.replace("%bOFFen%", IsON ? "" : "disabled");
      Str1.replace("%OptionList%", HTMLOptionList);

      Str1.replace("%EffChRand%", (EffChangeType==ECT_RANDOM)     ? "checked" : "" );
      Str1.replace("%EffChSeq%",  (EffChangeType==ECT_SEQUENTIAL) ? "checked" : "" );
      Str1.replace("%EffChNone%", (EffChangeType==ECT_NO_CHANGE)  ? "checked" : "" );
      //
      Str1.replace("%ChngeInterv%", String(EffChangeInterval));
      Str1.replace("%BriVal%", String(Brightness)); 
      //
      webserver.send(200, "text/html", Str1 ); 
      file.close();
    }
     else 
       {
        Serial.print( webserver.uri() );
        webserver.send(404, "text/plain", "404: File NOT FOUND!");
       }
}

String getWiFiModeString(WiFiMode_t wifimode)  //WIFI_OFF=0, WIFI_STA=1, WIFI_AP=2, WIFI_AP_STA=3
{
  String wifimodestr;
  switch (wifimode)  
    {
      case WIFI_OFF:
        wifimodestr = String("WIFI_OFF");
        break;
      case WIFI_STA:
        wifimodestr = String("WIFI_STA");
        break;
      case WIFI_AP:
        wifimodestr = String("WIFI_AP");
        break;
      case WIFI_AP_STA:
        wifimodestr = String("WIFI_AP_STA");
        break;
      default:
        wifimodestr = String("Unknown??");
    }
  return wifimodestr;  
}

String UptimeString()
{
 String uptime="";
 long days=0;
 long hours=0;
 long mins=0;
 long secs=0;
 secs = millis()/1000; //convect milliseconds to seconds
 mins=secs/60; //convert seconds to minutes
 hours=mins/60; //convert minutes to hours
 days=hours/24; //convert hours to days
 secs=secs-(mins*60); //subtract the coverted seconds to minutes in order to display 59 secs max 
 mins=mins-(hours*60); //subtract the coverted minutes to hours in order to display 59 minutes max
 hours=hours-(days*24); //subtract the coverted hours to days in order to display 23 hours max
 //Display results
 if (days>0) // days will displayed only if value is greater than zero
 {
   uptime += String(days) + "d ";
 }
 if (hours<10) uptime += "0" + String(hours) + "h:"; else uptime += String(hours) + "h:";
 if (mins<10)  uptime += "0" + String(mins)  + "m:"; else uptime += String(mins)  + "m:";
 if (secs<10)  uptime += "0" + String(secs)  + "s";  else uptime += String(secs)  + "s";
 return uptime;
}

void handleDiagInfo()
{
  Serial.println("handleDiagInfo()");
  if (SPIFFS.exists("/diag-info.html")) 
    {// If the file exists
      File file = SPIFFS.open("/diag-info.html", "r");   
      //size_t sent = server.streamFile(file, "text/html");

      String Str1;
      Str1 = file.readString();
      //  replacements
      Str1.replace("$WiFiMode$",  getWiFiModeString(WiFi.getMode()) );  //replace
      Str1.replace("$SSID$",      WiFi.SSID()                  );  //replace
      Str1.replace("$RSSI$",      String(WiFi.RSSI())          );  //replace
      Str1.replace("$MAC$",       WiFi.macAddress()            );  //replace
      Str1.replace("$BSSID$",     WiFi.BSSIDstr()              );  //replace
      Str1.replace("$IP$",        WiFi.localIP().toString()    );  //replace
      Str1.replace("$SubMask$",   WiFi.subnetMask().toString() );  //replace
      Str1.replace("$GateWayIP$", WiFi.gatewayIP().toString()  );  //replace
      Str1.replace("$DNSIP$",     WiFi.dnsIP().toString()      );  //replace
      Str1.replace("$HostName$",  WiFi.hostname()              );  //replace
      Str1.replace("$FreeHeapSize$", String(ESP.getFreeHeap()) );  //replace
      Str1.replace("$Uptime$",    UptimeString()               );  //replace
      Str1.replace("$CompiledOn$",String(__DATE__)+String(" ")+String(__TIME__) );  //replace
      Str1.replace("$ChipID$",    String(ESP.getChipId(), HEX) );  //replace
      Str1.replace("$FlashChipID$",       String(ESP.getFlashChipId(), HEX) );  //replace
      Str1.replace("$FlashChipSize$",     String(ESP.getFlashChipSize()) );  //replace
      Str1.replace("$FlashChipRealSize$", String(ESP.getFlashChipRealSize()) );  //replace
      Str1.replace("$FlashChipSpeed$",    String(ESP.getFlashChipSpeed()) );  //replace
      Str1.replace("$FlashChipMode$",    String(ESP.getFlashChipMode()) );  //replace
      //
      webserver.send(200, "text/html", Str1 ); 
      file.close();
    }
     else 
       {
        Serial.print( webserver.uri() );
        webserver.send(404, "text/plain", "404: File NOT FOUND!");
       }

};

void SelectEffect(uint8_t Num)
{
  if (effectnum == Num-1) return; //alreay selected
  effectnum = Num-1;
  if (IsON)
    {
    Tmr_effects.SetOnTimer(Effects[effectnum].EffectFunc);
    Tmr_effects.ResetCounter();
    }
  //Combine HTMLOptionList to use in HTTP resp, only when effect changed for optimization
  CombineHTMLOptionList();
  Tmr_effectChange.SetEnabled(false); Tmr_effectChange.SetEnabled(true); //to restart timer
  Serial.printf("Change effect to #%d, ", effectnum+1);
  Serial.print("Name: "); Serial.println(Effects[effectnum].EffectName);
}

void processHTTPReq()
{
  String S;
  uint16_t N;
  //Active buttons (ON, OFF)
  S = webserver.arg("Active");
  if (S.equalsIgnoreCase("ON")) turnON_OFF(true);
    else if (S.equalsIgnoreCase("OFF")) turnON_OFF(false);
   
  //NextEffect 
  if (webserver.hasArg("NextEffect")) Tmr_EffectChangeCB();
  
  //Select Effect
  N = webserver.arg("Effect").toInt();
  if (N>0) SelectEffect(N);
  
  //Effect Change (Random, Sequential, NoChange)
  S = webserver.arg("effectchange");
  if (S.equalsIgnoreCase("RANDOM")) ChangeEffectChangeType(ECT_RANDOM);
   else if (S.equalsIgnoreCase("SEQUENTIAL")) ChangeEffectChangeType(ECT_SEQUENTIAL);
    else if (S.equalsIgnoreCase("NONE")) ChangeEffectChangeType(ECT_NO_CHANGE);

  //Effect Change Interval  changeinterval=61
  N = webserver.arg("changeinterval").toInt();
  if (N>0) ChangeEffChangeInterval(N);  

  //Brightness
  N = webserver.arg("brightness").toInt();
  if ((N>0) and (N<=255)) SetBrightness(N);
}

void SetBrightness(uint8_t newBrightness)
{
  if (Brightness == newBrightness) return; //already set
  Serial.printf("Change Brightness from %d to %d\n", Brightness, newBrightness);
  Brightness = newBrightness;
  FastLED.setBrightness(Brightness);
}

void ChangeEffChangeInterval(uint16_t NewEffectChangeInterval)
{  
  if (EffChangeInterval == NewEffectChangeInterval) return; //already set
  Serial.printf("Change EffectChangeInterval from %dSec to %dSec\n", EffChangeInterval, NewEffectChangeInterval);
  EffChangeInterval = NewEffectChangeInterval;
  Tmr_effectChange.SetInterval(EffChangeInterval * 1000); 
}

void ChangeEffectChangeType(TEffectChangeType NewEffectChangeType)
{
  if (EffChangeType == NewEffectChangeType) return; //already set
  Serial.printf("Change ECT from %d to %d\n", EffChangeType, NewEffectChangeType);
  EffChangeType = NewEffectChangeType;
}

void turnON_OFF(boolean on)
{
  //if ( (on) && (IsON) ) return; //already ON
  //if ( (!on) && (!IsON) ) return; //already OFF
  if (on==IsON) return; //already in this mode
  IsON=on;
  Tmr_effectChange.SetEnabled(on);
  Tmr_effects.ResetCounter();
  if (on)
    {
    Tmr_OnBoardLED.SetEnabled(false);  
    digitalWrite(onboard_led, HIGH); //off
    Tmr_effects.SetOnTimer(Effects[effectnum].EffectFunc);
    Tmr_effects.SetEnabled(true);
    Serial.printf("Turn ON\r\n");
    }
    else 
    {
    Tmr_OnBoardLED.SetEnabled(true);
    Tmr_effects.SetOnTimer(TimerOFFFunc1);
    OFFtimestamp=millis();
    Serial.printf("Turn OFF\r\n");
    }
}

void OTA_onStart()
{
  String type;
  switch (ArduinoOTA.getCommand())
    {
    case U_FLASH: type = "SKETCH"; break;
    case U_FS:    type = "FILESYSTEM"; break; // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    default:      type = "UNKNOWN";
    } 
  Serial.println("Start updating " + type);
  FastLED.clear();
  FastLED.show();     
};

boolean g_OTAError=false;;

void OTA_onEnd()
{
  Serial.println("\r\nEnd");
  CRGB c;
  if (g_OTAError) c=CRGB::Red; else c=CRGB::Green; //Lime
  //FastLED.clear();
  for (uint16_t i=0; i<NUM_LEDS; i++) 
    {
      if ( (i % 2) == 0) { leds[i]=c; } //do not change the rest, stays after onProgress
    }
  FastLED.show();
  //not needed, leds stay until restart
  //delay(250);
  //FastLED.clear();
  //FastLED.show();
};

void OTA_onProgress(unsigned int progress, unsigned int total)
{
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  uint16_t nn = (progress / (total / NUM_LEDS));
  fill_solid(leds, nn, CRGB::Blue);
  FastLED.show();
};

void OTA_onError(ota_error_t error)
{
  g_OTAError = true; //save in global
  Serial.printf("Error[%u]: ", error);
  switch (error)
    {
    case OTA_AUTH_ERROR:    Serial.println("Auth Failed");    break;
    case OTA_BEGIN_ERROR:   Serial.println("Begin Failed");   break;
    case OTA_CONNECT_ERROR: Serial.println("Connect Failed"); break;
    case OTA_RECEIVE_ERROR: Serial.println("Receive Failed"); break;
    case OTA_END_ERROR:     Serial.println("End Failed");     break;
    default:                Serial.println("UNKNOWN Failure"); 
    } 
  FastLED.clear();
  FastLED.show();                     
};

void TimerOFFFunc1()
{
  if (Tmr_effects.GetCounter()==1) 
    {
      //effect initialization is here
      Serial.printf("Turning OFF tmr\r\n");
    }
  fadeToBlackBy( leds, NUM_LEDS, 32);
  FastLED.show();  
}

void StaticFramesDemo()
{  
  //Demo started here

FastLED.clear ();
uint8_t a;
uint8_t c= 81; //Q 81
matrix.draw_fillrect(RectangleWH(Point(0,0), 15,15), CRGB::DarkBlue); //background
//matrix.draw_2DGradient(CRGB::White, CRGB::Red, CRGB::Green, CRGB::Blue ); //background


//matrix.drawLetter(c, -2, -2, CRGB::Yellow);
//matrix.drawLetter(c, 12, 12, CRGB::Red);
String S = String("ABCabc123  ");
matrixText.setString(&S);
matrixText.drawString(0,4, CRGB::Red);
Serial.printf("minOffsetX_JustHide: %d\n", matrixText.minOffsetX_JustHide());
Serial.printf("maxOffsetX_JustHide: %d\n", matrixText.maxOffsetX_JustHide());
Serial.printf("minOffsetY_JustHide: %d\n", matrixText.minOffsetY_JustHide());
Serial.printf("maxOffsetY_JustHide: %d\n", matrixText.maxOffsetY_JustHide());
Serial.println();
Serial.printf("minOffsetX_MaxVisibility: %d\n", matrixText.minOffsetX_MaxVisibility());
Serial.printf("maxOffsetX_MaxVisibility: %d\n", matrixText.maxOffsetX_MaxVisibility());
Serial.printf("minOffsetY_MaxVisibility: %d\n", matrixText.minOffsetY_MaxVisibility());
Serial.printf("maxOffsetY_MaxVisibility: %d\n", matrixText.maxOffsetY_MaxVisibility());
FastLED.show();
delay(1000);
FastLED.clear();


int16_t i=0;
for (byte n=0; n<2; n++)
{
CRGB c=CHSV(random8(), 255, 255);
S = String(random(999999));
matrixText.setString(&S);
while (i >= matrixText.minOffsetX_JustHide())
  {
  FastLED.clear ();
  matrixText.drawString(&S, i, 4, c);
  delay(30);
  FastLED.show();
  i--;
  }
c=CHSV(random8(), 255, 255);  
S = String(random(999999));
while (i <= matrixText.maxOffsetX_JustHide())
  {
  FastLED.clear ();
  matrixText.drawString(&S, i, 4, c);
  delay(30);
  FastLED.show();
  i++;
  }
  Serial.println(ESP.getFreeHeap());
}


for (byte n=0; n<2; n++)
{
CRGB c=CHSV(random8(), 255, 255);
S = String(random(99));
while (i >= matrixText.minOffsetY_JustHide())
  {
  FastLED.clear ();
  matrixText.drawString(&S, 0, i, c);
  delay(50);
  FastLED.show();
  i--;
  }
 c=CHSV(random8(), 255, 255);  
 S = String(random(99));
while (i <= matrixText.maxOffsetY_JustHide())
  {
  FastLED.clear ();
  matrixText.drawString(&S, 0, i, c);
  delay(50);
  FastLED.show();
  i++;
  }
  Serial.println(ESP.getFreeHeap());
}


/*
for (int16_t i=30; i>-100; i--) 
{
  //matrix.draw_fillrect(RectangleWH(Point(0,0), 15,15), CRGB::DarkBlue); //background
  matrixText.drawCString("123ABCabc+-", i,4, CRGB::Red);
  //Serial.println(i);
  FastLED.show();
  delay(50);
}*/

FastLED.show();
delay(10000);


return;



  //gradient lines
FastLED.clear ();
matrix.draw_line_gradient(5, 1,    5, 12,  CRGB::Red, CRGB::Blue);//gradient, vertical lines
matrix.draw_line_gradient(7, 12,   7, 1,   CRGB::Red, CRGB::Blue);//gradient, vertical lines
FastLED.show();
WaitForSerial("Gradient vertical lines...");



//shifts
//prepare test screen
FastLED.clear ();
matrix.draw_2DRainbow(0, 8, 8);
RectangleWH ttt = {Point(6,3), 6, 8};  
matrix.draw_2DGradient(ttt, CRGB::Red, CRGB::Yellow, CRGB::Yellow, CRGB::Green);

//Rectangle www = RectangleWHToRectangle(ttt);

FastLED.show();
WaitForSerial("prep for shift tst...");
 //

//CRGB ArrInVertShift[7] ={CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,};
//CRGB ArrInHorShift[9]={CRGB::White, CRGB::White, CRGB::White, CRGB::White, CRGB::White, CRGB::White, CRGB::White, CRGB::White, CRGB::White};



RectangleWH q1 = {Point(0,0), 7, 7};  
RectangleWH q2 = {Point(8,0), 7, 7};  
RectangleWH q3 = {Point(0,8), 7, 7};  
RectangleWH q4 = {Point(8,8), 7, 7};  

//while(true)
for( byte i=0; i < 15; i++)
//for( byte j=0; j < 50; j++)
  {

//  Serial.printf("j: %d, i: %d\n", j,i);

  //matrix.Shift_RectDir( i, ArrInVertShift, CRGB::Red, CRGB::Blue ); //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
  //matrix.Shift_RectLeftRound();

if (i<8)
  {
  matrix.Shift_RectDir(q1, 5, CHSV(0,   255, 255), CHSV(32,  255, 255));
  matrix.Shift_RectDir(q2, 3, CHSV(62,  255, 255), CHSV(96,  255, 255));
  matrix.Shift_RectDir(q3, 7, CHSV(128, 255, 255), CHSV(224, 255, 255));
  matrix.Shift_RectDir(q4, 1, CHSV(192, 255, 255), CHSV(160, 255, 255));
  }
   else
    {
    matrix.Shift_RectDir(q1, 5, CRGB::Black, CRGB::Black);
    matrix.Shift_RectDir(q2, 3, CRGB::Black, CRGB::Black);
    matrix.Shift_RectDir(q3, 7, CRGB::Black, CRGB::Black);
    matrix.Shift_RectDir(q4, 1, CRGB::Black, CRGB::Black);
    }
  
  FastLED.show();
  //WaitForSerial("shift line left...");
  delay(250);
  }





FastLED.clear ();
matrix.draw_line_gradient(1, 5,    12, 5,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
matrix.draw_line_gradient(12,7,    1,  7,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
FastLED.show();
WaitForSerial("Gradient horizontal lines...");

FastLED.clear ();
matrix.draw_line_gradient(1, 1,    6, 12,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
matrix.draw_line_gradient(1, 12,   6,  1,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
FastLED.show();
WaitForSerial("Gradient ANY lines...");

  
  
  //Dots
  FastLED.clear ();
  matrix.setPixelColor(0,0, CRGB::Red);
  matrix.setPixelColor(1,0, CRGB::Red);
  matrix.setPixelColor(0,15, CRGB::Blue);
  matrix.setPixelColor(15,0, CRGB::Blue);
  matrix.setPixelColor(15,15, CRGB::Green);
  
  matrix.setPixelColor(Point(5,5), CRGB::White);
  matrix.setPixelColor(Point(5,10), CRGB::White);
  matrix.setPixelColor(Point(10,5), CRGB::White);
  matrix.setPixelColor(Point(10,10), CRGB::White);
  matrix.setPixelColor(100,100, CRGB::White); //ERROR, out of bounds, will be ignored
  
  FastLED.show();
  WaitForSerial("Dots...");

  //Lines
  FastLED.clear ();
  //matrix.draw_line (Point(0,7),   Point(0,15),  CRGB::Red);
  matrix.draw_line (Point(0,15),  Point(2,7),  CRGB::Blue);
  matrix.draw_line (Point(2,7),   Point(6,15),  CRGB::Green);
  matrix.draw_line (Point(6,15),   Point(6,7),  CRGB::Yellow);
  matrix.draw_line (Point(6,7),   Point(14,15),  CRGB::Magenta);
  matrix.draw_line (Point(14,15),   Point(9,15),  CRGB::Red, false);
  matrix.draw_line (Point(8,13),   Point(12,9),  CRGB::Pink);

  matrix.draw_line (Point(0,0),   Point(3,3),  CRGB::Gold);
  matrix.draw_line (Point(3,3),   Point(6,0),  CRGB::Gold);
  matrix.draw_line (Point(6,0),   Point(9,3),  CRGB::Gold);
  matrix.draw_line (Point(9,3),   Point(12,0),  CRGB::Gold);
  matrix.draw_line (Point(12,0),   Point(15,3),  CRGB::Gold);

  matrix.draw_line (Point(0,1),   Point(3,4),  CRGB::White);
  matrix.draw_line (Point(3,4),   Point(6,1),  CRGB::White);
  matrix.draw_line (Point(6,1),   Point(9,4),  CRGB::White);
  matrix.draw_line (Point(9,4),   Point(12,1),  CRGB::White);
  matrix.draw_line (Point(12,1),   Point(15,4),  CRGB::White);
  
  matrix.draw_line (Point(0,2),   Point(3,5),  CRGB::Red);
  matrix.draw_line (Point(3,5),   Point(6,2),  CRGB::Red);
  matrix.draw_line (Point(6,2),   Point(9,5),  CRGB::Red);
  matrix.draw_line (Point(9,5),   Point(12,2),  CRGB::Red);
  matrix.draw_line (Point(12,2),   Point(15,5),  CRGB::Red);

  matrix.draw_line (Point(0,3),   Point(3,6),  CRGB::DarkSalmon);
  matrix.draw_line (Point(3,6),   Point(6,3),  CRGB::DarkSalmon);
  matrix.draw_line (Point(6,3),   Point(9,6),  CRGB::DarkSalmon);
  matrix.draw_line (Point(9,6),   Point(12,3),  CRGB::DarkSalmon);
  matrix.draw_line (Point(12,3),   Point(15,6),  CRGB::DarkSalmon);
  FastLED.show();
  WaitForSerial("Lines...");

  //Rectangles using Lines
  FastLED.clear ();
  matrix.draw_line (0,0,   15,0,  CRGB::Red, false);
  matrix.draw_line (15,0,  15,15, CRGB::Green, false);
  matrix.draw_line (15,15, 0,15,  CRGB::Blue, false);
  matrix.draw_line (0,15,  0,0,   CRGB::Yellow, false);
  //
  matrix.draw_line (2,2,   13,2,  CRGB::White, false);
  matrix.draw_line (13,2,  13,13, CRGB::White, false);
  matrix.draw_line (13,13, 2,13,  CRGB::White, false);
  matrix.draw_line (2,13,  2,2,   CRGB::White, false);
  FastLED.show();
  WaitForSerial("Rectangles using Lines...");

  //Rectangles using draw_rect()
  FastLED.clear ();
  
  matrix.draw_rect(4,4, 0,0, CRGB::Red);
  matrix.draw_rect(Point(5,5), Point(10,10), CRGB::Brown);
  matrix.draw_rect(RectangleWH(Point(11,11), 4,4)  , CRGB::Salmon);
  matrix.draw_rect(RectangleWH(Point(0, 11), 4,4), CRGB::DarkSalmon);
  matrix.draw_rect(Point(11,4), Point(15,0), CRGB::LightPink);
  
  matrix.draw_rect(Rectangle(Point(1,6),  Point(3,9)),   CRGB::Red); 
  matrix.draw_rect(Rectangle(Point(12,9), Point(14,6)),  CRGB::Green);
  matrix.draw_rect(Rectangle(Point(9,14), Point(6, 12)), CRGB::Blue);
  matrix.draw_rect(Rectangle(Point(9,1),  Point(6, 3)),  CRGB::White);

  FastLED.show();
  WaitForSerial("Rectangles using draw_rect()...");

  //Filled Rectangles
  FastLED.clear ();
  matrix.draw_fillrect(RectangleWH(Point(15,15), 0,0), CRGB::Blue);
  matrix.draw_fillrect(14,14, 13,13, CRGB::Blue);
  matrix.draw_fillrect(Point(12,12), Point(10,10), CRGB::Blue);
  matrix.draw_fillrect(RectangleWH(Point(6,6), 3,3), CRGB::Blue);
  matrix.draw_fillrect(RectangleWH(Point(1,1), 4,4), CRGB::Blue);

  matrix.draw_fillrect(Rectangle(Point(0,15),  Point(2,13)),  CRGB::White);
  matrix.draw_fillrect(Rectangle(Point(3,12),  Point(5,10)),  CRGB::White);
  
  matrix.draw_fillrect(Rectangle(Point(15,0),  Point(14,1)),  CRGB::Red);
  matrix.draw_fillrect(Rectangle(Point(13,2),  Point(12,3)),  CRGB::Red);
  matrix.draw_fillrect(Rectangle(Point(11,4),  Point(10,5)),  CRGB::Red);
  
  FastLED.show();
  WaitForSerial("Filled Rectangles...");

  //Quarter-Circles
  FastLED.clear ();
  matrix.draw_quarterCircle(7, 7, 8, 0x02+0x04+0x08, CRGB::Magenta, true);
  matrix.draw_quarterCircle(7, 7, 7, 0x01+0x04+0x08, CRGB::Red, true);
  matrix.draw_quarterCircle(7, 7, 6, 0x01+0x02+0x04, CRGB::Blue, false);
  matrix.draw_quarterCircle(7, 7, 5, 0x01+0x04+0x08, CRGB::Green, false);
  matrix.draw_quarterCircle(7, 7, 4, 0x01+0x02+0x04, CRGB::Yellow, false);
  matrix.draw_quarterCircle(7, 7, 2, 0x01+0x04, CRGB::Crimson, true);
  FastLED.show();
  WaitForSerial("Quarter-Circles...");

  
  //Quarter-FilledCircles
  FastLED.clear ();
  matrix.draw_quarterFillCircle(7, 7, 8, 0x02+0x04+0x08, CRGB::Magenta, true);
  matrix.draw_quarterFillCircle(7, 7, 7, 0x01+0x04+0x08, CRGB::Red, true);
  matrix.draw_quarterFillCircle(7, 7, 6, 0x01+0x02+0x04, CRGB::Blue, false);
  matrix.draw_quarterFillCircle(7, 7, 5, 0x01+0x04+0x08, CRGB::Green, false);
  matrix.draw_quarterFillCircle(7, 7, 4, 0x01+0x02+0x04, CRGB::Yellow, false);
  matrix.draw_quarterFillCircle(7, 7, 2, 0x01+0x04, CRGB::Crimson, true); 
  FastLED.show();
  WaitForSerial("Quarter-FilledCircles...");

  //Circles
  FastLED.clear ();
  matrix.draw_circle(7, 7, 8, CRGB::Yellow);
  matrix.draw_circle(7, 7, 7, CRGB::Red);  
  matrix.draw_circle(7, 7, 6, CRGB::Green);
  matrix.draw_circle(7, 7, 5, CRGB::Blue);
  matrix.draw_circle(7, 7, 4, CRGB::FireBrick);
  matrix.draw_circle(7, 7, 3, CRGB::Pink);
  matrix.draw_circle(7, 7, 2, CRGB::Blue);
  matrix.draw_circle(7, 7, 1, CRGB::Magenta);
  matrix.draw_circle(7, 7, 0, CRGB::White);
  FastLED.show();
  WaitForSerial("Circles...");

  //Filled Circles
  FastLED.clear ();
  matrix.draw_fillCircle(7, 7, 8, CRGB::Yellow);
  matrix.draw_fillCircle(7, 7, 7, CRGB::Red);  
  matrix.draw_fillCircle(7, 7, 6, CRGB::Green);
  matrix.draw_fillCircle(7, 7, 5, CRGB::Blue);
  matrix.draw_fillCircle(7, 7, 4, CRGB::FireBrick);
  matrix.draw_fillCircle(7, 7, 3, CRGB::Pink);
  matrix.draw_fillCircle(7, 7, 2, CRGB::Blue);
  matrix.draw_fillCircle(7, 7, 1, CRGB::Magenta);
  matrix.draw_fillCircle(7, 7, 0, CRGB::White);
  FastLED.show();
  WaitForSerial("Filled Circles...");

  //2D Rainbow, full size
  matrix.draw_2DRainbow( 0, Point(7,7),  4, 32);
  FastLED.show();
  WaitForSerial("2D Rainbow, full matrix...");
  
  //2D Rainbow, four frames
  FastLED.clear ();
  RectangleWH R1 = {Point(0,0), 7, 7};
  matrix.draw_2DRainbow(R1, 0, Point(0,0), 4, 32);

  RectangleWH R2 = {Point(8,0), 7, 7};
  matrix.draw_2DRainbow(R2, 64, Point(8,0), 16, 16);
  
  RectangleWH R3 = {Point(0,8), 7, 7};
  matrix.draw_2DRainbow(R3, 128, Point(0,8), 4, 4);
  
  RectangleWH R4 = {Point(8,8), 7, 7};
  matrix.draw_2DRainbow(R4, 192, Point(8,8), 32, 4);
  FastLED.show();
  WaitForSerial("2D Rainbow, four frames...");
  FastLED.clear ();

  //2D gradient
  FastLED.clear ();
  RectangleWH RG1 = {Point(0,7), 4, 4};
  RectangleWH RG2 = {Point(7,0), 4, 4};
  RectangleWH RG3 = {Point(9,9), 4, 4};
  CRGB c1 =  CRGB::Red;
  CRGB c2 =  CRGB::Green;
  CRGB c3 =  CRGB::Blue;
  CRGB c4 =  CRGB::Yellow;
  matrix.draw_2DGradient(RG1, c1, c2, c3, c4); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
  matrix.draw_2DGradient(RG2, c1, c2, c3, c4); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
  matrix.draw_2DGradient(RG3, c1, c2, c3, c4); // LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor)
  FastLED.show();
  WaitForSerial("2D Gradient...");
  FastLED.clear ();

  //2D plasma
  matrix.draw_2DPlasma(0,0,0, 50);
  FastLED.show();
  WaitForSerial("2D Plasma...");

//2D plasma - Palette
  CRGB color1 = CHSV( HUE_BLUE, 255, 255);
  CRGB color2 = CHSV( HUE_RED, 255, 255);
  CRGB black  = CRGB::Black;
  CRGBPalette16 MyPalette = CRGBPalette16
                     ( black,  black, black,  black, 
                       black,  black, color1, color2,
                       color1, black, black,  black,
                       black,  black, black,  black );

 matrix.draw_2DPlasmaPal(MyPalette, 0, 0, 0, 50 );  //((i%2)==0)
 FastLED.show();
 WaitForSerial("2D Plasma Palette...");

  FastLED.clear ();
}

CRGBPalette16 GetRandomPalette()
{
uint8_t rndpal=random8(13); //MAX switch number!
CRGBPalette16 res;
switch (rndpal)
  {
  // Pre-defined palettes: CloudColors_p, LavaColors_p, OceanColors_p, ForestColors_p, RainbowColors_p, RainbowStripeColors_p, PartyColors_p, HeatColors_p
  case 0: {res = CloudColors_p; break;}
  case 1: {res = LavaColors_p; break;}
  case 2: {res = OceanColors_p; break;}
  case 3: {res = ForestColors_p; break;}
  case 4: {res = RainbowColors_p; break;}
  case 5: {res = RainbowStripeColors_p; break;}
  case 6: {res = PartyColors_p; break;}
  case 7: {res = HeatColors_p; break;}
  case 8:
    { 
    //random palette - four random colors 
    res = CRGBPalette16(CHSV(random8(), 255, 32), 
                        CHSV(random8(), 255, 255), 
                        CHSV(random8(), 128, 255), 
                        CHSV(random8(), 255, 255));  
    break; 
    }
        
  case 9: 
    {
    //Black And White Striped Palette
    fill_solid( res, 16, CRGB::Black);  // 'black out' all 16 palette entries... 
    res[0] = CRGB::White;         //and set every fourth one to white.
    res[4] = CRGB::White;
    res[8] = CRGB::White;
    res[12] = CRGB::White; 
    break;
    }
        
  case 10: 
    {
    //PurpleAndGreenPalette
    CRGB purple = CHSV(HUE_PURPLE, 255, 255);
    CRGB green  = CHSV(HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    res = CRGBPalette16(green,  green,  black,  black,
                        purple, purple, black,  black,
                        green,  green,  black,  black,
                        purple, purple, black,  black);
    break;
    }

  case 11:
    {
    //three neibor color palette
    fill_solid( res, 16, CRGB::Black);  // 'black out' all 16 palette entries... 
    uint8_t c = random8();
    res[7] = CHSV(c-16, 255, 128);
    res[8] = CHSV(c,    255, 255);
    res[9] = CHSV(c+16, 255, 128);
    break;
    }
              
  default: 
    {
    //one random color palette
    fill_solid( res, 16, CRGB::Black);  // 'black out' all 16 palette entries... 
    res[8] = CHSV(random8(), 255, 255);
    break;
    }     
  }//end of switch  
Serial.printf("Random Palette selector: %d\n", rndpal);  
return res;  
}

boolean ArePalettesEqual ( CRGBPalette16 &pal1, CRGBPalette16 &pal2)
{
if (pal1==pal2) return true; //same references
for(uint8_t i=0; i <= 15; i++)
  {
  if ( pal1[i] != pal2[i] ) {return false;}
  }
return true;
}
