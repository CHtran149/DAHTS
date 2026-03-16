#ifndef __OLED_LCD_SSD1306_H
#define __OLED_LCD_SSD1306_H 

#include <stdint.h>
#include <string.h>
#include "fonts.h"
#include <neorv32.h>
#include <neorv32_twi.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_DEACTIVATE_SCROLL 0x2E // Stop scroll
#define SSD1306_I2C_ADDR 0x3C // Địa chỉ 7-bit chuẩn (0x78 dịch phải 1 bit)

typedef enum { SSD1306_COLOR_BLACK = 0, SSD1306_COLOR_WHITE = 1 } SSD1306_COLOR_t;

typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
    uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
} SSD1306_Name;

void SSD1306_UpdateScreen(SSD1306_Name* SSD1306);
uint8_t SSD1306_Init(SSD1306_Name* SSD1306);
void SSD1306_Fill(SSD1306_Name* SSD1306, SSD1306_COLOR_t color);
void SSD1306_DrawPixel(SSD1306_Name* SSD1306, uint16_t x, uint16_t y, SSD1306_COLOR_t color);
void SSD1306_GotoXY(SSD1306_Name* SSD1306, uint16_t x, uint16_t y);
char SSD1306_Putc(SSD1306_Name* SSD1306, char ch, FontDef_t* Font, SSD1306_COLOR_t color);
char SSD1306_Puts(SSD1306_Name* SSD1306, char* str, FontDef_t* Font, SSD1306_COLOR_t color);
void SSD1306_Clear(SSD1306_Name* SSD1306);
void SSD1306_ON(SSD1306_Name* SSD1306);
void SSD1306_OFF(SSD1306_Name* SSD1306);

#endif
