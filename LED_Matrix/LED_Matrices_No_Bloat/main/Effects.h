/*
 * Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Portions of this code are adapted from "Funky Clouds" by Stefan Petrick: https://gist.github.com/anonymous/876f908333cd95315c35
 * Portions of this code are adapted from "NoiseSmearing" by Stefan Petrick: https://gist.github.com/StefanPetrick/9ee2f677dbff64e3ba7a
 * Copyright (c) 2014 Stefan Petrick
 * http://www.stefan-petrick.de/wordpress_beta
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef Effects_H
#define Effects_H

/* ---------------------------- GLOBAL CONSTANTS ----------------------------- */

const int  MATRIX_CENTER_X = VPANEL_W / 2;
const int  MATRIX_CENTER_Y = VPANEL_H / 2;
// US vs GB, huh? :)
//const byte MATRIX_CENTRE_X = MATRIX_CENTER_X - 1;
//const byte MATRIX_CENTRE_Y = MATRIX_CENTER_Y - 1;
#define MATRIX_CENTRE_X MATRIX_CENTER_X
#define MATRIX_CENTRE_Y MATRIX_CENTER_Y


const uint16_t NUM_LEDS = (VPANEL_W * VPANEL_H) + 1; // one led spare to capture out of bounds

// forward declaration
uint16_t XY16( uint16_t x, uint16_t y);

/* Convert x,y co-ordinate to flat array index. 
 * x and y positions start from 0, so must not be >= 'real' panel width or height 
 * (i.e. 64 pixels or 32 pixels.).  Max value: VPANEL_W-1 etc.
 * Ugh... uint8_t - really??? this weak method can't cope with 256+ pixel matrices :(
 */
uint16_t XY( uint8_t x, uint8_t y) 
{
  return XY16(x, y);
}

/**
 *  The one for 256+ matrices
 *  otherwise this:
 *    for (uint8_t i = 0; i < VPANEL_W; i++) {}
 *  turns into an infinite loop
 */
uint16_t XY16( uint16_t x, uint16_t y) 
{
    if( x >= VPANEL_W) return 0;
    if( y >= VPANEL_H) return 0;

    return (y * VPANEL_W) + x + 1; // everything offset by one to compute out of bounds stuff - never displayed by ShowFrame()
}

class Effects {
public:
  CRGB *leds;

  Effects(){
    // we do dynamic allocation for leds buffer, otherwise esp32 toolchain can't link static arrays of such a big size for 256+ matrices
    leds = (CRGB *)malloc(NUM_LEDS * sizeof(CRGB));
    ClearFrame();
  }
  ~Effects(){
    free(leds);
  }

  /* The only 'framebuffer' we have is what is contained in the leds and leds2 variables.
   * We don't store what the color a particular pixel might be, other than when it's turned
   * into raw electrical signal output gobbly-gook (i.e. the DMA matrix buffer), but this * is not reversible.
   * 
   * As such, any time these effects want to write a pixel color, we first have to update
   * the leds or leds2 array, and THEN write it to the RGB panel. This enables us to 'look up' the array to see what a pixel color was previously, each drawFrame().
   */
  void drawBackgroundFastLEDPixelCRGB(int16_t x, int16_t y, CRGB color)
  {
	  leds[XY(x, y)] = color;
	  //matrix.drawPixelRGB888(x, y, color.r, color.g, color.b); 
  }

  // write one pixel with the specified color from the current palette to coordinates
  void Pixel(int x, int y, uint8_t colorIndex) {
    leds[XY(x, y)] = ColorFromCurrentPalette(colorIndex);
    //matrix.drawPixelRGB888(x, y, temp.r, temp.g, temp.b); // now draw it?
  }

  void ShowFrame() {
    currentPalette = targetPalette;

	for (int y=0; y<VPANEL_H; ++y){
  	    for (int x=0; x<VPANEL_W; ++x){
    		uint16_t _pixel = XY16(x,y);
    		virtualDisp->drawPixelRGB888( x, y, leds[_pixel].r, leds[_pixel].g, leds[_pixel].b);
	    } // end loop to copy fast led to the dma matrix
	}
  }

