#include "ov495_spi_operation.h"

static t_libsif_cfg libsif_cfg;
extern t_sf g_sf;
bool I2C_ACCESS_READ_FLAG = 1; //1;

void ov495_spi_init()
{
	//initial SPI flash parameter to structure var
	spi_flash_parameter_initial();

	////////////////////////////////////////////////////////////////////
	// reset CPU
	////////////////////////////////////////////////////////////////////
	ov495_WriteReg8(0x80203516, 0x00);
	//ov495_WriteReg8(0x80203010, ov495_ReadReg8(0x80203010) | BIT2);
	////////////////////////////////////////////////////////////////////
	ov495_WriteReg8(SC_BASE_ADDR + 0x200u, 0x14u);		//vco 900 isp 150 sclk 150		
	ov495_WriteReg8(SC_BASE_ADDR + 0x201u, 0x00u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x202u, 0xd8u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x203u, 0x00u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x204u, 0x00u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x205u, 0x00u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x206u, 0xb0u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x207u, 0x04u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x020u, 0x02u);
	ov495_WriteReg8(SC_BASE_ADDR + 0x022u, 0x01u);

	ov495_WriteReg32(SC_BASE_ADDR+ 0x018u, 0xffffffff);	//open all clock
	ov495_WriteReg32(SC_BASE_ADDR+ 0x01cu, 0x0fffffff);	//open all clock
	ov495_WriteReg32(SC_BASE_ADDR+ 0x23cu, 0xffffffff);	//open all clock
	ov495_WriteReg32(SC_BASE_ADDR+ 0x010u, 0x00000000);	//clear all rst
	ov495_WriteReg32(SC_BASE_ADDR+ 0x014u, 0x00000000);	//clear all rst

	ov495_WriteReg8(0x80203516, 0x00);
	ov495_WriteReg8(0x80203010, ov495_ReadReg8(0x80203010) |BIT2);
	ov495_WriteReg8(0x80203516, 0x00);

	libsif_init(SYSTEM_CLK, &libsif_cfg, SIF_OP_CPU, SIF_MODE_MASTER, 24000000);
	libsif_cfg.cs_sel = 0;
	libsif_cfg.dma_32bit_en = 0;
	sf_init(&libsif_cfg, &g_sf);
	I2C_ACCESS_READ_FLAG = 0; //1;
}

bool ov495_spi_erase()
{
	SPI_Erase(&libsif_cfg);
	return 1;
}

bool ov495_spi_page_read(unsigned int src_addr, unsigned char *des_buf)
{
	SPI_Read(&libsif_cfg, src_addr, des_buf, MAX_READ_SIZE);
	return 1;
}

bool ov495_spi_page_write(unsigned char* data, unsigned int wr_addr)
{
	SPI_Write(&libsif_cfg, data, g_sf.page_size, wr_addr);
	return 1;
}

unsigned char ov495_RD_SPI_STATUS()
{
	return sf_rd_status_reg(&libsif_cfg);
}