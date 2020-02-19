#ifndef __SPI__DEFINE_
#define __SPI__DEFINE_

typedef enum{
	ERASE_SECTOR,///< erase sector
	ERASE_BLOCK,///< erase block
	ERASE_CHIP///< erase whole chip
}ERASE_TYPE;

#define DELAY_LOOP Sleep

typedef unsigned char  		u8;  
typedef unsigned short 		u16; 
typedef unsigned int  		u32; 

#define SCCBM0 0
#define SCCBM1 1
#define ADDR16 0
#define ADDR8  1

#define SIF_MODE_SLAVE   	0x0
#define SIF_MODE_MASTER  	0x1

#define SIF_OP_DMA 			0x1
#define SIF_OP_CPU			0x0

//------SPI AS MASTER------//
#define SPI_M_CTRL 			0x00
#define SPI_M_START_CTRL	0x01
#define SPI_M_TSIZE_L 		0x02
#define SPI_M_TSIZE_H		0x09
#define SPI_M_TSIZE_START_L	0x03
#define SPI_M_TSIZE_START_H	0x0a
#define SPI_M_RSIZE_L 		0x04
#define SPI_M_RSIZE_H 		0x0b
#define SPI_M_RSIZE_START_L	0x05
#define SPI_M_RSIZE_START_H	0x0c
#define SPI_M_DIV_RATE 		0x06
#define SPI_M_CS_CNT 		0x07
#define SPI_M_SLV_SEL 		0x08
#define SPI_M_TXD			0x10
#define SPI_M_RXD			0x11
#define	SPI_M_FIFO_ST		0x12
#define	SPI_M_INT_EN		0x20
#define SPI_M_INT_ST		0x21
#define SPI_M_TX_FIFO_CNT	0x22
#define SPI_M_RX_FIFO_CNT	0x23
#define SPI_M_INT_CLR		0x24
#define SPI_M_TX_TH		    0x30
#define SPI_M_RX_TH		    0x31
//------SPI AS SLAVE-------//
#define SPI_S_CTRL 			0x00
#define SPI_S_CTRL1 		0x01
#define SPI_S_CTRL3 		0x03
#define SPI_S_TXD 			0x04
#define SPI_S_RXD 			0x05
#define SPI_S_TX_TH		    0x06
#define SPI_S_RX_TH		    0x07

////////////////////////////////////////////////////
#define   BIT0 		     0x00000001
#define   BIT1 		     0x00000002
#define   BIT2 		     0x00000004
#define   BIT3 		     0x00000008
#define   BIT4 		     0x00000010
#define   BIT5 		     0x00000020
#define   BIT6 		     0x00000040
#define   BIT7 		     0x00000080
#define   BIT8           0x00000100
#define   BIT9           0x00000200
#define   BIT10          0x00000400
#define   BIT11          0x00000800
#define   BIT12          0x00001000
#define   BIT13          0x00002000
#define   BIT14          0x00004000
#define   BIT15          0x00008000
#define   BIT16          0x00010000
#define   BIT17          0x00020000
#define   BIT18          0x00040000
#define   BIT19          0x00080000
#define   BIT20          0x00100000
#define   BIT21          0x00200000
#define   BIT22          0x00400000
#define   BIT23          0x00800000
#define   BIT24          0x01000000
#define   BIT25          0x02000000
#define   BIT26          0x04000000
#define   BIT27          0x08000000
#define   BIT28          0x10000000
#define   BIT29          0x20000000
#define   BIT30          0x40000000
#define   BIT31          0x80000000

typedef struct s_sf{
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
    u8 div;                		///< this param's value ranges from 0 to 15. Each value has corresponding clock divider value.  sif clock = system clock/(divider value). The system clock can be got by calling sysclk_get(). sif library will calculate this param from sif clock.
    u8 cs_sel;					///< cs pin select. set to 0 or 1.
}t_libsif_cfg;

#define PAD_CLK					24000000u

void spiflash_init(t_sf* sf);

int sccb_sif_hw_init(t_libsif_cfg *cfg, t_sf * sf);
int sccb_sf_erase(unsigned int flash_addr, unsigned char erase_type);
void sccb_sf_rd_data( unsigned int src_addr, unsigned int *des_addr,unsigned int len);
void sccb_sf_page_program(u32 * rd_addr, unsigned int len, u32 * wr_addr);
unsigned char sf_read_deviceID();

extern unsigned char Read_ISP_Reg8(unsigned int Address);
extern void Write_ISP_Reg8(unsigned int Address,unsigned char value);

#endif 

