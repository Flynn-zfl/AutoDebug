#include "..\\stdafx.h"
//#include "PrintExtension.h"
#include "OV496.h"
#include "..\\I2C\\i2c.h"

unsigned short OV496crc16_ccitt(unsigned char *buf, int len)
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

#define OV496_SYS_BASE        (0x8000)
#define OV496_SYS_SSRR        (0x0000)
#define OV496_SYS_CRSR        (0x0010)
#define OV496_SYS_WDMR        (0x0018)
#define OV496_SYS_WDCR        (0x001C)
#define OV496_SYS_SLSRR_0     (0x0088)

#define OV496_DUMMY_R  (0x9c00)    //0x80209c00, 512bytes
#define CMD_SPI_IDLE    0x00
#define CMD_SPI_WRITE   0x01
#define CMD_SPI_READ    0x02
typedef struct OV496BlockInfo{
	unsigned int nPramAddr;
	unsigned int nSpiAddr;
	unsigned int nOpCode;
} OV496sBlockinfo;
OV496sBlockinfo gBlockInfo[4];


#define HOSTCMD_CRC_ADDR      0x3000
#define HOSTCMD_START_ADDR    0x3002
#define HOSTCMD_RD_ADDR       0x3C00
#define HOSTCMD_TRIG_ADDR     0x8160
#define HOSTCMD_STATUS_ADDR   0x9c14

#define HOSTCMD_STATUS_DONE     0x99
#define HOSTCMD_STATUS_DOING    0x88
#define HOSTCMD_STATUS_ERR      0x55
#define HOSTCMD_STATUS_CRC_ERR  0x44
#define HOSTCMD_STATUS_RESET    0x00

BOOL OV496SendHostCmd(unsigned char nID, unsigned char *pBuf, unsigned int size)
{
	unsigned int   nPageAddr = 0;
	unsigned char  pCrcBuf[1024*3];
	unsigned short nCrcBufOffset = 0;
	unsigned short nCrcCnt = 0;	
	unsigned short CalCrc = 0;	
	unsigned char  nValue[2];
	unsigned short j = 0;
	unsigned char nSegment = 0;	

	//send host command			
	nCrcBufOffset = 0;
	for(j=0; j<size; j+=PAGE_SIZE){	
		if((size - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = size - j;
		}		
		nPageAddr = HOSTCMD_START_ADDR + j;		
		//BurstWriteRegister(nID, nPageAddr, pBuf + j, 1, nSegment);	
		BurstWriteRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);	
		for(nCrcCnt=0; nCrcCnt<nSegment; nCrcCnt++){
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 0] = (nPageAddr&0xff00)>>8;
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 1] = (nPageAddr&0xff);
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 2] = *(pBuf + j + nCrcCnt);
			nPageAddr ++;
		}
		nCrcBufOffset = nCrcBufOffset + 3*nSegment;
	}

	nPageAddr = HOSTCMD_TRIG_ADDR;  	
	pCrcBuf[nCrcBufOffset + 0] = (nPageAddr&0xff00)>>8;
	pCrcBuf[nCrcBufOffset + 1] = (nPageAddr&0xff);
	pCrcBuf[nCrcBufOffset + 2] = 0x01;
	nCrcBufOffset = nCrcBufOffset + 3;

	//write the crc value	
	CalCrc = OV496crc16_ccitt(pCrcBuf, nCrcBufOffset);		
	nValue[1] = CalCrc & 0xff;
	nValue[0] = (CalCrc>>8) & 0xff;		
	BurstWriteRegister(nID, HOSTCMD_CRC_ADDR, &nValue[0], 1, 2);

	//trigger host command int	
	nValue[0] = 0x01;
	BurstWriteRegister(nID, HOSTCMD_TRIG_ADDR, nValue, 1, 1); 

	//cmd status		
	unsigned char nTryTime = 0;
	nValue[0] = 0x00;
	BurstReadRegisterEx(nID, HOSTCMD_STATUS_ADDR, nValue, 1, 1);
	while(nValue[0] != HOSTCMD_STATUS_DONE){
		Sleep(1);
		nValue[0] = 0x00;			
		BurstReadRegisterEx(nID, HOSTCMD_STATUS_ADDR, nValue, 1, 1);
		nTryTime++;
		if(nTryTime >= 200){
			return FALSE;
		}			
	}	

	return TRUE;
}



