#include "OLED_LCD_SSD1306.h"

static void SSD1306_WriteCmd(SSD1306_Name* SSD1306, uint8_t Cmd) {
    uint8_t data;
    neorv32_twi_generate_start();
    
    data = (SSD1306_I2C_ADDR << 1);
    neorv32_twi_transfer(&data, 0); // 0 = Không cần MACK sau địa chỉ
    
    data = 0x00;
    neorv32_twi_transfer(&data, 0);
    
    data = Cmd;
    neorv32_twi_transfer(&data, 0);
    
    neorv32_twi_generate_stop();
}


void SSD1306_UpdateScreen(SSD1306_Name* SSD1306) {
    uint8_t temp;
    for (uint8_t m = 0; m < 8; m++) {
        SSD1306_WriteCmd(SSD1306, 0xB0 + m);
        SSD1306_WriteCmd(SSD1306, 0x00);
        SSD1306_WriteCmd(SSD1306, 0x10);

        neorv32_twi_generate_start();
        
        temp = (SSD1306_I2C_ADDR << 1);
        neorv32_twi_transfer(&temp, 1);
        
        temp = 0x40; // Data mode
        neorv32_twi_transfer(&temp, 1);
        
        for(uint16_t i = 0; i < SSD1306_WIDTH; i++) {
            temp = SSD1306->SSD1306_Buffer[SSD1306_WIDTH * m + i];
            neorv32_twi_transfer(&temp, 1);
        }
        neorv32_twi_generate_stop();
    }
}
uint8_t SSD1306_Init(SSD1306_Name* SSD1306) {
    /* A little delay */
	uint32_t p = 2500;
	while(p>0)
		p--;
	
	/* Init LCD */
	SSD1306_WriteCmd(SSD1306,0xAE); //display off
	SSD1306_WriteCmd(SSD1306,0x20); //Set Memory Addressing Mode   
	SSD1306_WriteCmd(SSD1306,0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	SSD1306_WriteCmd(SSD1306,0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	SSD1306_WriteCmd(SSD1306,0xC8); //Set COM Output Scan Direction
	SSD1306_WriteCmd(SSD1306,0x00); //---set low column address
	SSD1306_WriteCmd(SSD1306,0x10); //---set high column address
	SSD1306_WriteCmd(SSD1306,0x40); //--set start line address
	SSD1306_WriteCmd(SSD1306,0x81); //--set contrast control register
	SSD1306_WriteCmd(SSD1306,0xFF);
	SSD1306_WriteCmd(SSD1306,0xA1); //--set segment re-map 0 to 127
	SSD1306_WriteCmd(SSD1306,0xA6); //--set normal display
	SSD1306_WriteCmd(SSD1306,0xA8); //--set multiplex ratio(1 to 64)
	SSD1306_WriteCmd(SSD1306,0x3F); //
	SSD1306_WriteCmd(SSD1306,0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	SSD1306_WriteCmd(SSD1306,0xD3); //-set display offset
	SSD1306_WriteCmd(SSD1306,0x00); //-not offset
	SSD1306_WriteCmd(SSD1306,0xD5); //--set display clock divide ratio/oscillator frequency
	SSD1306_WriteCmd(SSD1306,0xF0); //--set divide ratio
	SSD1306_WriteCmd(SSD1306,0xD9); //--set pre-charge period
	SSD1306_WriteCmd(SSD1306,0x22); //
	SSD1306_WriteCmd(SSD1306,0xDA); //--set com pins hardware configuration
	SSD1306_WriteCmd(SSD1306,0x12);
	SSD1306_WriteCmd(SSD1306,0xDB); //--set vcomh
	SSD1306_WriteCmd(SSD1306,0x20); //0x20,0.77xVcc
	SSD1306_WriteCmd(SSD1306,0x8D); //--set DC-DC enable
	SSD1306_WriteCmd(SSD1306,0x14); //
	SSD1306_WriteCmd(SSD1306,0xAF); //--turn on SSD1306 panel
	

	SSD1306_WriteCmd(SSD1306,SSD1306_DEACTIVATE_SCROLL);

	/* Clear screen */
	SSD1306_Fill(SSD1306, SSD1306_COLOR_BLACK);
	
	/* Update screen */
	SSD1306_UpdateScreen(SSD1306);
	
	/* Set default values */
	SSD1306->CurrentX = 0;
	SSD1306->CurrentY = 0;
	
	/* Initialized OK */
	SSD1306->Initialized = 1;
    return 1;
}

void SSD1306_Fill(SSD1306_Name* SSD1306, SSD1306_COLOR_t color) 
{
	memset(SSD1306->SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306->SSD1306_Buffer));
}

void SSD1306_DrawPixel(SSD1306_Name* SSD1306, uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
	if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) 
	{
		return;
	}
	if (SSD1306->Inverted) 
	{
		color = (SSD1306_COLOR_t)!color;
	}
	if (color == SSD1306_COLOR_WHITE) 
	{
		SSD1306->SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} 
	else 
	{
		SSD1306->SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}


void SSD1306_GotoXY(SSD1306_Name* SSD1306, uint16_t x, uint16_t y)
{
	SSD1306->CurrentX = x+2;
	SSD1306->CurrentY = y;
}

char SSD1306_Putc(SSD1306_Name* SSD1306, char ch, FontDef_t* Font, SSD1306_COLOR_t color) 
{
	uint32_t i, b, j;
	if(SSD1306_WIDTH <= (SSD1306->CurrentX + Font->FontWidth) || SSD1306_HEIGHT <= (SSD1306->CurrentY + Font->FontHeight)) 
	{
		return 0;
	}
	for (i = 0; i < Font->FontHeight; i++) 
	{
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) 
		{
			if ((b << j) & 0x8000) 
			{
				SSD1306_DrawPixel(SSD1306, SSD1306->CurrentX + j, (SSD1306->CurrentY + i), (SSD1306_COLOR_t) color);
			} 
			else 
			{
				SSD1306_DrawPixel(SSD1306, SSD1306->CurrentX + j, (SSD1306->CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	SSD1306->CurrentX += Font->FontWidth;
	return ch;
}

char SSD1306_Puts(SSD1306_Name* SSD1306, char* str, FontDef_t* Font, SSD1306_COLOR_t color) {
	while (*str) {
		if (SSD1306_Putc(SSD1306, *str, Font, color) != *str) {
			return *str;
		}
		str++;
	}
	return *str;
}

void SSD1306_Clear(SSD1306_Name* SSD1306)
{
	SSD1306_Fill(SSD1306, SSD1306_COLOR_BLACK);
  SSD1306_UpdateScreen(SSD1306);
}
void SSD1306_ON(SSD1306_Name* SSD1306) 
{
	SSD1306_WriteCmd(SSD1306,0x8D);  
	SSD1306_WriteCmd(SSD1306,0x14);  
	SSD1306_WriteCmd(SSD1306,0xAF);  
}
void SSD1306_OFF(SSD1306_Name* SSD1306) 
{
	SSD1306_WriteCmd(SSD1306,0x8D);  
	SSD1306_WriteCmd(SSD1306,0x10);
	SSD1306_WriteCmd(SSD1306,0xAE);  
}

