#include "stdafx.h"	//please remove this file if no need
#include "spi.h"

#define WRITE_ENABLE_CMD            0x06
#define WRITE_DISABLE_CMD           0x04
#define READ_STATUS_REG1_CMD        0x05
#define WRITE_STATUS_REG_CMD        0x01
#define READ_DATA_CMD               0x03
#define FAST_READ_CMD               0x0b
#define PAGE_PROGRAM_CMD            0x02
#define ERASE_SECTOR_CMD         	0xd7
#define ERASE_BLOCK_CMD             0xd8
#define CHIP_ERASE_CMD					 0xc7
#define POWER_DOWN_CMD              0xb9
#define MANUFACTURER_CMD            0xab

static void Pm25LV512_init(void)
{
	//debug_printf("MX25L1_init init\n");	
}

void spiflash_init(t_sf* sf)
{
	if (sf == NULL) return;
	sf->device_init = Pm25LV512_init;
	sf->page_size = 256;
	sf->sector_size = 4096;
	sf->block_size = 64*1024;
	sf->op_size = 0;
	sf->fDMA = 0;
	sf->timeout = 0x100000;
	sf->deviceID = 0x7b;
	sf->cmd_read_status_reg1 = READ_STATUS_REG1_CMD;
	sf->cmd_write_disable = WRITE_DISABLE_CMD;
	sf->cmd_write_enable = WRITE_ENABLE_CMD;
	sf->deviceid_dummy_bytes = 0;
	sf->cmd_deviceid = MANUFACTURER_CMD;
	sf->cmd_chip_erase = CHIP_ERASE_CMD;
	sf->cmd_erase_sector = ERASE_SECTOR_CMD;
	sf->cmd_erase_block = ERASE_BLOCK_CMD;
	sf->cmd_fast_read = FAST_READ_CMD;
	sf->cmd_read_data = READ_DATA_CMD;
	sf->cmd_page_program = PAGE_PROGRAM_CMD;
	sf->cmd_write_status_reg = WRITE_STATUS_REG_CMD;
}
