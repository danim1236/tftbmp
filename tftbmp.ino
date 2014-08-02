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
TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, 0);

// the file itself
File bmpFile;

// information we extract about the bitmap file
int bmpWidth, bmpHeight;
uint8_t bmpDepth, bmpImageoffset;


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
  
  if (! bmpReadHeader(bmpFile)) { 
     return;
  }
}

void loop()
{
  tft.setRotation(0);
  tft.fillScreen(0);
  bmpdraw(bmpFile, 0, 0);
  delay(1000);

}


/*********************************************/

#define BUFFPIXEL 20

void bmpdraw(File f, int x, int y) {
  bmpFile.seek(bmpImageoffset);
  
  uint32_t time = millis();
  uint16_t p;
  uint8_t g, b;
  int i, j;
  
  uint8_t sdbuffer[3 * BUFFPIXEL];  // 3 * pixels to buffer
  uint8_t buffidx = 3*BUFFPIXEL;
  
  for (i=0; i< bmpHeight; i++) {
    // bitmaps are stored with the BOTTOM line first so we have to move 'up'

    if (tft.getRotation() == 3) {
      tft.goTo(x, y+bmpHeight-i); 
    } else if  (tft.getRotation() == 2) {
      tft.goTo(x+i, y);
    } else if  (tft.getRotation() == 1) {
        tft.goTo(x+bmpWidth-i, y+bmpHeight); 
    } else if  (tft.getRotation() == 0) {
      tft.goTo(x+bmpWidth, y+i); 
    }
    
    for (j=0; j<bmpWidth; j++) {
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

boolean bmpReadHeader(File f) {
   // read header
  uint32_t tmp;
  
  if (read16(f) != 0x4D42) {
    // magic bytes missing
    return false;
  }
 
  // read file size
  tmp = read32(f);  
  
  // read and ignore creator bytes
  read32(f);
  
  bmpImageoffset = read32(f);  
  
  // read DIB header
  tmp = read32(f);
  bmpWidth = read32(f);
  bmpHeight = read32(f);

  
  if (read16(f) != 1)
    return false;
    
  bmpDepth = read16(f);

  if (read32(f) != 0) {
    // compression not supported!
    return false;
  }
  
  return true;
}

/*********************************************/

// These read data from the SD card file and convert them to big endian 
// (the data is stored in little endian format!)

// LITTLE ENDIAN!
uint16_t read16(File f) {
  uint16_t d;
  uint8_t b;
  b = f.read();
  d = f.read();
  d <<= 8;
  d |= b;
  return d;
}


// LITTLE ENDIAN!
uint32_t read32(File f) {
  uint32_t d;
  uint16_t b;
 
  b = read16(f);
  d = read16(f);
  d <<= 16;
  d |= b;
  return d;
}


