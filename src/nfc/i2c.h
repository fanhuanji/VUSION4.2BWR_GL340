
#ifndef I2C_H
#define I2C_H
#include <stdbool.h>
#include <stdint.h>
#include <cc2510fx.h>
#include "util.h"

// sbit SDA = P1^0;
// sbit SCL = P1^1;

#define SDA P0_4
#define SCL P0_6
#define SDA_OUT() P0DIR |= BV(4);
#define SDA_IN() P0DIR &= ~BV(4);


void I2C_init(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_ack(void);
void I2C_nack(void);
uint8_t I2C_write(unsigned char dado);
uint8_t I2C_read(void);
// void i2c_device_write(unsigned char slave ,unsigned char reg_add ,unsigned char dado);
// unsigned char i2c_device_read(unsigned char slave, unsigned char reg_add);



#define NFC_R_ADDR 0xAB
#define NFC_W_ADDR 0xAA

#define BBV(x) ((uint8_t)((uint8_t)1u << (uint8_t)(x)))

#define NFC_CONFIGU_REG_MEMA 0x3Au
#define NFC_SESSION_REG_MEMA 0xFEu

#define NFC_CONFIGU_NC_REG_REGA 0x00u
#define NFC_CONFIGU_NC_REG_PTHRU_ON_OFF BBV(6)


#define NFC_SESSION_NC_REG_REGA 0x00u
#define NFC_SESSION_NC_REG_PTHRU_ON_OFF BBV(6)
#define NFC_SESSION_NC_REG_FD_ON_OFF ((uint8_t)0x3cu)
#define NFC_SESSION_NS_REG_REGA 0x06u

#define NFC_SESSION_NS_REG_NDEF_DATA_READ BBV(7)

#define NFC_SESSION_NS_REG_I2C_LOCKED BBV(6)
#define NFC_SESSION_NS_REG_RF_LOCKED BBV(5)

#define NFC_SESSION_NS_REG_SRAM_I2C_READY BBV(4)
#define NFC_SESSION_NS_REG_SRAM_RF_READY BBV(3)
#define NFC_SESSION_NS_REG_RF_FIELD_PRESENT BBV(0)


typedef uint8_t NfcMemoryBlk[16];
bool Nfc_MemWriteBlk(uint8_t mema, const NfcMemoryBlk buf);
bool Nfc_MemReadBlk(uint8_t mema, NfcMemoryBlk buf);
bool Nfc_MemWrite(uint8_t mema, uint8_t *buf);
bool Nfc_MemRead(uint8_t mema, uint8_t *buf);
bool Nfc_WriteReg(uint8_t mema, uint8_t rega, uint8_t mask, uint8_t regdat);
bool Nfc_ReadReg(uint8_t mema, uint8_t rega, uint8_t *regdat);

bool Nfc_FullDump(uint8_t *buf, uint16_t bufLen);
bool Nfc_DirSet(bool nfc2iic);
bool Nfc_I2cLock(bool enable);
bool Nfc_ConfigurePassThru(bool enable);
bool Nfc_IsFieldActive(void);
bool Nfc_SetFdPin(uint8_t on, uint8_t off);

#endif // I2C_H
