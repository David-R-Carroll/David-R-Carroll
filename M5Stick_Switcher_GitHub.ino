
/*
################################################################################
#                                                                              #
#                  M5StickC Production Switcher Simulator.                     #
#                    Badly written by David R. Carroll.                        #
#                      Creative Commons CC BY-NC 2023                          #
#                        This is the Program Monitor sim.                      #
#                          It recieves text commamnds from the panel.          #
#                                                                              #
################################################################################   

#include <M5StickC.h>

#include "Free_Fonts.h"

These are the 160 x 80 images
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

This file defines a star shape.
#include "StarWipeHex.h"

#include <WiFi.h>
IPAddress ip(192, 168, 2, 240);
IPAddress gateway(192, 168, 2, 0);
IPAddress subnet(255, 255, 0, 0);

const char *ssid = "Your WiFi SSID";
const char *password = "Your Password";

WiFiServer server(21);

uint16_t LCD_Buffer[12800];
const uint16_t *LCD_Pointer;

const uint16_t *Key_Pointer;

uint16_t BKGD_Buffer[12800];
const uint16_t *BKGD_Pointer;

uint16_t PV_Buffer[12800];
const uint16_t *PV_Pointer;

const uint16_t *PST_Pointer;
const uint16_t *Temp_Pointer;
const uint16_t *From_Pointer;
const uint16_t *To_Pointer;

int16_t Wipe_Border = WHITE;

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

void DoTransition(int gRate) {
  switch (TransType) {  // Auto Trans
    case 0:             //Dissolve
      Dissolve(From_Pointer, To_Pointer, gRate);
      break;
    case 1:  // Wipe
      if (TransWipe == 1) {
        Wipe_SV(From_Pointer, To_Pointer, gRate);
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

void DVE_Square(int g_Rate) {
  float Size;
  int x;
  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                DVE_Square Duration=");
  // Serial.println(millis() - myTime);
}

void Tween_DVE(int g_Rate) {
  // Serial.print("Tween_DVE" );

  int x;
  float Tw_DVESize;
  float Tw_DVExPos;
  float Tw_DVEyPos;
  float Tw_DVEPersp;
  float Interval;
  Interval = float(128 / g_Rate);

  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                Tween_DVE Duration=");
  // Serial.println(millis() - myTime);
}

void Wipe_SV(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int wipe_x;
  int wipe_y;
  int x;
  int y;
  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                Wipe_SV Duration=");
  // Serial.println(millis() - myTime);
}

void Wipe_Race(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int x;
  int y;

  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                Wipe_Race Duration=");
  // Serial.println(millis() - myTime);
}

void StarWipe(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int cell;
  int x;
  int y;
  int z;
  unsigned int StarByte;
  unsigned int pixel;

  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                StarWipe Duration=");
  // Serial.println(millis() - myTime);
}

void Checkers(int g_Rate) {
  int x;
  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                Checkers Duration=");
  // Serial.println(millis() - myTime);
}

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

void DVE_PushR(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int wipe_x;
  int wipe_y;
  int x;
  int y;
  unsigned long myTime;
  myTime = millis();

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
  // Serial.print("                DVE_PushR Duration=");
  // Serial.println(millis() - myTime);
}

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

  unsigned long myTime;
  myTime = millis();

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

    // delay(g_Rate);
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = g_To[x];
  }
  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

  // Serial.print("                Dissolve Duration=");
  // Serial.println(millis() - myTime);
}

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

  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                DVE Duration=");
  // Serial.println(millis() - myTime);
}

void DVE_Angels(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  int x;
  int y;
  int not_y;
  int z;
  int wipe_y;

  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                DVE_Angels Duration=");
  // Serial.println(millis() - myTime);
}

void DVE_4Box(const uint16_t g_From[], const uint16_t g_To[], int g_Rate) {

  // void DVE(const uint16_t g_From[], const uint16_t g_To[], float Pos_x, float Pos_y, float Size, float Pos_Persp) {


  int x;
  int y;
  int Wipe;
  int notWipe;

  unsigned long myTime;
  myTime = millis();

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

  // Serial.print("                DVE_4Box Duration=");
  // Serial.println(millis() - myTime);
}

void DVE_SqueezeH(int g_Rate) {
  float Size;
  int x;
  unsigned long myTime;
  myTime = millis();

  for (Size = 100; Size >= 0; Size = Size - g_Rate) {
    DVE_SqH(To_Pointer, From_Pointer, Size);
    M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Buffer);
  }
  for (x = 0; x < 12800; x++) {
    LCD_Buffer[x] = To_Pointer[x];
  }

  M5.Lcd.drawBitmap(0, 0, 160, 80, LCD_Buffer);
  LCD_Pointer = LCD_Buffer;

  // Serial.print("                DVE_SqueezeH Duration=");
  // Serial.println(millis() - myTime);
}


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

void setup(void) {
  // Start serial communication for debugging purposes
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);

  M5.begin();

  M5.Lcd.setRotation(3);
  M5.Lcd.setTextColor(TFT_WHITE);
  //dtostrf("Dave TV", 4 , 1, chBuffer);
  M5.Lcd.setFreeFont(FMB9);
}

void loop(void) {
  int x;
  int y;
  int incomingByte = 0;
  char c;
  //String Line = "";
  bool New_Command = false;

  String R_Command = "";
  int Pos = 0;
  String Command = "";
  String Parameter = "";
  int intParameter;

  x = StarWipe_bits[3][241];

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

  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {  // if you get a client,
    while (client.connected()) {
      if (client.available()) {  // if there's bytes to read from the client,
        c = client.read();
        Serial.print(c);

        if (c == '\r') {
          c = client.read();  //read the LF(10)
          New_Command = true;
          Serial.println();
        } else {
          R_Command += c;
        }

        if (New_Command) {
          // Serial.println();

          Pos = R_Command.indexOf(":");
          if (Pos > 0) {
            Command = R_Command.substring(0, Pos);
            Parameter = R_Command.substring(Pos + 1);
            intParameter = Parameter.toInt();
          } else {
            Serial.println("Bad Command");
          }

          if (Command == "KEY") {
            KeyBus = intParameter;
            Key_Pointer = GetStill(intParameter);
            if (KeyTally == 1) {
              // Serial.print("DVExPos=");
              // Serial.println(DVExPos);
              DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
              PV_Pointer = PV_Buffer;
              M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Pointer);
              LCD_Pointer = PV_Pointer;
            }
          }

          if (Command == "BKGD") {
            BKGDBus = intParameter;
            BKGD_Pointer = GetStill(intParameter);
            if (KeyTally == 0) {
              M5.Lcd.drawBitmap(0, 0, 160, 80, BKGD_Pointer);
              LCD_Pointer = BKGD_Pointer;
            } else {
              DVE(BKGD_Pointer, Key_Pointer, DVExPos, DVEyPos, DVESize, DVEPersp);
              PV_Pointer = PV_Buffer;
              M5.Lcd.drawBitmap(0, 0, 160, 80, PV_Pointer);
              LCD_Pointer = PV_Pointer;
            }
          }

          if (Command == "PST") {
            PSTBus = intParameter;
            PST_Pointer = GetStill(intParameter);
          }

          if (Command == "MEAUTO") {
            SetTransition(TransIncl, KeyTally);

            DoTransition(intParameter);
          }

          if (Command == "MECUT") {
            SetTransition(TransIncl, KeyTally);
            M5.Lcd.drawBitmap(0, 0, 160, 80, To_Pointer);
            LCD_Pointer = To_Pointer;
            if (TransIncl == 1 || TransIncl == 3) {
              Temp_Pointer = BKGD_Pointer;
              BKGD_Pointer = PST_Pointer;
              PST_Pointer = Temp_Pointer;
            }
          }

          if (Command == "PGM_RECALL") {
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

            if (KeyTally == 0) {
              From_Pointer = PV_Buffer;
              To_Pointer = BKGD_Pointer;
            } else {
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

          /*
                  Serial.print("DVExPos=");
                  Serial.print(DVExPos);
                  Serial.print(", DVEyPos=");
                  Serial.print(DVEyPos);
                  Serial.print(", DVESIZE=");
                  Serial.println(DVESize);
               */

          R_Command = "";
          New_Command = false;
        }
      }
    }

    delay(100);

  } else {
    // M5.Lcd.fillScreen(TFT_DARKCYAN);
    M5.Lcd.drawString("Not connected", 5, 25, GFXFF);
    M5.Lcd.drawString("to panel", 5, 40, GFXFF);
  }

  delay(200);
}
