#include "display/epd.h"
#include "isr.h"
#include "time/time.h"
#include "util.h"

#include "flash/flash.h"
#include "hal_clock.h"
#include "nfc/i2c.h"

#define MANAGED_XRAM_START 0xF000

void blink(void)
{
    LED_ON();
    delayMs(5);
    LED_OFF();
}

void EPD_Test(void)
{

    Epd_ClearDisplay(0, 0);
    Epd_Refresh();

    // Epd_DisplayTextScreen();
    // Epd_Refresh();

    for (int i = 0; i < 0; ++i) {
        Epd_ClearDisplay(0x0Fu, 0xF0u);
        Epd_Refresh();
        //blink();
        delayMs(5000);

        Epd_ClearDisplay(0xF0u, 0x0Fu);
        Epd_Refresh();
        //blink();
        delayMs(5000);
    }
    // FF00 红色
    // 00FF 黑色
    // FFFF 白色

    // Epd_ClearDisplay(0xFFu, 0xFFu);
    // Epd_Refresh();

    Epd_Deinit();
}

#define XRAM_BEGIN (void *)0xF000
#define XRAM_END (void *)0xFEFF


__code static const unsigned char crc_table[] =
{
    0x00,0x31,0x62,0x53,0xc4,0xf5,0xa6,0x97,0xb9,0x88,0xdb,0xea,0x7d,0x4c,0x1f,0x2e,
    0x43,0x72,0x21,0x10,0x87,0xb6,0xe5,0xd4,0xfa,0xcb,0x98,0xa9,0x3e,0x0f,0x5c,0x6d,
    0x86,0xb7,0xe4,0xd5,0x42,0x73,0x20,0x11,0x3f,0x0e,0x5d,0x6c,0xfb,0xca,0x99,0xa8,
    0xc5,0xf4,0xa7,0x96,0x01,0x30,0x63,0x52,0x7c,0x4d,0x1e,0x2f,0xb8,0x89,0xda,0xeb,
    0x3d,0x0c,0x5f,0x6e,0xf9,0xc8,0x9b,0xaa,0x84,0xb5,0xe6,0xd7,0x40,0x71,0x22,0x13,
    0x7e,0x4f,0x1c,0x2d,0xba,0x8b,0xd8,0xe9,0xc7,0xf6,0xa5,0x94,0x03,0x32,0x61,0x50,
    0xbb,0x8a,0xd9,0xe8,0x7f,0x4e,0x1d,0x2c,0x02,0x33,0x60,0x51,0xc6,0xf7,0xa4,0x95,
    0xf8,0xc9,0x9a,0xab,0x3c,0x0d,0x5e,0x6f,0x41,0x70,0x23,0x12,0x85,0xb4,0xe7,0xd6,
    0x7a,0x4b,0x18,0x29,0xbe,0x8f,0xdc,0xed,0xc3,0xf2,0xa1,0x90,0x07,0x36,0x65,0x54,
    0x39,0x08,0x5b,0x6a,0xfd,0xcc,0x9f,0xae,0x80,0xb1,0xe2,0xd3,0x44,0x75,0x26,0x17,
    0xfc,0xcd,0x9e,0xaf,0x38,0x09,0x5a,0x6b,0x45,0x74,0x27,0x16,0x81,0xb0,0xe3,0xd2,
    0xbf,0x8e,0xdd,0xec,0x7b,0x4a,0x19,0x28,0x06,0x37,0x64,0x55,0xc2,0xf3,0xa0,0x91,
    0x47,0x76,0x25,0x14,0x83,0xb2,0xe1,0xd0,0xfe,0xcf,0x9c,0xad,0x3a,0x0b,0x58,0x69,
    0x04,0x35,0x66,0x57,0xc0,0xf1,0xa2,0x93,0xbd,0x8c,0xdf,0xee,0x79,0x48,0x1b,0x2a,
    0xc1,0xf0,0xa3,0x92,0x05,0x34,0x67,0x56,0x78,0x49,0x1a,0x2b,0xbc,0x8d,0xde,0xef,
    0x82,0xb3,0xe0,0xd1,0x46,0x77,0x24,0x15,0x3b,0x0a,0x59,0x68,0xff,0xce,0x9d,0xac
};

