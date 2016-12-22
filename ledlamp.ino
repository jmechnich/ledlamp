#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <ClickEncoder.h>
#include <TimerOne.h>
#include <RGBConverter.h>


#include "defs.h"

//const uint8_t** cpresets = cblackbody;
//const uint8_t** cpresets = cfluor;
//const uint8_t** cpresets = cgas;
const uint8_t* cpresets[] = {ccandle,csodvapor,csodhigh,0};

/*-------------------------------------------------------------------------
 *  Variables
 *-------------------------------------------------------------------------*/
Adafruit_NeoPixel pixels =
    Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ClickEncoder *encoder;
int16_t last=-1, value=-1;
bool longPress = false;
bool isOn = false;

uint8_t rgbcolor[3] = {0};
double  hsvcolor[3] = {0};
uint8_t rgbsave[3] = {0};
uint8_t brightness  = 255;
    
uint8_t curpreset = 0;
uint8_t general_mode = RGB;
uint8_t mode = general_mode == RGB ? ALL : VAL;

const uint8_t fadetime   = 10;
uint8_t fadensteps = 50;

uint8_t fadecounter  = 0;
uint8_t fadestep     = 0;
bool    fading       = false;
double  fadestart[3] = {0};
uint8_t fadetarget[3]  = {0};

void startFade(const uint8_t* rgb, const uint8_t* rgbcur=0)
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
  fadensteps = uint8_t(max/5+1.5);
}

void fadeStep() 
{
  if(!fading) return;
  if(++fadestep==fadensteps) {
    fading = false;
    memcpy(rgbcolor,fadetarget,3);
    return;
  }

  Serial.print("fadestep ");
  Serial.print(fadestep);
  Serial.print(" color");
  
  for(uint8_t i=0;i<3;++i) {
    double color = fadestart[i] + \
        double(fadestep)/fadensteps*(fadetarget[i]-fadestart[i]);
    if(color < 0)
        color = 0;
    else if(color > 255)
        color = 255;
    rgbcolor[i] = uint8_t(color+0.5);
    Serial.print(" ");
    Serial.print(rgbcolor[i]);
  }
  Serial.println();
}
  
/*-------------------------------------------------------------------------
 *  General helper functions
 *-------------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------------
 *  Strip (WS2812 leds)
 *-------------------------------------------------------------------------*/
void setStrip( uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(r,g,b));
    pixels.show();
  }
}

void setStrip( const uint8_t rgb[3]) {
  setStrip( rgb[RED], rgb[GREEN], rgb[BLUE]);
}

void setStrip() {
  setStrip(rgbcolor);
}

/*-------------------------------------------------------------------------
 *  RGB LED of encoder
 *-------------------------------------------------------------------------*/
void setLED( uint8_t r, uint8_t g, uint8_t b, uint8_t bright) {
  analogWrite(PIN_RED,  255-uint8_t(r/255.*bright+0.5));
  analogWrite(PIN_GREEN,255-uint8_t(g/255.*bright+0.5));
  analogWrite(PIN_BLUE, 255-uint8_t(b/255.*bright+0.5));
}

void setLED( const uint8_t rgb[3], uint8_t bright) {
  setLED( rgb[RED], rgb[GREEN], rgb[BLUE], bright);
}

void setLED() 
{
  setLED(rgbcolor, brightness);
}

void blinkLED(bool invert=false) 
{
  if(invert)
      setLED(cwhite,255);
  else
      setLED(cblack,255);
  delay(DT_BLINK);
  setLED();
}

void colorBlinkLED(const uint8_t rgb[3], uint8_t count=1) 
{
  for( uint8_t i=0; i<count; ++i)
  {
    setLED(rgb,brightness);
    delay(DT_BLINK);
    setLED(cblack,brightness);
    delay(DT_BLINK);
  }
  setLED();
}

void cycleLED( const uint8_t* rgbs[3])
{
  size_t i=0;
  while(rgbs[i])
  {
    setLED(rgbs[i],brightness);
    delay(DT_CYCLE);
    ++i;
  }
  setLED();
}

