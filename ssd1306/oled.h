/*
 * oled.h
 *
 *  Created on: 13 Haz 2023
 *      Author: pickle
 */

#ifndef SSD1306_OLED_H_
#define SSD1306_OLED_H_

#ifndef DevAddr
#define DevAddr 0x78 //if does not works, try 0x7A
#endif

#ifndef Screen_Width
#define Screen_Width 132
#endif

#ifndef Screen_Height
#define Screen_Height 64
#endif



#include "stdlib.h"
#include "string.h"
#include "stm32u5xx_hal.h"
#include "fonts.h"

void WRITECOMMAND(uint8_t);

void SCREEN_WRITE(uint8_t *data);

void SCREEN_UPDATE(void);

void BUFFER_RESET(void);

void DRAW_PIXEL(uint8_t x, uint8_t y);

void DRAW_LINE_HORIZONTAL(uint8_t y, uint8_t a, uint8_t b);

void DRAW_LINE_VERTICAL(uint8_t x, uint8_t a, uint8_t b);

void PUT_CHAR(char ch, FontDef_t* Font, uint8_t x, uint8_t y);

void PUT_STR(char* str, FontDef_t* Font, uint8_t x, uint8_t y);

void DEV_INIT(void);

void INVERT_COLOR(void);

void MODE_FRAME(void);


#endif /* SSD1306_OLED_H_ */
