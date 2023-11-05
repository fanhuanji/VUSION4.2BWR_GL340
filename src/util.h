
#ifndef _UTIL_H_
#define _UTIL_H_

#include <cc2510fx.h>
#include <stdbool.h>


// void SetDebugArray(uint8_t *buf, uint16_t offset, uint16_t size);

#define BV(x) (1 << (x))
#define st(x) \
    do        \
    {         \
        x     \
    } while (__LINE__ == -1)

#define HAL_ENABLE_INTERRUPTS() st(EA = 1;)
#define HAL_DISABLE_INTERRUPTS() st(EA = 0;)
#define HAL_INTERRUPTS_ARE_ENABLED() (EA)

typedef unsigned char halIntState_t;
#define HAL_ENTER_CRITICAL_SECTION(x) st(x = EA; HAL_DISABLE_INTERRUPTS();)
#define HAL_EXIT_CRITICAL_SECTION(x) st(EA = x;)
#define HAL_CRITICAL_STATEMENT(x) st(halIntState_t _s; HAL_ENTER_CRITICAL_SECTION(_s); x; HAL_EXIT_CRITICAL_SECTION(_s);)

#define LED_BOOST_ON() P2_2 = 1
#define LED_BOOST_OFF() P2_2 = 0
#define LED_ON() P2_1 = 1
#define LED_OFF() P2_1 = 0
#define LED_TOGGLE() P2_1 ^= 1
#define LED_INIT() do { \
        P2DIR |= BV(1) | BV(2); \
        LED_OFF();                \
    } while (0)

#endif