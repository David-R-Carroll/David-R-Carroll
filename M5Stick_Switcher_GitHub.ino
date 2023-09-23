
/*
###################################################################################
#                                                                                 #
#                M5StickC Production Switcher Simulator. v 1.0 Build 2345678      #
#                  Badly written by David R. Carroll.                             #
#                    Creative Commons CC BY-NC 2.0 2023                           #
#                      This is the Program monitor sim.                           #
#                        Preview is the same, just a use a different IP           #
#                          It recieves text commamnds from the panel or Telnet.   #
#                                                                                 #
###################################################################################
*/

// How does this thing work?
// We open a socket and listen for text messages.
// These instructions are sent from the Python panel program but can be sent by any telnet client.
// For example:
// BKGD:1      // Set the Background bus to image 1 (Bars)
// PST:4       // Set the PV (Preview) bus to image 4 (Canadian Flag)
// TRANSTYPE:1 // Prepare to do a Wipe. 
// TRANSWIPE:3 // Prepare to do a Star Wipe
// MEAUTO:2    // Transition from PV to PGM in 1 second (60 frames) 
//
// Understanding this code and the panel program rely on some knowledge of how 
// real switchers work and common TV broadcasting terms.
// For more information, watch Broadcast News (1987), and excellent movie.
// I don't reccomend buying a textbook. I did that once and it ruinned my life.



#include <M5StickC.h>

#include "Free_Fonts.h"

// These are the 160 x 80 images sent to the M5 LCD screen.
// Each uint16_t word is one pixel in 565 format.
#include "Wash1.h"
#include "Bars.h"
#include "Anna.h"
#include "Flag.h"
#include "Anchor.h"
#include "Stana.h"
#include "AngelsRed.h"
#include "BlueCK.h"
#include "Dixie.h"
#include "DVEGrid.h"

// This file defines the starwipe wipe's shape.
#include "StarWipeHex.h"

#include <WiFi.h>

// If you want to use a second (preview) M5StickC 
// then make a copy of this program and assign a different IP to it.

IPAddress ip(0, 0, 0, 0);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(255, 255, 0, 0);

const char* ssid     = "Your WiFi name here";
const char* password = "Your WiFi (and nothing else!) password here";

// Use a different port if you like. Just match it in the panel.
WiFiServer server(21);

// This buffer contains the image that will be pushed to the M5 screen
uint16_t LCD_Buffer[12800];
const uint16_t *LCD_Pointer;

// This points to the current key image .h file.
const uint16_t *Key_Pointer;

// Current BKGD (background) bus image
uint16_t BKGD_Buffer[12800];
const uint16_t *BKGD_Pointer;

// Current PST (preset) bus image
uint16_t PV_Buffer[12800];
const uint16_t *PV_Pointer;

// These pointers are passed to the various Dissolve / Wipe / DVE subs.
const uint16_t *PST_Pointer;
const uint16_t *Temp_Pointer;
const uint16_t *From_Pointer;
const uint16_t *To_Pointer;

int16_t Wipe_Border = WHITE;

// Globals. Too many Globals.

int KeyBus;
int BKGDBus;
int PSTBus;
int TransIncl = 1;
int KeyTally = 0;
int TransType = 0;
int TransDVE = 2;
int TransWipe = 2;
int TransDir = 2;
float DVESize;
float DVExPos;
float DVEyPos;
float DVEPersp;

// About now you may be wondering how many naming conventions is he going to use?
// Read on.

int Mem_KeyBus;
int Mem_BKGDBus;
int Mem_PSTBus;
int Mem_TransIncl = 1;
int Mem_KeyTally = 0;
int Mem_TransType = 0;
int Mem_TransDVE = 2;
int Mem_TransWipe = 2;
int Mem_TransDir = 2;
float Mem_DVESize;
float Mem_DVExPos;
float Mem_DVEyPos;
float Mem_DVEPersp;

// Finally, some (terrible) code!

