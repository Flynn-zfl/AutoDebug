#include "stdafx.h"
//#include "PrintExtension.h"
#include "OV494.h"
#include "i2c.h"

unsigned short crc16_ccitt(unsigned char *buf, int len)
{
	int counter;
	unsigned short crc = 0;
	for( counter = 0; counter < len; counter++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *buf++)&0x00FF];
	return crc;
}

#define MAP_SCCB_ADDR_16K    (0x0000)
#define MAP_SCCB_ADDR    (0x4000)
#define SRAM_BLOCK_0     (0)
#define SRAM_BLOCK_1     (4)
#define MAP_FW_ADDR      (0x80180000)
#define MAP_DATA_ADDR    (0x80180000+4096*32)

#define OV494_SYS_BASE        (0x8000)
#define OV494_SYS_SSRR        (0x0000)
#define OV494_SYS_CRSR        (0x0010)
#define OV494_SYS_WDMR        (0x0018)
#define OV494_SYS_WDCR        (0x001C)
#define OV494_SYS_SLSRR_0     (0x0088)

#define OV494_DUMMY_R  (0x9c00)    //0x80209c00, 512bytes
#define CMD_SPI_IDLE    0x00
#define CMD_SPI_WRITE   0x01
#define CMD_SPI_READ    0x02
typedef struct BlockInfo{
	unsigned int nPramAddr;
	unsigned int nSpiAddr;
	unsigned int nOpCode;
} sBlockinfo;
sBlockinfo gBlockInfo[4];
void OV494InitRam()
{	
	int i = 0;	
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned int value32 = 0;	

	//step1: disable WD	
	//hold cpu
	value = 0x88;
	BurstWriteRegister(0x48, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);

	//clear watchdog status
	addr = OV494_SYS_BASE + OV494_SYS_CRSR;
	BurstReadRegister(0x48,  addr, &value, 1, 1);
	value = value & (~BIT1);
	BurstWriteRegister(0x48, addr, &value, 1, 1);

	//clear watchdog count register
	value32 = 0;	
	addr = OV494_SYS_BASE + OV494_SYS_WDCR;
	BurstWriteRegister(0x48, addr, (unsigned char *)(&value32), 1, 4);

	//set watchdog time period register and enable watchdog, wait for reboot.
	value32 = 0x3fffffff;	
	addr = OV494_SYS_BASE + OV494_SYS_WDMR;
	BurstWriteRegister(0x48, addr, (unsigned char *)(&value32), 1, 4);	

	//step2: hold cpu	
	value = 0x88;
	BurstWriteRegister(0x48, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);
	unsigned char nTryTimes = 0;
	while(nTryTimes < 10){
		value = 0x00;
		BurstReadRegister(0x48, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);	//read back
		nTryTimes ++;
		if(value == 0x88){
			break;
		}else{
			value = 0x88;
			BurstWriteRegister(0x48, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);
		}
	}

	//step3: clear 4 block dummy status registers(1 block is 4K byte)	
	i = sizeof(gBlockInfo);
	for(i=0; i<sizeof(gBlockInfo)/sizeof(gBlockInfo[0]); i++){
		gBlockInfo[i].nPramAddr = MAP_DATA_ADDR + SECT_SIZE*i;
		gBlockInfo[i].nSpiAddr = i * 0x1000;
		gBlockInfo[i].nOpCode = CMD_SPI_IDLE;
	}
	BurstWriteRegister(0x48, OV494_DUMMY_R, (unsigned char *) (&gBlockInfo[0]), 1, sizeof(gBlockInfo));	

	//step4: mapping SCCBS 16K memory for downloading FW	
	addr = MAP_FW_ADDR;    //0x80180000
	BurstWriteRegister(0x48, 0x9238, (unsigned char *) (&addr), 1, 4);
	addr = (MAP_FW_ADDR + 16*1024);    //0x80180000~16K for larger than 16K FW->0x4000 
	BurstWriteRegister(0x48, 0x9244, (unsigned char *) (&addr), 1, 4);
	addr = 8*1024-1;    //0x80180000+16K~24K ->0x0000 
	BurstWriteRegister(0x48, 0x9248, (unsigned char *) (&addr), 1, 4);	
}