void OV496InitRam(unsigned char nID)
{	
	int i = 0;	
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned int value32 = 0;	

	//step1: unmask sccbs access	
	BurstReadRegisterEx(nID, 0x81a0, &value, 1, 1);
	if(value == 0x96){
		value = 0x00;
		BurstWriteRegister(nID, 0x93c0, &value, 1, 1);		
	}else if(value == 0x94){
		value = 0x07;
		BurstWriteRegister(nID, 0x93c0, &value, 1, 1);		
	}else{
		value = 0x07;
		BurstWriteRegister(nID, 0x93c0, &value, 1, 1);		
		BurstReadRegisterEx(nID, 0x81a0, &value, 1, 1);
		if(value == 0x94){
		}else{
			value = 0x00;
			BurstWriteRegister(nID, 0x93c0, &value, 1, 1);
		}
	}	
	//end of step1
	
	//restore function closed by safety mode
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	value = 0xe8;
	BurstWriteRegister(nID, 0x8100, &value, 1, 1);		
	value = 0x00;
	BurstWriteRegister(nID, 0x8092, &value, 1, 1);
	//end of restore safety mode
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	value = 0x88;
	BurstWriteRegister(nID, OV496_SYS_BASE + OV496_SYS_SLSRR_0, &value, 1, 1);

	//clear watchdog status
	addr = OV496_SYS_BASE + OV496_SYS_CRSR;
	BurstReadRegisterEx(nID,  addr, &value, 1, 1);
	value = value & (~BIT1);
	BurstWriteRegister(nID, addr, &value, 1, 1);

	//clear watchdog count register
	value32 = 0;	
	addr = OV496_SYS_BASE + OV496_SYS_WDCR;
	BurstWriteRegister(nID, addr, (unsigned char *)(&value32), 1, 4);

	//set watchdog time period register and enable watchdog, wait for reboot.
	value32 = 0x3fffffff;	
	addr = OV496_SYS_BASE + OV496_SYS_WDMR;
	BurstWriteRegister(nID, addr, (unsigned char *)(&value32), 1, 4);	

	//step2: hold cpu	
	value = 0x88;
	BurstWriteRegister(nID, OV496_SYS_BASE + OV496_SYS_SLSRR_0, &value, 1, 1);
	unsigned char nTryTimes = 0;
	while(nTryTimes < 10){
		value = 0x00;
		BurstReadRegisterEx(nID, OV496_SYS_BASE + OV496_SYS_SLSRR_0, &value, 1, 1);	//read back
		nTryTimes ++;
		if(value == 0x88){
			break;
		}else{
			value = 0x88;
			BurstWriteRegister(nID, OV496_SYS_BASE + OV496_SYS_SLSRR_0, &value, 1, 1);
		}
	}

	//step3: clear 4 block dummy status registers(1 block is 4K byte)	
	i = sizeof(gBlockInfo);
	for(i=0; i<sizeof(gBlockInfo)/sizeof(gBlockInfo[0]); i++){
		gBlockInfo[i].nPramAddr = MAP_DATA_ADDR + SECT_SIZE*i;
		gBlockInfo[i].nSpiAddr = i * 0x1000;
		gBlockInfo[i].nOpCode = CMD_SPI_IDLE;
	}
	BurstWriteRegister(nID, OV496_DUMMY_R, (unsigned char *) (&gBlockInfo[0]), 1, sizeof(gBlockInfo));	

	//step4: mapping SCCBS 16K memory for downloading FW	
	addr = MAP_FW_ADDR;    //0x80180000
	BurstWriteRegister(nID, 0x9238, (unsigned char *) (&addr), 1, 4);
	addr = (MAP_FW_ADDR + 16*1024);    //0x80180000~16K for larger than 16K FW->0x4000 
	BurstWriteRegister(nID, 0x9244, (unsigned char *) (&addr), 1, 4);
	addr = 8*1024-1;    //0x80180000+16K~24K ->0x0000 
	BurstWriteRegister(nID, 0x9248, (unsigned char *) (&addr), 1, 4);	
}

void OV496RunRam(unsigned char nID)
{
	unsigned int addr = 0;
	unsigned char value = 0;

	// mapping SCCBS 16K memory for SPI data	
	addr = MAP_DATA_ADDR;    //make sure it is not conflict with small burn FW map
	BurstWriteRegister(nID, 0x9238, (unsigned char *) (&addr), 1, 4);		

	//run FW from pram
	addr = OV496_SYS_BASE + OV496_SYS_SSRR;
	value = value & (~0x3c);	
	value = value | 0x3c;	
	BurstWriteRegister(nID, addr, &value, 1, 1);			

	//step2: release from hold cpu	
	value = 0x80;
	BurstWriteRegister(nID, OV496_SYS_BASE + OV496_SYS_SLSRR_0, &value, 1, 1);
}

void OV496Reboot(unsigned char nID)
{
	//set register to reboot FW from SPI flash
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned int value32 = 0;	

	//clear watchdog status
	addr = OV496_SYS_BASE + OV496_SYS_CRSR;
	BurstReadRegisterEx(nID,  addr, &value, 1, 1);
	value = value & (~BIT1);
	BurstWriteRegister(nID, addr, &value, 1, 1);

	//clear watchdog count register
	value32 = 0;	
	addr = OV496_SYS_BASE + OV496_SYS_WDCR;
	BurstWriteRegister(nID, addr, (unsigned char *)(&value32), 1, 4);

	//set watchdog time period register and enable watchdog, wait for reboot.
	value32 = (BIT31 + BIT30 + 0x100);	
	addr = OV496_SYS_BASE + OV496_SYS_WDMR;
	BurstWriteRegister(nID, addr, (unsigned char *)(&value32), 1, 4);	
}

