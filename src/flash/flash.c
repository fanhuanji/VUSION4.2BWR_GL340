

#include "flash.h"
#include <cc2510fx.h>
#include <stdbool.h>




inline void Flash_Select(bool select)
{
    FLASH_CS = !select;
}

static uint8_t Flash_ExchangeData(uint8_t data)
{
    U1DBUF = data;
    while (U1CSR & 0x01)
    {
        *(__xdata volatile uint8_t*)0xFE00 = U1CSR;
    }
    return U1DBUF;
}

bool Flash_CheckId(void)
{
    Flash_Select(true);
    Flash_ExchangeData(0x90u);
    Flash_ExchangeData(0x00u); // 3 Byte dummy
    Flash_ExchangeData(0x00u);
    Flash_ExchangeData(0x00u);
    uint8_t mid = Flash_ExchangeData(0xFFu); // 0xEF
    uint8_t did = Flash_ExchangeData(0xFFu); // 0x11
    Flash_Select(false);
    return (mid == 0xEFu) && (did == 0x11u);
}

void Flash_EnterPowerDown(void)
{
    Flash_Select(true);
    Flash_ExchangeData(0xB9u);
    Flash_Select(false);
}

/**
 * @brief 释放复位并判断ChipId
 * 
 * @return true 
 * @return false 
 */
bool Flash_ReleasePowerDown(void)
{
    Flash_Select(true);
    Flash_ExchangeData(0xABu);
    Flash_ExchangeData(0x00u); // 3 Byte dummy
    Flash_ExchangeData(0x00u);
    Flash_ExchangeData(0x00u);
    uint8_t did = Flash_ExchangeData(0xFFu); 
    Flash_Select(false);
    return (did == 0x11u);
}

/**
 * @brief 
 * 
 * @param addr 
 * @param len 只有16位宽，内存受限
 */
void Flash_FastRead(uint32_t addr, uint8_t *buf, uint16_t len)
{
    Flash_Select(true);
    Flash_ExchangeData(0x0Bu);
    Flash_ExchangeData((addr & 0xFF0000u) >> 16u);
    Flash_ExchangeData((addr & 0xFF00u) >> 8u);
    Flash_ExchangeData((addr & 0xFFu) >> 0u);
    Flash_ExchangeData(0xFFu); // dummy
    for (uint16_t i = 0; i < len; ++i) {
        buf[i] = Flash_ExchangeData(i & 0xFFu);
    }
    Flash_Select(false);
}

void NfcFlash_IoInit(void)
{
    P1DIR |= BV(FLASH_B_CS);
    FLASH_CS = 1; // Flas CS
    NFCFLASH_PWR = 0;
    P1DIR |= BV(NFCFLASH_B_PWR) // NFCPWR output
        | BV(FLASH_B_SCK) 
        | BV(FLASH_B_MOSI) 
        | BV(FLASH_B_CS); 
    P1DIR &= ~(BV(FLASH_B_MISO));
    PERCFG |= 0x02;       // USART1 alternative 2 location SPI
    U1CSR = 0;             // SPI mode/master/clear flags
    P1SEL |= BV(FLASH_B_SCK) | BV(FLASH_B_MOSI) | BV(FLASH_B_MISO);                        // MISO/MOSI/CLK peripheral functions // SS仅当从机使用时才配置为外设
    U1BAUD = 0;            // baud M
    U1GCR = BV(5) | 17; // SCK-low idle, DATA-1st clock edge, MSB first

    P1DIR &= ~BV(NFC_B_FD);
}

void NfcFlash_Pwr(bool enable)
{
    P1_0 = enable ? 1 : 0;
}