void OV494RunRam()
{
	unsigned int addr = 0;
	unsigned char value = 0;

	// mapping SCCBS 16K memory for SPI data	
	addr = MAP_DATA_ADDR;    //make sure it is not conflict with small burn FW map
	BurstWriteRegister(0x48, 0x9238, (unsigned char *) (&addr), 1, 4);		

	//run FW from pram
	addr = OV494_SYS_BASE + OV494_SYS_SSRR;
	value = value & (~0x3c);	
	value = value | 0x3c;	
	BurstWriteRegister(0x48, addr, &value, 1, 1);			

	//step2: release from hold cpu	
	value = 0x80;
	BurstWriteRegister(0x48, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);
}

void OV494Reboot()
{
	//set register to reboot FW from SPI flash
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned int value32 = 0;	

	//clear watchdog status
	addr = OV494_SYS_BASE + OV494_SYS_CRSR;
	BurstReadRegister(0x48,  addr, &value, 1, 1);
	value = value & (~BIT1);
	BurstWriteRegister(0x48, addr, &value, 1, 1);

	//clear watchdog count register
	value32 = 0;	
	addr = OV494_SYS_BASE + OV494_SYS_WDCR;
	BurstWriteRegister(0x48, addr, (unsigned char *)(&value32), 1, 4);

	//set watchdog time period register and enable watchdog, wait for reboot.
	value32 = (BIT31 + BIT30 + 0x100);	
	addr = OV494_SYS_BASE + OV494_SYS_WDMR;
	BurstWriteRegister(0x48, addr, (unsigned char *)(&value32), 1, 4);	
}

#define OV494_SCCB_BASE       (0x9200)
#define OV494_SCCBS_CRC_SET   (0x006A)
#define OV494_SCCBS_CRC_VALUE (0x0068)
BOOL SendFwSector(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned char i = 0;
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned char nRamBlock = 0;
	unsigned int nPageAddr = 0;
	unsigned char pCrcBuf[8*1024*3];
	unsigned int nCrcBufOffset = 0;
	unsigned int nCrcCnt = 0;
	unsigned short SccbCrc = 0;
	unsigned short CalCrc = 0;
	unsigned char nValue[4];	
	unsigned char nOV = 0;
	DWORD dwReturn = 0;

	//step1 get pram status from dummy register	
	BurstReadRegister(nID, OV494_DUMMY_R, (unsigned char *)(gBlockInfo), 1, sizeof(gBlockInfo));
	for(i=0; i<sizeof(gBlockInfo)/sizeof(gBlockInfo[0]); i++){
		if(gBlockInfo[i].nOpCode == CMD_SPI_IDLE){
			break;
		}
	}
	if(i >= sizeof(gBlockInfo)/sizeof(gBlockInfo[0])){
		return 0;
	}

	//step2 send data to internal pram by sccbs
	nValue[0] = 0x00;
	BurstWriteRegister(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_SET, nValue, 1, 1);	
	BurstReadRegister(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_VALUE, nValue, 1, 2);	//clear SCCBS CRC value

	//write to internal 4K sram
	nCrcBufOffset = 0;
	for(j=0; j<nSect; j+=PAGE_SIZE){	
		if((nSect - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}		
		nPageAddr = MAP_SCCB_ADDR + (gBlockInfo[i].nPramAddr - MAP_DATA_ADDR) + j;		
		BurstWriteRegister(nID, nPageAddr, pBuf + j, 1, nSegment);			
		for(nCrcCnt=0; nCrcCnt<nSegment; nCrcCnt++){
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 0] = (nPageAddr&0xff00)>>8;
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 1] = (nPageAddr&0xff);
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 2] = *(pBuf + j + nCrcCnt);
			nPageAddr ++;
		}
		nCrcBufOffset = nCrcBufOffset + 3*nSegment;
	}

	//calculate and check with SCCBS crc value
	BurstReadRegister(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_VALUE, nValue, 1, 2);
	SccbCrc = (nValue[1]<<8) +nValue[0];
	CalCrc = crc16_ccitt(pCrcBuf, nCrcBufOffset);
	if(SccbCrc != CalCrc){
		return 0;		
	}

	//step3 start to burn it to spi flash	
	gBlockInfo[i].nSpiAddr = nFWoffset;
	gBlockInfo[i].nOpCode = CMD_SPI_WRITE;	
	BurstWriteRegister(nID, OV494_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));			

	return 1;
}

