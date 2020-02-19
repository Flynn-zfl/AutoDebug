#pragma once


typedef unsigned char BYTE;
//typedef char INT8;
typedef unsigned char UINT8;
//typedef int INT16;
//typedef unsigned int UINT16;
//typedef long INT32;
//typedef unsigned long UINT32;

#define SPI_READ	0x03
#define SPI_FREAD	0x0B
#define SPI_BE4		0x20
#define SPI_BE32	0x52
#define SPI_BE64	0xD8
#define SPI_CE1		0x60
#define SPI_CE2		0xC7
#define SPI_BP		0x02	// some chips byte program, some do page program
#define SPI_PP		0x02	// instruction code is the same
#define SPI_SP		0xAF
#define SPI_WREN	0x06
#define SPI_WRDI	0x04
#define SPI_PS		0x36
#define SPI_UPS		0x39
#define SPI_RSPR	0x3C
#define SPI_RDSR	0x05
#define SPI_WRSR	0x01
#define SPI_RDID	0x9F
#define SPI_DP		0xB9
#define SPI_RES		0xAB
#define SPI_RDSR_SPRL	0x80
#define SPI_RDSR_SPM	0x40
#define SPI_RDSR_WPP	0x10
#define SPI_RDSR_SWP1	0x08
#define SPI_RDSR_SWP0	0x04
#define SPI_RDSR_WEL	0x02
#define SPI_RDSR_BUSY	0x01

#define SPI_BASE			0x0300

#define REG_SPI_CTRL0		SPI_BASE+0x0
#define REG_SPI_CTRL1		SPI_BASE+0x1
#define REG_SPI_CTRL2		SPI_BASE+0x2
#define REG_SPI_CTRL3		SPI_BASE+0x3
#define REG_SPI_CTRL4		SPI_BASE+0x4
#define REG_SPI_CTRL5		SPI_BASE+0x5
#define REG_SPI_CTRL6		SPI_BASE+0x6
#define REG_SPI_DATA_IN		SPI_BASE+0x8
#define REG_SPI_DATA_OUT	SPI_BASE+0x8
#define REG_SPI_STATUS		SPI_BASE+0xC	// not used
#define REG_SPI_WBUFF_NFO	SPI_BASE+0xD
#define REG_SPI_RBUFF_NFO	SPI_BASE+0xE
#define SPI_RBUFF_NFO		SPI_BASE+0xE

#define SPI_SS_ASSERT	WriteReg16(0x0a, REG_SPI_CTRL3, 0x04);
#define SPI_SS_RELEASE	WriteReg16(0x0a, REG_SPI_CTRL3, 0x00);
#define SPI_FW_GRAB		WriteReg16(0x0a, REG_SPI_CTRL0, 0x03);
#define SPI_FW_RELEASE	WriteReg16(0x0a, REG_SPI_CTRL0, 0x02);

#define SPI_PAGE_OFFSET		0x1300

void SpiPollBusy();
UINT32 SpiRDID();
void SpiUnlock();
void SpiSectorErase(UINT8 sector);
void SpiChipErase();
void SpiWriteCmd(UINT32 addr);
void SpiReadCmdSlow(UINT32 addr);
void SpiReadCmdFast(UINT32 addr);
BYTE SpiGetByte();
void SpiPutByte(BYTE dat);
void SpiReadPage(UINT16 addr, UINT16 len);
void SpiWritePage(BYTE* data, UINT32 len);
bool CheckHeader(UINT8 *header, char *text);

void PollLo(UINT16 addr, BYTE bitmask);
void PollData(UINT16 addr, int match);
