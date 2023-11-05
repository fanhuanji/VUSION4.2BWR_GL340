
#ifndef FLASH_H
#define FLASH_H
#include <stdbool.h>
#include <stdint.h>
#include <cc2510fx.h>
#include "util.h"

void NfcFlash_IoInit(void);
void NfcFlash_Pwr(bool enable);

bool Flash_CheckId(void);
bool Flash_ReleasePowerDown(void);
void Flash_FastRead(uint32_t addr, uint8_t *buf, uint16_t len);


#define NFC_FD P1_1
#define NFCFLASH_PWR P1_0
#define FLASH_CS P1_4
#define FLASH_MOSI P1_6
#define FLASH_MISO P1_7
#define FLASH_SCK P1_5

#define NFC_B_FD 1
#define NFCFLASH_B_PWR 0
#define FLASH_B_CS 4
#define FLASH_B_MOSI 6
#define FLASH_B_MISO 7
#define FLASH_B_SCK 5

#define NFC_FD_ON() (!NFC_FD)
#define NFC_FD_OFF() (!!NFC_FD)

#endif // FLASH_H