/*-------------------------------------------------------------------------
 *  Mode switching
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

void nextModeRGB() {
  mode = (mode+1)%MODES_SZ;
  Serial.print("Switching to mode ");
  Serial.println(mode);
  uint8_t tmpcolor[3] = {0};
  if( mode < ALL) {
    setLED(crgb[mode],brightness);
    tmpcolor[mode] = rgbcolor[mode];
  }
  else {
    setLED(cwhite,brightness);
    uint8_t avg = colAverage();
    for (uint8_t i=0; i<3; ++i) {
      rgbcolor[i] = avg;
      tmpcolor[i] = avg;
    }
    setStrip();
  }
  delay(DT_BLINK);
  setLED(tmpcolor,brightness);
}

void nextModeHSV() {
  mode = (mode+1)%(MODES_SZ-1);
  Serial.print("Switching to mode ");
  Serial.println(mode);
  colorBlinkLED(chsv[0],mode+1);
  setLED();
}

void nextMode() {
  switch(general_mode)
  {
  case HSV:
    if( mode > VAL)
        mode = VAL;
    nextModeHSV();
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
    cycleLED(chsv);
    mode = VAL;
    refresh();
    break;
  case RGB:
    cycleLED(crgb);
    mode = RED;
    refresh();
    break;
  case MAX:
    cycleLED(cmax);
    setLED(cwhite,brightness);
    setStrip(cwhite);
    break;
  default:
    break;
  }
}

void nextPreset() 
{
  if(cpresets[++curpreset] == 0)
      curpreset = 0;
  startFade(cpresets[curpreset]);
  Serial.print("Preset ");
  Serial.println(curpreset);
  
  //setRGB(cpresets[curpreset]);
  //refresh();
}

/*-------------------------------------------------------------------------
 *  Color change
 *-------------------------------------------------------------------------*/
void setRGB(uint8_t r, uint8_t g, uint8_t b)
{
  rgbcolor[RED]   = r;
  rgbcolor[GREEN] = g;
  rgbcolor[BLUE]  = b;
  RGBConverter::rgbToHsv(r, g, b, hsvcolor);
}

void setRGB(const uint8_t rgb[3])
{
  setRGB(rgb[RED],rgb[GREEN],rgb[BLUE]);
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
  RGBConverter::rgbToHsv(rgbcolor[RED], rgbcolor[GREEN], rgbcolor[BLUE], hsvcolor);
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
  RGBConverter::rgbToHsv(rgbcolor[RED], rgbcolor[GREEN], rgbcolor[BLUE], hsvcolor);
}

void setHSV(double h, double s, double v)
{
  hsvcolor[HUE] = h;
  hsvcolor[SAT] = s;
  hsvcolor[VAL] = v;
  RGBConverter::hsvToRgb(h, s, v, rgbcolor);
}

void setHSV(double hsv[3])
{
  setHSV(hsv[HUE],hsv[SAT],hsv[VAL]);
}

void decColHSV() {
  hsvcolor[mode] -= HSVSTEP;
  if (hsvcolor[mode] < 0)
  {
    if( mode == HUE)
        hsvcolor[mode] += 1;
    else
    {
      blinkLED(mode==VAL);
      hsvcolor[mode] = 0;
    }
  }
  RGBConverter::hsvToRgb(hsvcolor[HUE], hsvcolor[SAT], hsvcolor[VAL], rgbcolor);
}

void incColHSV() {
  hsvcolor[mode] += HSVSTEP;
  if (hsvcolor[mode] > 1)
  {
    if( mode == HUE)
        hsvcolor[mode] -= 1;
    else
    {
      blinkLED();
      hsvcolor[mode] = 1;
    }
  }
  RGBConverter::hsvToRgb(hsvcolor[HUE], hsvcolor[SAT], hsvcolor[VAL], rgbcolor);
}

void decCol()
{
  switch(general_mode)
  {
  case HSV:
    decColHSV();
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
    incColHSV();
    break;
  case RGB:
    incColRGB();
    break;
  default:
    break;
  }
}

/*-------------------------------------------------------------------------
 *  Encoder handlers
 *-------------------------------------------------------------------------*/
void timerIsr() {
  encoder->service();
}

bool toggleOn() {
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
  if (brightness==0) {
    blinkLED();
    return;
  }
  brightness -= 5;
  pixels.setBrightness(brightness);
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
  pixels.setBrightness(brightness);
  refresh();
}

/*-------------------------------------------------------------------------
 *  setup() and loop() functions
 *-------------------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);

  pixels.begin();
  pixels.setBrightness(brightness);
  memcpy(rgbsave,cpresets[curpreset],3);
  refresh();

  encoder = new ClickEncoder(A1, A0, A2, 4, LOW, HIGH);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

}

void loop() {
  if (fading && (++fadecounter%fadetime == 0))
  {
    fadecounter = 0;
    fadeStep();
    refresh();
  }
  value += encoder->getValue();
  if (value != last) {
    if (value < last)
        handleTurnLeft();
    else
        handleTurnRight();
    last = value;
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
    case ClickEncoder::Held:
      if(!longPress)
      {
        handleLongPress();
      }
      longPress = true;
      break;
    case ClickEncoder::Released:
      longPress = false;
      break;
    case ClickEncoder::Clicked:
      handleClick();
      break;
    case ClickEncoder::DoubleClicked:
      handleDoubleClick();
      break;
    default:
      break;
    }
  }    
}
