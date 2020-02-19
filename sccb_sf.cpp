#include "stdafx.h"		//please remove this file if no need
//#include "../fw_code.h" //please remove this file
#include "spi.h"

#define SPI_BASE_ADDR	0x80250000
#define SC_BASE_ADDR	0x80800000
static t_sf * sccb_rom_g_sf = NULL;

static void sf_reset_dma(void)
{
	Write_ISP_Reg8(SC_BASE_ADDR+0x0015, Read_ISP_Reg8(SC_BASE_ADDR+0x0015) | BIT0);
	DELAY_LOOP(50); //delay some time
	Write_ISP_Reg8(SC_BASE_ADDR+0x0015, Read_ISP_Reg8(SC_BASE_ADDR+0x0015) & (~BIT0));
}

static void sf_reset_spi(void)
{
	Write_ISP_Reg8(SPI_BASE_ADDR + 0x0001, 0x00u);
	Write_ISP_Reg8(SPI_BASE_ADDR + 0x0000, 0x00u);
}

static void sif_module_reset(void)
{
	// reset sif
	Write_ISP_Reg8(SC_BASE_ADDR+0x0012, Read_ISP_Reg8(SC_BASE_ADDR+0x0012) | BIT6);
	Write_ISP_Reg8(SC_BASE_ADDR+0x0012, Read_ISP_Reg8(SC_BASE_ADDR+0x0012) & (~BIT6));
}

int sccb_sif_hw_init(t_libsif_cfg *cfg, t_sf * sf)
{
	sif_module_reset();

	/*************************************************************************
	disable the gpio sel
	SC REG 0x0053 --r_gpio1_sel[15:8]
	if gpio[10]=1'b1 && gpio1_oe[10]=1'b0,then SSN PAD be selected as gpio 
	if gpio[11]=1'b1 && gpio1_oe[11]=1'b0,then MOSI PAD be selected as gpio
	if gpio[12]=1'b1 && gpio1_oe[12]=1'b0,then MISO PAD be selected as gpio
	**************************************************************************/
	Write_ISP_Reg8(SC_BASE_ADDR + 0x0053, Read_ISP_Reg8(SC_BASE_ADDR + 0x0053) & 0xe3); 

	// [2] r_ping1_ie
	Write_ISP_Reg8(SC_BASE_ADDR + 0x016c, Read_ISP_Reg8(SC_BASE_ADDR + 0x016c) | 0x04);

	// release spi reset
	// [6]:spi reset,high active,default=1
	Write_ISP_Reg8(SC_BASE_ADDR + 0x0012, Read_ISP_Reg8(SC_BASE_ADDR + 0x0012) & 0xbf); 

	// enable spi clock
	// [6]:spi clock,high active,default=0
	Write_ISP_Reg8(SC_BASE_ADDR + 0x001a, Read_ISP_Reg8(SC_BASE_ADDR + 0x001a) & 0x40); 

	if(cfg->cs_sel == 1) {
		// cs1 share pin with gpio4, so disable GPIO function firstly
		Write_ISP_Reg8(SC_BASE_ADDR + 0x0050, Read_ISP_Reg8(SC_BASE_ADDR + 0x0050) & (~BIT4));
		Write_ISP_Reg8(SC_BASE_ADDR + 0x016e, Read_ISP_Reg8(SC_BASE_ADDR + 0x016e) | BIT0);
	}
	// if select the spi as spi master,then M_SCK,SSN,MOSI output enable automatically
	Write_ISP_Reg8(SC_BASE_ADDR + 0x016d, Read_ISP_Reg8(SC_BASE_ADDR + 0x016d) | 0x30); 

	// select SPI_SCK,SSN,MOSI PAD input disable,MISO input enable
	Write_ISP_Reg8(SC_BASE_ADDR + 0x0178, Read_ISP_Reg8(SC_BASE_ADDR + 0x0178) & 0xf8);

	sccb_rom_g_sf = sf;
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_DIV_RATE, cfg->div);//set spi rate

	return 0;
}	



void sccb_sf_tx_cfg(unsigned int tx_len,unsigned int tx_pos)
{
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_TSIZE_L,(unsigned char)(tx_len & 0xff));
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_TSIZE_H,(unsigned char)((tx_len>>8) & 0xff));
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_TSIZE_START_L,(unsigned char)(tx_pos & 0xff));
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_TSIZE_START_H,(unsigned char)((tx_pos>>8) & 0xff));
}