#define OV496_SCCB_BASE       (0x9200)
#define OV496_SCCBS_CRC_SET   (0x006A)
#define OV496_SCCBS_CRC_VALUE (0x0068)
BOOL OV496SendFwSector(unsigned char nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
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
	BurstReadRegisterEx(nID, OV496_DUMMY_R, (unsigned char *)(gBlockInfo), 1, sizeof(gBlockInfo));
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
	BurstWriteRegister(nID, OV496_SCCB_BASE + OV496_SCCBS_CRC_SET, nValue, 1, 1);	
	BurstReadRegisterEx(nID, OV496_SCCB_BASE + OV496_SCCBS_CRC_VALUE, nValue, 1, 2);	//clear SCCBS CRC value

	//write to internal 4K sram
	nCrcBufOffset = 0;
	for(j=0; j<nSect; j+=PAGE_SIZE){	
		if((nSect - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}		
		nPageAddr = MAP_SCCB_ADDR + (gBlockInfo[i].nPramAddr - MAP_DATA_ADDR) + j;		
		//BurstWriteRegister(nID, nPageAddr, pBuf + j, 1, nSegment);
		BurstWriteRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);	
		for(nCrcCnt=0; nCrcCnt<nSegment; nCrcCnt++){
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 0] = (nPageAddr&0xff00)>>8;
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 1] = (nPageAddr&0xff);
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 2] = *(pBuf + j + nCrcCnt);
			nPageAddr ++;
		}
		nCrcBufOffset = nCrcBufOffset + 3*nSegment;
	}

	//calculate and check with SCCBS crc value
	BurstReadRegisterEx(nID, OV496_SCCB_BASE + OV496_SCCBS_CRC_VALUE, nValue, 1, 2);
	SccbCrc = (nValue[1]<<8) +nValue[0];
	CalCrc = OV496crc16_ccitt(pCrcBuf, nCrcBufOffset);
	if(SccbCrc != CalCrc){
		return 0;		
	}

	//step3 start to burn it to spi flash	
	gBlockInfo[i].nSpiAddr = nFWoffset;
	gBlockInfo[i].nOpCode = CMD_SPI_WRITE;	
	//BurstWriteRegister(nID, OV496_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));			
	BurstWriteRegisterEx(nID, OV496_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));			

	return 1;
}

void OV496GetFwSector(unsigned char nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned char i = 0;
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned char nRamBlock = 0;
	unsigned int nPageAddr = 0;

	//step1 get pram status from dummy register	
	BurstReadRegisterEx(nID, OV496_DUMMY_R, (unsigned char *)(gBlockInfo), 1, sizeof(gBlockInfo));
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
	BurstWriteRegister(nID, OV496_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));			

	//step3 wait for one sector/4k data is ready in pram	
	while(gBlockInfo[i].nOpCode != CMD_SPI_IDLE){
		BurstReadRegisterEx(nID, OV496_DUMMY_R + i * sizeof(gBlockInfo[i]), (unsigned char *)(&gBlockInfo[i]), 1, sizeof(gBlockInfo[i]));	
	}			

	//step4 get data from internal pram
	for(j=0; j<nSect; j+=PAGE_SIZE){
		if((nSect - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}
		nPageAddr = MAP_SCCB_ADDR + (gBlockInfo[i].nPramAddr - MAP_DATA_ADDR) + j;    //for internal 16K mapping ram test
		BurstReadRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);	
		//Sleep(TIMER_INTERVAL);
	}			
}

BOOL OV496SendFwRam(unsigned char nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
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

	//start OV496 SCCBS CRC	
	nValue[0] = 0x00;
	BurstWriteRegister(nID, OV496_SCCB_BASE + OV496_SCCBS_CRC_SET, nValue, 1, 1);	
	BurstReadRegisterEx(nID, OV496_SCCB_BASE + OV496_SCCBS_CRC_VALUE, nValue, 1, 2);	//clear SCCBS CRC value

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

		//BurstWriteRegister(nID, nPageAddr, pBuf + j, 1, nSegment);
		BurstWriteRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);

		for(nCrcCnt=0; nCrcCnt<nSegment; nCrcCnt++){
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 0] = (nPageAddr&0xff00)>>8;
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 1] = (nPageAddr&0xff);
			pCrcBuf[nCrcBufOffset + nCrcCnt*3 + 2] = *(pBuf + j + nCrcCnt);
			nPageAddr ++;
		}
		nCrcBufOffset = nCrcBufOffset + 3*nSegment;
	}
	//calculate and check with SCCBS crc value
	BurstReadRegisterEx(nID, OV496_SCCB_BASE + OV496_SCCBS_CRC_VALUE, nValue, 1, 2);
	SccbCrc = (nValue[1]<<8) +nValue[0];
	CalCrc = OV496crc16_ccitt(pCrcBuf, nCrcBufOffset);
	if(SccbCrc != CalCrc){
		return 0;		
	}	

	return 1;
}

void OV496GetFwRam(unsigned char nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
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
		//BurstReadRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);	
		BurstReadRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);	
	}		
}