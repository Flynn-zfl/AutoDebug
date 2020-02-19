#include "..\\stdafx.h"
#include "LIB_SPI.h"
#include "LIB_OV495.h"

extern t_sf g_sf;
extern t_sf * rom_g_sf;


/*************************************************************************************************************
Function:
SET SPI FLASH PARAMETERS TO STRUCTURE.
PLEASE CHANGE THESE PARAMETERS DEPENDS ON YOUR SPI FLASH TYPE
*************************************************************************************************************/
void spi_flash_parameter_initial(void)
{
	//init flash parameter
	g_sf.device_init = NULL;
	g_sf.page_size = 256;
	g_sf.sector_size = 4096;
	g_sf.block_size = 64*1024;
	g_sf.op_size = 0;
	g_sf.fDMA = 0;
	g_sf.timeout = 0x100000;
	g_sf.deviceID = 0x7b;
	g_sf.cmd_read_status_reg1 = READ_STATUS_REG1_CMD;
	g_sf.cmd_write_disable = WRITE_DISABLE_CMD;
	g_sf.cmd_write_enable = WRITE_ENABLE_CMD;
	g_sf.deviceid_dummy_bytes = 0;
	g_sf.cmd_deviceid = MANUFACTURER_CMD;
	g_sf.cmd_chip_erase = CHIP_ERASE_CMD;
	g_sf.cmd_erase_sector = ERASE_SECTOR_CMD;
	g_sf.cmd_erase_block = ERASE_BLOCK_CMD;
	g_sf.cmd_fast_read = FAST_READ_CMD;
	g_sf.cmd_read_data = READ_DATA_CMD;
	g_sf.cmd_page_program = PAGE_PROGRAM_CMD;
	g_sf.cmd_write_status_reg = WRITE_STATUS_REG_CMD;
}



/*
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
PLEASE DO NOT CHANGE FOLLOWING CODE,COPY&PASTE ONLY
PLEASE DO NOT CHANGE FOLLOWING CODE,COPY&PASTE ONLY
PLEASE DO NOT CHANGE FOLLOWING CODE,COPY&PASTE ONLY
"重要的事情说3遍"
&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
*/
signed int libsif_hw_init(t_libsif_cfg *cfg)
{

	//sif_module_reset();
	ov495_WriteReg8(SC_BASE_ADDR + 0x12u, ov495_ReadReg8(SC_BASE_ADDR + 0x12u) | BIT6);//reset sif module
	ov495_WriteReg8(SC_BASE_ADDR + 0x12u, ov495_ReadReg8(SC_BASE_ADDR + 0x12u) & (~BIT6));

	/*************************************************************************
	 disable the gpio sel
	 SC REG 0x0053 --r_gpio1_sel[15:8]
	 if gpio[10]=1'b1 && gpio1_oe[10]=1'b0,then SSN PAD be selected as gpio 
	 if gpio[11]=1'b1 && gpio1_oe[11]=1'b0,then MOSI PAD be selected as gpio
	 if gpio[12]=1'b1 && gpio1_oe[12]=1'b0,then MISO PAD be selected as gpio
	**************************************************************************/
	ov495_WriteReg8(SC_BASE_ADDR + 0x0053u, ov495_ReadReg8(SC_BASE_ADDR + 0x0053u) & 0xe3u); 
	
	// [2] r_ping1_ie
	ov495_WriteReg8(SC_BASE_ADDR + 0x016cu, ov495_ReadReg8(SC_BASE_ADDR + 0x016cu) | 0x04u);
	
	// release spi reset
	// [6]:spi reset,high active,default=1
	ov495_WriteReg8(SC_BASE_ADDR + 0x0012u, ov495_ReadReg8(SC_BASE_ADDR + 0x0012u) & 0xbfu); 

	// enable spi clock
	// [6]:spi clock,high active,default=0
	ov495_WriteReg8(SC_BASE_ADDR + 0x001au, ov495_ReadReg8(SC_BASE_ADDR + 0x001au) | 0x40u); 

	if(cfg->op_mode == SIF_MODE_MASTER) {
		if(cfg->cs_sel == 1u) {
			// cs1 share pin with gpio4, so disable GPIO function firstly
			ov495_WriteReg8(SC_BASE_ADDR + 0x0050u, ov495_ReadReg8(SC_BASE_ADDR + 0x0050u) & (~BIT4));
			ov495_WriteReg8(SC_BASE_ADDR + 0x016eu, ov495_ReadReg8(SC_BASE_ADDR + 0x016eu) | BIT0);
		}

		if(cfg->io_mode != SPI) {
			// enable 4-wire mode
			ov495_WriteReg8(SC_BASE_ADDR + 0x025cu, ov495_ReadReg8(SC_BASE_ADDR + 0x025cu) | BIT0);
			// diable gpio function for io2 ,io3
			ov495_WriteReg32(SC_BASE_ADDR + 0x0080u, ov495_ReadReg32(SC_BASE_ADDR + 0x0080u) &(~(BIT15|BIT16)));
		} else{
		    // if select the spi as spi master,then M_SCK,SSN,MOSI output enable automatically
		    ov495_WriteReg8(SC_BASE_ADDR + 0x016du, ov495_ReadReg8(SC_BASE_ADDR + 0x016du) | 0x30u); 
		    // select SPI_SCK,SSN,MOSI PAD input disable,MISO input enable
		    ov495_WriteReg8(SC_BASE_ADDR + 0x0178u, (ov495_ReadReg8(SC_BASE_ADDR + 0x0178u) | (unsigned char)BIT3) & (unsigned char)(~(BIT0|BIT1|BIT2)));
		}
	} else {
		// if select the spi as spi slave,then MISO output enable automatically
		ov495_WriteReg8(SC_BASE_ADDR + 0x016du, (ov495_ReadReg8(SC_BASE_ADDR + 0x016du) & 0xdfu) | 0x10u); 

		// select SPI_SCK,SSN,MOSI PAD input disable,MISO input enable
		ov495_WriteReg8(SC_BASE_ADDR + 0x0178u, (ov495_ReadReg8(SC_BASE_ADDR + 0x0178u) |BIT0|BIT1|BIT2) & (~BIT3));
	}

	return 0;
}		

