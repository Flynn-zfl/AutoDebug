#pragma once

#include "LIB_SPI.h"
#include "LIB_OV495.h"

#define MAX_READ_SIZE	1024

//@brief INIT SPI function, including SPI flash parameters initialization: 
//page size, sector size, block size, operation cmds.
void ov495_spi_init();

//@brief ERASE SPI whole memeroy 
bool ov495_spi_erase();

//@brief READ page data from SPI flash
//@param[in] src_addr: read start address
//@param[out] des_buf: buffer for page data reading out
//@retval 0: failed; 1: success
bool ov495_spi_page_read(unsigned int src_addr, unsigned char* des_buf);

//@brief READ page data from SPI flash
//@param[in] data: page data to write in SPI flash
//@param[in] wr_addr: SPI flash address to write in
//@retval 0: failed; 1: success
bool ov495_spi_page_write(unsigned char* data, unsigned int wr_addr);

unsigned char ov495_RD_SPI_STATUS();