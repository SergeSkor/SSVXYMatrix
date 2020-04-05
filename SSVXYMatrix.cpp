//
// By Serge Skorodinsky, 11/15/2019
//

#include "SSVXYMatrix.h"
#include <FastLED.h>
#include "SSVRGBGradientCalc.h"

//rectangle conversion functions
Rectangle NormalizeRectangle(Rectangle R)
{
  uint8_t p1x = min(R.P1.X, R.P2.X);
  uint8_t p1y = min(R.P1.Y, R.P2.Y);
  uint8_t p2x = max(R.P1.X, R.P2.X);
  uint8_t p2y = max(R.P1.Y, R.P2.Y);
 return Rectangle(Point(p1x,p1y), Point(p2x,p2y) );
}

RectangleWH RectangleToRectangleWH(Rectangle R)
{
  Rectangle NR = NormalizeRectangle(R);
  return RectangleWH(NR.P1, NR.P2.X-NR.P1.X, NR.P2.Y-NR.P1.Y);
}


Rectangle RectangleWHToRectangle(RectangleWH R)
{
  uint8_t p2x = R.LeftBottom.X + R.Width;
  uint8_t p2y = R.LeftBottom.Y + R.Height;
  return Rectangle(R.LeftBottom, Point(p2x, p2y) );
}


//XYMatrix Class
XYMatrix::XYMatrix(CRGB* aLeds, uint8_t MatrixWidth, uint8_t MatrixHeight, bool MatrixZigzagLayout)
{
  _leds=aLeds;
  _MatrixWidth = MatrixWidth;
  _MatrixHeight = MatrixHeight;
  _MatrixZigzagLayout = MatrixZigzagLayout;
}

//convert x,y to index. Does not care about out of bounds
uint16_t XYMatrix::XY( uint8_t x, uint8_t y) 
{
  uint16_t index;
  if( _MatrixZigzagLayout) 
    {
    //zigzag
    if( y & 0x01) 
      {
      // Odd rows run backwards
      uint8_t reverseX = (_MatrixWidth - 1) - x;
      index = (y * _MatrixWidth) + reverseX;
      } 
       else 
        {
        // Even rows run forwards
        index = (y * _MatrixWidth) + x;
        }
    }
  else 
    { 
    //not zigzag  
    index = (y * _MatrixWidth) + x; 
    }
  return index;
}

uint16_t XYMatrix::XY( Point P)
{
  return XY(P.X, P.Y);
}

void XYMatrix::setPixelColor(uint8_t x, uint8_t y, CRGB color)
{
  if( x >= _MatrixWidth) return; //error
  if( y >= _MatrixHeight) return; //error
  //if( x < 0) return; //x is unsigned
  //if( y < 0) return; //y is unsigned
  _leds[ XY(x, y)] = color; 
}

void XYMatrix::setPixelColor(Point P, CRGB color)
{
  setPixelColor(P.X, P.Y, color);
}

CRGB XYMatrix::getPixelColor(uint8_t x, uint8_t y)
{
  if( x >= _MatrixWidth) return 0; //error
  if( y >= _MatrixHeight) return 0; //error
  //if( x < 0) return; //x is unsigned
  //if( y < 0) return; //y is unsigned
  return _leds[XY(x, y)];
}

CRGB XYMatrix::getPixelColor(Point P)
{
  getPixelColor(P.X, P.Y);
}

uint8_t XYMatrix::getMatrixWidth()
{return _MatrixWidth;}

uint8_t XYMatrix::getMatrixHeight()
{return _MatrixHeight;}

bool XYMatrix::isZigzagLayout()
{return _MatrixZigzagLayout;}


