#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <ClickEncoder.h>
#include <TimerOne.h>
#include <RGBConverter.h>

#define PIN        6
#define NUMPIXELS 24

#define PIN_RED   11
#define PIN_GREEN  3
#define PIN_BLUE   5

#define RGBMAX   255
#define RGBSTEP    3

#define HSXMAX   1.0
#define HSXSTEP  0.01

#define DT_BLINK 150
#define DT_CYCLE 200

const uint8_t  cblack[3]   = {0};
const uint8_t  cwhite[3]   = {RGBMAX,RGBMAX,RGBMAX};
const uint8_t  cred[3]     = {RGBMAX,0,0};
const uint8_t  cgreen[3]   = {0,RGBMAX,0};
const uint8_t  cblue[3]    = {0,0,RGBMAX};
const uint8_t  cyellow[3]  = {RGBMAX,RGBMAX,0};
const uint8_t  cmagenta[3] = {RGBMAX,0,RGBMAX};
const uint8_t  ccyan[3]    = {0,RGBMAX,RGBMAX};
const uint8_t* crgb[]      = {cred,cgreen,cblue,0};
const uint8_t* chsv[]      = {cyellow,cblack,cyellow,cblack,0};
const uint8_t* chsl[]      = {ccyan,cblack,ccyan,cblack,0};
const uint8_t* cmax[]      = {cwhite,cblack,cwhite,cblack,0};

Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ClickEncoder *encoder;
int16_t last, value;
bool longPress = false;
bool isOn = false;

double  hsxcolor[3] = {0.2,0.4,0.2};
uint8_t rgbcolor[3] = {0};

enum GENMODES {
    HSV,
    HSL,
    RGB,
    MAX,
    GENMODES_SZ,
};

enum MODES {
    RED=0,
    YELLOW=0,
    HUE=0,
    GREEN=1,
    MAGENTA=1,
    SAT=1,
    BLUE=2,
    CYAN=2,
    VAL=2,
    LIGHT=2,
    ALL=3,
    MODES_SZ,
};

uint8_t general_mode = HSV;
uint8_t mode = general_mode == RGB ? ALL : VAL;

uint8_t colAverage() {
  int16_t avg = 0;
  for (uint8_t i=0; i<3; ++i)
      avg += rgbcolor[i];
  avg /= 3;
  if( avg > RGBMAX)
      return RGBMAX;
  else if (avg < 0)
      return 0;
  return avg;
}

void timerIsr() {
  encoder->service();
}

void setStrip( uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(r,g,b));
    pixels.show();
  }
}

void setStrip( const uint8_t color[3]) {
  setStrip( color[RED], color[GREEN], color[BLUE]);
}

void updateStrip() {
  setStrip( rgbcolor);
}

void setLED( uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PIN_RED,  255-r);
  analogWrite(PIN_GREEN,255-g);
  analogWrite(PIN_BLUE, 255-b);
}

void setLED( const uint8_t color[3]) {
  setLED( color[RED], color[GREEN], color[BLUE]);
}

void blinkLED(bool invert=false) 
{
  if(invert)
      setLED(cwhite);
  else
      setLED(cblack);
  delay(DT_BLINK);
  setLED(rgbcolor);
}

void colorBlinkLED(const uint8_t color[3], uint8_t count=1) 
{
  for( uint8_t i=0; i<count; ++i)
  {
    setLED(color);
    delay(DT_BLINK);
    setLED(cblack);
    delay(DT_BLINK);
  }
  setLED(rgbcolor);
}

void cycleLED( const uint8_t* colors[3])
{
  size_t i=0;
  while(colors[i])
  {
    setLED(colors[i]);
    delay(DT_CYCLE);
    ++i;
  }
  setLED(rgbcolor);
}

void updateLEDRGB() {
  if( mode < ALL) {
    uint8_t tmpcolor[3] = {0};
    tmpcolor[mode] = rgbcolor[mode];
    setLED( tmpcolor);
  }
  else {
    setLED( rgbcolor);
  }
}

void updateLEDHSX() {
  setLED( rgbcolor);
}

void nextModeRGB() {
  mode = (mode+1)%MODES_SZ;
  Serial.print("Switching to mode ");
  Serial.println(mode);
  uint8_t tmpcolor[3] = {0};
  if( mode < ALL) {
    setLED( crgb[mode]);
    tmpcolor[mode] = rgbcolor[mode];
  }
  else {
    setLED( cwhite);
    uint8_t avg = colAverage();
    for (uint8_t i=0; i<3; ++i) {
      rgbcolor[i] = avg;
      tmpcolor[i] = avg;
    }
    updateStrip();
  }
  delay(DT_BLINK);
  setLED(tmpcolor);
}

void nextModeHSX() {
  mode = (mode+1)%(MODES_SZ-1);
  Serial.print("Switching to mode ");
  Serial.println(mode);
  if( general_mode == HSV)
      colorBlinkLED(chsv[0],mode+1);
  else
      colorBlinkLED(chsl[0],mode+1);
  setLED( rgbcolor);
}

