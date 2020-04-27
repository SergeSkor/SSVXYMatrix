#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"
#include "SSVXYMatrix.h"
FASTLED_USING_NAMESPACE

#define DATA_PIN    D8 //15  //ESP8266 Lolin ESP-12E module
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS         96
#define FRAMES_PER_SECOND  50


// Params for width and height
const uint8_t MatrixWidth = 16;
const uint8_t MatrixHeight = 16;

#define NUM_LEDS (MatrixWidth * MatrixHeight)

CRGB leds[NUM_LEDS];

unsigned long func_exec_cnt=0;
char effect_init_str[] = "...effect #%d init.\n";

XYMatrix matrix(leds, MatrixWidth, MatrixHeight, true);

CRGBPalette16 Palette1, Palette2;

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//ahead declarations
void Effect1Func();
void Effect2Func();
void Effect3Func();
void Effect4Func();
void Effect5Func();
void Effect6Func();
void Effect7Func();
void Effect8Func();
void Effect9Func();
void Effect10Func();
void Effect11Func();
void Effect12Func();
void Effect13Func();
void Effect14Func();
void Effect15Func();

SimplePatternList gPatterns = { 
                                Effect1Func, 
                                Effect2Func, 
                                Effect3Func, 
                                Effect4Func, 
                                Effect5Func, 
                                Effect6Func, 
                                Effect7Func, 
                                Effect8Func, 
                                Effect9Func, 
                                Effect10Func, 
                                Effect11Func, 
                                Effect12Func, 
                                Effect13Func, 
                                Effect14Func,
                                Effect15Func
                                };

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  func_exec_cnt=0;
  Serial.printf("Change effect to #%d\n", gCurrentPatternNumber+1);
}

void setup() {
  delay(300); 
  Serial.begin(115200);
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  Serial.println();
  //demo
  StaticFramesDemo();
  Serial.printf("\n\nEffect #%d\n", gCurrentPatternNumber+1);
  FastLED.clear ();
  FastLED.show();  
}

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  //Increment every execution of effect-function. Resets when efefct changes.
  //Used for initialisation effects or to do some timing inside of the function.
  func_exec_cnt++; 
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000/FRAMES_PER_SECOND); //FastLED delay() is not working good on some effects, do knot know why.
  delay(1000/FRAMES_PER_SECOND); 
  // do some periodic updates
  EVERY_N_SECONDS( 30 ) { nextPattern(); } // change patterns periodically
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

void Effect1Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }

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
}

void Effect2Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / MatrixWidth));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / MatrixHeight));
    
  RectangleWH R1 = {Point(0,0), 15, 15};
  matrix.draw_2DRainbow(R1, ms/65536*1000, Point(7,7), yHueDelta32/32768, xHueDelta32/32768);
}

void Effect3Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  uint8_t posX = triwave8(beat8(37)) /16;
  uint8_t posY = triwave8(beat8(23)) /16;
  uint8_t hue = beat8(5);
  matrix.setPixelColor(posX,posY, CHSV( hue, 255, 255) );
}

void Effect4Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
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
}

void Effect5Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  long how_often=250; //mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS   
    {
     uint8_t s=random(3,10); //square size
     RectangleWH R = {Point(random(15-s), random(15-s)), s, s};
     if (random(255) > 128) 
       matrix.draw_rect(R, CHSV( random(255), 255, 255));
      else 
       matrix.draw_fillrect(R, CHSV( random(255), 255, 255));
    }
}

void Effect6Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  unsigned long how_often=250; //mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {
     uint8_t r=random(2,6); //radius
     uint8_t cx = random (r, MatrixWidth-r);
     uint8_t cy = random (r, MatrixHeight-r);
     if (random(255) > 128) 
       matrix.draw_circle(cx, cy, r, CHSV( random(255), 255, 255));
      else 
       matrix.draw_fillCircle(cx, cy, r, CHSV( random(255), 255, 255));
    }
}