void XYMatrix::draw_line(int x1, int y1, int x2, int y2, CRGB color, boolean includeLastPoint)
{
    //https://ru.wikibooks.org/wiki/%D0%A0%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B8_%D0%B0%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC%D0%BE%D0%B2/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%91%D1%80%D0%B5%D0%B7%D0%B5%D0%BD%D1%85%D1%8D%D0%BC%D0%B0
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    if ((deltaX == 0) && (deltaY == 0))  //added to draw point if start and end points are the same
      {
        setPixelColor(x1,y1,color); //bounds check
        return;
      }
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;
    //
    if (includeLastPoint) 
      {
        //_leds[ XY(x2, y2) ] = color;  //no bounds check
        setPixelColor(x2,y2,color); //bounds check
      }

    if (deltaX ==0)
      {
        //vert.line, faster
        while (y1 != y2)
          {
            setPixelColor(x1,y1,color);
            y1 += signY;
          }
        return;  
      }

    if (deltaY ==0)
      {
        //hor.line, faster
        while (x1 != x2)
          {
            setPixelColor(x1,y1,color);
            x1 += signX;
          }
        return;  
      }
    
    //ANY line, but vert and hor are done above, faster
    int error = deltaX - deltaY;
    while(x1 != x2 || y1 != y2) 
   {
        //_leds[ XY(x1, y1) ]  = color;  //no bounds check
        setPixelColor(x1,y1,color); //bounds check
        
        const int error2 = error * 2;
        //
        if(error2 > -deltaY) 
        {
            error -= deltaY;
            x1 += signX;
        }
        if(error2 < deltaX) 
        {
            error += deltaX;
            y1 += signY;
        }
    }
}

void XYMatrix::draw_line(Point P1, Point P2, CRGB color, boolean includeLastPoint)
{
  draw_line(P1.X, P1.Y, P2.X, P2.Y, color, includeLastPoint);
}

////gradient line, different name to not confuse with default parameter "includeLastPoint"
void XYMatrix::draw_line_gradient(int x1, int y1, int x2, int y2, CRGB color1, CRGB color2, boolean includeLastPoint) //gradient
{
    //https://ru.wikibooks.org/wiki/%D0%A0%D0%B5%D0%B0%D0%BB%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D0%B8_%D0%B0%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC%D0%BE%D0%B2/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%91%D1%80%D0%B5%D0%B7%D0%B5%D0%BD%D1%85%D1%8D%D0%BC%D0%B0
    const int deltaX = abs(x2 - x1);
    const int deltaY = abs(y2 - y1);
    if ((deltaX == 0) && (deltaY == 0))  //added to draw point if start and end points are the same
      {
        setPixelColor(x1,y1,color1); //bounds check
        return;
      }
    const int signX = x1 < x2 ? 1 : -1;
    const int signY = y1 < y2 ? 1 : -1;
    //
    if (includeLastPoint) 
      {
        //_leds[ XY(x2, y2) ] = color;  //no bounds check
        setPixelColor(x2,y2,color2); //bounds check
      }

    if (deltaX ==0)
      {
        //vert.line, faster
        RGBGradientCalculator GC (color1,  color2,  y1, y2);
        while (y1 != y2)
          {
            setPixelColor(x1,y1, GC.GetRGBGradientColor(y1) );
            y1 += signY;
          }
        return;  
      }

    if (deltaY ==0)
      {
        //hor.line, faster
        RGBGradientCalculator GC (color1,  color2,  x1, x2);
        while (x1 != x2)
          {
            setPixelColor(x1,y1, GC.GetRGBGradientColor(x1) );
            x1 += signX;
          }
        return;  
      }
    
    //ANY line, but vert and hor are done above, faster
    //gradient by bigger delta, X or Y
    RGBGradientCalculator GC; //create gradient calculator
    if (deltaX > deltaY) GC.SetScale (color1,  color2,  x1, x2); //set scale according to bigger delta
                    else GC.SetScale (color1,  color2,  y1, y2);
    int error = deltaX - deltaY;
    while(x1 != x2 || y1 != y2) 
   {
        //gradient by bigger delta
        if (deltaX > deltaY) setPixelColor(x1,y1, GC.GetRGBGradientColor(x1) ); 
                        else setPixelColor(x1,y1, GC.GetRGBGradientColor(y1) ); 
        
        const int error2 = error * 2;
        //
        if(error2 > -deltaY) 
        {
            error -= deltaY;
            x1 += signX;
        }
        if(error2 < deltaX) 
        {
            error += deltaX;
            y1 += signY;
        }
    }
}

void XYMatrix::draw_line_gradient(Point P1, Point P2, CRGB color1, CRGB color2, boolean includeLastPoint) //gradient
{
  draw_line_gradient(P1.X, P1.Y, P2.X, P2.Y, color1, color2, includeLastPoint);
}


void XYMatrix::draw_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, CRGB color)
{
  draw_line(x1,y1, x1,y2, color, false);
  draw_line(x1,y2, x2,y2, color, false);
  draw_line(x2,y2, x2,y1, color, false);
  draw_line(x2,y1, x1,y1, color, false);
}

