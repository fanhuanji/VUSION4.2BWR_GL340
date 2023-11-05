
#ifndef HAL_CLOCK_H
#define HAL_CLOCK_H

#include <stdint.h>
#include <cc2510fx.h>


/* Pre defined values for source, as used in halPowerClkMgmtSetMainClkSrc() */
#define CRYSTAL           0x00  /*  High speed Crystal Oscillator Control */
#define RC                0x01  /*  Low power RC Oscillator */

// Bit masks to check CLKCON register
#define OSC_BIT           0x40  // bit mask used to select/check the system clock oscillator
#define TICKSPD_BITS      0x38  // bit mask used to check the timer ticks output setting
#define CLKSPD_BIT        0x03  // bit maks used to check the clock speed
#define MAIN_OSC_BITS     0x7F  // bit mask used to control the system clock oscillator
                                // e.g. ~MAIN_OSC_BITS can be used to start Crystal OSC

// Bit masks to check SLEEP register
#define XOSC_STABLE_BIT   0x40  // bit mask used to check the stability of XOSC
#define HFRC_STB_BIT      0x20  // bit maks used to check the stability of the High-frequency RC oscillator
#define OSC_PD_BIT        0x04  // bit maks used to power down system clock oscillators

/*==== TYPES =================================================================*/
/*==== EXPORTS ===============================================================*/

/*==== MACROS=================================================================*/

// Macro for checking status of the high frequency RC oscillator.
#define HIGH_FREQUENCY_RC_OSC_STABLE (SLEEP & HFRC_STB_BIT)

// Macro for getting the clock division factor
#define CLKSPD  (CLKCON & CLKSPD_BIT)


// Macro for getting the timer tick division factor.
#define TICKSPD ((CLKCON & TICKSPD_BITS) >> 3)

// Macro for checking status of the crystal oscillator
#define XOSC_STABLE (SLEEP & XOSC_STABLE_BIT)

/*==== FUNCTIONS =============================================================*/

/******************************************************************************
* @fn  halPowerClkMgmtSetMainClkSrc
*
* @brief
* Function for setting the main clock oscillator source, turns off the
* clock source not used changing to XOSC will take approx 150 us
*  Settings TICKSPD equal CLKSPD
*
* Parameters:
*
* @param  UINT8 source
*
* @return void
*
******************************************************************************/
void halPowerClkMgmtSetMainClkSrc(uint8_t source);

#endif // HAL_CLOCK_H
