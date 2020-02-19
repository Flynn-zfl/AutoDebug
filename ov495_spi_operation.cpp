#include "ov495_spi_operation.h"

static t_libsif_cfg libsif_cfg;
extern t_sf g_sf;

void spi_init()
{
	//initial SPI flash parameter to structure var
	spi_flash_parameter_initial();

	////////////////////////////////////////////////////////////////////
	// reset CPU
	////////////////////////////////////////////////////////////////////
	ov495_WriteReg8(0x80203516, 0x00);
	ov495_WriteReg8(0x80203010, ov495_ReadReg8(0x80203010) | BIT2);
	////////////////////////////////////////////////////////////////////

	//set OV495 PLL to our target value
	ov495_pll0_set(
				PAD_CLK,				//int ref_clk        , //OSC frequency, reserved for auto calculation
				300000000,				//int system_clk     , //system clock frequency,reserved for auto calculation  
				150000000,				//int isp_clk        , //isp clock frequency   ,reserved for auto calculation  
				0,						//int auto_en        , //unused, enable auto caculation
				0,						//int r_pll0_bypass  ,
				5,						//int r_pll0_prediv  ,
				0,						//int r_pll0_dsm     ,
				0x4b,					//int r_pll0_loopdivp,
				0,						//int r_pll0_loopdivs,
				6,						//int r_pll0_div0    ,    
				3						//int r_pll0_div1        
			);

	ov495_WriteReg32(SC_BASE_ADDR+0x018,0xffffffff);	//open all clock
	ov495_WriteReg32(SC_BASE_ADDR+0x01c,0x0fffffff);	//open all clock 
	ov495_WriteReg32(SC_BASE_ADDR+0x23c,0xffffffff);	//open all clock 
	ov495_WriteReg32(SC_BASE_ADDR+0x010,0x00000000);	//clear all rst
	ov495_WriteReg32(SC_BASE_ADDR+0x014,0x00000000);	//clear all rst

	libsif_init(SYSTEM_CLK, &libsif_cfg, SIF_OP_CPU, SIF_MODE_MASTER, 24000000);
	libsif_cfg.cs_sel = 0;
	libsif_cfg.dma_32bit_en = 0;
	sf_init(&libsif_cfg, &g_sf);
}

bool spi_erase()
{
	SPI_Erase(&libsif_cfg);
	return 1;
}

bool spi_page_read(unsigned int src_addr, unsigned char *des_buf)
{
	SPI_Read(&libsif_cfg, src_addr, des_buf, MAX_READ_SIZE);
	return 1;
}

bool spi_page_write(unsigned char* data, unsigned int wr_addr)
{
	SPI_Write(&libsif_cfg, data, g_sf.page_size, wr_addr);
	return 1;
}