void XYMatrix::draw_rect(Point P1, Point P2, CRGB color)
{
  draw_rect(P1.X,P1.Y, P2.X, P2.Y, color);
}

void XYMatrix::draw_rect(RectangleWH Rect, CRGB color)
{
  draw_rect(Rect.LeftBottom.X, Rect.LeftBottom.Y, Rect.LeftBottom.X+Rect.Width, Rect.LeftBottom.Y+Rect.Height, color);
}

void XYMatrix::draw_rect(Rectangle Rect, CRGB color)
{
  draw_rect(Rect.P1.X, Rect.P1.Y, Rect.P2.X, Rect.P2.Y, color);
}

void XYMatrix::draw_fillrect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, CRGB color)
{
  for( uint8_t yi = min(y0,y1); yi <= max(y0,y1); yi++)
  {
  draw_line(x0,yi, x1,yi, color);
  }
}

void XYMatrix::draw_fillrect(Point P1, Point P2, CRGB color)
{
  draw_fillrect(P1.X,P1.Y, P2.X, P2.Y, color);
}

void XYMatrix::draw_fillrect(RectangleWH Rect, CRGB color)
{
  draw_fillrect(Rect.LeftBottom.X, Rect.LeftBottom.Y, Rect.LeftBottom.X+Rect.Width, Rect.LeftBottom.Y+Rect.Height, color);
}

void XYMatrix::draw_fillrect(Rectangle Rect, CRGB color)
{
  draw_fillrect(Rect.P1.X, Rect.P1.Y, Rect.P2.X, Rect.P2.Y, color);
}

/**************************************************************************/
/*!
    Copied from Adafruit_GFX library
    
    @brief    Quarter-circle drawer, used to do circles and roundrects
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    cornername  Maskbit to indicate which quarters of the circle: 
                          0x1 (left-bottom), 0x2 (right-bottom) 0x4 (right-top) 0x8 (left-top)
    @param    color color
*/
void XYMatrix::draw_quarterCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, CRGB color, boolean includeEdges) 
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;
   if (includeEdges)
     {
      if ( (cornername & 0x01) || (cornername & 0x08) ) {setPixelColor(x0-r, y0  , color);} //left point
      if ( (cornername & 0x04) || (cornername & 0x08) ) {setPixelColor(x0  , y0+r, color);} //top point
      if ( (cornername & 0x02) || (cornername & 0x04) ) {setPixelColor(x0+r, y0  , color);} //right point
      if ( (cornername & 0x01) || (cornername & 0x02) ) {setPixelColor(x0  , y0-r, color);} //bottom point
     }
    while (x<y) 
    {
        if (f >= 0) 
        {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x01) 
        {
          //left-bottom
          setPixelColor(x0 - y, y0 - x, color);
          setPixelColor(x0 - x, y0 - y, color);
        }
        if (cornername & 0x02) 
        {
          //right-bottom
          setPixelColor(x0 + x, y0 - y, color);
          setPixelColor(x0 + y, y0 - x, color);
        }

        if (cornername & 0x04) 
        {
          //right-top
          setPixelColor(x0 + x, y0 + y, color);
          setPixelColor(x0 + y, y0 + x, color);
        }

        if (cornername & 0x08) 
        {
          //left-top
          setPixelColor(x0 - y, y0 + x, color);
          setPixelColor(x0 - x, y0 + y, color);
        }
    }
}

