
#ifndef ISR_H
#define ISR_H
#include <cc2510fx.h>

void time_isr(void) __interrupt(WDT_VECTOR);

#endif // ISR_H