uint8_t crc8(uint8_t *ptr, uint8_t len)
{
    uint8_t  crc = 0x00;
    while (len--) {
        crc = crc_table[crc ^ *ptr++];
    }
    return (crc);
}

// void WaitDebugStub(void)
// {
//     P1_0 = 0;
//     for (volatile __xdata uint8_t *i = (volatile __xdata uint8_t *)XRAM_BEGIN; i <= (volatile __xdata uint8_t *)XRAM_END; ++i) {
//         *i = 0xA5;
//     }
//     /*
//     for (volatile __xdata uint8_t *i = 0xFF20; i <= 0xFF40; ++i) {
//         *i = 0xA5;
//     }
//     */
//     while (1) {
//         __asm__(".db 0xA5;");
//     }
// }

enum {
    FSM_Init,
    FSM_WAIT_RF,        // 空闲状态，等待RF中断
    FSM_SETUP_RF,       // RF在位 配置NFC芯片透传
    FSM_HANDSHAKE_A,    // 等待NFC读数据
    FSM_HANDSHAKE_B,    // 回读
    FSM_I2N_A,          // I2C -> NFC
    FSM_I2N_B,          // I2C -> NFC
    FSM_N2I_A,          // I2C -> NFC
    FSM_N2I_B,          // I2C -> NFC
    FSM_RF_LOST,        // 重置各状态并切换回空闲
} g_NfcFsmState;
#define StateTransit(x) g_NfcFsmState = (x)
#define StateLostGuard(sessionReg) { \
        if ((NFC_SESSION_NS_REG_RF_FIELD_PRESENT & sessionReg) == 0) { \
            StateTransit(FSM_RF_LOST); \
            break; \
        } \
    }

#define SRAM_I2C_BLK_0 0xF8u
#define SRAM_I2C_BLK_1 0xF9u
#define SRAM_I2C_BLK_2 0xFAu
#define SRAM_I2C_BLK_3 0xFBu


#define DebugLog(id) do { \
        I2C_start(); \
        I2C_write(id); \
        I2C_write((uint8_t)g_NfcFsmState); \
        I2C_write((uint8_t)sessionReg); \
        I2C_write((uint8_t)(receivedSeg >> 8)); \
        I2C_write((uint8_t)receivedSeg); \
        I2C_write((uint8_t)NFC_FD); \
        I2C_stop(); \
    } while (0)

#define NFC_GUARD_BRK(x) if (!(x)) {break;}
// struct {
//     uint8_t raw;
//     // NfcMemoryBlk blkBuf[4];
// } 

typedef union {
    uint8_t buf[64];
    struct {
        uint8_t pld[60];
        uint8_t cmd; // NFC:cmd I2C:ACK
        uint8_t seq;
        uint8_t _;
        uint8_t crc8;
    } data;
    struct {
        uint8_t pld[52];
        uint8_t challenge[4];
        uint8_t state[4];
        uint8_t cmd;
        uint8_t seq;
        uint8_t _;
        uint8_t crc8;
    } hs_i2n;
    struct {
        uint8_t pld[52];
        uint8_t response[4];
        uint8_t cmdPld[4];
        uint8_t ack;
        uint8_t seq;
        uint8_t _;
        uint8_t crc8;
    } hs_n2i;
    struct {
        uint8_t blk0[16];
        uint8_t blk1[16];
        uint8_t blk2[16];
        uint8_t blk3[16];
    };
} NfcBuf;
NfcBuf g_nfcBuf;

#define NFC_CMD_NACK 'N'
#define NFC_CMD_ACK  'A'

#define nfcBuf g_nfcBuf.buf
static uint8_t sessionReg = 0;
static uint16_t receivedSeg = 0;