void decColRGB() {
  if (mode < ALL) {
    if (rgbcolor[mode] >= RGBSTEP)
        rgbcolor[mode] -= RGBSTEP;
    else
        rgbcolor[mode] = 0;
  }
  else {
    uint8_t avg = colAverage();
    for (uint8_t i=0; i<3; ++i)
        rgbcolor[i] = avg;
    if (avg >= RGBSTEP) {
      for (uint8_t i=0; i<3; ++i)
          rgbcolor[i] -= RGBSTEP;
    }
  }
}

void decColHSX() {
  hsxcolor[mode] -= HSXSTEP;
  if (hsxcolor[mode] < 0)
  {
    if( mode == HUE)
        hsxcolor[mode] += 1;
    else
    {
      blinkLED(mode==VAL);
      hsxcolor[mode] = 0;
    }
  }
  Serial.print("Setting value ");
  Serial.println(hsxcolor[mode]);
}

void incColRGB() {
  if (mode < ALL) {
    if (rgbcolor[mode] <= RGBMAX-RGBSTEP)
        rgbcolor[mode] += RGBSTEP;
    else
        rgbcolor[mode] = RGBMAX;
  }
  else {
    uint8_t maximum = 0;
    for (uint8_t i=0; i<3; ++i)
    {
      if (rgbcolor[i] > maximum)
          maximum = rgbcolor[i];
    }
    for (uint8_t i=0; i<3; ++i)
        rgbcolor[i] = maximum;
    if (maximum <= RGBMAX-RGBSTEP) {
      for (uint8_t i=0; i<3; ++i)
          rgbcolor[i] += RGBSTEP;
    }
  }
}

void incColHSX() {
  hsxcolor[mode] += HSXSTEP;
  if (hsxcolor[mode] > 1)
  {
    if( mode == HUE)
        hsxcolor[mode] -= 1;
    else
    {
      blinkLED();
      hsxcolor[mode] = 1;
    }
  }
  Serial.print("Setting value ");
  Serial.println(hsxcolor[mode]);
}

void nextMode()
{
  switch(general_mode)
  {
  case HSV:
  case HSL:
    if( mode > VAL)
        mode = VAL;
    nextModeHSX();
    break;
  case RGB:
    nextModeRGB();
    break;
  default:
    break;
  }
}

void nextGenMode()
{
  general_mode = (general_mode+1)%GENMODES_SZ;
  switch( general_mode)
  {
  case HSV:
    RGBConverter::rgbToHsv(
        rgbcolor[RED], rgbcolor[GREEN], rgbcolor[BLUE], hsxcolor);
    cycleLED(chsv);
    mode = VAL;
    display();
    break;
  case HSL:
    RGBConverter::rgbToHsl(
        rgbcolor[RED], rgbcolor[GREEN], rgbcolor[BLUE], hsxcolor);
    cycleLED(chsl);
    mode = LIGHT;
    display();
    break;
  case RGB:
    cycleLED(crgb);
    mode = RED;
    display();
    break;
  case MAX:
    cycleLED(cmax);
    isOn = true;
    setLED(cwhite);
    setStrip(cwhite);
    break;
  default:
    break;
  }
}

void updateLED()
{
  switch(general_mode)
  {
  case HSV:
  case HSL:
    updateLEDHSX();
    break;
  case RGB:
    updateLEDRGB();
    break;
  default:
    break;
  }
}

void decCol()
{
  switch(general_mode)
  {
  case HSV:
  case HSL:
    decColHSX();
    break;
  case RGB:
    decColRGB();
    break;
  default:
    break;
  }
}

void incCol()
{
  switch(general_mode)
  {
  case HSV:
  case HSL:
    incColHSX();
    break;
  case RGB:
    incColRGB();
    break;
  default:
    break;
  }
}

void display()
{
  switch(general_mode)
  {
  case HSV:
    RGBConverter::hsvToRgb(
        hsxcolor[HUE], hsxcolor[SAT], hsxcolor[VAL], rgbcolor);
    break;
  case HSL:
    RGBConverter::hslToRgb2(
        hsxcolor[HUE], hsxcolor[SAT], hsxcolor[LIGHT], rgbcolor);
    break;
  case MAX:
    return;
  default:
    break;
  }
  updateLED();    
  if( !isOn) return;
  updateStrip();
}

void setup() {
  last = -1;
  value= -1;

  Serial.begin(9600);

  pixels.begin();

  encoder = new ClickEncoder(A1, A0, A2, 4, LOW, HIGH);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  setStrip(cblack);
  display();
}

void loop() {
  value += encoder->getValue();
  
  if (value != last) {
    if (value < last)
        decCol();
    else
        incCol();
    
    display();
    last = value;
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
    case ClickEncoder::Held:
      if(!longPress)
      {
        nextGenMode();
      }
      longPress = true;
      break;
    case ClickEncoder::Released:
      longPress = false;
      break;
    case ClickEncoder::Clicked:
          nextMode();
      break;
    case ClickEncoder::DoubleClicked:
        isOn = !isOn;
        if( isOn)
            display();
        else
        {
          setStrip(cblack);
        }
      break;
    default:
      break;
    }
  }    
}