/**************************************************************************/
/*!
    Copied from Adafruit_GFX library
    
    @brief    Quarter-circle drawer with fill, used to do circles and roundrects
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    cornername  Maskbit to indicate which quarters of the circle: 
                          0x1 (left-bottom), 0x2 (right-bottom) 0x4 (right-top) 0x8 (left-top)
    @param    color color
*/
void XYMatrix::draw_quarterFillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, CRGB color, boolean includeEdges) 
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;
   if (includeEdges)
     {
      if ( (cornername & 0x01) || (cornername & 0x08) ) {draw_line(x0-r, y0,   x0, y0, color);} //left hor line
      if ( (cornername & 0x04) || (cornername & 0x08) ) {draw_line(x0  , y0+r, x0, y0, color);} //top vert line
      if ( (cornername & 0x02) || (cornername & 0x04) ) {draw_line(x0+r, y0  , x0, y0, color);} //right hor line
      if ( (cornername & 0x01) || (cornername & 0x02) ) {draw_line(x0  , y0-r, x0, y0, color);} //bottom vert line
     }
    while (x<y) 
    {
        if (f >= 0) 
        {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x01) 
        {
          //left-bottom
          draw_line(x0 - y, y0 - x, x0 - 1, y0 - x, color); //hor.line
          draw_line(x0 - x, y0 - y, x0 - 1, y0 - y, color); //hor.line
        }
        if (cornername & 0x02) 
        {
          //right-bottom
         draw_line(x0 + x, y0 - y, x0 + 1, y0 - y, color); //hor.line
         draw_line(x0 + y, y0 - x, x0 + 1, y0 - x, color); //hor.line
        }
        if (cornername & 0x04) 
        {
          //right-top
          draw_line(x0 + x, y0 + y, x0 + 1, y0 + y, color); //hor.line
          draw_line(x0 + y, y0 + x, x0 + 1, y0 + x, color); //hor.line
        }
        if (cornername & 0x08) 
        {
          //left-top
          draw_line(x0 - y, y0 + x, x0 - 1, y0 + x, color); //hor.line
          draw_line(x0 - x, y0 + y, x0 - 1, y0 + y, color); //hor.line
        }
    }
}

void XYMatrix::draw_circle(int16_t x0, int16_t y0, int16_t r, CRGB color)
{
  draw_quarterCircle(x0, y0, r, 0x0f, color, true); 
}

void XYMatrix::draw_fillCircle(int16_t x0, int16_t y0, int16_t r, CRGB color)
{
  draw_quarterFillCircle(x0, y0, r, 0x0f, color, true); 
}


void XYMatrix::draw_2DRainbow(RectangleWH Rect, byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue)
{
  OriginHue = OriginHue - ((origin.X-Rect.LeftBottom.X)*XDeltaHue) - ((origin.Y-Rect.LeftBottom.Y)*YDeltaHue);
  byte lineStartHue = OriginHue;
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++) //number of dots is (Height+1)
    {
    byte pixelHue = lineStartHue;      
    for( byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++) //number of dots is (Width+1)
      {
      //_leds[ XY(x, y) ] = CHSV( pixelHue, 255, 255); //no bounds check
      setPixelColor(x, y, CHSV( pixelHue, 255, 255) ); //bounds check
      pixelHue += XDeltaHue;
      }
    lineStartHue += YDeltaHue;
    }
}

void XYMatrix::draw_2DRainbow(Rectangle Rect, byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue)
{
  RectangleWH RWH = RectangleToRectangleWH(Rect);
  draw_2DRainbow( RWH, OriginHue, origin, XDeltaHue, YDeltaHue);
}

void XYMatrix::draw_2DRainbow(byte OriginHue, int8_t XDeltaHue, int8_t YDeltaHue)
{
  struct RectangleWH RWH = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  draw_2DRainbow(RWH, OriginHue, Point(0,0), XDeltaHue, YDeltaHue);
}


void XYMatrix::draw_2DRainbow(byte OriginHue, int originX, int originY, int8_t XDeltaHue, int8_t YDeltaHue)
{
  OriginHue = OriginHue - (originX*XDeltaHue) - (originY*YDeltaHue);
  draw_2DRainbow(OriginHue, XDeltaHue, YDeltaHue);
}

void XYMatrix::draw_2DRainbow(byte OriginHue, Point origin, int8_t XDeltaHue, int8_t YDeltaHue)
{
 draw_2DRainbow(OriginHue, origin.X, origin.Y, XDeltaHue, YDeltaHue);
}


void XYMatrix::draw_2DGradient(RectangleWH Rect, CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor)
{
  RGBGradientCalculator LeftGradient  (LeftBottomColor,  LeftTopColor,  Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height);
  RGBGradientCalculator RightGradient (RightBottomColor, RightTopColor, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height);
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++) //number of dots is (Height+1)
    {
    CRGB cl = LeftGradient.GetRGBGradientColor(y);
    CRGB cr = RightGradient.GetRGBGradientColor(y);
    fill_gradient_RGB(_leds,  XY(Rect.LeftBottom.X,y), cl, XY(Rect.LeftBottom.X+Rect.Width,y), cr ); 
    }
}

