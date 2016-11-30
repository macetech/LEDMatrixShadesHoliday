// FireworksXY
// Quick and dirty 2-D fireworks simulation using FastLED.
// Adapted from code by Mark Kriegsman, July 2013



#define MODEL_BORDER 3
#define MODEL_WIDTH  (MODEL_BORDER + kMatrixWidth + MODEL_BORDER)
#define MODEL_HEIGHT (MODEL_BORDER + kMatrixWidth + MODEL_BORDER)

#define PIXEL_X_OFFSET ((MODEL_WIDTH  - kMatrixWidth ) / 2)
#define PIXEL_Y_OFFSET ((MODEL_HEIGHT - kMatrixHeight) / 2)

#define WINDOW_X_MIN (PIXEL_X_OFFSET)
#define WINDOW_X_MAX (WINDOW_X_MIN + kMatrixWidth - 1)
#define WINDOW_Y_MIN (PIXEL_Y_OFFSET)
#define WINDOW_Y_MAX (WINDOW_Y_MIN + kMatrixHeight - 1)

byte overrun;

void screenscale( accum88 a, byte N, byte& screen, byte& screenerr)
{
  byte ia = a >> 8;
  screen = scale8( ia, N);
  byte m = screen * (256 / N);
  screenerr = (ia - m) * scale8(255,N);
  return;
}

static int16_t scale15by8_local( int16_t i, fract8 scale )
{
    int16_t result;
    result = (int32_t)((int32_t)i * scale) / 256;
    return result;
}

saccum78 gGravity = -15;
fract8  gBounce = 200;
fract8  gDrag = 250;
bool gSkyburst = 0;

accum88 gBurstx;
accum88 gBursty;
saccum78 gBurstxv;
saccum78 gBurstyv;
byte gBurstcolor;

#define NONE 0
#define SHELL 1
#define SPARK 2

class Dot {
public:
  byte    show;
  byte    theType;
  accum88 x;
  accum88 y;
  saccum78 xv;
  saccum78 yv;
  accum88 r;
  byte    color;

  Dot()
  {
    show = 0;
    theType = 0;
    x =  0;
    y =  0;
    xv = 0;
    yv = 0;
    r  = 0;
    color = 0;
  }

  void Draw()
  {
    if( !show) return;
    byte ix, xe, xc;
    byte iy, ye, yc;
    screenscale( x, MODEL_WIDTH-1, ix, xe);
    screenscale( y, MODEL_HEIGHT-1, iy, ye);
    ix -= PIXEL_X_OFFSET;
    iy = kMatrixHeight - (iy - PIXEL_Y_OFFSET) - 1;
    
    yc = 255 - ye;
    xc = 255 - xe;
    
    
    
    byte c00 = dim8_video( scale8( scale8( color, yc), xc));
    byte c01 = dim8_video( scale8( scale8( color, ye), xc)); 
    byte c10 = dim8_video( scale8( scale8( color, yc), xe));
    byte c11 = dim8_video( scale8( scale8( color, ye), xe));

    XYPWM(ix,iy,c00);
    XYPWM(ix,iy+1,c01);
    XYPWM(ix+1,iy,c10);
    XYPWM(ix+1,iy+1,c11);
  }
  
  void Move()
  {
    saccum78 oyv = yv;
    
    if( !show) return;
    yv += gGravity;
    //xv = scale15by8_local( xv, gDrag);    
    //yv = scale15by8_local( yv, gDrag);

    if( theType == SPARK) {
      xv = scale15by8_local( xv, gDrag);    
      yv = scale15by8_local( yv, gDrag);
      if (color > 0) color--;
      if( !color) {
        show = 0;
      }
    }

    // if we'd hit the ground, bounce
    if( yv < 0 && (y < (-yv)) ) {
      if( theType == SPARK ) {
        show = 0;
      } else {
        yv = -yv;
        yv = scale15by8_local( yv, gBounce);
        if( yv < 500 ) {
          show = 0;
        }
      }
    }
    
    if( (yv < -300) /* && (!(oyv < 0))*/ ) {
      // pinnacle
      if( theType == SHELL ) {

        if( (y > (uint16_t)(0x8000)) /*&& (random8() < 64)*/) {

        }

        show = 0;

        gSkyburst = 1;
        gBurstx = x;
        gBursty = y;
        gBurstxv = xv;
        gBurstyv = yv;
        gBurstcolor = color;        
      }
    }
    if( theType == SPARK) {
      if( ((xv >  0) && (x > xv)) ||
          ((xv < 0 ) && (x < (0xFFFF + xv))) )  {
        x += xv;
      } else {
        show = 0;
      }
    } else {
      x += xv;
    }
    y += yv;
    
  }
  
  void GroundLaunch()
  {
    yv = 600 + random16(300 + (25 * kMatrixHeight));
    xv = (int16_t)random16(600) - (int16_t)300;
    y = 0;
    x = 0x8000;
    color = 255;
    show = 1;
  }
  
  void Skyburst( accum88 basex, accum88 basey, saccum78 basedv, byte basecolor)
  {
    yv = (int16_t)0 + (int16_t)random16(2000) - (int16_t)1000;
    xv = basedv + (int16_t)random16(2000) - (int16_t)1000;
    y = basey;
    x = basex;
    color = basecolor;
    theType = SPARK;
    show = 1;
  }

};

#define NUM_SPARKS 20
Dot gSparks[NUM_SPARKS];

