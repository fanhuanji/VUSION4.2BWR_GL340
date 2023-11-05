
#include "i2c.h"
#include "time/time.h"

void I2C_HalfClk(void)
{
    __asm__("NOP");
    
    //__asm__("NOP");
    //__asm__("NOP");
    //__asm__("NOP");

    //__asm__("NOP");    
    // __asm__("NOP");
    // __asm__("NOP");
    // __asm__("NOP");
}

void I2C_init(void)
{
    P0DIR |= BV(4) | BV(6);
    SDA = 1;
    SCL = 1;
}

void I2C_start(void)
{
    I2C_HalfClk();
    SDA_OUT();
    SDA = 0;
    I2C_HalfClk();
    SCL = 0;
    I2C_HalfClk();
}

void I2C_stop(void)
{
    SDA_OUT();
    SDA = 0;
    SCL = 1;
    I2C_HalfClk();
    SDA = 1;
    I2C_HalfClk();
}

void I2C_ack(void)
{
    SCL = 0;
    SDA_OUT();
    SDA = 0;
    I2C_HalfClk();
    SCL = 1;
    I2C_HalfClk();
    SCL = 0;
}

void I2C_nack(void)
{
    SCL = 0;
    SDA_OUT();
    SDA = 1;
    I2C_HalfClk();
    SCL = 1;
    I2C_HalfClk();
    SCL = 0;
}

/**
 * @brief 
 * 
 * @param Data 
 * @return uint8_t 0=ACK 1=NACK
 */
uint8_t I2C_write(unsigned char Data)
{
    for (uint8_t i = 0; i < 8; i++) {
        if ((Data & 0x80) == 0) {
            SDA = 0;
        } else {
            SDA = 1;
        }
        __asm__("NOP");
        SCL = 1;
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        // I2C_HalfClk();
        SCL = 0;
        Data <<= 1;
    }
    SDA = 0;
    SDA_IN();
        __asm__("NOP");
        __asm__("NOP");
    SCL = 1;
        __asm__("NOP");
        __asm__("NOP");
    volatile uint8_t ack = SDA;
    SCL = 0;
    __asm__("NOP");
    SDA_OUT();
    return ack;
}

uint8_t I2C_read(void)
{
    SDA_IN();
    unsigned char i;
    volatile uint8_t Data = 0;
    
    for (i = 0; i < 8; i++) {
        SCL = 0;
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        __asm__("NOP");
        SCL = 1;
        __asm__("NOP");
        if (SDA) {
            Data |= 1;
        }
        if (i < 7) {
            Data <<= 1;
        }
    }
	__asm__("NOP");
	__asm__("NOP");
    SCL = 0;
    I2C_HalfClk();
    return Data;
}

// bool Nfc_MemRead(uint8_t mema, uint8_t *buf)
// {
//     bool ret = true;
//     I2C_start();
//     ret &= !I2C_write(NFC_W_ADDR);
//     ret &= !I2C_write(mema); // MEMA
//     I2C_stop();
//     delayMs(1);
//     I2C_start();
//     ret &= !I2C_write(NFC_R_ADDR);
//     for (uint8_t i = 0; i < 16; ++i) {
//         buf[i] = I2C_read();
//         I2C_ack();
//     }
//     I2C_stop();
//     return ret;
// }
// bool Nfc_MemWrite(uint8_t mema, uint8_t *buf)
// {
//     bool ret = true;
//     I2C_start();
//     ret &= !I2C_write(NFC_W_ADDR);
//     ret &= !I2C_write(mema); // MEMA
//     for (uint8_t i = 0; i < 16; ++i) {
//         ret &= !I2C_write(buf[i]);
//     }
//     I2C_stop();
//     delayMs(5);
//     return ret;
// }