void XYMatrix::draw_2DGradient(Rectangle Rect, CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor)
{
  RectangleWH RWH = RectangleToRectangleWH(Rect);
  draw_2DGradient ( RWH, LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor );
}

void XYMatrix::draw_2DGradient(CRGB LeftBottomColor, CRGB LeftTopColor, CRGB RightBottomColor, CRGB RightTopColor)
{
  struct RectangleWH R = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  draw_2DGradient(R, LeftBottomColor, LeftTopColor, RightBottomColor, RightTopColor);
}

//shifts
CRGB XYMatrix::ShiftHorLineLeft(int16_t X0, int16_t X1, int16_t Y, CRGB colorIN) //X0<=X1
{
  CRGB tmp_c, ret_c;
  ret_c = getPixelColor(X0, Y);
  if (X0<=X1)
    {
    for (uint16_t x = X0+1; x <= X1 ; x++) //from X max to X min
      {
      tmp_c = getPixelColor(x, Y);
      setPixelColor(x-1, Y, tmp_c);
      }
    setPixelColor(X1, Y, colorIN);
    }
  return ret_c;
}

void XYMatrix::Shift_RectLeft(RectangleWH Rect, CRGB ColorArrIn[] )
{
  byte i=0;
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++)
  {
    ShiftHorLineLeft(Rect.LeftBottom.X, Rect.LeftBottom.X+Rect.Width, y, ColorArrIn[i++]);
  }
}

void XYMatrix::Shift_RectLeft(Rectangle Rect, CRGB ColorArrIn[] )
{
  RectangleWH RWH = RectangleToRectangleWH(Rect);
  Shift_RectLeft( RWH, ColorArrIn );
}

void XYMatrix::Shift_RectLeft( CRGB ColorArrIn[] ) //whole matrix
{
  struct RectangleWH RWH = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  Shift_RectLeft( RWH, ColorArrIn );
}

void XYMatrix::Shift_RectLeftRound(RectangleWH Rect)
{
  CRGB c;
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++)
  {
    c = getPixelColor(Rect.LeftBottom.X, y);
    ShiftHorLineLeft(Rect.LeftBottom.X, Rect.LeftBottom.X+Rect.Width, y, c);
  }
}

void XYMatrix::Shift_RectLeftRound(Rectangle Rect)
{
  RectangleWH RWH = RectangleToRectangleWH(Rect);
  Shift_RectLeftRound( RWH );
}

void XYMatrix::Shift_RectLeftRound( ) //whole matrix
{
  struct RectangleWH RWH = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  Shift_RectLeftRound( RWH );
}

//shifts left, single color
void XYMatrix::Shift_RectLeft(RectangleWH Rect, CRGB ColorIn)
{
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++)
  {
    ShiftHorLineLeft(Rect.LeftBottom.X, Rect.LeftBottom.X+Rect.Width, y, ColorIn);
  }

}

//shift right
CRGB XYMatrix::ShiftHorLineRight(int16_t X0, int16_t X1, int16_t Y, CRGB colorIN) //X0<=X1
{
  CRGB tmp_c, ret_c;
  ret_c = getPixelColor(X1, Y);
  if (X0<=X1)
    {
    for (int16_t x = X1-1; x >= X0 ; x--) //from X max to X min
      {
      tmp_c = getPixelColor(x, Y);
      setPixelColor(x+1, Y, tmp_c);
      }
    setPixelColor(X0, Y, colorIN);
    }
  return ret_c;
}

void XYMatrix::Shift_RectRight(RectangleWH Rect, CRGB ColorArrIn[] )
{
  byte i=0;
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++)
  {
    ShiftHorLineRight(Rect.LeftBottom.X, Rect.LeftBottom.X+Rect.Width, y, ColorArrIn[i++]);
  }
}

//shifts right round
void XYMatrix::Shift_RectRightRound(RectangleWH Rect)
{
  CRGB c;
  for (byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++)
    {
    c = getPixelColor(Rect.LeftBottom.X+Rect.Width, y);
    ShiftHorLineRight(Rect.LeftBottom.X, Rect.LeftBottom.X+Rect.Width, y, c);
    }
}

//shifts right, single color
void XYMatrix::Shift_RectRight(RectangleWH Rect, CRGB ColorIn)
{
  for( byte y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++)
  {
    ShiftHorLineRight(Rect.LeftBottom.X, Rect.LeftBottom.X+Rect.Width, y, ColorIn);
  }

}

