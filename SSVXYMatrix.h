#ifndef FastLED_2D_h
#define FastLED_2D_h

//
// By Serge Skorodinsky, 11/15/2019
//

#include <FastLED.h>

struct Point
{
  uint8_t X;
  uint8_t Y;
  Point() = default;  //{X=0; Y=0;} //default struct constructor
  Point(uint8_t _x, uint8_t _y) {X=_x; Y=_y;} //struct constructor
};


struct Rectangle  //Rectangle specified by two diagonal points (any direction)  //TODO: add to group: draw_2DRainbow()
{
  Point P1;
  Point P2;
  Rectangle() = default; //default constructor
  Rectangle(Point _P1, Point _P2) {P1=_P1; P2=_P2;}
  Rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {P1=Point(x1,y1), P2=Point(x2,y2);}
};


struct RectangleWH  //Rectangle specified by LeftBottom point and Width, Height
{
  Point LeftBottom;
  uint8_t Width;  //positive, if 0 - one dot, so number of dots is (Width+1)
  uint8_t Height; //positive, if 0 - one dot, so number of dots is (Height+1)
  RectangleWH() = default; //{LeftBottom=Point(0,0); Width=0; Height=0;} //default struct constructor
  RectangleWH(Point _leftbottom, uint8_t _width, uint8_t _height) {LeftBottom=_leftbottom; Width=_width; Height=_height;} //struct constructor
};

//rectangle conversion functions
Rectangle NormalizeRectangle(Rectangle R);
RectangleWH RectangleToRectangleWH(Rectangle R);
Rectangle RectangleWHToRectangle(RectangleWH R);

