// SSP2 Lab Assignment
#include <stdint.h>

/*
External flash memory pins
P1.0    : SCK2
P1.1    : MOSI2
p1.4    : MISO2
p1.10   : CS
*/

#if defined(LAB_05_P1) || defined(LAB_05_P2)
void adesto_cs(void);
void adesto_ds(void);
#endif

#if defined(LAB_05_SPI) || defined(LAB_05_P0)
void ssp__init(uint32_t max_clock_mhz);
uint8_t ssp__exchange_byte(uint8_t data_out);
#endif