// Get the still pointer associated with requested panel button number.
const uint16_t *GetStill(int Xpt) {
  const uint16_t *Return_Pointer;
  switch (Xpt) {
    case 0:
      Return_Pointer = Wash1_bits;
      break;
    case 1:
      Return_Pointer = Bars_bits;
      break;
    case 2:
      Return_Pointer = Anna_bits;
      break;
    case 3:
      Return_Pointer = Flag_bits;
      break;
    case 4:
      Return_Pointer = Anchor_bits;
      break;
    case 5:
      Return_Pointer = Stana_bits;
      break;
    case 6:
      Return_Pointer = AngelsRed_bits;
      break;
    case 7:
      Return_Pointer = BlueCK_bits;
      break;
    case 8:
      Return_Pointer = DVEGrid_bits;
      break;
    case 9:
      Return_Pointer = Dixie_bits;
      break;
  }
  return (Return_Pointer);
}

// This sets up the buffers based on the panel's next transition buttons (BKGD, KEY, BKGD + KEY)
void SetTransition(int TransIncl, int KeyTally) {
  int x;
  for (x = 0; x < 12800; x++) {
    BKGD_Buffer[x] = LCD_Buffer[x];
  }

  switch (TransIncl) {
    case 1:  // BKGD
      if (KeyTally == 0) {
        From_Pointer = BKGD_Pointer;
        To_Pointer = PST_Pointer;
      } else {
        DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
        for (x = 0; x < 12800; x++) {
          BKGD_Buffer[x] = PV_Buffer[x];
        }
        From_Pointer = BKGD_Buffer;

        DVE(PST_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
        To_Pointer = PV_Buffer;
      }
      break;

    case 2:  // KEY
      if (KeyTally == 0) {
        From_Pointer = PV_Buffer;
        To_Pointer = BKGD_Pointer;
      } else {
        DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
        From_Pointer = BKGD_Pointer;
        To_Pointer = PV_Buffer;
      }
      break;

    case 3:  // BKGD + KEY
      if (KeyTally == 0) {
        From_Pointer = PV_Buffer;
        To_Pointer = PST_Pointer;
      } else {
        DVE(PST_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
        From_Pointer = BKGD_Pointer;
        To_Pointer = PV_Buffer;
      }
      break;
  }
}

// This calls the correct subroutine based on the Transition Type and Dissolve, Wipe or DVE type selected on the panel. 
void DoTransition(int gRate) {
  switch (TransType) {  // Auto Trans
    case 0:             //Dissolve
      Dissolve(From_Pointer, To_Pointer, gRate);
      break;
    case 1:  // Wipe
      if (TransWipe == 1) {
        Wipe_SplitH(From_Pointer, To_Pointer, gRate);
      }
      if (TransWipe == 2) {
        Wipe_Race(From_Pointer, To_Pointer, gRate);
      }
      if (TransWipe == 3) {
        StarWipe(From_Pointer, To_Pointer, gRate);
      }
      if (TransWipe == 4) {
        Checkers(gRate);
      }
      break;
    case 2:  // DVE
      if (TransDVE == 1) {
        DVE_PushR(From_Pointer, To_Pointer, gRate);
      }
      if (TransDVE == 2) {
        DVE_4Box(From_Pointer, To_Pointer, gRate);
      }
      if (TransDVE == 3) {
        DVE_SqueezeH(gRate);
      }
      if (TransDVE == 4) {
        DVE_Angels(From_Pointer, To_Pointer, gRate);
      }
      break;
  }
  if (TransIncl == 1 || TransIncl == 3) {
    Temp_Pointer = BKGD_Pointer;
    BKGD_Pointer = PST_Pointer;
    PST_Pointer = Temp_Pointer;
  }
}

// This does a centered DVE wipe from 0 to 100%
void DVE_Square(int g_Rate) {
  float Size;
  int x;

  for (Size = 0; Size < 101; Size = Size + g_Rate) {
    DVE(From_Pointer, To_Pointer, 0, 0, Size, 0);
    M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Buffer);
    // delay(g_Rate);
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = To_Pointer[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// This performs Effects Dissolve where a DVE starts and ends at arbitrary points and size.
void Tween_DVE(int g_Rate) {

  int x;
  float Tw_DVESize;
  float Tw_DVExPos;
  float Tw_DVEyPos;
  float Tw_DVEPersp;
  float Interval;
  Interval = float(128 / g_Rate);

  Tw_DVESize = (Mem_DVESize - DVESize) / Interval;
  Tw_DVExPos = (Mem_DVExPos - DVExPos) / Interval;
  Tw_DVEyPos = (Mem_DVEyPos - DVEyPos) / Interval;
  Tw_DVEPersp = (Mem_DVEPersp - DVEPersp) / Interval;

  for (x = 0; x < Interval; x++) {
    DVESize = DVESize + Tw_DVESize;
    DVExPos = DVExPos + Tw_DVExPos;
    DVEyPos = DVEyPos + Tw_DVEyPos;
    DVEPersp = DVEPersp + Tw_DVEPersp;

    DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
    M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Buffer);
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = PV_Buffer[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// Does a Split Horizontal wipe.
void Wipe_SplitH(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int wipe_x;
  int wipe_y;
  int x;
  int y;

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_From[x];
  }
  for (x = 0; x < 78; x++) {
    for (y = 0; y < 80; y++) {
      LCD_Buffer[80 + x + y * 160] = g_To[80 + x + y * 160];
      LCD_Buffer[80 - x + y * 160] = g_To[80 - x + y * 160];

      LCD_Buffer[81 + x + y * 160] = Wipe_Border;
      LCD_Buffer[82 + x + y * 160] = Wipe_Border;
      LCD_Buffer[79 - x + y * 160] = Wipe_Border;
      LCD_Buffer[78 - x + y * 160] = Wipe_Border;
    }

    if (x % g_Rate == 0) {
      M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
      delay(30);
    }
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// Yet another wipe. Just try it so see what it does.
void Wipe_Race(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int x;
  int y;

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_From[x];
  }
  for (x = 0; x < 158; x++) {
    for (y = 0; y < 20; y++) {

      LCD_Buffer[x + y * 160] = g_To[x + y * 160];
      LCD_Buffer[x + (y + 40) * 160] = g_To[x + (y + 40) * 160];
      LCD_Buffer[x + 1 + y * 160] = Wipe_Border;
      LCD_Buffer[x + 2 + y * 160] = Wipe_Border;
      LCD_Buffer[x + 1 + (y + 40) * 160] = Wipe_Border;
      LCD_Buffer[x + 2 + (y + 40) * 160] = Wipe_Border;

      LCD_Buffer[159 - x + (y + 20) * 160] = g_To[159 - x + (y + 20) * 160];
      LCD_Buffer[159 - x + (y + 60) * 160] = g_To[159 - x + (y + 60) * 160];
      LCD_Buffer[159 - x - 1 + (y + 20) * 160] = Wipe_Border;
      LCD_Buffer[159 - x - 2 + (y + 20) * 160] = Wipe_Border;
      LCD_Buffer[159 - x - 1 + (y + 60) * 160] = Wipe_Border;
      LCD_Buffer[159 - x - 2 + (y + 60) * 160] = Wipe_Border;
    }
    if (x % g_Rate == 0) {
      M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
      delay(20);
    }
  }

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// Star Wipe This is more like a media wipe based on a pattern in StarWipeHex.h
void StarWipe(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int cell;
  int x;
  int y;
  int z;
  unsigned int StarByte;
  unsigned int pixel;

  for (cell = 0; cell < 12; cell++) {  //6
    for (y = 0; y < 80; y++) {         //80
      for (x = 0; x < 40; x++) {       //40
        StarByte = StarWipe_bits[cell][y * 40 + x];
        for (z = 0; z < 4; z++) {
          pixel = StarByte >> (6 - z * 2) & 0b00000011;

          switch (pixel) {
            case 0:  //Dissolve
              LCD_Buffer[y * 160 + x * 4 + z] = g_From[y * 160 + x * 4 + z];
              break;
            case 1:  //Dissolve
              LCD_Buffer[y * 160 + x * 4 + z] = g_To[y * 160 + x * 4 + z];
              break;
            case 3:  //Dissolve

              LCD_Buffer[y * 160 + x * 4 + z] = 0xFFFF;
              break;
          }
        }
      }
    }

    M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
    delay(192 / g_Rate);
  }

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// The screen is divided into 8*4 squares and transitioned one by one.
void Checkers(int g_Rate) {
  int x;

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = From_Pointer[x];
  }
  FillSquare(0, 0, g_Rate);
  FillSquare(140, 60, g_Rate);
  FillSquare(20, 20, g_Rate);
  FillSquare(120, 40, g_Rate);
  FillSquare(40, 40, g_Rate);
  FillSquare(100, 20, g_Rate);
  FillSquare(60, 60, g_Rate);
  FillSquare(80, 0, g_Rate);

  FillSquare(0, 60, g_Rate);
  FillSquare(140, 0, g_Rate);
  FillSquare(20, 40, g_Rate);
  FillSquare(120, 20, g_Rate);
  FillSquare(40, 20, g_Rate);
  FillSquare(100, 40, g_Rate);
  FillSquare(60, 0, g_Rate);
  FillSquare(80, 60, g_Rate);

  FillSquare(20, 0, g_Rate);
  FillSquare(100, 0, g_Rate);
  FillSquare(40, 0, g_Rate);
  FillSquare(120, 0, g_Rate);
  FillSquare(60, 20, g_Rate);
  FillSquare(140, 20, g_Rate);
  FillSquare(60, 40, g_Rate);
  FillSquare(140, 40, g_Rate);
  FillSquare(40, 60, g_Rate);
  FillSquare(120, 60, g_Rate);
  FillSquare(20, 60, g_Rate);
  FillSquare(100, 60, g_Rate);
  FillSquare(0, 40, g_Rate);
  FillSquare(80, 40, g_Rate);
  FillSquare(0, 20, g_Rate);
  FillSquare(80, 20, g_Rate);

  LCD_Pointer = LCD_Buffer;

}

// Used by Checkers() above
void FillSquare(int xStart, int yStart, int g_Rate) {
  int x;
  int y;

  for (x = xStart; x < xStart + 20; x++) {
    for (y = yStart; y < yStart + 20; y++) {
      LCD_Buffer[x + y * 160] = Wipe_Border;
    }
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  delay(32 / g_Rate);

  for (x = xStart; x < xStart + 20; x++) {
    for (y = yStart; y < yStart + 20; y++) {
      LCD_Buffer[x + y * 160] = To_Pointer[x + y * 160];
    }
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  delay(32 / g_Rate);
}

// Simple Push Right DVE effect.
void DVE_PushR(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int wipe_x;
  int wipe_y;
  int x;
  int y;

  for (wipe_x = 0; wipe_x <= 160; wipe_x++) {
    for (x = 0; x < wipe_x; x++) {
      for (y = 0; y < 80; y++) {
        LCD_Buffer[x + y * 160] = g_To[x + y * 160];
      }
    }
    for (x = 0; x < 160 - wipe_x; x++) {
      for (y = 0; y < 80; y++) {
        LCD_Buffer[x + wipe_x + y * 160] = g_From[x + y * 160];
      }
    }
    if (wipe_x < 160 - 2) {
      for (x = wipe_x; x <= wipe_x + 3; x++) {
        for (y = 0; y < 80; y++) {
          LCD_Buffer[x + y * 160] = Wipe_Border;
        }
      }
    }
    if (wipe_x % g_Rate == 0) {
      M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
      delay(13);
    }
  }
  LCD_Pointer = LCD_Buffer;
  
}

// It dissolves! This needs to turn each 565 pixel into RGB and back to work.
void Dissolve(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {
  float wipe_x;
  float FF = 0x100;

  int x = 10;
  int y = 20;
  int z;
  int16_t p_From;
  int16_t p_To;
  float p_From_Red;
  float p_From_Green;
  float p_From_Blue;
  float p_To_Red;
  float p_To_Green;
  float p_To_Blue;
  int p_LCD_Red;
  int p_LCD_Green;
  int p_LCD_Blue;
  boolean Done;

  for (wipe_x = 2; wipe_x <= FF; wipe_x = wipe_x + g_Rate) {
    for (x = 0; x < 160; x++) {
      for (y = 0; y < 80; y++) {
        p_From_Red = float((g_From[x + y * 160] & 0b1111100000000000) >> 11);
        p_From_Green = float((g_From[x + y * 160] & 0b0000011111100000) >> 5);
        p_From_Blue = float(g_From[x + y * 160] & 0b0000000000011111);

        p_To_Red = float((g_To[x + y * 160] & 0b1111100000000000) >> 11);
        p_To_Green = float((g_To[x + y * 160] & 0b0000011111100000) >> 5);
        p_To_Blue = float(g_To[x + y * 160] & 0b0000000000011111);

        p_LCD_Red = int(p_From_Red + (p_To_Red - p_From_Red) * wipe_x / FF);
        p_LCD_Green = int(p_From_Green + (p_To_Green - p_From_Green) * wipe_x / FF);
        p_LCD_Blue = int(p_From_Blue + (p_To_Blue - p_From_Blue) * wipe_x / FF);

        LCD_Buffer[x + y * 160] = (p_LCD_Red << 11) | (p_LCD_Green << 5) | p_LCD_Blue;
      }
    }

    M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
    LCD_Pointer = LCD_Buffer;
    
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// This called to size and position an image. It is called by other effect subs that need it.
// Pos_x and Pos_y = 0 is the center of the LCD screen. Size is 0 to 100 (%)  
void DVE(const uint16_t g_From[], const uint16_t g_To[], float Pos_x, float Pos_y, float Size, float Pos_Persp) {

  float x_Pos;
  float y_Pos;
  float Float_x;
  float Float_y;
  float Float_Persp;
  float Adj_Persp;

  int x;
  int y;

  float x_Size;
  float y_Size;

  x_Size = 160 * Size / 100;
  y_Size = 80 * Size / 100;

  if (Size > 75) {
    Pos_Persp = Pos_Persp * (100 - Size) / 25;
  }

  Adj_Persp = Pos_Persp / 100 * Size / 100;

  for (x = 0; x < 12800; x++) {
    PV_Buffer[x] = g_From[x];
  }

  for (x = 0; x < 160; x++) {
    Float_x = x;
    Float_Persp = (80 - x) * Adj_Persp;
    for (y = 0; y < 80; y++) {
      Float_y = y;

      x_Pos = Float_x / 160 * x_Size + x_Size / 2;
      x_Pos = x_Pos + 80 - x_Size + Pos_x;

      y_Pos = Float_y / 80 * y_Size + y_Size / 2;
      y_Pos = y_Pos + (40 - Float_y) / 40 * Float_Persp;
      y_Pos = y_Pos + 40 - y_Size + Pos_y;

      if (!((x_Pos < 0 || x_Pos > 159) || (y_Pos < 0 || y_Pos > 79))) {
        if (((x < 7 || x > 154) || (y < 7 || y > 74)) && (Size < 98 && Size > 2)) {
          PV_Buffer[int(x_Pos) + int(y_Pos) * 160] = Wipe_Border;
        } else {
          PV_Buffer[int(x_Pos) + int(y_Pos) * 160] = g_To[x + y * 160];
        }
      }
    }
  }

}

// If you're old enough to remember the Charlie's Angels TV show, you will know this effect.
void DVE_Angels(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int x;
  int y;
  int not_y;
  int z;
  int wipe_y;

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_From[x];
  }

  for (wipe_y = 0; wipe_y < 80; wipe_y++) {
    not_y = 80 - wipe_y;

    for (x = 0; x < 50; x++) {
      for (y = 0; y < not_y; y++) {
        LCD_Buffer[x + y * 160] = g_From[x + (y + wipe_y) * 160];
      }
      for (y = not_y; y < 80; y++) {
        LCD_Buffer[x + y * 160] = g_To[x + (y - not_y) * 160];
      }
      if (not_y > 3) {
        for (z = not_y - 3; z <= not_y; z++) {
          LCD_Buffer[x + z * 160] = Wipe_Border;
        }
      }
    }

    for (x = 50; x < 110; x++) {
      for (y = 0; y < wipe_y; y++) {
        LCD_Buffer[x + y * 160] = g_To[x + (not_y + y) * 160];
      }
      for (y = wipe_y; y < 80; y++) {
        LCD_Buffer[x + y * 160] = g_From[x + (y - wipe_y) * 160];
      }
      if (wipe_y < 76) {
        for (z = wipe_y; z <= wipe_y + 3; z++) {
          LCD_Buffer[x + z * 160] = Wipe_Border;
        }
      }
    }

    for (x = 110; x < 160; x++) {
      for (y = 0; y < not_y; y++) {
        LCD_Buffer[x + y * 160] = g_From[x + (y + wipe_y) * 160];
      }
      for (y = not_y; y < 80; y++) {
        LCD_Buffer[x + y * 160] = g_To[x + (y - not_y) * 160];
      }

      if (not_y > 3) {
        for (z = not_y - 3; z <= not_y; z++) {
          LCD_Buffer[x + z * 160] = Wipe_Border;
        }
      }
    }

    if (wipe_y % g_Rate == 0) {
      M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
      delay(35);
    }
  }

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }

  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

  
}

// Push 1/4 of the image from left/right and up/down.
// this fails in a pretty way if a DVE key is involved.
void DVE_4Box(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int x;
  int y;
  int Wipe;
  int notWipe;

  for (x = 0; x < 12800; x++) {
    PV_Buffer[x] = g_From[x];
  }

  for (Wipe = 0; Wipe < 80; Wipe++) {
    notWipe = 80 - Wipe;
    for (x = 0; x < Wipe; x++) {  // Upper Left
      for (y = 0; y < 40; y++) {
        PV_Buffer[x + y * 160] = g_To[x + notWipe + y * 160];
      }
    }

    for (x = 80; x < Wipe + 80; x++) {  // Lower Right
      for (y = 40; y < 80; y++) {
        PV_Buffer[x + y * 160] = g_To[x + notWipe + y * 160];
      }
    }


    for (x = 80; x < 160; x++) {  // Upper Right
      for (y = 0; y < Wipe / 2; y++) {
        PV_Buffer[x + y * 160] = g_To[x + (y + notWipe) * 160];
      }
    }

    for (x = 0; x < 80; x++) {  // Lower Left
      for (y = 40; y < Wipe; y++) {
        PV_Buffer[x + y * 160] = g_To[x + (y + notWipe) * 160];
      }
    }

    for (x = 0; x < 12800; x++) {
      LCD_Buffer[x] = PV_Buffer[x];
    }

    if (Wipe % g_Rate == 0) {
      M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
      delay(35);
    }
  }

  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);

}

// Squeeze the image from left and right side. No biggie.
void DVE_SqueezeH(int g_Rate) {
  float Size;
  int x;

  for (Size = 100; Size >= 0; Size = Size - g_Rate) {
    DVE_SqH(To_Pointer, From_Pointer, Size);
    M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Buffer);
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = To_Pointer[x];
  }

  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

}

// Sub used by DVE_SqueezeH()
void DVE_SqH(const uint16_t g_From[], const uint16_t g_To[], float Size) {

  float x_Pos;
  float y_Pos;
  float Float_x;
  float Float_y;
  float Float_Persp;
  float Adj_Persp;

  int x;
  int y;

  float x_Size;
  float y_Size;

  x_Size = 160 * Size / 100;
  y_Size = 80;

  for (x = 0; x < 12800; x++) {
    PV_Buffer[x] = g_From[x];
  }

  for (x = 0; x < 160; x++) {
    Float_x = x;
    for (y = 0; y < 80; y++) {
      Float_y = y;

      x_Pos = Float_x / 160 * x_Size + x_Size / 2;
      x_Pos = x_Pos + 80 - x_Size;

      y_Pos = Float_y / 80 * y_Size + y_Size / 2;
      y_Pos = y_Pos + 40 - y_Size;

      if (x < 4 || x > 156) {
        PV_Buffer[int(x_Pos) + int(y_Pos) * 160] = Wipe_Border;
      } else {
        PV_Buffer[int(x_Pos) + int(y_Pos) * 160] = g_To[x + y * 160];
      }
    }
  }
}

// Mostly standard stuff.
void setup(void) {
  
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);

  M5.begin();

  M5.Lcd.setRotation(3);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setFreeFont(FMB9);
}


void loop(void) {
  // int x;
  // int y;
  char c; 
  bool New_Command = false;

  String R_Command = "";
  int Pos = 0;
  String Command = "";
  String Parameter = "";
  int intParameter;

  // x = StarWipe_bits[3][241];

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Not Connected to: ");
    Serial.println(WiFi.localIP());

    WiFi.begin(ssid, password);  
    WiFi.config(ip, gateway, subnet);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting");
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.println("Connected");
  }
  server.begin();

  WiFiClient client = server.available();

  if (client) {  // Connected to the panel or telnet.
    while (client.connected()) {

      if (client.available()) { // Start reading in a panel command.  
        c = client.read(); 
        Serial.print(c); // Print the characters as we get them.

        if (c == '\r') {
          c = client.read();  //skip the LF(10)
          New_Command = true; // We have something to parse.
          Serial.println();
        } else {
          R_Command += c;
        }

        // Parse the panel message that's in this format COMMAND:parameter (a single number)
        if (New_Command) {
          Pos = R_Command.indexOf(":");

          if (Pos > 0) {
            Command = R_Command.substring(0, Pos);
            Parameter = R_Command.substring(Pos + 1);
            intParameter = Parameter.toInt();

          } else {

            Serial.println("Bad Command");
          }

          // Set up variables based on the command and intParameter.
          // Only a few of these change the LCD screen right away.

          // A button was pressed on the Key bus.
          if (Command == "KEY") {
            KeyBus = intParameter;
            Key_Pointer = GetStill(intParameter);

            // The Key DVE is on-air so update the LCD.
            if (KeyTally == 1) {
              DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
              PV_Pointer = PV_Buffer;
              M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Pointer);
              LCD_Pointer = PV_Pointer;
            }
          }


          if (Command == "BKGD") { // A button was pressed on the Background bus.
            BKGDBus = intParameter;
            BKGD_Pointer = GetStill(intParameter);

            
            if (KeyTally == 0) { // Update the LCD with the just the image
              M5.Lcd.drawBitmap(0, 0, 160, 80, BKGD_Pointer);
              LCD_Pointer = BKGD_Pointer;

            } else { // Update the LCD with the image and the Key DVE.
              DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
              PV_Pointer = PV_Buffer;
              M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Pointer);
              LCD_Pointer = PV_Pointer;
            }
          }

          if (Command == "PST") { // Just update the PSTbus for future use.
            PSTBus = intParameter;
            PST_Pointer = GetStill(intParameter);
          }

          if (Command == "MEAUTO") { // Do the transition with all the parameters recieved so far.
            SetTransition(TransIncl, KeyTally);

            DoTransition(intParameter);
          }

          if (Command == "MECUT") { // Swap the BKGD and PST buffers and display the new program image.
            SetTransition(TransIncl, KeyTally);
            M5.Lcd.drawBitmap(0, 0, 160, 80, To_Pointer);
            LCD_Pointer = To_Pointer;

            if (TransIncl == 1 || TransIncl == 3) {
              Temp_Pointer = BKGD_Pointer;
              BKGD_Pointer = PST_Pointer;
              PST_Pointer = Temp_Pointer;
            }
          }

          if (Command == "PGM_RECALL") { // Copy all then Mem_ commands/parameters recieved into current parameters 
            Key_Pointer = GetStill(Mem_KeyBus);
            BKGD_Pointer = GetStill(Mem_BKGDBus);
            PST_Pointer = GetStill(Mem_PSTBus);
            TransIncl = Mem_TransIncl;
            KeyTally = Mem_KeyTally;
            TransType = Mem_TransType;
            TransDVE = Mem_TransDVE;
            TransWipe = Mem_TransWipe;
            TransDir = Mem_TransDir;
            DVESize = Mem_DVESize;
            DVExPos = Mem_DVExPos;
            DVEyPos = Mem_DVEyPos;
            DVEPersp = Mem_DVEPersp;

            if (KeyTally == 0) {  // Cut with memory parameters.
              From_Pointer = PV_Buffer;
              To_Pointer = BKGD_Pointer;
              
            } else { // Cut and add the DVE Key. 
              DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
              From_Pointer = BKGD_Pointer;
              To_Pointer = PV_Buffer;
            }

            M5.Lcd.drawBitmap(0, 0, 160, 80, To_Pointer);
            LCD_Pointer = To_Pointer;
            KeyBus = Mem_KeyBus;
            BKGDBus = Mem_BKGDBus;
            PSTBus = Mem_PSTBus;
          }

          // This why we use Mem_ parameters. 
          // We need to "tween" between the existing and the Mem_ values. 
          if (Command == "EFF_RECALL") {

            if (Mem_KeyBus == KeyBus && Mem_BKGDBus == BKGDBus && Mem_KeyTally == 1 && KeyTally == 1) {
              Tween_DVE(intParameter);

            } else {
              Key_Pointer = GetStill(Mem_KeyBus);
              PST_Pointer = GetStill(Mem_BKGDBus);
              KeyTally = Mem_KeyTally;
              TransType = Mem_TransType;
              TransWipe = Mem_TransWipe;
              TransDVE = Mem_TransDVE;
              DVESize = Mem_DVESize;
              DVExPos = Mem_DVExPos;
              DVEyPos = Mem_DVEyPos;
              DVEPersp = Mem_DVEPersp;

              if (KeyTally == 0) {
                From_Pointer = LCD_Pointer;
                To_Pointer = PST_Pointer;

              } else {
                DVE(PST_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
                From_Pointer = LCD_Pointer;
                To_Pointer = PV_Buffer;
              }
              DoTransition(intParameter);
            }
            KeyBus = Mem_KeyBus;
            BKGDBus = Mem_BKGDBus;
            PSTBus = Mem_PSTBus;
          }

          // All of these ifs just copy revieved parameters into variables. Yawn...
          if (Command == "TRANSINCL") {
            TransIncl = intParameter;
          }
          if (Command == "KEYTALLY") {
            KeyTally = intParameter;
          }
          if (Command == "TRANSTYPE") {
            TransType = intParameter;
          }
          if (Command == "TRANSDVE") {
            TransDVE = intParameter;
          }
          if (Command == "TRANSWIPE") {
            TransWipe = intParameter;
          }
          if (Command == "TRANSDIR") {
            TransDir = intParameter;
          }
          if (Command == "DVESIZE") {
            DVESize = float(intParameter);
          }
          if (Command == "DVEXPOS") {
            DVExPos = float(intParameter);
          }
          if (Command == "DVEYPOS") {
            DVEyPos = float(intParameter);
          }
          if (Command == "DVEPERSP") {
            DVEPersp = float(intParameter);
          }

          // Memory recall parameters
          if (Command == "Mem_KEY") {
            Mem_KeyBus = intParameter;
          }
          if (Command == "Mem_BKGD") {
            Mem_BKGDBus = intParameter;
          }
          if (Command == "Mem_PST") {
            Mem_PSTBus = intParameter;
          }
          if (Command == "Mem_TRANSINCL") {
            Mem_TransIncl = intParameter;
          }
          if (Command == "Mem_KEYTALLY") {
            Mem_KeyTally = intParameter;
          }
          if (Command == "Mem_TRANSTYPE") {
            Mem_TransType = intParameter;
          }
          if (Command == "Mem_TRANSDVE") {
            Mem_TransDVE = intParameter;
          }
          if (Command == "Mem_TRANSWIPE") {
            Mem_TransWipe = intParameter;
          }
          if (Command == "Mem_TRANSDIR") {
            Mem_TransDir = intParameter;
          }
          if (Command == "Mem_DVESIZE") {
            Mem_DVESize = float(intParameter);
          }
          if (Command == "Mem_DVEXPOS") {
            Mem_DVExPos = float(intParameter);
          }
          if (Command == "Mem_DVEYPOS") {
            Mem_DVEyPos = float(intParameter);
          }
          if (Command == "Mem_DVEPERSP") {
            Mem_DVEPersp = float(intParameter);
          }

          // Clear this message and get ready for the next one.
          R_Command = ""; 
          New_Command = false;
        }
      }
    }

    // Take a break you deserve it.
    delay(100);

  // Can't connect to the panel (or Telnet) so display that on the M5StickC Screen.
  } else {
    // M5.Lcd.fillScreen(TFT_DARKCYAN);
    M5.Lcd.drawString("Not connected", 5, 25, GFXFF);
    M5.Lcd.drawString("to panel", 5, 40, GFXFF);
  }

  // You don't deserve this break but you get it anyway.
  delay(200);
}
