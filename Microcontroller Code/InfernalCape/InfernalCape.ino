/* 
   REQUIRES the following Arduino libraries:
 - FastLED Library: https://github.com/pixelmatix/FastLED
 - AnimatedGIF Library:  https://github.com/bitbank2/AnimatedGIF
 */
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <SD.h>
#include <AnimatedGIF.h>

#define PIN_LEDS 13
#define LED_RES_X 25 // Number of pixels wide of each INDIVIDUAL panel module. 
#define LED_RES_Y 60 // Number of pixels tall of each INDIVIDUAL panel module.
#define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW
#define MAX_POWER_MILLIAMPS 13000

const uint8_t MATRIX_WIDTH = LED_RES_X * NUM_COLS;
const uint8_t MATRIX_HEIGHT = LED_RES_Y * NUM_ROWS;
const uint16_t NUM_LEDS = (MATRIX_WIDTH * MATRIX_HEIGHT);
const uint16_t xybuff = (LED_RES_X * LED_RES_Y);

CRGB leds[NUM_LEDS];
CRGB black = CRGB::Black;


  uint16_t XY( uint8_t x, uint8_t y) {
    if( x >= MATRIX_WIDTH || x < 0 || y >= MATRIX_HEIGHT || y < 0) return -1;
    uint16_t i;
    uint8_t yi = y / LED_RES_Y;
    y %= LED_RES_Y;   
    if( yi & 0x01) {
      if( x & 0x01) {
        uint8_t reverseY = (LED_RES_Y - 1) - y;
        i = (((LED_RES_X * NUM_COLS) - x - 1) * LED_RES_Y) + reverseY;
      } else {
        i = (((LED_RES_X * NUM_COLS) - x - 1) * LED_RES_Y) + y;
      }
    } else {
      if( x & 0x01) {
        uint8_t reverseY = (LED_RES_Y - 1) - y ;
        i = (x * LED_RES_Y) + reverseY;
      } else {
        i = (x * LED_RES_Y) + y;
      }
    }
    i += xybuff * yi * NUM_COLS;
    return i;
}


AnimatedGIF gif;
File f;
int x_offset, y_offset;



// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw)
{
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  x = pDraw->iX; // current line
  y = pDraw->iY + pDraw->y; // current line
  
  if (iWidth > (MATRIX_WIDTH - x))
      iWidth = (MATRIX_WIDTH - x);

  if (y >= MATRIX_HEIGHT || x >= MATRIX_WIDTH || iWidth < 1)
       return; 
       
  uint8_t *s, c;
  CRGB *usPalette = (CRGB *)pDraw->pPalette;
  s = pDraw->pPixels;
  for (int i=0; i<iWidth; i++) {
    c = s[i];
    if (c != 0xFF)
      leds[XY(i+x, y)] = usPalette[c];
  }
} /* GIFDraw() */


void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  Serial.print("Playing gif: ");
  Serial.println(fname);
  f = SD.open(fname);
  if (f)
  {
    *pSize = f.size();
    return (void *)&f;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

unsigned long start_tick = 0;

// void ShowGIF(char *name)
// {
//   start_tick = millis();
   
//   if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
//   {
//     x_offset = (MATRIX_WIDTH - gif.getCanvasWidth())/2;
//     if (x_offset < 0) x_offset = 0;
//     y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight())/2;
//     if (y_offset < 0) y_offset = 0;              
//     memset(leds, 0, sizeof(leds)); 
//     while (gif.playFrame(false, NULL))
//     { 
//       FastLED.show();
//     }
//     gif.close();
//   }

// } /* ShowGIF() */

void ShowGIF(char *name)
{
    start_tick = millis();
    if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
    {
        x_offset = (MATRIX_WIDTH - gif.getCanvasWidth()) / 2;
        if (x_offset < 0) x_offset = 0;
        y_offset = (MATRIX_HEIGHT - gif.getCanvasHeight()) / 2;
        if (y_offset < 0) y_offset = 0;

        memset(leds, 0, sizeof(leds)); 

        // Infinite loop for the GIF
        while (true)
        {
            // Play through all frames
            while (gif.playFrame(false, NULL))
            {
                FastLED.show();
                delay(30);
            }
            // Restart the GIF from the beginning
            gif.reset();
        }

        gif.close();
    }
    else
    {
        Serial.printf("Failed to open GIF: %s\n", name);
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting AnimatedGIFs Sketch");

  // Start SD Card
  Serial.println(" * Loading SD Card");
  if(!SD.begin(5)){
        Serial.println("SD Card Mount Failed");
  }
  FastLED.setMaxPowerInVoltsAndMilliamps( 5, MAX_POWER_MILLIAMPS);
  FastLED.addLeds<WS2812B,PIN_LEDS,GRB>(leds,NUM_LEDS);
  FastLED.setBrightness(128);
  gif.begin(GIF_PALETTE_RGB8888);
}

String gifDir = "/gifs"; // play all GIFs in this directory on the SD card
char filePath[256] = { 0 };
File root, gifFile;

void loop() 
{
   while (1) // run forever
   {
      root = SD.open(gifDir);
      if (root)
      {
           gifFile = root.openNextFile();
            while (gifFile)
            {
              if (!gifFile.isDirectory()) // play it
              {
                memset(filePath, 0x0, sizeof(filePath));
                snprintf(filePath, sizeof(filePath), "%s/%s", gifDir.c_str(), gifFile.name());
                
                // Show the GIF
                ShowGIF(filePath);
              }
              gifFile.close();
              gifFile = root.openNextFile();
            }
         root.close();
      } // root
      delay(1000); // pause before restarting
   } // while
}