//configure the rx length and position
void sccb_sf_rx_cfg(unsigned int rx_len,unsigned int rx_pos)
{
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_RSIZE_L,(unsigned char)(rx_len & 0xff));
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_RSIZE_H,(unsigned char)((rx_len>>8) & 0xff));
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_RSIZE_START_L,(unsigned char)(rx_pos & 0xff));
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_RSIZE_START_H,(unsigned char)((rx_pos>>8) & 0xff));
}

int sccb_sif_write_sf(unsigned char *src_adr, int len)
{
	int i;
	// cpu mode
	// cpu mode, master
	/*************************************************************************
	enable spi
	[7:6]:reserved
	[5]:spi master enable(only use indicate outside the spi master enable)
	[4]:spi LSB first for rx
	[3]:spi LSB first for tx
	[2]:spi CPHA
	[1]:spi CPOL
	[0]:spi enable
	**************************************************************************/
	Write_ISP_Reg8(SPI_BASE_ADDR + 0x0000, 0x01); 

	/***************************************************************************
	spi start
	[7:3]:reserved
	[2]:spi start
	[1]:cs keep low in block transmit,when tx fifo full or rx fifo empty
	[0]:cs keep low between block transmit,when tx fifo full or rx fifo empty
	****************************************************************************/
	Write_ISP_Reg8(SPI_BASE_ADDR + 0x0001, 0x04);  

	// actual data transmit
	for(i = 0; i < len; i++) {
		while((Read_ISP_Reg8(SPI_BASE_ADDR + 0x0012) & BIT1) == BIT1) {
		}
		Write_ISP_Reg8(SPI_BASE_ADDR + 0x0010, *(src_adr + i));
	}

	return len;
}

int sccb_sif_read_sf(unsigned char *dst_adr, int len)
{
	int i;

	// cpu mode
	// cpu mode, master
	//actual data receive
	for(i = 0; i < len; i++) {
		while((Read_ISP_Reg8(SPI_BASE_ADDR + 0x0012) & BIT2) != BIT2) {
		}
		*(dst_adr + i) = Read_ISP_Reg8(SPI_BASE_ADDR + 0x0011);
	}

	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_START_CTRL, 0x00); 
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_CTRL, 0x00); 

	return len;
}
void sccb_sf_rd_data(unsigned int src_addr,unsigned int *des_addr,unsigned int len)
{
	unsigned char   wr_data[4];
	wr_data[0] = 0x03;
	wr_data[1] = (src_addr>>16)&0xff;
	wr_data[2] = (src_addr>>8)&0xff;
	wr_data[3] = src_addr&0xff;

	//set input/output
	sccb_sf_tx_cfg(4,0);//TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg(len,4);//RX Length: len, RX Position: 4

	sccb_sif_write_sf( wr_data, 4);
	sccb_sif_read_sf( (unsigned char *)(des_addr), len);

	return;
}

static void sccb_sf_delay(unsigned int timeout)
{
	//{volatile unsigned int d; for(d=0; d<timeout; d++);}
}

static void sccb_sf_check_tx()
{
	unsigned char tx_fifo_cnt;

	do {
		sccb_sf_delay(255);
		tx_fifo_cnt = Read_ISP_Reg8(SPI_BASE_ADDR + SPI_M_TX_FIFO_CNT);
	} while(tx_fifo_cnt != 0);

	sccb_sf_delay(20);

	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_INT_CLR, 0x10u);//clear interrupt flag
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_START_CTRL, 0x00u);//disable start
	Write_ISP_Reg8(SPI_BASE_ADDR + SPI_M_CTRL, 0x00u);//disable spi.clear fifo and fsm
}

void sccb_sf_wr_enable()
{
	u32 tx_len, rx_len;
	u32 tx_pos, rx_pos;
	u8 wr_data[1];	

	tx_len = 0x01u;
	rx_len = 0x00u;
	tx_pos = 0x00u;
	rx_pos = 0x00u;

	sccb_sf_tx_cfg( tx_len, tx_pos);	// TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg( rx_len, rx_pos);	// RX Length: 0 byte, RX Position: 0

	wr_data[0] = sccb_rom_g_sf->cmd_write_enable;
	if(sccb_sif_write_sf(wr_data, tx_len) == tx_len){
		;
	}else{
		;
	}

	sccb_sf_check_tx();
}

void sccb_sf_page_program(u32 * rd_addr, unsigned int len, u32 * wr_addr)
{
	u32 tx_len, rx_len;
	u32	tx_pos, rx_pos;
	u8 wr_data[4];

	sccb_sf_wr_enable();

	tx_len = 4u + len;	// 1byte cmd + 3byte address + 256byte per page
	rx_len = 0x00u;
	tx_pos = 0x00u;
	rx_pos = 0x00u;

	sccb_sf_tx_cfg( tx_len, tx_pos);	// TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg( rx_len, rx_pos);	// RX Length: 0 byte, RX Position: 0

	wr_data[0] = sccb_rom_g_sf->cmd_page_program;
	wr_data[1] = ((*wr_addr) >> 16) & 0xffu;
	wr_data[2] = ((*wr_addr) >> 8) & 0xffu;
	wr_data[3] = (*wr_addr) & 0xffu;

	if(sccb_sif_write_sf((u8 *)(rd_addr), len) == len){
		;
	}else{
		;
	}

	sccb_sf_check_tx();
}

