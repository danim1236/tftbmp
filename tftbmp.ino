#include <SD.h>
#define LCD_CS A3    
#define LCD_CD A2    
#define LCD_WR A1    
#define LCD_RD A0   

//Duemilanove/Diecimila/UNO/etc ('168 and '328 chips) microcontoller:



#define SD_CS 10     // Set the chip select line to whatever you use (10 doesnt conflict with the library)

// In the SD card, place 24 bit color BMP files (be sure they are 24-bit!)
// There are examples in the sketch folder

#include "TFTLCD.h"

// our TFT wiring
#define LCD_RESET A4
TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// the file itself
File bmpFile;

void setup()
{
  tft.reset();
  
  // find the TFT display
  uint16_t identifier = tft.readRegister(0x0);
  if (identifier != 0x9325 && identifier != 0x9328) {
    while (1);
  }  
 
  tft.initDisplay();
  
  pinMode(10, OUTPUT);

  if (!SD.begin(10)) {
    return;
  }

  bmpFile = SD.open("woof2.bmp");

  if (! bmpFile) {
    while (1);
  }
  bmpdraw(bmpFile);
}

void loop()
{
  delay(1000);
}


/*********************************************/

#define BUFFPIXEL 20
#define BMP_WIDTH  240
#define BMP_HEIGHT 320

void bmpdraw(File f) {
  uint8_t bmpImageoffset;

  bmpFile.seek(10);
  bmpImageoffset = read32(f);  
  bmpFile.seek(bmpImageoffset);
  
  uint32_t time = millis();
  uint16_t p;
  uint8_t g, b;
  int i, j;
  
  uint8_t sdbuffer[3 * BUFFPIXEL];  // 3 * pixels to buffer
  uint8_t buffidx = 3*BUFFPIXEL;
  
  tft.goTo(0, 0); 
    
  for (i=0; i< BMP_HEIGHT; i++) {
    // bitmaps are stored with the BOTTOM line first so we have to move 'up'

    for (j=0; j<BMP_WIDTH; j++) {
      // read more pixels
      if (buffidx >= 3*BUFFPIXEL) {
        bmpFile.read(sdbuffer, 3*BUFFPIXEL);
        buffidx = 0;
      }
      
      // convert pixel from 888 to 565
      b = sdbuffer[buffidx++];     // blue
      g = sdbuffer[buffidx++];     // green
      p = sdbuffer[buffidx++];     // red
      
      p >>= 3;
      p <<= 5;
      
      g >>= 3;
      p |= g;
      p <<= 5;
      
      b >>= 3;
      p |= b;
     
       // write out the 16 bits of color
      tft.writeData(p);
    }
  }
}

/*********************************************/

// These read data from the SD card file and convert them to big endian 
// (the data is stored in little endian format!)

// LITTLE ENDIAN!
uint16_t read16(File f) {
  return f.read() | ((uint16_t)f.read()) << 8;
}

// LITTLE ENDIAN!
uint32_t read32(File f) {
  return read16(f) | ((uint32_t)read16(f)) << 16;
}


