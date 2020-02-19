#pragma once
#include "ov490_spi.h"

//@brief INIT SPI function, including SPI flash parameters initialization: 
//page size, sector size, block size, operation cmds.
void spi_init();

//@brief ERASE SPI memeroy 
//@param[in] src_addr: erase start address
//@param[in] erase_type: three types supported: sector, block, whole chip 
//@retval 0: failed; 1: success
bool spi_erase(unsigned int src_addr, unsigned char erase_type);

//@brief READ page data from SPI flash
//@param[in] src_addr: read start address
//@param[out] des_addr: buffer for page data reading out
//@retval 0: failed; 1: success
bool spi_page_read(unsigned int src_addr, unsigned int *des_addr);

//@brief READ page data from SPI flash
//@param[in] data: page data to write in SPI flash
//@param[in] wr_addr: SPI flash address to write in
//@retval 0: failed; 1: success
bool spi_page_write(unsigned int* data, unsigned int wr_addr);