void Effect7Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  unsigned long how_often=250; //mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {
     uint8_t x1 = random (MatrixWidth-1);
     uint8_t y1 = random (MatrixHeight-1);
     uint8_t x2 = random (MatrixWidth-1);
     uint8_t y2 = random (MatrixHeight-1);
     matrix.draw_line_gradient(x1, y1, x2, y2, CHSV( random(255), 255, 255), CHSV( random(255), 255, 255));
    }
}

void Effect8Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int16_t r = 2;
  uint8_t posCX = beatsin8(20, r, MatrixWidth-r, 0, 0 );
  uint8_t posCY = beatsin8(20, r, MatrixHeight-r, 0, 64 ); //90 deg phase shift
  uint8_t hue = beat8(5);
  matrix.draw_circle(posCX, posCY, r, CHSV(hue, 255, 255) );
}


void Effect9Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization may be here
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
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
}

void Effect10Func()
{
  static uint8_t dir;
  if (func_exec_cnt==0) 
    {
      //effect initialization is here
      FastLED.clear();
      matrix.draw_rect(2, 2, 13, 13,  CRGB::Green);
      matrix.draw_fillrect(5, 5, 10, 10,  CRGB::Yellow);
      matrix.draw_line(0, 0,  15, 15, CRGB::Blue,true);
      matrix.draw_line(0, 15, 15, 0,  CRGB::Red, true);
      dir=0;
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  long how_often=5000; //mS
  //if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {
     dir = random (8);
    }
    
  how_often=100; //mS
  //if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {
     matrix.Shift_RectRoundDir(dir);
    }
}

void Effect11Func()
{
  static uint8_t dir;
  if (func_exec_cnt==0) 
    {
      //effect initialization is here
      FastLED.clear();
      dir=0;
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  long how_often=500; //mS
  //if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {
     dir = random (8);
    }
    
  how_often=75; //mS
  //if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {

     CRGB c1 =  CHSV( triwave8(beat8(13)), 255, 255); //13
     CRGB c2 =  CHSV( triwave8(beat8(8)),  255, 255); //8
     //fadeToBlackBy( leds, NUM_LEDS, 40);
     matrix.Shift_RectDir(dir, c1, c2);
    }
}

void Effect12Func()
{
  static uint8_t dir;
  if (func_exec_cnt==0) 
    {
      //effect initialization is here
      FastLED.clear();
      dir=0;
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }

  long how_often=500; //mS
  //if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {
     dir = random (8);
    }
    
  how_often=75; //mS
  //if ((Tmr_effects.GetCounter() % ( how_often / Tmr_effects.GetInterval() ) ) == 0) //every how_often mS
  if ((func_exec_cnt % ( how_often * FRAMES_PER_SECOND / 1000 ) ) == 0) //every how_often mS
    {

//beatsin8(f, r, MatrixHeight-r, 0, 64+170 ); //90 deg phase shift

     CRGB c1[16]; for( byte i=0; i <= 15; i++) {c1[i]=CHSV(beatsin8(5, i), 255, 255);} //not done yet
     CRGB c2[16]; for( byte i=0; i <= 15; i++) {c2[i]=CHSV(beatsin8(7, i), 255, 255);} //not done yet
     //fadeToBlackBy( leds, NUM_LEDS, 40);
     matrix.Shift_RectDir(dir, c1, c2);
    }
}

void Effect13Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization is here
      FastLED.clear();
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  matrix.draw_2DPlasma(0, 0, func_exec_cnt*2 , 30, true, true);
}

void Effect14Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization is here
      Palette1 = GetRandomPalette();
      FastLED.clear();
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }
  matrix.draw_2DPlasmaPal(Palette1, 0, 0, func_exec_cnt*2 , 30, true, true, LINEARBLEND);
}

void Effect15Func()
{
  if (func_exec_cnt==0) 
    {
      //effect initialization is here
      Palette1 = GetRandomPalette();
      Palette2 = GetRandomPalette(); //target palette
      FastLED.clear();
      Serial.printf(effect_init_str, gCurrentPatternNumber+1);
    }

if (func_exec_cnt % 50 == 0)
  {
  //check periodically, if palettes are the same - get new random
  if (ArePalettesEqual(Palette1, Palette2))
    { Palette2 = GetRandomPalette();} //change target palette
  }
  nblendPaletteTowardPalette( Palette1, Palette2, 24);  //fade Palette1 to Palette2.
  matrix.draw_2DPlasmaPal(Palette1, 0, 0, func_exec_cnt*2 , 30, true, true, LINEARBLEND);
}

