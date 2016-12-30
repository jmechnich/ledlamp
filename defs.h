#ifndef DEFS_H
#define DEFS_H

#define RGBMAX   255

// basic colors
const uint8_t  cblack[3]   = {0};
const uint8_t  cwhite[3]   = {RGBMAX,RGBMAX,RGBMAX};
const uint8_t  cred[3]     = {RGBMAX,0,0};
const uint8_t  cgreen[3]   = {0,RGBMAX,0};
const uint8_t  cblue[3]    = {0,0,RGBMAX};
const uint8_t  cyellow[3]  = {RGBMAX,RGBMAX,0};
const uint8_t  cmagenta[3] = {RGBMAX,0,RGBMAX};
const uint8_t  ccyan[3]    = {0,RGBMAX,RGBMAX};

// blackbody colors
const uint8_t  ccandle[3]     = {255, 147,  41}; // Candle, 1900K !
const uint8_t  ctun40[3]      = {255, 197, 143}; // 40W Tungsten, 2600K
const uint8_t  ctun100[3]     = {255, 214, 170}; // 100W Tungsten, 2850K
const uint8_t  chalogen[3]    = {255, 241, 224}; // Halogen, 3200K
const uint8_t  ccarbon[3]     = {255, 250, 244}; // Carbon Arc, 5200K
const uint8_t  chighnoon[3]   = {255, 255, 251}; // High Noon Sun, 5400K
const uint8_t  cdirectsun[3]  = {255, 255, 255}; // Direct Sunlight, 6000K
const uint8_t  covercast[3]   = {201, 226, 255}; // Overcast Sky, 7000K
const uint8_t  cclearsky[3]   = { 64, 156, 255}; // Clear Blue Sky, 20000K
const uint8_t* cblackbody[] = {ccandle,ctun40,ctun100,chalogen,ccarbon,chighnoon,cdirectsun,covercast,cclearsky,0};

// flourescent colors
const uint8_t  cfluorwarm[3]  = {255, 244, 229}; // Warm Fluorescent
const uint8_t  cfluorstd[3]   = {244, 255, 250}; // Standard Fluorescent
const uint8_t  cfluorcool[3]  = {212, 235, 255}; // Cool White Fluorescent
const uint8_t  cfluorfull[3]  = {255, 244, 242}; // Full Spectrum Fluorescent
const uint8_t  cfluorgrow[3]  = {255, 239, 247}; // Grow Light Fluorescent
const uint8_t  cfluorblack[3] = {167,   0, 255}; // Black Light Fluorescent
const uint8_t* cfluor[] = {cfluorwarm,cfluorstd,cfluorcool,cfluorfull,cfluorgrow,cfluorblack,0};

// gas light colors
const uint8_t  cmercvapor[3] = {216, 247, 255}; // Mercury Vapor
const uint8_t  csodvapor[3]  = {255, 209, 178}; // Sodium Vapor !
const uint8_t  chalide[3]    = {242, 252, 255}; // Metal Halide
const uint8_t  csodhigh[3]   = {255, 183,  76}; // High Pressure Sodium !
const uint8_t* cgas[] = {cmercvapor,csodvapor,chalide,csodhigh,0};

#endif