//shifts up
CRGB XYMatrix::ShiftVertLineUp(int16_t X, int16_t Y0, int16_t Y1, CRGB colorIN) //helper function, Y0<=Y1, returns colorOUT (not used for now)
{
  CRGB tmp_c, ret_c;
  ret_c = getPixelColor(X, Y1);
  if (Y0<=Y1)
    {
    for (int16_t y = Y1-1; y >= Y0 ; y--) //from Y max to Y min
      {
      tmp_c = getPixelColor(X, y);
      setPixelColor(X, y+1, tmp_c);
      }
    setPixelColor(X, Y0, colorIN);
    }
  return ret_c;
}

void XYMatrix::Shift_RectUp(RectangleWH Rect, CRGB ColorArrIn[] )
{
  byte i=0;
  for( byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++)
  {
    ShiftVertLineUp(x, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height, ColorArrIn[i++]);
  }
}

//shifts up round
void XYMatrix::Shift_RectUpRound(RectangleWH Rect)
{
  CRGB c;
  for (byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++)
    {
    c = getPixelColor(x, Rect.LeftBottom.Y+Rect.Height);
    ShiftVertLineUp(x, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height, c);
    }
}

//shifts up, single color
void XYMatrix::Shift_RectUp(RectangleWH Rect, CRGB ColorIn)
{  
  for( byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++)
  {
    ShiftVertLineUp(x, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height, ColorIn);
  }
}

//shifts down
CRGB XYMatrix::ShiftVertLineDown(int16_t X, int16_t Y0, int16_t Y1, CRGB colorIN) //helper function, Y0<=Y1, returns colorOUT (not used for now)
{
  CRGB tmp_c, ret_c;
  ret_c = getPixelColor(X, Y0);
  if (Y0<=Y1)
    {
    for (uint16_t y = Y0+1; y <= Y1 ; y++) //from Y max to Y min
      {
      tmp_c = getPixelColor(X, y);
      setPixelColor(X, y-1, tmp_c);
      }
    setPixelColor(X, Y1, colorIN);
    }
  return ret_c;
}

void XYMatrix::Shift_RectDown(RectangleWH Rect, CRGB ColorArrIn[] )
{
  byte i=0;
  for( byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++)
  {
    ShiftVertLineDown(x, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height, ColorArrIn[i++]);
  }
}

//shifts down round
void XYMatrix::Shift_RectDownRound(RectangleWH Rect)
{
  CRGB c;
  for (byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++)
    {
    c = getPixelColor(x, Rect.LeftBottom.Y);
    ShiftVertLineDown(x, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height, c);
    }
}

//shifts down, single color
void XYMatrix::Shift_RectDown(RectangleWH Rect, CRGB ColorIn)
{
  for( byte x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++)
  {
    ShiftVertLineDown(x, Rect.LeftBottom.Y, Rect.LeftBottom.Y+Rect.Height, ColorIn);
  }

}

//shifts round with direction
void XYMatrix::Shift_RectRoundDir(RectangleWH Rect, uint8_t Direction) //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
{
  CRGB c;
  switch (Direction)
  {
    case 0:  //Up
      Shift_RectUpRound(Rect);
      break;

    case 1:  //UpRight
      Shift_RectUpRound(Rect);
      Shift_RectRightRound(Rect);
      break;  
    
    case 2:  //Right
      Shift_RectRightRound(Rect);
      break;
    
    case 3:  //DownRight
      Shift_RectDownRound(Rect);
      Shift_RectRightRound(Rect);
      break;  

    case 4:  //Down
      Shift_RectDownRound(Rect);
      break;

    case 5:  //DownLeft
      Shift_RectDownRound(Rect);
      Shift_RectLeftRound(Rect);
      break;  
    
    case 6:  //Left
      Shift_RectLeftRound(Rect);
      break;
    
    case 7:  //UpLeft
      Shift_RectUpRound(Rect);
      Shift_RectLeftRound(Rect);
      break;  
  }//end of switch
}

void XYMatrix::Shift_RectRoundDir(uint8_t Direction)  //whole matrix  //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
{
  struct RectangleWH RWH = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  Shift_RectRoundDir( RWH, Direction );
}

