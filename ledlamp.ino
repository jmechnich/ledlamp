#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include "defs.h"
#include "encoder.h"

// WS2812 parameters
#define PIN        6
#define NUMPIXELS 24

// encoder RGB led pins
#define PIN_RED   11
#define PIN_GREEN  3
#define PIN_BLUE   5

// encoder led blink delay
#define DT_BLINK 150

// rgb components
enum {
    RED=0,
    GREEN=1,
    BLUE=2,
};

// encoder led color while off
const uint8_t  csleep[3]   = {1,0,0};

//#define DEBUG

/*-------------------------------------------------------------------------
 *  Variables
 *-------------------------------------------------------------------------*/
Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

Encoder *encoder = Encoder::instance();

uint8_t rgbcolor[3] = {0};
uint8_t rgbsave[3]  = {0};
uint8_t brightness  = 255;
    
const uint8_t fadetime = 10;
uint8_t fadensteps = 50;

uint8_t fadecounter   = 0;
uint8_t fadestep      = 0;
bool    fading        = false;
double  fadestart[3]  = {0};
uint8_t fadetarget[3] = {0};

const uint8_t* cpresets[] = {csodhigh,ccandle,csodvapor,0};
uint8_t curpreset = 0;

bool isOn = false;

/*-------------------------------------------------------------------------
 *  Strip (WS2812 leds)
 *-------------------------------------------------------------------------*/
void setStrip( uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(r,g,b));
  }
  pixels.show();
}

void setStrip( const uint8_t rgb[3], uint8_t bright) {
  setStrip( uint8_t(rgb[RED]/255.*bright+0.5),
            uint8_t(rgb[GREEN]/255.*bright+0.5),
            uint8_t(rgb[BLUE]/255.*bright+0.5));
}

void setStrip() {
  setStrip(rgbcolor,brightness);
}

/*-------------------------------------------------------------------------
 *  RGB LED of encoder
 *-------------------------------------------------------------------------*/
void setLED(const uint8_t rgb[3], uint8_t bright) 
{
  encoder->setLED(
      uint8_t(rgb[RED]  /255.*bright+0.5),
      uint8_t(rgb[GREEN]/255.*bright+0.5),
      uint8_t(rgb[BLUE] /255.*bright+0.5));
}

void setLED() 
{
  setLED(rgbcolor,brightness);
}

void blinkLED() 
{
  setLED(cblack,255);
  delay(DT_BLINK);
  setLED();
}

/*-------------------------------------------------------------------------
 *  Fading
 *-------------------------------------------------------------------------*/
void startFade(const uint8_t rgb[3], const uint8_t* rgbcur=0)
{
  fading = true;
  fadestep = 0;
  fadecounter = 0;
  memcpy(fadetarget,rgb,3);
  if(!rgbcur) rgbcur = rgbcolor;

  double max = 0;
  for(uint8_t i=0;i<3;++i) {
      fadestart[i] = rgbcur[i];
      if(fabs(fadestart[i]-rgb[i]) > max)
          max = fabs(fadestart[i]-rgb[i]);
  }
  //if(max > 50) max = 50;
  fadensteps = uint8_t(brightness/255.*max/5+1.5);
}

void fadeStep() 
{
  if(!fading) return;
  if(++fadestep==fadensteps) {
    fading = false;
    memcpy(rgbcolor,fadetarget,3);
    return;
  }

#ifdef DEBUG
  Serial.print("fadestep ");
  Serial.print(fadestep);
  Serial.print(" color");
#endif
  
  for(uint8_t i=0;i<3;++i) {
    double color = fadestart[i] + \
        double(fadestep)/fadensteps*(fadetarget[i]-fadestart[i]);
    if(color < 0)
        color = 0;
    else if(color > 255)
        color = 255;
    rgbcolor[i] = uint8_t(color+0.5);
#ifdef DEBUG
    Serial.print(" ");
    Serial.print(rgbcolor[i]);
  }
  Serial.println();
#else
  }
#endif
}
  
/*-------------------------------------------------------------------------
 *  Update functions
 *-------------------------------------------------------------------------*/
void refresh()
{
  if(!isOn && (!fading)) {
    setLED(csleep,255);
  } else {
    setLED();
  }
  setStrip();
}

void nextPreset() 
{
  if(cpresets[++curpreset] == 0)
      curpreset = 0;
  startFade(cpresets[curpreset]);
#ifdef DEBUG
  Serial.print("Preset ");
  Serial.println(curpreset);
#endif
}

/*-------------------------------------------------------------------------
 *  Encoder handlers
 *-------------------------------------------------------------------------*/
void toggleOn() {
  isOn = !isOn;
  if(isOn) {
    startFade(rgbsave);
  } else {
    memcpy(rgbsave,rgbcolor,3);
    startFade(cblack);
  }
}

void handleLongPress() {
  if(isOn) {
    nextPreset();
  }
}

void handleClick() {
  toggleOn();
}

void handleDoubleClick() {
  if(isOn) {
  }
}

void handleTurnLeft() 
{
  if(!isOn) return;
  if (brightness==0)
      return;
  brightness -= 5;
  refresh();
}

void handleTurnRight() 
{
  if(!isOn) return;
  if (brightness==255) {
    blinkLED();
    return;
  }
  brightness += 5;
  refresh();
}

void handleTimer() 
{
  if (fading && (++fadecounter%fadetime == 0))
  {
    fadecounter = 0;
    fadeStep();
    refresh();
  }
}

/*-------------------------------------------------------------------------
 *  setup() and loop() functions
 *-------------------------------------------------------------------------*/
void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  
  encoder->setPins(PIN_RED,PIN_GREEN,PIN_BLUE);
  
  encoder->setCallback(Encoder::LEFT,   handleTurnLeft);
  encoder->setCallback(Encoder::RIGHT,  handleTurnRight);
  encoder->setCallback(Encoder::LPRESS, handleLongPress);
  encoder->setCallback(Encoder::CLICK,  handleClick);
  encoder->setCallback(Encoder::DCLICK, 0);//handleDoubleClick);
  encoder->setCallback(Encoder::TIMER,  handleTimer);

  pixels.begin();
  memcpy(rgbsave,cpresets[curpreset],3);
  refresh();
}

void loop() {
  encoder->update();
}