void GetFwSector(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned char i = 0;
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned char nRamBlock = 0;
	unsigned int nPageAddr = 0;

	//step1 get pram status from dummy register	
	BurstReadRegister(nID, OV494_DUMMY_R, (unsigned char *)(gBlockInfo), 1, sizeof(gBlockInfo));
	for(i=0; i<sizeof(gBlockInfo)/sizeof(gBlockInfo[0]); i++){
		if(gBlockInfo[i].nOpCode == CMD_SPI_IDLE){
			break;
		}
	}
	if(i >= sizeof(gBlockInfo)/sizeof(gBlockInfo[0])){
		return;
	}	

	//step2 get data from spi flash to internal pram	
	gBlockInfo[i].nSpiAddr = nFWoffset;
	gBlockInfo[i].nOpCode = CMD_SPI_READ;	
	BurstWriteRegister(nID, OV494_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));			

	//step3 wait for one sector/4k data is ready in pram	
	while(gBlockInfo[i].nOpCode != CMD_SPI_IDLE){
		BurstReadRegister(nID, OV494_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));	
	}			

	//step4 get data from internal pram
	for(j=0; j<nSect; j+=PAGE_SIZE){
		if((nSect - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}
		nPageAddr = MAP_SCCB_ADDR + (gBlockInfo[i].nPramAddr - MAP_DATA_ADDR) + j;    //for internal 16K mapping ram test
		BurstReadRegister(nID, nPageAddr, pBuf + j, 1, nSegment);	
		//Sleep(TIMER_INTERVAL);
	}			
}

BOOL SendFwRam(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned char nRamBlock = 0;
	unsigned int nPageAddr = 0;
	unsigned char pCrcBuf[8*1024*3];
	unsigned int nCrcBufOffset = 0;
	unsigned int nCrcCnt = 0;
	unsigned short SccbCrc = 0;
	unsigned short CalCrc = 0;
	unsigned char nValue[4];

	//start OV494 SCCBS CRC	
	nValue[0] = 0x00;
	BurstWriteRegister(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_SET, nValue, 1, 1);	
	BurstReadRegister(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_VALUE, nValue, 1, 2);	//clear SCCBS CRC value

	//write to internal 4K sram
	nCrcBufOffset = 0;
	for(j=0; j<nSect; j+=PAGE_SIZE){	
		if((nSect - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}		

		//for burn FW larger 16K
		if(nFWoffset < 16*1024){
			nPageAddr = MAP_SCCB_ADDR+nFWoffset + j;
		}else{
			nPageAddr = MAP_SCCB_ADDR_16K + (nFWoffset-16*1024) + j;
		}		

		BurstWriteRegister(nID, nPageAddr, pBuf + j, 1, nSegment);

		for(nCrcCnt=0; nCrcCnt<nSegment; nCrcCnt++){
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 0] = (nPageAddr&0xff00)>>8;
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 1] = (nPageAddr&0xff);
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 2] = *(pBuf + j + nCrcCnt);
			nPageAddr ++;
		}
		nCrcBufOffset = nCrcBufOffset + 3*nSegment;
	}
	//calculate and check with SCCBS crc value
	BurstReadRegister(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_VALUE, nValue, 1, 2);
	SccbCrc = (nValue[1]<<8) +nValue[0];
	CalCrc = crc16_ccitt(pCrcBuf, nCrcBufOffset);
	if(SccbCrc != CalCrc){
		return 0;		
	}	

	return 1;
}

void GetFwRam(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned int nPageAddr = 0;

	for(j=0; j<nSect; j+=PAGE_SIZE){
		if((nSect - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}							
		nPageAddr = MAP_SCCB_ADDR+nFWoffset + j;    //for internal 16K mapping ram test		
		BurstReadRegister(nID, nPageAddr, pBuf + j, 1, nSegment);			
	}		
}