signed int libsif_init(unsigned int in_clk, t_libsif_cfg *cfg, unsigned char dma, unsigned char master, unsigned int clk)
{
	memset(cfg, 0, sizeof(t_libsif_cfg));
	cfg->base_addr = SPI_BASE_ADDR;

	if(master != 0u){
		cfg->op_mode = SIF_MODE_MASTER;
	} else {
		cfg->op_mode = SIF_MODE_SLAVE;
	}

	if(dma != 0u) {
		cfg->op_dma = 1;
	} else {
		cfg->op_dma = 0;
	}

	cfg->io_mode = SPI;

	cfg->cs_sel = 0;
	cfg->cpol = 0;
	cfg->cpha = 0;
	cfg->slsb = 0;
	cfg->dc_enabled = 1;
	cfg->dma_32bit_en = 0;//asic say don't support this function at 08/20/2015

	// M_SCK = spi_clk_i/((REG06+1)*2), such as spi_clk_i=50MHz, the M_SCK=50/((div+1)*2)=5Mbps
	signed int div = (signed int)in_clk / (signed int)clk / 2 - 1;
	if(div < 0 ) {
		div = 0;
	}
	cfg->div = (unsigned char)div;

	signed int ret = libsif_hw_init(cfg);

	return ret;
}


signed int libsif_read(t_libsif_cfg *cfg, unsigned char *buf, unsigned int len)
{
	unsigned int i, j, cnt;

	if(cfg->op_mode == SIF_MODE_MASTER) {
		
		//configure the baud rate
		ov495_WriteReg8(cfg->base_addr + SPI_M_DIV_RATE, cfg->div); 

		//open the spi interrupt,maybe now no use
		ov495_WriteReg8(cfg->base_addr + SPI_M_INT_EN, 0x3fu); 

		ov495_WriteReg8(cfg->base_addr + SPI_M_SLV_SEL, cfg->cs_sel); 

		//configure the rx length and position
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_LL, (unsigned char)(len & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_LH, (unsigned char)((len >> 8u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_HL, (unsigned char)((len >> 16u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_HH, (unsigned char)((len >> 24u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_START_L, (unsigned char)(cfg->rx_start & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_START_H, (unsigned char)((cfg->rx_start >>8u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_LL, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_LH, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_HL, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_HH, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_START_L, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_START_H, 0u);

		//spi enable
		ov495_WriteReg8(cfg->base_addr + SPI_M_CTRL, 
						(unsigned char)((unsigned char)BIT0 & (unsigned char)(~BIT1) & (unsigned char)(~BIT2) & (unsigned char)(~BIT4)) | (cfg->cpol << 1u) | (cfg->cpha << 2u) | (cfg->slsb << 4u)); 

		if(cfg->op_dma == SIF_OP_CPU) {
			ov495_WriteReg8(cfg->base_addr + SPI_M_START_CTRL, 0x04u);//spi start 

			// actual data receive
			/*for(i = 0; i < len; i++) {
				while((ov495_ReadReg8(cfg->base_addr + SPI_M_FIFO_ST) & (unsigned char)BIT2) != (unsigned char)BIT2){
				}
				*(buf + i) = ov495_ReadReg8(cfg->base_addr + SPI_M_RXD);
			}*/
			for(i = 0; i < len;) {
                while((cnt = ov495_ReadReg8(cfg->base_addr + SPI_M_RX_FIFO_CNT))  == 0x00u){
                }

                for(j=0; j<cnt; j++)
                     *(buf + i+j) = ov495_ReadReg8(cfg->base_addr + SPI_M_RXD);

                i+=cnt;
            }
			//Sleep(1);
			while((ov495_ReadReg8(cfg->base_addr + SPI_M_INT_ST) & (unsigned char)BIT4) == 0u){//wait rx finished
			}
			ov495_WriteReg8(cfg->base_addr + SPI_M_INT_CLR, (unsigned char)BIT4);//clear rx status

			//when tx finish,re-assert spi_start and disable the spi
			ov495_WriteReg8(cfg->base_addr + SPI_M_START_CTRL, 0x00u); 
			ov495_WriteReg8(cfg->base_addr + SPI_M_CTRL, 0x00u); 
		}
	}
	return (signed int)len;
}


signed int libsif_write(t_libsif_cfg *cfg, unsigned char *buf, unsigned int len)
{
	unsigned int i, j, cnt;
	if(cfg->op_mode == SIF_MODE_MASTER) {
		//spi master configure
		ov495_WriteReg8(cfg->base_addr + SPI_M_DIV_RATE, cfg->div); 

		//open the spi interrupt,maybe now no use
		ov495_WriteReg8(cfg->base_addr + SPI_M_INT_EN, 0x3fu); 
		
		ov495_WriteReg8(cfg->base_addr + SPI_M_SLV_SEL, cfg->cs_sel); 

		ov495_WriteReg8(cfg->base_addr + SPI_M_CS_CNT,ov495_ReadReg8(cfg->base_addr + SPI_M_CS_CNT) & 0x0fu); 
		ov495_WriteReg8(cfg->base_addr + SPI_M_CS_CNT,ov495_ReadReg8(cfg->base_addr + SPI_M_CS_CNT) | (cfg->io_mode<<4)); 

		// configure the tx length and position
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_LL, (unsigned char)(len & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_LH, (unsigned char)((len >> 8u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_HL, (unsigned char)((len >> 16u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_HH, (unsigned char)((len >> 24u) & 0xffu));
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_START_L, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_TSIZE_START_H, 0u);
		
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_LL, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_LH, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_HL, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_HH, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_START_L, 0u);
		ov495_WriteReg8(cfg->base_addr + SPI_M_RSIZE_START_H, 0u);
		
		//spi enable
		ov495_WriteReg8(cfg->base_addr + SPI_M_CTRL, 
						(unsigned char)(BIT0 & (unsigned char)(~BIT1) & (unsigned char)(~BIT2) & (unsigned char)(~BIT3)) | (cfg->cpol << 1u) | (cfg->cpha << 2u) | (cfg->slsb << 3u)); 

		if(cfg->op_dma == SIF_OP_CPU) {
			ov495_WriteReg8(cfg->base_addr + SPI_M_START_CTRL, 0x04u);//spi start 

			// actual data transmit
			/*for(i = 0; i < len; i++) {
				while((ov495_ReadReg8(cfg->base_addr + SPI_M_FIFO_ST) & (unsigned char)BIT1) == (unsigned char)BIT1){
				}
				ov495_WriteReg8(cfg->base_addr + SPI_M_TXD, *(buf + i));
			}*/
			for(i = 0; i < len; ) {
				while((ov495_ReadReg8(cfg->base_addr + SPI_M_FIFO_ST) & (unsigned char)BIT1) == (unsigned char)BIT1){
				}

				cnt = (len-i>32)?32:(len-i);
				for (j=0; j<cnt; j++)
					ov495_WriteReg8(cfg->base_addr + SPI_M_TXD, *(buf + i+j));

				i+=cnt;
			}			
			//Sleep(1);
			while((ov495_ReadReg8(cfg->base_addr + SPI_M_INT_ST) & (unsigned char)BIT5) == 0u){//wait tx finished
			}
			ov495_WriteReg8(cfg->base_addr + SPI_M_INT_CLR, BIT5);//clear tx status
			
			// when tx finished, re-assert spi_start and disable the spi
			ov495_WriteReg8(cfg->base_addr + SPI_M_START_CTRL, 0x00u); 
			if(cfg->chain == 0u) {
				ov495_WriteReg8(cfg->base_addr + SPI_M_CTRL, 0x00u);//cs pin will be set to high 
			}
			//Sleep(1);
		}
	}

	return (signed int)len;
}



signed int sf_init(t_libsif_cfg *cfg, t_sf * sf)
{
	rom_g_sf = sf;
	ov495_WriteReg8(cfg->base_addr + SPI_M_DIV_RATE, cfg->div);//set spi rate
	return 0;
}

signed int SPI_Erase(t_libsif_cfg *cfg)
{
	signed int ret;

	ret = libsif_write(cfg, &rom_g_sf->cmd_write_enable, 1u); 
	ret = libsif_write(cfg, (unsigned char *)(&rom_g_sf->cmd_chip_erase), 1u); 

	return ret;
}


unsigned char sf_rd_status_reg(t_libsif_cfg *cfg)
{
	unsigned char sr;
	signed int ret;
	cfg->chain = 1u;
	ret = libsif_write(cfg, &rom_g_sf->cmd_read_status_reg1, 1u); 

	cfg->chain = 0u;
	cfg->rx_start = 0u;
	ret = libsif_read(cfg, (unsigned char *)&sr, 1u);

	if(ret != 0) {
		sr = 0u;
	}

	return sr;
}

signed int sf_wr_status_reg(t_libsif_cfg *cfg, unsigned char status)
{
	signed int ret;
	unsigned char cmd[2];
	cmd[0] = rom_g_sf->cmd_write_status_reg;
	cmd[1] = status;

	ret = libsif_write(cfg, cmd, 2u);

	return ret;
}

signed int SPI_Write(t_libsif_cfg *cfg, unsigned char *data, unsigned int len, unsigned int addr)
{
	signed int ret;

	ret = libsif_write(cfg, &rom_g_sf->cmd_write_enable, 1u); 

	unsigned char cmd[4];
	cmd[0] = rom_g_sf->cmd_page_program;
	cmd[1] = (unsigned char)((addr>>16u)&0xffu);
	cmd[2] = (unsigned char)((addr>>8u)&0xffu);
	cmd[3] = (unsigned char)(addr&0xffu);

	cfg->chain = 1u;
	ret = libsif_write(cfg, cmd, 4u); 

	cfg->chain = 0u;

	ret = libsif_write(cfg, data, len);

	while((sf_rd_status_reg(cfg)&(unsigned char)BIT0) == (unsigned char)BIT0){
	}

	return ret;
}

signed int SPI_Read(t_libsif_cfg *cfg, unsigned int addr, unsigned char *data, unsigned int len)
{
	signed int ret;
	unsigned char cmd[4];

	cmd[0] = 0x03u;
	cmd[1] = (unsigned char)((addr>>16u)&0xffu);
	cmd[2] = (unsigned char)((addr>>8u)&0xffu);
	cmd[3] = (unsigned char)(addr&0xffu);

	cfg->chain = 1u;
	ret = libsif_write(cfg, cmd, 4u);

	cfg->chain = 0u;
	cfg->rx_start = 0u;
	
	ret = libsif_read(cfg, data, len);

	cfg->rx_start = 0u;

	return ret;
}




signed int sf_rd_status(t_libsif_cfg *cfg, unsigned int reg, unsigned int txlen, unsigned int *des_addr, unsigned int len)
{
	signed int ret;
	unsigned char cmd[5];
	cmd[0] = (unsigned char)((reg>>24u)&0xffu) ;
	cmd[1] = (unsigned char)((reg>>16u)&0xffu) ;
	cmd[2] = (unsigned char)((reg>>8u )&0xffu) ;
	cmd[3] = (unsigned char)((reg    )&0xffu) ;
	cmd[4] =  0u;

	if((cfg->io_mode == IO2) || (cfg->io_mode ==IO4))
	{
       ov495_WriteReg8(cfg->base_addr+0x0du, 1u);
	}else if((cfg->io_mode == DPI) || (cfg->io_mode ==QPI)) {
       ov495_WriteReg8(cfg->base_addr+0x0du, txlen);  // cmd len
	}

	cfg->chain = 1u;
	ret = libsif_write(cfg, cmd, txlen);

	cfg->chain = 0u;
	if(cfg->io_mode == IO4){
		cfg->rx_start = 2u;
	}else{
		cfg->rx_start = 0u;
	}
	ret = libsif_read(cfg, (unsigned char *)(des_addr), len);
	cfg->rx_start = 0u;

	return ret;
}

signed int sf_wr_status(t_libsif_cfg *cfg, unsigned int reg, unsigned int len)
{
	signed int ret;
	unsigned char cmd[4];

	cmd[0] = (unsigned char)((reg>>24u)&0xffu) ;
	cmd[1] = (unsigned char)((reg>>16u)&0xffu) ;
	cmd[2] = (unsigned char)((reg>>8u )&0xffu) ;
	cmd[3] = (unsigned char)((reg    )&0xffu);

	cfg->chain = 0u;

	ret = libsif_write(cfg, cmd, len);

	cfg->rx_start = 0u;

	return ret;
}
