

#include "hal_clock.h"
#include "util.h"

void halPowerClkMgmtSetMainClkSrc(uint8_t source)
{
    // source can have the following values:
    // CRYSTAL 0x00  /*  High speed Crystal Oscillator Control */
    // RC      0x01  /*  Low power RC Oscillator */
    
    SLEEP &= ~OSC_PD_BIT;

    while (!HIGH_FREQUENCY_RC_OSC_STABLE);
    CLKCON = 0xC9u;
    CLKCON = 0x49u;
    

    while (!XOSC_STABLE);
    CLKCON = 0xC9u;
        __asm__("NOP");
    CLKCON = 0x89u;
    (void)source;
}