  // scale the brightness of the screenbuffer down
  void DimAll(byte value)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].nscale8(value);
    }
  }  

  void ClearFrame()
  {
      memset(leds, 0x00, NUM_LEDS * sizeof(CRGB)); // flush
  }

  // palettes
  // KEEP ALL PALETTE CODE
  static const int paletteCount = 9;
  int paletteIndex = -1;
  TBlendType currentBlendType = LINEARBLEND;
  CRGBPalette16 currentPalette;
  CRGBPalette16 targetPalette;
  char* currentPaletteName;

  static const int HeatColorsPaletteIndex = 6;

  void Setup() {
    // currentPalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
    currentPalette = OceanColors_p;
    loadPalette(1);
    // NoiseVariablesSetup();
  }

  void loadPalette(int index) {
    paletteIndex = index;

    if (paletteIndex >= paletteCount)
      paletteIndex = 0;
    else if (paletteIndex < 0)
      paletteIndex = paletteCount - 1;

    switch (paletteIndex) {
      case 0:
        targetPalette = RainbowColors_p;
        currentPaletteName = (char *)"Rainbow";
        break;
      case 1:
        targetPalette = OceanColors_p;
        currentPaletteName = (char *)"Ocean";
        break;
      case 2:
        targetPalette = CloudColors_p;
        currentPaletteName = (char *)"Cloud";
        break;
      case 3:
        targetPalette = ForestColors_p;
        currentPaletteName = (char *)"Forest";
        break;
      case 4:
        targetPalette = PartyColors_p;
        currentPaletteName = (char *)"Party";
        break;
      case 5:
        setupGrayscalePalette();
        currentPaletteName = (char *)"Grey";
        break;
      case HeatColorsPaletteIndex:
        targetPalette = HeatColors_p;
        currentPaletteName = (char *)"Heat";
        break;
      case 7:
        targetPalette = LavaColors_p;
        currentPaletteName = (char *)"Lava";
        break;
      case 8:
        setupIcePalette();
        currentPaletteName = (char *)"Ice";
        break;
    }
  }

  void setPalette(String paletteName) {
    if (paletteName == "Rainbow")
      loadPalette(0);
    else if (paletteName == "Ocean")
      loadPalette(1);
    else if (paletteName == "Cloud")
      loadPalette(2);
    else if (paletteName == "Forest")
      loadPalette(3);
    else if (paletteName == "Party")
      loadPalette(4);
    else if (paletteName == "Grayscale")
      loadPalette(5);
    else if (paletteName == "Heat")
      loadPalette(6);
    else if (paletteName == "Lava")
      loadPalette(7);
    else if (paletteName == "Ice")
      loadPalette(8);
  }

  void setupGrayscalePalette() {
    targetPalette = CRGBPalette16(CRGB::Black, CRGB::White);
  }

  void setupIcePalette() {
    targetPalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
  }

  // create a square twister to the left or counter-clockwise
  // x and y for center, r for radius
  void SpiralStream(int x, int y, int r, byte dimm) {
    for (int d = r; d >= 0; d--) { // from the outside to the inside
      for (int i = x - d; i <= x + d; i++) {
        leds[XY16(i, y - d)] += leds[XY16(i + 1, y - d)]; // lowest row to the right
        leds[XY16(i, y - d)].nscale8(dimm);
      }
      for (int i = y - d; i <= y + d; i++) {
        leds[XY16(x + d, i)] += leds[XY16(x + d, i + 1)]; // right column up
        leds[XY16(x + d, i)].nscale8(dimm);
      }
      for (int i = x + d; i >= x - d; i--) {
        leds[XY16(i, y + d)] += leds[XY16(i - 1, y + d)]; // upper row to the left
        leds[XY16(i, y + d)].nscale8(dimm);
      }
      for (int i = y + d; i >= y - d; i--) {
        leds[XY16(x - d, i)] += leds[XY16(x - d, i - 1)]; // left column down
        leds[XY16(x - d, i)].nscale8(dimm);
      }
    }
  }

  void BresenhamLine(int x0, int y0, int x1, int y1, byte colorIndex)
  {
    BresenhamLine(x0, y0, x1, y1, ColorFromCurrentPalette(colorIndex));
  }

  void BresenhamLine(int x0, int y0, int x1, int y1, CRGB color)
  {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;) {
      leds[XY16(x0, y0)] += color;
      if (x0 == x1 && y0 == y1) break;
      e2 = 2 * err;
      if (e2 > dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 < dx) {
        err += dx;
        y0 += sy;
      }
    }
  }

  CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND) {
    return ColorFromPalette(currentPalette, index, brightness, currentBlendType);
  }
  
};

#endif
