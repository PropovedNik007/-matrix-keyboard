/*
 * lcd.h
 *
 *  Created on: 27 но€б. 2017 г.
 *      Author: Dmitry
 */

#ifndef LCD_H_
#define LCD_H_

void init_lcd(void);
void LCDchar(int ch, int pos);
void LCDdigit(uint16_t val, int pos);
void LCDdec(uint16_t val, int pos);


#endif /* LCD_H_ */
