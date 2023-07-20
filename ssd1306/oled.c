/*
 * oled.c
 *
 *  Created on: 13 Haz 2023
 *      Author: pickle
 */

#include "oled.h"

static uint8_t Buffer[Screen_Height*Screen_Width/8];

extern I2C_HandleTypeDef hi2c1;

void DEV_INIT(void){

	while (HAL_I2C_IsDeviceReady(&hi2c1, DevAddr, 1, 20000) != HAL_OK) {	//wait until device is okay
		;
	}

	WRITECOMMAND(0xAE); //display off
	WRITECOMMAND(0x20); //Set Memory Addressing Mode
	WRITECOMMAND(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WRITECOMMAND(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	WRITECOMMAND(0xC8); //Set COM Output Scan Direction
	WRITECOMMAND(0x00); //---set low column address
	WRITECOMMAND(0x10); //---set high column address
	WRITECOMMAND(0x40); //--set start line address
	WRITECOMMAND(0x81); //--set contrast control register
	WRITECOMMAND(0xFF);
	WRITECOMMAND(0xA1); //--set segment re-map 0 to 127
	WRITECOMMAND(0xA6); //--set normal display
	WRITECOMMAND(0xA8); //--set multiplex ratio(1 to 64)
	WRITECOMMAND(0x3F); //
	WRITECOMMAND(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WRITECOMMAND(0xD3); //-set display offset
	WRITECOMMAND(0x00); //-not offset
	WRITECOMMAND(0xD5); //--set display clock divide ratio/oscillator frequency
	WRITECOMMAND(0xF0); //--set divide ratio
	WRITECOMMAND(0xD9); //--set pre-charge period
	WRITECOMMAND(0x22); //
	WRITECOMMAND(0xDA); //--set com pins hardware configuration
	WRITECOMMAND(0x12);
	WRITECOMMAND(0xDB); //--set vcomh
	WRITECOMMAND(0x20); //0x20,0.77xVcc
	WRITECOMMAND(0x8D); //--set DC-DC enable
	WRITECOMMAND(0x14); //
	WRITECOMMAND(0xAF); //--turn on SSD1306 panel
}

void BUFFER_RESET(void){
	for(uint16_t i=0; i<Screen_Height*Screen_Width/8; i++){
		Buffer[i]&=0;
	}
}

void WRITECOMMAND(uint8_t data) {
	uint8_t dt[2];
	dt[0] = 0x00;
	dt[1] = data;
	HAL_I2C_Master_Transmit(&hi2c1, DevAddr, dt, 2, 10);
}

void SCREEN_WRITE(uint8_t *data){
	uint8_t dt[256];
	dt[0] = 0x40;
	for(int i = 0; i < Screen_Width; i++)
	dt[i+3] = data[i];
	HAL_I2C_Master_Transmit(&hi2c1, DevAddr, dt, Screen_Width+1, 10);
}

void SCREEN_UPDATE(void){
	for(int i=0; i<8; i++){
		WRITECOMMAND(0xB0 + i);
		WRITECOMMAND(0x10);
		SCREEN_WRITE(&Buffer[Screen_Width*i]);
	}
}

void DRAW_PIXEL(uint8_t x, uint8_t y){
	Buffer[x+(y/8)*Screen_Width]|=1<<(y%8);
}

void DRAW_LINE_HORIZONTAL(uint8_t y, uint8_t a, uint8_t b){
	if(a<0)
		a=0;
	else if(a>Screen_Width)
		a=Screen_Width;
	if(b>Screen_Width)
		b=Screen_Width;
	else if(b<0)
		b=0;
	if (b >= a) {
		for (uint8_t i = a; i < b + 1; i++) {
			Buffer[Screen_Width * (y / 8) + i] |= 1 << (y % 8);
		}
	}
}

void DRAW_LINE_VERTICAL(uint8_t x, uint8_t a, uint8_t b){
	Buffer[(a/8)*Screen_Width+x]|=(~0)<<(a%8);
	for(uint8_t i=(a/8)+1; i<(b/8); i++){
		Buffer[x+Screen_Width*i]|=~0;
	}
	Buffer[(b/8)*Screen_Width+x]|=~((~1)<<((b)%8));
}

/*void PUT_CHAR(char *ch, uint8_t x, uint8_t y){
	for(int i=0; i<5; i++){
		Buffer[x+(y/8)*Screen_Width+i]|=myletter_a[i]<<(y%8);
		Buffer[x+(y/8)*Screen_Width+i+Screen_Width]|=myletter_a[i]>>(8-(y%8));
	}
}*/

void INVERT_COLOR(void){
	for(uint16_t i=0; i<Screen_Height*Screen_Width/8; i++){
		Buffer[i]=~Buffer[i];
	}
}

void MODE_FRAME(void){
	  DRAW_LINE_HORIZONTAL(0, 5, 122);
	  DRAW_LINE_HORIZONTAL(63, 5, 122);
	  DRAW_LINE_VERTICAL(0, 5, 58);
	  DRAW_LINE_VERTICAL(127, 5, 58);

	  DRAW_LINE_HORIZONTAL(2, 7, 120);
	  DRAW_LINE_HORIZONTAL(61, 7, 120);
	  DRAW_LINE_VERTICAL(2, 7, 56);
	  DRAW_LINE_VERTICAL(125, 7, 56);

	  DRAW_LINE_HORIZONTAL(4, 9, 118);
	  DRAW_LINE_HORIZONTAL(59, 9, 118);
	  DRAW_LINE_VERTICAL(4, 9, 54);
	  DRAW_LINE_VERTICAL(123, 9, 54);
}

void PUT_CHAR(char ch, FontDef_t* Font, uint8_t x, uint8_t y) {
	uint32_t i, b, j;

	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				DRAW_PIXEL(x+j, y+i);
			}
		}
	}
}

void PUT_STR(char* str, FontDef_t* Font, uint8_t x, uint8_t y) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		PUT_CHAR(*str, Font, x, y);
		str++;
		x+=Font->FontWidth;
	}
}