bool Nfc_MemReadBlk(uint8_t mema, NfcMemoryBlk buf)
{
    bool ret = true;
    I2C_start();
    ret &= !I2C_write(NFC_W_ADDR);
    ret &= !I2C_write(mema); // MEMA
    I2C_stop();
    // delayMs(1); // SRAM无需延时
    I2C_start();
    ret &= !I2C_write(NFC_R_ADDR);
    for (uint8_t i = 0; i < 16; ++i) {
        buf[i] = I2C_read();
        I2C_ack();
    }
    I2C_stop();
    return ret;
}
bool Nfc_MemWriteBlk(uint8_t mema, const NfcMemoryBlk buf)
{
    bool ret = true;
    I2C_start();
    ret &= !I2C_write(NFC_W_ADDR);
    ret &= !I2C_write(mema); // MEMA
    for (uint8_t i = 0; i < 16; ++i) {
        ret &= !I2C_write(buf[i]);
    }
    I2C_stop();
    // delayMs(5); // 写RAM无需延时
    return ret;
}

/**
 * @brief 
 * 
 * @param mema 
 * @param rega 
 * @param regdat 
 * @return true ACK
 * @return false NACK
 */
bool Nfc_ReadReg(uint8_t mema, uint8_t rega, uint8_t *regdat)
{
    uint8_t ret = false;
    I2C_start();
    ret |= I2C_write(NFC_W_ADDR);
    ret |= I2C_write(mema); // MEMA
    ret |= I2C_write(rega); // REGA
    I2C_stop();
    // if (ret) {
    //     return false;
    // }
    I2C_start();
    ret |= I2C_write(NFC_R_ADDR);
    *regdat = I2C_read();
    I2C_ack();
    I2C_stop();
    return !ret;
}
bool Nfc_WriteReg(uint8_t mema, uint8_t rega, uint8_t mask, uint8_t regdat)
{
    bool ret = true;
    I2C_start();
    ret &= !I2C_write(NFC_W_ADDR);
    ret &= !I2C_write(mema); // MEMA
    ret &= !I2C_write(rega); // REGA
    ret &= !I2C_write(mask); // MASK
    ret &= !I2C_write(regdat);
    I2C_stop();
    return ret;
}

// bool Nfc_FullDump(uint8_t *buf, uint16_t bufLen)
// {
//     if (bufLen < 1024) {
//         return false;
//     }
//     bool ret = true;
//     for (uint16_t blk = 0; blk < 0x3B; ++blk) {
//         ret &= !Nfc_MemRead(blk, (buf[16 * blk]));
//     }
//     return ret;
// }


/**
 * @brief 通过Session寄存器开启pass-through功能
 * 
 * @param enable 
 */
bool Nfc_ConfigurePassThru(bool enable)
{
    uint8_t reg = 0;
    if (enable) {
        reg = NFC_SESSION_NC_REG_PTHRU_ON_OFF;
    } else {
        reg = (uint8_t)~NFC_SESSION_NC_REG_PTHRU_ON_OFF;
    }
    return Nfc_WriteReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NC_REG_REGA, NFC_SESSION_NC_REG_PTHRU_ON_OFF, reg);
    // }
}

bool Nfc_DirSet(bool nfc2iic)
{
    uint8_t reg = 0;
    if (nfc2iic) {
        reg = 1;
    } else {
        reg = (uint8_t)~1;
    }
    return Nfc_WriteReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NC_REG_REGA, 1, reg);
}

bool Nfc_I2cLock(bool enable)
{
    uint8_t reg = 0;
    if (enable) {
        reg = NFC_SESSION_NS_REG_I2C_LOCKED;
    } else {
        reg = (uint8_t)~NFC_SESSION_NS_REG_I2C_LOCKED;
    }
    return Nfc_WriteReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NS_REG_REGA, NFC_SESSION_NS_REG_I2C_LOCKED, reg);
}

/**
 * @brief 通过NS_REG判断当前是否有NFC存在
 * @return true 
 * @return false 
 */
bool Nfc_IsFieldActive(void)
{
    uint8_t reg = 0;
    if (Nfc_ReadReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NS_REG_REGA, &reg)) {
        return reg & NFC_SESSION_NS_REG_RF_FIELD_PRESENT;
    }
    return false;
}


bool Nfc_SetFdPin(uint8_t on, uint8_t off)
{
    uint8_t reg = (on << 2) | (off << 4);
    return Nfc_WriteReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NC_REG_REGA, NFC_SESSION_NC_REG_FD_ON_OFF, reg);
}