//shifts (no round) with direction 
void XYMatrix::Shift_RectDir(RectangleWH Rect, uint8_t Direction, CRGB ColorArrInVertShift[], CRGB ColorArrInHorShift[] ) //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
{
  switch (Direction)
  {
    case 0:  //Up
      Shift_RectUp(Rect, ColorArrInVertShift);
      break;

    case 1:  //UpRight
      Shift_RectUp(Rect, ColorArrInVertShift);
      Shift_RectRight(Rect, ColorArrInHorShift);
      break;  
    
    case 2:  //Right
      Shift_RectRight(Rect, ColorArrInHorShift);
      break;
    
    case 3:  //DownRight
      Shift_RectDown(Rect, ColorArrInVertShift);
      Shift_RectRight(Rect, ColorArrInHorShift);
      break;  

    case 4:  //Down
      Shift_RectDown(Rect, ColorArrInVertShift);
      break;

    case 5:  //DownLeft
      Shift_RectDown(Rect, ColorArrInVertShift);
      Shift_RectLeft(Rect, ColorArrInHorShift);
      break;  
    
    case 6:  //Left
      Shift_RectLeft(Rect, ColorArrInHorShift);
      break;
    
    case 7:  //UpLeft
      Shift_RectUp(Rect, ColorArrInVertShift);
      Shift_RectLeft(Rect, ColorArrInHorShift);
      break;  
  }//end of switch
}

void XYMatrix::Shift_RectDir(uint8_t Direction, CRGB ColorArrInVertShift[], CRGB ColorArrInHorShift[] ) //whole matrix //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
{
  struct RectangleWH RWH = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  Shift_RectDir(RWH, Direction, ColorArrInVertShift, ColorArrInHorShift );
}

void XYMatrix::Shift_RectDir(RectangleWH Rect, uint8_t Direction, CRGB ColorInVertShift, CRGB ColorInHorShift ) //single colors //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
{
  switch (Direction)
  {
    case 0:  //Up
      Shift_RectUp(Rect, ColorInVertShift);
      break;

    case 1:  //UpRight
      Shift_RectUp(Rect, ColorInVertShift);
      Shift_RectRight(Rect, ColorInHorShift);
      break;  
    
    case 2:  //Right
      Shift_RectRight(Rect, ColorInHorShift);
      break;
    
    case 3:  //DownRight
      Shift_RectDown(Rect, ColorInVertShift);
      Shift_RectRight(Rect, ColorInHorShift);
      break;  

    case 4:  //Down
      Shift_RectDown(Rect, ColorInVertShift);
      break;

    case 5:  //DownLeft
      Shift_RectDown(Rect, ColorInVertShift);
      Shift_RectLeft(Rect, ColorInHorShift);
      break;  
    
    case 6:  //Left
      Shift_RectLeft(Rect, ColorInHorShift);
      break;
    
    case 7:  //UpLeft
      Shift_RectUp(Rect, ColorInVertShift);
      Shift_RectLeft(Rect, ColorInHorShift);
      break;  
  }//end of switch
}

void XYMatrix::Shift_RectDir(uint8_t Direction, CRGB ColorInVertShift, CRGB ColorInHorShift ) //single colors, whole matrix //0:Up, 1:UpRight, 2:Right, 3:DownRight, 4:Down, 5:DownLeft, 6:Left, 7:UpLeft
{
  struct RectangleWH RWH = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  Shift_RectDir(RWH, Direction, ColorInVertShift, ColorInHorShift );
}

//Plasma-like effects, based on FastLED inoise() function - “fixed point implementation of perlin’s Simplex Noise”
void XYMatrix::draw_2DPlasma(RectangleWH Rect, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl, boolean ExpandDR)
{
  for( int x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++) //number of dots is (Width+1)
  {
    int nx =  (NoiseScale*x) + NoiseSpaceX;
    for( int y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++) //number of dots is (Height+1)
    {
    int ny = (NoiseScale*y) + NoiseSpaceY;
    uint8_t NoiseValue = inoise8(nx, ny, NoiseSpaceZ);

    if (ExpandDR)
      {
      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      NoiseValue = qsub8(NoiseValue, 16);
      NoiseValue = qadd8(NoiseValue, scale8(NoiseValue, 39));
      }

	uint8_t bri;
	if (UseBriControl)
	  {
      //optional second (mirrored) parameter use as a brightness
      bri = inoise8(ny, nx, NoiseSpaceZ);
	  if (ExpandDR)
        {
        // brighten up, as the color palette itself often contains the 
        // light/dark dynamic range desired
        if( bri > 127 ) {bri = 255;} else {bri = dim8_raw( bri * 2);}
		}
	  }
	  else 
	    {bri = 255;}
	//set pixel color
	    //for paletet usage
        //CRGB color = ColorFromPalette( PaletteToUse, NoiseValue, bri, LINEARBLEND); //LINEARBLEND NOBLEND
        //setPixelColor(x, y, color);
	//for non-palette usage
    setPixelColor(x, y, CHSV(NoiseValue, 255, bri));
    }
  }
}