void ClearMemBlk(NfcMemoryBlk blk)
{
    for (uint8_t i = 0; i < sizeof(blk); ++i) {
        blk[i] = 0;
    }
}

void WriteMemBlk(uint8_t blk[16], uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    (void)b;
    blk[12] = a;
    blk[14] = c;
    blk[15] = d;
}

extern void inline Epd_SendData(uint8_t data);
void WriteMemBlkToEpd(const uint8_t blk[16], uint8_t cnt)
{
    for (uint8_t i = 0; i < cnt; ++i) {
        EPD_CS = 0;
        U0DBUF = blk[i];
        while (U0CSR & 0x01)
        {
        }
        EPD_CS = 1;
    }
}

#define DO_EPD_REFRESH

void Nfc_Fsm(void)
{
    static uint8_t ackVal = 0;
    //DebugLog(0x10);
    switch (g_NfcFsmState) {
    case FSM_Init: {
        StateTransit(FSM_WAIT_RF);
        break;
    }
    case FSM_WAIT_RF: {
        if (Nfc_IsFieldActive()) { // 改成中断管脚
            StateTransit(FSM_SETUP_RF);
        }
        break;
    }
    case FSM_SETUP_RF: { // 初始化通信
        // NfcFlash_Pwr(true);
        NFC_GUARD_BRK(Nfc_SetFdPin(0x3u, 0x3u));
        NFC_GUARD_BRK(Nfc_DirSet(false));
        NFC_GUARD_BRK(Nfc_ConfigurePassThru(true));
        // 写入握手数据
        ClearMemBlk(nfcBuf);
        // WriteMemBlk(nfcBuf, 'H', 'S', 0xDE, 0xAD);
        g_nfcBuf.hs_i2n.cmd = NFC_CMD_ACK;

        
        NFC_GUARD_BRK(Nfc_MemWriteBlk(SRAM_I2C_BLK_3, nfcBuf));
        StateTransit(FSM_HANDSHAKE_A);
        break;
    }
    case FSM_HANDSHAKE_A: { // 03 等待NFC读取握手数据
        NFC_GUARD_BRK(Nfc_ReadReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NS_REG_REGA , &sessionReg));
        StateLostGuard(sessionReg);
        if ((NFC_FD == 0) || ((sessionReg & NFC_SESSION_NS_REG_SRAM_RF_READY) == 0))
        {
            // NFC读取握手数据
            NFC_GUARD_BRK(Nfc_DirSet(true)); // 等待接收握手数据
            StateTransit(FSM_HANDSHAKE_B);
        }
        break;
    }
    case FSM_HANDSHAKE_B: { // 04 等待NFC写入握手数据
        NFC_GUARD_BRK(Nfc_ReadReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NS_REG_REGA , &sessionReg));
        StateLostGuard(sessionReg);
        if ((NFC_FD == 0) || ((sessionReg & NFC_SESSION_NS_REG_SRAM_I2C_READY) != 0))
        {
            // NFC已完成握手数据写入
            // todo 此处读取握手数据并做校验
            receivedSeg = 0;
#ifdef DO_EPD_REFRESH
            Epd_BeginBlack(); // 准备接收黑色图像
#endif // DO_EPD_REFRESH
            ackVal = true;
            StateTransit(FSM_I2N_A);
        }
        break;
    }
    case FSM_I2N_A: { // 05
        // 发送ACK
        NFC_GUARD_BRK(Nfc_DirSet(false));
        if (ackVal) {
            g_nfcBuf.data.cmd = NFC_CMD_ACK;
        } else {
            g_nfcBuf.data.cmd = NFC_CMD_NACK;
        }
        g_nfcBuf.data._ = (uint8_t)receivedSeg;
        
        NFC_GUARD_BRK(Nfc_MemWriteBlk(SRAM_I2C_BLK_3, g_nfcBuf.blk3));
        StateTransit(FSM_I2N_B);
        break;
    }
    case FSM_I2N_B: { // 06
        // 等待NFC读取
        NFC_GUARD_BRK(Nfc_ReadReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NS_REG_REGA , &sessionReg));
        StateLostGuard(sessionReg);
        if ((NFC_FD == 0) || ((sessionReg & NFC_SESSION_NS_REG_SRAM_RF_READY) == 0)) {
            // NFC已读取ACK
            NFC_GUARD_BRK(Nfc_DirSet(true)); // 等待接收图像数据
            StateTransit(FSM_N2I_A);
        }
        break;
    }
    case FSM_N2I_A: { // 07 等待NFC写入图像
        NFC_GUARD_BRK(Nfc_ReadReg(NFC_SESSION_REG_MEMA, NFC_SESSION_NS_REG_REGA , &sessionReg));
        StateLostGuard(sessionReg);


            if (receivedSeg == 250) {
#ifdef DO_EPD_REFRESH
                Epd_BeginRed(); // 准备接收图像
#endif // DO_EPD_REFRESH
            } else if (receivedSeg == 500) {
#ifdef DO_EPD_REFRESH
                Epd_Refresh();
#endif // DO_EPD_REFRESH
                StateTransit(FSM_RF_LOST);
                break;
            }

        if ((NFC_FD == 0) || ((sessionReg & NFC_SESSION_NS_REG_SRAM_I2C_READY) != 0)) {
            // NFC已完成图像数据写入
            // 读取数据
            NFC_GUARD_BRK(Nfc_I2cLock(true));
            NFC_GUARD_BRK(Nfc_MemReadBlk(SRAM_I2C_BLK_0, g_nfcBuf.blk0));
            NFC_GUARD_BRK(Nfc_MemReadBlk(SRAM_I2C_BLK_1, g_nfcBuf.blk1));
            NFC_GUARD_BRK(Nfc_MemReadBlk(SRAM_I2C_BLK_2, g_nfcBuf.blk2));
            NFC_GUARD_BRK(Nfc_MemReadBlk(SRAM_I2C_BLK_3, g_nfcBuf.blk3));
            uint8_t crc = crc8(g_nfcBuf.buf, 63);
            if (crc == g_nfcBuf.data.crc8) {
                blink();
                WriteMemBlkToEpd(g_nfcBuf.blk0, 16);
                WriteMemBlkToEpd(g_nfcBuf.blk1, 16);
                WriteMemBlkToEpd(g_nfcBuf.blk2, 16);
                WriteMemBlkToEpd(g_nfcBuf.blk3, 12);
                ackVal = true;
                receivedSeg++;
            } else {
                ackVal = false;
            }
            NFC_GUARD_BRK(Nfc_I2cLock(false));
            g_nfcBuf.data.crc8 = ~crc;
    //DebugLog(0x2A);
            
            StateTransit(FSM_I2N_A);
    //DebugLog(0x3A);
        }
        break;
    }
    case FSM_N2I_B: {
        StateTransit(FSM_WAIT_RF);
        break;
    }
    case FSM_RF_LOST: {
        Nfc_ConfigurePassThru(false);
        NFC_GUARD_BRK(Nfc_DirSet(false));
        NFC_GUARD_BRK(Nfc_SetFdPin(0x0u, 0x0u));
        StateTransit(FSM_WAIT_RF);
        break;
    }
    }
    //DebugLog(0x20);
}

void main(void)
{
    halPowerClkMgmtSetMainClkSrc(0);
    HAL_ENABLE_INTERRUPTS();
    time_init();
    LED_INIT();
    NfcFlash_IoInit();
    // NfcFlash_Pwr(false);

    // WaitDebugStub();

    Epd_Init();
    //Epd_ClearDisplay(0xFF, 0xFF);
    //Epd_Refresh();

    NfcFlash_Pwr(true);


    delayMs(50);
    Flash_ReleasePowerDown();
    Flash_CheckId();

    
    I2C_init();
    //DebugLog(0x70);

    while (1) {
        Nfc_Fsm();
        //__asm__(".db 0xA5;");
        //__asm__(".db 0x00;");
        continue;
        
    }
}