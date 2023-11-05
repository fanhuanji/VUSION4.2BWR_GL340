#ifndef _EPD_H_
#define _EPD_H_

#include <stdint.h>

#define EPD_B_PWR 0   //P0_0
#define EPD_B_CS 1    //P0_1
#define EPD_B_DC 2    //P1_2 - low command, high data
#define EPD_B_BUSY 3  //P1_3 - low busy
#define EPD_B_RESET 0 //P2_0 - low reset

#define EPD_PWR P0_0
#define EPD_CS P0_1
#define EPD_DC P1_2
#define EPD_BUSY P1_3
#define EPD_RESET P2_0


void Epd_Init(void);
void Epd_ClearDisplay(uint8_t black, uint8_t red);
void Epd_Refresh(void);
void Epd_Deinit(void);


void Epd_TestPattern_ChessBoard(uint8_t pat);

void Epd_BeginBlack(void);
void Epd_BeginRed(void);
void Epd_LoadBatch(const uint8_t *buf);


#endif