void XYMatrix::draw_2DPlasma(Rectangle Rect, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl, boolean ExpandDR)
{
  RectangleWH RWH = RectangleToRectangleWH(Rect);
  draw_2DPlasma (RWH, NoiseSpaceX, NoiseSpaceY, NoiseSpaceZ, NoiseScale, UseBriControl, ExpandDR);
}

void XYMatrix::draw_2DPlasma(uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl, boolean ExpandDR)
{
  struct RectangleWH R = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  draw_2DPlasma (R, NoiseSpaceX, NoiseSpaceY, NoiseSpaceZ, NoiseScale, UseBriControl, ExpandDR);
}

//Plasma-like Palette effects, based on FastLED inoise() function - “fixed point implementation of perlin’s Simplex Noise” 
void XYMatrix::draw_2DPlasmaPal(RectangleWH Rect, CRGBPalette16 PalToUse, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl, boolean ExpandDR, TBlendType PalBlendType)
{
  for( int x = Rect.LeftBottom.X; x < Rect.LeftBottom.X+Rect.Width+1; x++) //number of dots is (Width+1)
  {
    int nx =  (NoiseScale*x) + NoiseSpaceX;
    for( int y = Rect.LeftBottom.Y; y < Rect.LeftBottom.Y+Rect.Height+1; y++) //number of dots is (Height+1)
    {
    int ny = (NoiseScale*y) + NoiseSpaceY;
    uint8_t NoiseValue = inoise8(nx, ny, NoiseSpaceZ);

    if (ExpandDR)
      {
      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      NoiseValue = qsub8(NoiseValue, 16);
      NoiseValue = qadd8(NoiseValue, scale8(NoiseValue, 39));
      }

	uint8_t bri;
	if (UseBriControl)
	  {
      //optional second (mirrored) parameter use as a brightness
      bri = inoise8(ny, nx, NoiseSpaceZ);
	  if (ExpandDR)
        {
        // brighten up, as the color palette itself often contains the 
        // light/dark dynamic range desired
        if( bri > 127 ) {bri = 255;} else {bri = dim8_raw( bri * 2);}
		}
	  }
	  else 
	    {bri = 255;}
	//set pixel color
	//for paletet usage	
    CRGB color = ColorFromPalette( PalToUse, NoiseValue, bri, PalBlendType); //LINEARBLEND, NOBLEND
    setPixelColor(x, y, color);
	    //for non-paletet usage	
	    //setPixelColor(x, y, CHSV(NoiseValue, 255, bri));
    }
  }	
}

void XYMatrix::draw_2DPlasmaPal(Rectangle Rect, CRGBPalette16 PalToUse, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl, boolean ExpandDR, TBlendType PalBlendType)
{
  RectangleWH RWH = RectangleToRectangleWH(Rect);
  draw_2DPlasmaPal(Rect, PalToUse, NoiseSpaceX, NoiseSpaceY, NoiseSpaceZ, NoiseScale, UseBriControl, ExpandDR, PalBlendType);
}

void XYMatrix::draw_2DPlasmaPal(CRGBPalette16 PalToUse, uint16_t NoiseSpaceX, uint16_t NoiseSpaceY, uint16_t NoiseSpaceZ, uint16_t NoiseScale, boolean UseBriControl, boolean ExpandDR, TBlendType PalBlendType)
{
  struct RectangleWH R = {Point(0,0), _MatrixWidth-1, _MatrixHeight-1};
  draw_2DPlasmaPal(R, PalToUse, NoiseSpaceX, NoiseSpaceY, NoiseSpaceZ, NoiseScale, UseBriControl, ExpandDR, PalBlendType);
}


 //End of XYMatrix Class