void StaticFramesDemo()
{  
//Demo started here

unsigned long d = 1000; //mS delay

  //gradient lines
FastLED.clear ();
matrix.draw_line_gradient(5, 1,    5, 12,  CRGB::Red, CRGB::Blue);//gradient, vertical lines
matrix.draw_line_gradient(7, 12,   7, 1,   CRGB::Red, CRGB::Blue);//gradient, vertical lines
FastLED.show();
Serial.println("Gradient vertical lines...");
delay(d);


FastLED.clear ();
matrix.draw_line_gradient(1, 5,    12, 5,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
matrix.draw_line_gradient(12,7,    1,  7,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
FastLED.show();
Serial.println("Gradient horizontal lines...");
delay(d);

FastLED.clear ();
matrix.draw_line_gradient(1, 1,    6, 12,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
matrix.draw_line_gradient(1, 12,   6,  1,  CRGB::Red, CRGB::Blue);//gradient, horizontal lines
FastLED.show();
Serial.println("Gradient ANY lines...");
delay(d);

  
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
Serial.println("Dots...");
delay(d);

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
Serial.println("Lines...");
delay(d);

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
Serial.println("Rectangles using Lines...");
delay(d);

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
Serial.println("Rectangles using draw_rect()...");
delay(d);

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
Serial.println("Filled Rectangles...");
delay(d);

  //Quarter-Circles
  FastLED.clear ();
  matrix.draw_quarterCircle(7, 7, 8, 0x02+0x04+0x08, CRGB::Magenta, true);
  matrix.draw_quarterCircle(7, 7, 7, 0x01+0x04+0x08, CRGB::Red, true);
  matrix.draw_quarterCircle(7, 7, 6, 0x01+0x02+0x04, CRGB::Blue, false);
  matrix.draw_quarterCircle(7, 7, 5, 0x01+0x04+0x08, CRGB::Green, false);
  matrix.draw_quarterCircle(7, 7, 4, 0x01+0x02+0x04, CRGB::Yellow, false);
  matrix.draw_quarterCircle(7, 7, 2, 0x01+0x04, CRGB::Crimson, true);
  FastLED.show();
  Serial.println("Quarter-Circles...");
  delay(d);

  
  //Quarter-FilledCircles
  FastLED.clear ();
  matrix.draw_quarterFillCircle(7, 7, 8, 0x02+0x04+0x08, CRGB::Magenta, true);
  matrix.draw_quarterFillCircle(7, 7, 7, 0x01+0x04+0x08, CRGB::Red, true);
  matrix.draw_quarterFillCircle(7, 7, 6, 0x01+0x02+0x04, CRGB::Blue, false);
  matrix.draw_quarterFillCircle(7, 7, 5, 0x01+0x04+0x08, CRGB::Green, false);
  matrix.draw_quarterFillCircle(7, 7, 4, 0x01+0x02+0x04, CRGB::Yellow, false);
  matrix.draw_quarterFillCircle(7, 7, 2, 0x01+0x04, CRGB::Crimson, true); 
  FastLED.show();
  Serial.println("Quarter-FilledCircles...");
  delay(d);

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
  Serial.println("Circles...");
  delay(d);

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
  Serial.println("Filled Circles...");
  delay(d);

  //2D Rainbow, full size
  matrix.draw_2DRainbow( 0, Point(7,7),  4, 32);
  FastLED.show();
  Serial.println("2D Rainbow, full matrix...");
  delay(d);
  
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
  Serial.println("2D Rainbow, four frames...");
  delay(d);

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
  Serial.println("2D Gradient...");
  delay(d);
  
  FastLED.clear ();
  //2D plasma
  matrix.draw_2DPlasma(0,0,0, 50);
  FastLED.show();
  Serial.println("2D Plasma...");
  delay(d);

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
 Serial.println("2D Plasma Palette...");
 delay(d);

 FastLED.clear ();
}
