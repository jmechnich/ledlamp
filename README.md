Arduino project for an WS2812-based LED lamp, controlled by an encoder with a push-button.

## Features
* Double-click switches lamp on/off.
* Long press cycles color modes: HSV (power-on default), HSL, RGB, maximum white. The selected mode is indicated by the encoder LED.
* Single-click cycles colorspace components. The selected component is indicated by the encoder LED.
* Encoder rotation changes the selected colorspace component.

## Library Dependencies
* [adafruit NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel)
* [RGB conversion library](https://github.com/jmechnich/RGBConverter)
* [Encoder library](https://github.com/jmechnich/encoder)
* [TimerOne (needed by encoder library)](https://github.com/PaulStoffregen/TimerOne)

## Hardware used in original project
* [Arduino Pro Mini 5V](https://www.arduino.cc/en/Main/ArduinoBoardProMini)
* [Sparkfun Rotary Encoder - Illuminated (RGB)](https://www.sparkfun.com/products/10982)
* [adafruit NeoPixel Ring w/ 24 Leds](https://www.adafruit.com/products/1586)
 
## Wishlist
* Add a mode for running pre-defined programs (e.g. rainbow pattern, scanners, etc.).
