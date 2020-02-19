/**********************************************************************************************************************
Name:LIB_OV495.H
Function:Initial OV495 after reset CPU,other read write operation
Date:2016/07/19
**********************************************************************************************************************/
#ifndef LIB_OV495_HEADER
#define LIB_OV495_HEADER

#define   BIT0 		     0x00000001u
#define   BIT1 		     0x00000002u
#define   BIT2 		     0x00000004u
#define   BIT3 		     0x00000008u
#define   BIT4 		     0x00000010u
#define   BIT5 		     0x00000020u
#define   BIT6 		     0x00000040u
#define   BIT7 		     0x00000080u
#define   BIT8           0x00000100u
#define   BIT9           0x00000200u
#define   BIT10          0x00000400u
#define   BIT11          0x00000800u
#define   BIT12          0x00001000u
#define   BIT13          0x00002000u
#define   BIT14          0x00004000u
#define   BIT15          0x00008000u
#define   BIT16          0x00010000u
#define   BIT17          0x00020000u
#define   BIT18          0x00040000u
#define   BIT19          0x00080000u
#define   BIT20          0x00100000u
#define   BIT21          0x00200000u
#define   BIT22          0x00400000u
#define   BIT23          0x00800000u
#define   BIT24          0x01000000u
#define   BIT25          0x02000000u
#define   BIT26          0x04000000u
#define   BIT27          0x08000000u
#define   BIT28          0x10000000u
#define   BIT29          0x20000000u
#define   BIT30          0x40000000u
#define   BIT31          0x80000000u

///////////////////////////////////////////////////////////////////////////

#define SPI_BASE_ADDR			0x80207000u
#define SCCBS_BASE_ADDR			0x80203500u
#define SC_BASE_ADDR			0x80203000u

#define SIF_MODE_SLAVE   	(0x0u)
#define SIF_MODE_MASTER  	(0x1u)

#define SIF_OP_DMA 			(0x1u)
#define SIF_OP_CPU			(0x0u)
#define SPI                 (0u)
#define DPI                 (4u)
#define QPI                 (5u)
#define IO2                 (6u)
#define IO4                 (7u)


//------SPI AS MASTER------//
#define SPI_M_CTRL 			0x00u
#define SPI_M_START_CTRL	0x01u
#define SPI_M_TSIZE_LL 		0x02u
#define SPI_M_TSIZE_LH		0x09u
#define SPI_M_TSIZE_HL 		0x33u
#define SPI_M_TSIZE_HH		0x34u
#define SPI_M_TSIZE_START_L	0x03u
#define SPI_M_TSIZE_START_H	0x0au
#define SPI_M_RSIZE_LL 		0x04u
#define SPI_M_RSIZE_LH 		0x0bu
#define SPI_M_RSIZE_HL 		0x35u
#define SPI_M_RSIZE_HH 		0x36u
#define SPI_M_RSIZE_START_L	0x05u
#define SPI_M_RSIZE_START_H	0x0cu
#define SPI_M_DIV_RATE 		0x06u
#define SPI_M_CS_CNT 		0x07u
#define SPI_M_SLV_SEL 		0x08u
#define SPI_M_TXD			0x10u
#define SPI_M_RXD			0x11u
#define	SPI_M_FIFO_ST		0x12u
#define	SPI_M_INT_EN		0x20u
#define SPI_M_INT_ST		0x21u
#define SPI_M_TX_FIFO_CNT	0x22u
#define SPI_M_RX_FIFO_CNT	0x23u
#define SPI_M_INT_CLR		0x24u
#define SPI_M_TX_TH		    0x30u
#define SPI_M_RX_TH		    0x31u
//------SPI AS SLAVE-------//
#define SPI_S_CTRL 			0x00u
#define SPI_S_CTRL1 		0x01u
#define SPI_S_CTRL3 		0x03u
#define SPI_S_TXD 			0x04u
#define SPI_S_RXD 			0x05u
#define SPI_S_TX_TH		    0x06u
#define SPI_S_RX_TH		    0x07u


#define WRITE_ENABLE_CMD            0x06
#define WRITE_DISABLE_CMD           0x04
#define READ_STATUS_REG1_CMD        0x05
#define WRITE_STATUS_REG_CMD        0x01
#define READ_DATA_CMD               0x03
#define FAST_READ_CMD               0x0b
#define PAGE_PROGRAM_CMD            0x02
#define ERASE_SECTOR_CMD         	0xd7
#define ERASE_BLOCK_CMD             0xd8
#define CHIP_ERASE_CMD              0xc7
#define POWER_DOWN_CMD              0xb9
#define MANUFACTURER_CMD            0xab

#define SYSTEM_CLK	150000000
#define PAD_CLK		24000000//clock in OV495

#define I2C_BITRATE   400

void ov495_pll0_set(int ref_clk, int system_clk,int isp_clk ,int auto_en,int r_pll0_bypass,int r_pll0_prediv,\
		int r_pll0_dsm,int r_pll0_loopdivp,int r_pll0_loopdivs,int r_pll0_div0,int r_pll0_div1);

void ov495_WriteReg8(unsigned int Address,unsigned char value);

void ov495_WriteReg16(unsigned int Address,unsigned short value);

void ov495_WriteReg32(unsigned int Address,unsigned int value);

unsigned char ov495_ReadReg8(unsigned int Address);

unsigned short ov495_ReadReg16(unsigned int Address);

unsigned int ov495_ReadReg32(unsigned int Address);

#endif