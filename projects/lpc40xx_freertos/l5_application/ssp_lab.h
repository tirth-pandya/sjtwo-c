// SSP2 Lab Assignment
#include <stdint.h>

/*
External flash memory pins
P1.0    : SCK2
P1.1    : MOSI2
p1.4    : MISO2
p1.10   : CS
*/

#if defined(LAB_05_P1) || defined(LAB_05_P2) || defined(LAB_05_P3)
void adesto_cs(void);
void adesto_ds(void);
#endif

#if defined(LAB_05_SPI) || defined(LAB_05_P0)
void ssp__init(uint32_t max_clock_mhz);
uint8_t ssp__exchange_byte(uint8_t data_out);
#endif

#ifdef LAB_05_P3
void adesto_WEL_set(void);
uint8_t adesto_read_status(void);
void adesto_block_erase_64kb(uint32_t block_address);
void adesto_page_program(uint8_t page_address, uint8_t page_data);
uint8_t adesto_page_read(uint32_t page_address);
#endif