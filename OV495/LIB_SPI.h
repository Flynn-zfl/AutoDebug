/**********************************************************************************************************************
Name:LIB_SPI.H
Function:SPI flash operation allocated
Date:2016/07/19
**********************************************************************************************************************/
#ifndef LIB_SPI_HEADER
#define LIB_SPI_HEADER

typedef struct {
	void (*device_init)(void);
	unsigned int page_size;
	unsigned int sector_size;
	unsigned int block_size;
	unsigned int op_size;
	unsigned int timeout;
	unsigned char fDMA;
	unsigned char deviceID;
	unsigned char cmd_read_status_reg1;
	unsigned char cmd_write_disable;
	unsigned char cmd_write_enable;
	unsigned char cmd_deviceid;
	unsigned char deviceid_dummy_bytes;
	unsigned char cmd_chip_erase;
	unsigned char cmd_erase_sector;
	unsigned char cmd_erase_block;
	unsigned char cmd_fast_read;
	unsigned char cmd_read_data;
	unsigned char cmd_page_program;
	unsigned char cmd_write_status_reg;
}t_sf;

typedef struct{
	volatile unsigned int base_addr;              ///< sif control base address, sif library will set this.
    unsigned char op_mode;                 ///< sif mode: slave mode or master mode. app will set it.
//INTERNAL_START
    unsigned char trans_mode;              ///< sif transmit mode: 1.receive mode 2.transmit mode 3.transmit+receive mode. sif library will set it.
//INTERNAL_END
    unsigned char cpol;                    ///< clock polarity. cpol=0: Actived SCL is high, in idle modeSCL is low. cpol=1: Actived SCL is low, in idle mode SCL is high. 
    unsigned char cpha;                    ///< clock phase. cpha=0: sampleing of data occurs at odd edge(1, 3, 5,7....) of sck clock. cpha=1: sampleing of data occurs at even edge(2, 4, 6, 8....) of sck clock 
    unsigned char slsb;                    ///< shift lsb firstly. slsb=1: Dtata is transferred least significant bit first. slsb=0: Data is transferred most significant bit first. 
    unsigned char div;                		///< this param's value ranges from 0 to 15. Each value has corresponding clock divider value.  sif clock = system clock/(divider value). The system clock can be got by calling sysclk_get(). sif library will calculate this param from sif clock.
    unsigned char op_dma;                  ///< dma or cpu operation. app will set it.
//INTERNAL_START
    unsigned char ss_mode;                 ///< the mode of ss signal
    unsigned char endian;                  ///< data endian(big endian or small endian) 
    unsigned char intr_en;                 ///< enable sif interrupt
    unsigned char intr_fifo_cnt;           ///< fifo count when interrupt occurs
//INTERNAL_END
    unsigned char dc_enabled;              ///< enable data cache
    unsigned char cs_sel;					///< cs pin select. set to 0 or 1.
//INTERNAL_START
    int timeout;                ///< software timeout   
    void (*sif_dma_irq)(void);  ///< dma interrupt callback function
    void (*signal_trig)(void);  ///< triger signal to start sif 
	unsigned short dma_wait;				///< 0:don't use dma wait, >0:dma wait timeout 
//INTERNAL_END
	unsigned char dma_32bit_en;			///< 0: dma 32bit W/R disable, 1: dma 32bit W/R enable

	unsigned char chain;					///<use for sif cs pin control in blocks, 0: "spi enable and start" bits be set to 0 when tx finish, 1: only "spi start" be set to 0.
	unsigned short rx_start;				///<rx start size in one block
	unsigned char  io_mode;                ///< 0 normal mode ,4 dpi,5 qpi 6 2xio 7 4xio
}t_libsif_cfg;

typedef enum{
	ERASE_SECTOR,///< erase sector
	ERASE_BLOCK,///< erase block
	ERASE_CHIP///< erase whole chip
}ERASE_TYPE;

void spi_flash_parameter_initial(void);
signed int sf_init(t_libsif_cfg *cfg, t_sf * sf);

signed int SPI_Erase(t_libsif_cfg *cfg);
signed int SPI_Write(t_libsif_cfg *cfg, unsigned char *data, unsigned int len, unsigned int addr);
signed int SPI_Read(t_libsif_cfg *cfg, unsigned int addr, unsigned char *data, unsigned int len);

signed int libsif_hw_init(t_libsif_cfg *cfg);
signed int libsif_init(unsigned int in_clk, t_libsif_cfg *cfg, unsigned char dma, unsigned char master, unsigned int clk);
signed int libsif_read(t_libsif_cfg *cfg, unsigned char *buf, unsigned int len);
signed int libsif_write(t_libsif_cfg *cfg, unsigned char *buf, unsigned int len);

unsigned char sf_rd_status_reg(t_libsif_cfg *cfg);
signed int sf_wr_status_reg(t_libsif_cfg *cfg, unsigned char status);
signed int sf_rd_status(t_libsif_cfg *cfg, unsigned int reg, unsigned int txlen, unsigned int *des_addr, unsigned int len);
signed int sf_wr_status(t_libsif_cfg *cfg, unsigned int reg, unsigned int len);

#endif