//XYMatrix Class
class XYMatrix
{
  public:
    //constructors
    XYMatrix(CRGB* aLeds, uint8_t MatrixWidth, uint8_t MatrixHeight, bool MatrixZigzagLayout);
    //
    void setPixelColor(uint8_t x, uint8_t y, CRGB color);
    void setPixelColor(Point P, CRGB color);
    //
    CRGB getPixelColor(uint8_t x, uint8_t y);
    CRGB getPixelColor(Point P);
    //
    uint8_t getMatrixWidth();
    uint8_t getMatrixHeight();
    bool isZigzagLayout();
    //
    void draw_line(int x1, int y1, int x2, int y2, CRGB color, boolean includeLastPoint=true);
    void draw_line(Point P1, Point P2, CRGB color, boolean includeLastPoint=true);
    //gradient line, different name to not confuse with default parameter "includeLastPoint"
    void draw_line_gradient(int x1, int y1, int x2, int y2, CRGB color1, CRGB color2, boolean includeLastPoint=true); //gradient
    void draw_line_gradient(Point P1, Point P2, CRGB color1, CRGB color2, boolean includeLastPoint=true); //gradient
    //
    void draw_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, CRGB color);
    void draw_rect(Point P1, Point P2, CRGB color);
    void draw_rect(RectangleWH Rect, CRGB color);
    void draw_rect(Rectangle Rect, CRGB color);
    //
    void draw_fillrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, CRGB color);
    void draw_fillrect(Point P1, Point P2, CRGB color);
    void draw_fillrect(RectangleWH Rect, CRGB color);
    void draw_fillrect(Rectangle Rect, CRGB color);
    //
    void draw_2DRainbow(RectangleWH Rect, byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue);
    void draw_2DRainbow(Rectangle Rect, byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue);
    void draw_2DRainbow(byte OriginHue, int8_t XDeltaHue, int8_t YDeltaHue); //whole matrix
    void draw_2DRainbow(byte OriginHue, int originX, int originY, int8_t XDeltaHue, int8_t YDeltaHue); //whole matrix
    void draw_2DRainbow(byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue); //whole matrix
    //other types of params?
    //void draw_2DRainbow(Point P1, Point P2, byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue);
    
    //
    void draw_quarterCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, CRGB color, boolean includeEdges=true);  //from adafruit_GFX library
    void draw_quarterFillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, CRGB color, boolean includeEdges=true);  //from adafruit_GFX library
    void draw_circle(int16_t x0, int16_t y0, int16_t r, CRGB color);
    void draw_fillCircle(int16_t x0, int16_t y0, int16_t r, CRGB color);
    //
    void draw_2DGradient(RectangleWH Rect, CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor);
    void draw_2DGradient(Rectangle Rect, CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor);
    void draw_2DGradient(CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor); //whole matrix
    //other types of params?
    //void draw_2DGradient(Point P1, Point P2, CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor);

    //shifts left
    CRGB ShiftHorLineLeft(int16_t X0, int16_t X1, int16_t Y, CRGB colorIN); //helper function, X0<=X1, returns colorOUT (not used for now)
    void Shift_RectLeft(RectangleWH Rect, CRGB ColorArrIn[] );  //main?
    //shifts left round
    void Shift_RectLeftRound(RectangleWH Rect);
    //other left shifts
    void Shift_RectLeft(Rectangle Rect, CRGB ColorArrIn[]);    //single color?
    void Shift_RectLeft( CRGB ColorArrIn[] ); //whole matrix    //single color?
    //void Shift_RectLeft(Point P1, Point P2, CRGB ColorArrIn[] );  //other types of params?
    //other left shifts round
    void Shift_RectLeftRound(Rectangle Rect);
    void Shift_RectLeftRound( ); //whole matrix
    //void Shift_RectLeftRound(Point P1, Point P2 );  //other types of params?
    //shifts left, single color
    void Shift_RectLeft(RectangleWH Rect, CRGB ColorIn);

    //shifts right
    CRGB ShiftHorLineRight(int16_t X0, int16_t X1, int16_t Y, CRGB colorIN); //helper function, X0<=X1, returns colorOUT (not used for now)
    void Shift_RectRight(RectangleWH Rect, CRGB ColorArrIn[] );  //single color?   main?    
    //shifts right round
    void Shift_RectRightRound(RectangleWH Rect);
    //shifts right, single color
    void Shift_RectRight(RectangleWH Rect, CRGB ColorIn);

    //shifts up
    CRGB ShiftVertLineUp(int16_t X, int16_t Y0, int16_t Y1, CRGB colorIN); //helper function, Y0<=Y1, returns colorOUT (not used for now)
    void Shift_RectUp(RectangleWH Rect, CRGB ColorArrIn[] );  //single color?   main? 
    //shifts up round
    void Shift_RectUpRound(RectangleWH Rect);
    //shifts up, single color
    void Shift_RectUp(RectangleWH Rect, CRGB ColorIn);

    //shifts down
    CRGB ShiftVertLineDown(int16_t X, int16_t Y0, int16_t Y1, CRGB colorIN); //helper function, Y0<=Y1, returns colorOUT (not used for now)
    void Shift_RectDown(RectangleWH Rect, CRGB ColorArrIn[] );  //single color?   main? 
    //shifts down round
    void Shift_RectDownRound(RectangleWH Rect);
    //shifts down, single color
    void Shift_RectDown(RectangleWH Rect, CRGB ColorIn);

    //shifts round with direction
    void Shift_RectRoundDir(RectangleWH Rect, uint8_t Direction); //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
    void Shift_RectRoundDir(uint8_t Direction); //whole matrix    //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft    

    //shifts (no round) with direction 
    void Shift_RectDir(RectangleWH Rect, uint8_t Direction, CRGB ColorArrInVertShift[], CRGB ColorArrInHorShift[] ); //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
    void Shift_RectDir(uint8_t Direction, CRGB ColorArrInVertShift[], CRGB ColorArrInHorShift[] ); //whole matrix //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft    
    void Shift_RectDir(RectangleWH Rect, uint8_t Direction, CRGB ColorInVertShift, CRGB ColorInHorShift ); //single colors //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
    void Shift_RectDir(uint8_t Direction, CRGB ColorInVertShift, CRGB ColorInHorShift ); //single colors, whole matrix //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
    
	//Plasma-like effects, based on FastLED inoise() function - “fixed point implementation of perlin’s Simplex Noise”
	void draw_2DPlasma(RectangleWH Rect, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl=true, boolean ExpandDR=true);
    void draw_2DPlasma(Rectangle Rect,   uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl=true, boolean ExpandDR=true);
    void draw_2DPlasma(                  uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl=true, boolean ExpandDR=true);
	
	//Plasma-like Palette effects, based on FastLED inoise() function - “fixed point implementation of perlin’s Simplex Noise”
	void draw_2DPlasmaPal(RectangleWH Rect, CRGBPalette16 PalToUse, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl=true, boolean ExpandDR=true, TBlendType PalBlendType=LINEARBLEND);
	void draw_2DPlasmaPal(Rectangle Rect,   CRGBPalette16 PalToUse, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl=true, boolean ExpandDR=true, TBlendType PalBlendType=LINEARBLEND);
	void draw_2DPlasmaPal(                  CRGBPalette16 PalToUse, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl=true, boolean ExpandDR=true, TBlendType PalBlendType=LINEARBLEND);
	
  protected:
    CRGB* _leds;
    uint8_t _MatrixWidth;
    uint8_t _MatrixHeight;
    bool _MatrixZigzagLayout;
    uint16_t XY(uint8_t x, uint8_t y);     //convert x,y to index. Does not care about out of bounds
    uint16_t XY( Point P);
    //void circle(int x0, int y0, int radius, CRGB color, boolean fill); //helper function

    
}; //End of XYMatrix Class

#endif
//END OF FILE