static void sccb_sf_sector_erase(u32 addr)
{
	u32 tx_len, rx_len;
	u32 tx_pos, rx_pos;

	tx_len = 0x04u;
	rx_len = 0x00u;
	tx_pos = 0x00u;
	rx_pos = 0x00u;

	sccb_sf_wr_enable();

	sccb_sf_tx_cfg( tx_len, tx_pos);	// TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg( rx_len, rx_pos);	// RX Length: 0 byte, RX Position: 0

	u8 wr_data[4];
	wr_data[0] = sccb_rom_g_sf->cmd_erase_sector;
	wr_data[1] = (addr >> 16) & 0xffu;
	wr_data[2] = (addr >> 8) & 0xffu;
	wr_data[3] = addr & 0xffu;
	if(sccb_sif_write_sf(wr_data, tx_len) == tx_len){
		;
	}else{
		;
	}

	sccb_sf_check_tx();
}

static void sccb_sf_block_erase(u32 addr)
{
	u32 tx_len, rx_len;
	u32 tx_pos, rx_pos;

	tx_len = 0x04u;
	rx_len = 0x00u;
	tx_pos = 0x00u;
	rx_pos = 0x00u;

	sccb_sf_wr_enable();

	sccb_sf_tx_cfg( tx_len, tx_pos);	// TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg( rx_len, rx_pos);	// RX Length: 0 byte, RX Position: 0

	u8 wr_data[4];
	wr_data[0] = sccb_rom_g_sf->cmd_erase_block;
	wr_data[1] = (addr >> 16) & 0xffu;
	wr_data[2] = (addr >> 8) & 0xffu;
	wr_data[3] = addr & 0xffu;
	if(sccb_sif_write_sf(wr_data, tx_len) == tx_len){
		;
	}else{
		;
	}

	sccb_sf_check_tx();
}

static void sccb_sf_chip_erase()
{
	u32 tx_len, rx_len;
	u32 tx_pos, rx_pos;
	tx_len = 0x01u;
	rx_len = 0x00u;
	tx_pos = 0x00u;
	rx_pos = 0x00u;
	sccb_sf_wr_enable();

	sccb_sf_tx_cfg( tx_len, tx_pos);	// TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg( rx_len, rx_pos);	// RX Length: 0 byte, RX Position: 0

	u8 wr_data[1];
	wr_data[0] = sccb_rom_g_sf->cmd_chip_erase;
	if(sccb_sif_write_sf(wr_data, tx_len) == tx_len){
		;
	}else{
		;
	}

	sccb_sf_check_tx();
}

static unsigned char sccb_sf_rd_status_reg()
{
	u32 tx_len, rx_len;
	u32 tx_pos, rx_pos;
	u8 dsr;
	u8 wr_data[1];

	tx_len = 0x01u;
	rx_len = 0x01u;
	tx_pos = 0x00u;
	rx_pos = 0x01u;

	sccb_sf_tx_cfg( tx_len, tx_pos);	// TX Length: 1 byte, TX Position: 0
	sccb_sf_rx_cfg( rx_len, rx_pos);	// RX Length: 0 byte, RX Position: 0

	wr_data[0] = sccb_rom_g_sf->cmd_read_status_reg1;
	if(sccb_sif_write_sf(wr_data, tx_len) == tx_len){
		;
	}else{
		;
	}

	if(sccb_sif_read_sf( &dsr, rx_len) == rx_len){
		;
	}else{
		;
	}

	return dsr;
}

int sccb_sf_erase(unsigned int flash_addr, unsigned char erase_type)
{
	switch(erase_type) {
		case ERASE_CHIP:
			sccb_sf_chip_erase();
			break;
		case ERASE_SECTOR:
			flash_addr = flash_addr & (~(sccb_rom_g_sf->sector_size - 1));
			sccb_sf_sector_erase(flash_addr);
			break;
		case ERASE_BLOCK:
			flash_addr = flash_addr & (~(sccb_rom_g_sf->block_size - 1));
			sccb_sf_block_erase(flash_addr);
			break;
		default:
			break;
	}

	while((sccb_sf_rd_status_reg() & BIT0) == BIT0);
	return 0;
}