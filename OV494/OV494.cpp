#include "..\\stdafx.h"
#include "OV494.h"
#include "..\\I2C\\i2c.h"

//these two SCCB/I2C access functions need to be realized by customer on their platform
//unsigned char BurstReadRegister(unsigned char id, unsigned int addr, unsigned char *pdata, unsigned char addr_16_8, unsigned char len)
//{
//	return 1;
//}
//
//unsigned char BurstWriteRegisterEx(unsigned char id, unsigned int addr, unsigned char *pdata, unsigned char addr_16_8, unsigned char len)
//{
//	return 1;
//}
//end


tSF_STA_INFO gSfStaInfo;
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

void OV494InitRam(unsigned char nID)
{	
	int i = 0;	
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned int value32 = 0;	

	//step1: unmask sccbs access	
	//value = 0x07;
	//BurstWriteRegisterEx(0x48, 0x93c0, &value, 1, 1);	
	BurstReadRegister(nID, 0x81a0, &value, 1, 1);
	if(value == 0x96){
		value = 0x00;
		BurstWriteRegisterEx(nID, 0x93c0, &value, 1, 1);		
	}else if(value == 0x94){
		value = 0x07;
		BurstWriteRegisterEx(nID, 0x93c0, &value, 1, 1);		
	}else{
		value = 0x07;
		BurstWriteRegisterEx(nID, 0x93c0, &value, 1, 1);		
		BurstReadRegister(nID, 0x81a0, &value, 1, 1);
		if(value == 0x94){
		}else{
			value = 0x00;
			BurstWriteRegisterEx(nID, 0x93c0, &value, 1, 1);
		}
	}	
	//end of step1

	//restore function closed by safety mode
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	value = 0xe8;
	BurstWriteRegisterEx(nID, 0x8100, &value, 1, 1);		
	value = 0x00;
	BurstWriteRegisterEx(nID, 0x8092, &value, 1, 1);
	//end of restore safety mode
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//step2:
	value32 = 0x3fffffff;	
	addr = OV494_SYS_BASE + OV494_SYS_WDMR;
	BurstWriteRegisterEx(nID, addr, (unsigned char *)(&value32), 1, 4);	

	addr = OV494_SYS_BASE + OV494_SYS_CRSR;	
	value = 0x00;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	value32 = 0;	
	addr = OV494_SYS_BASE + OV494_SYS_WDCR;
	BurstWriteRegisterEx(nID, addr, (unsigned char *)(&value32), 1, 4);	
	//end of step2
	
	//step3:
	value = 0x88;
	BurstWriteRegisterEx(nID, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);
	unsigned char nTryTimes = 0;
	while(nTryTimes < 10){
		value = 0x00;
		BurstReadRegister(nID, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);	//read back
		nTryTimes ++;
		if(value == 0x88){
			break;
		}else{
			value = 0x88;
			BurstWriteRegisterEx(nID, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);
		}
	}
	//end of step3
	
	//step4:
	addr = MAP_FW_ADDR;    //0x80180000
	BurstWriteRegisterEx(nID, 0x9238, (unsigned char *) (&addr), 1, 4);
	addr = (MAP_FW_ADDR + 16*1024);    //0x80180000~16K for larger than 16K FW->0x4000 
	BurstWriteRegisterEx(nID, 0x9244, (unsigned char *) (&addr), 1, 4);
	addr = 8*1024-1;    //0x80180000+16K~24K ->0x0000 
	BurstWriteRegisterEx(nID, 0x9248, (unsigned char *) (&addr), 1, 4);	
	//end of step4
}

BOOL OV494RunRam(unsigned char nID)
{
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned char nFWflag[4];

	//step1:
	nFWflag[0] = 0;
	nFWflag[1] = 0;
	nFWflag[2] = 0;
	nFWflag[3] = 0;
	BurstWriteRegisterEx(nID, 0x3C00, &nFWflag[0], 1, 4);

	//step2:
	addr = OV494_SYS_BASE + OV494_SYS_SSRR;		//0x8000
	value = value & (~0x3c);	
	value = value | 0x3c;	
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);
	//end of step2

	//step3:
	value = 0x80;
	BurstWriteRegisterEx(nID, OV494_SYS_BASE + OV494_SYS_SLSRR_0, &value, 1, 1);	//8088
	//end of step3

	//step4:
	unsigned short nTryTime = 0;
	nFWflag[0] = 0;
	nFWflag[1] = 0;
	nFWflag[2] = 0;
	nFWflag[3] = 0;
	BurstReadRegister(nID, 0x3C00,  &nFWflag[0], 1, 4);
	while(((nFWflag[0]<<24)+(nFWflag[1]<<16)+(nFWflag[2]<<8)+(nFWflag[3]<<0)) != 0x11223344){
		Sleep(50);
		nFWflag[0] = 0;
		nFWflag[1] = 0;
		nFWflag[2] = 0;
		nFWflag[3] = 0;	
		BurstReadRegister(nID, 0x3C00,  &nFWflag[0], 1, 4);
		nTryTime++;
		if(nTryTime >= 100){
			return FALSE;
		}			
	}	
	//end of step4
	return TRUE;
}

void OV494Reboot(unsigned char nID)
{
	unsigned int addr = 0;
	unsigned char value = 0;
	unsigned int value32 = 0;	

	//step1:
	addr = OV494_SYS_BASE + OV494_SYS_CRSR;
	value = 0x00;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);
	//end of step1

	//step2:
	value32 = 0;	
	addr = OV494_SYS_BASE + OV494_SYS_WDCR;
	BurstWriteRegisterEx(nID, addr, (unsigned char *)(&value32), 1, 4);

	value32 = (BIT31 + BIT30 + 0x100);	
	addr = OV494_SYS_BASE + OV494_SYS_WDMR;
	BurstWriteRegisterEx(nID, addr, (unsigned char *)(&value32), 1, 4);	
	//end of step2
}

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
BOOL SendHostCmd(unsigned char nID, unsigned char *pBuf, unsigned int size)
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
		//BurstWriteRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);	
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
	CalCrc = crc16_ccitt(pCrcBuf, nCrcBufOffset);		
	nValue[1] = CalCrc & 0xff;
	nValue[0] = (CalCrc>>8) & 0xff;		
	BurstWriteRegisterEx(nID, HOSTCMD_CRC_ADDR, &nValue[0], 1, 2);

	//trigger host command int	
	nValue[0] = 0x01;
	BurstWriteRegisterEx(nID, HOSTCMD_TRIG_ADDR, nValue, 1, 1); 
	//Sleep(5);
	//cmd status		
	unsigned char nTryTime = 0;
	nValue[0] = 0x00;
	BurstReadRegister(nID, HOSTCMD_STATUS_ADDR, nValue, 1, 1);
	while(nValue[0] != HOSTCMD_STATUS_DONE){
		Sleep(1);
		//if(nValue[0] == HOSTCMD_STATUS_RESET) return FALSE;
		if(nValue[0] == HOSTCMD_STATUS_CRC_ERR) return FALSE;
		if(nValue[0] == HOSTCMD_STATUS_ERR) return FALSE;

		nValue[0] = 0x00;			
		BurstReadRegister(nID, HOSTCMD_STATUS_ADDR, nValue, 1, 1);
		nTryTime++;
		if(nTryTime >= 200){
			return FALSE;
		}			
	}	
	return TRUE;
}

void GetDataFromDevice(unsigned char nID, unsigned char *pBuf, unsigned int size)
{
	unsigned int   nPageAddr = 0;
	unsigned short j = 0;
	unsigned char nSegment = 0;	

	for(j=0; j<size; j+=PAGE_SIZE){	
		if((size - j) >= PAGE_SIZE){
			nSegment = PAGE_SIZE;
		}else{
			nSegment = size - j;
		}		
		nPageAddr = HOSTCMD_RD_ADDR + j;		
		//BurstReadRegister(nID, nPageAddr, pBuf + j, 1, nSegment);	
		BurstReadRegister(nID, nPageAddr, pBuf + j, 1, nSegment);	
	}
}

#define HOSTCMD_SF_ID       0xa5
#define HOSTCMD_SF_SID_CT   0x0f
#define HOSTCMD_SF_SID_ER   0x10
#define HOSTCMD_SF_SID_PP   0x11
#define HOSTCMD_SF_SID_PR   0x12

#define SF_CHIP_ER    0x00
#define SF_SECT_ER    0x01
#define SF_BLCK_ER    0x02
#define SF_PAGE_SIZE  0x100

#define SF_UNPROTECT  0x00
#define SF_PROTECT    0x01
BOOL FlashProtect(unsigned char nID, unsigned char on_off)
{
	unsigned char  pCmdBuf[512];	

	pCmdBuf[0] = HOSTCMD_SF_ID;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x02+sizeof(gSfStaInfo);
	pCmdBuf[3] = HOSTCMD_SF_SID_CT;
	if(on_off == 0){
		pCmdBuf[4] = SF_UNPROTECT;
	}else{
		pCmdBuf[4] = SF_PROTECT;
	}	
	memcpy(&pCmdBuf[5], &gSfStaInfo, sizeof(gSfStaInfo));
	if(SendHostCmd(nID, pCmdBuf, 0x03+0x02+sizeof(gSfStaInfo)) == FALSE){
		return FALSE;
	}

	return TRUE;
}

BOOL FlashErase(unsigned char nID, unsigned int addr)
{
	unsigned char  pCmdBuf[512];

	//step1 Erase 1 sector 4k	
	pCmdBuf[0] = HOSTCMD_SF_ID;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x05;
	pCmdBuf[3] = HOSTCMD_SF_SID_ER;
	pCmdBuf[4] = SF_SECT_ER;
	pCmdBuf[5] = (addr>>16) & 0xff;
	pCmdBuf[6] = (addr>>8) & 0xff;
	pCmdBuf[7] = (addr>>0) & 0xff;
	if(SendHostCmd(nID, pCmdBuf, 0x08) == FALSE){
		return FALSE;
	}

	return TRUE;
}

BOOL FlashChipErase(unsigned char nID)
{
	unsigned char  pCmdBuf[512];

	//step1 Erase 1 sector 4k	
	pCmdBuf[0] = HOSTCMD_SF_ID;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x05;
	pCmdBuf[3] = HOSTCMD_SF_SID_ER;
	pCmdBuf[4] = SF_CHIP_ER;
	pCmdBuf[5] = 0;
	pCmdBuf[6] = 0;
	pCmdBuf[7] = 0;
	if(SendHostCmd(nID, pCmdBuf, 0x08) == FALSE){
		return FALSE;
	}
	return TRUE;
}

BOOL SendFwSector(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int   j = 0;
	unsigned short nSegment = 0;		
	unsigned char  pCmdBuf[512];	

	//step1 Erase 1 sector 4k	
	pCmdBuf[0] = HOSTCMD_SF_ID;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x05;
	pCmdBuf[3] = HOSTCMD_SF_SID_ER;
	pCmdBuf[4] = SF_SECT_ER;
	pCmdBuf[5] = (nFWoffset>>16) & 0xff;
	pCmdBuf[6] = (nFWoffset>>8) & 0xff;
	pCmdBuf[7] = (nFWoffset>>0) & 0xff;
	if(SendHostCmd(nID, pCmdBuf, 0x08) == FALSE){
		return FALSE;
	}

	//step2 write to internal 4K sram	
	for(j=0; j<nSect; j+=SF_PAGE_SIZE){	
		if((nSect - j) >= SF_PAGE_SIZE){
			nSegment = SF_PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}		
		pCmdBuf[0] = HOSTCMD_SF_ID;
		pCmdBuf[1] = ((0x03+0x06+nSegment)>>8) & 0xff;;
		pCmdBuf[2] = (0x03+0x06+nSegment) & 0xff;
		pCmdBuf[3] = HOSTCMD_SF_SID_PP;		
		pCmdBuf[4] = ((nFWoffset+j)>>16) & 0xff;
		pCmdBuf[5] = ((nFWoffset+j)>>8) & 0xff;
		pCmdBuf[6] = ((nFWoffset+j)>>0) & 0xff;
		pCmdBuf[7] = (nSegment>>8) & 0xff;
		pCmdBuf[8] = (nSegment>>0) & 0xff;
		memcpy(&pCmdBuf[9], pBuf+j, nSegment);
		if(SendHostCmd(nID, pCmdBuf, (0x03+0x06+nSegment)) == FALSE){
			return FALSE;
		}		
	}
	
	return TRUE;
}

BOOL GetFwSector(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int   j = 0;
	unsigned short nSegment = 0;		
	unsigned char  pCmdBuf[512];	

	for(j=0; j<nSect; j+=SF_PAGE_SIZE){	
		if((nSect - j) >= SF_PAGE_SIZE){
			nSegment = SF_PAGE_SIZE;
		}else{
			nSegment = nSect - j;
		}		
		pCmdBuf[0] = HOSTCMD_SF_ID;
		pCmdBuf[1] = ((0x03+0x06+nSegment)>>8) & 0xff;;
		pCmdBuf[2] = (0x03+0x06+nSegment) & 0xff;
		pCmdBuf[3] = HOSTCMD_SF_SID_PR;		
		pCmdBuf[4] = ((nFWoffset+j)>>16) & 0xff;
		pCmdBuf[5] = ((nFWoffset+j)>>8) & 0xff;
		pCmdBuf[6] = ((nFWoffset+j)>>0) & 0xff;
		pCmdBuf[7] = (nSegment>>8) & 0xff;
		pCmdBuf[8] = (nSegment>>0) & 0xff;
		memcpy(&pCmdBuf[9], pBuf+j, nSegment);
		if(SendHostCmd(nID, pCmdBuf, (0x03+0x06)) == FALSE){
			return FALSE;
		}else{
			GetDataFromDevice(nID, pBuf+j, nSegment);
		}				
	}
	
	return TRUE;
}

#define OV494_SCCB_BASE       (0x9200)
#define OV494_SCCBS_CRC_SET   (0x006A)
#define OV494_SCCBS_CRC_VALUE (0x0068)
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
	BurstWriteRegisterEx(nID, OV494_SCCB_BASE + OV494_SCCBS_CRC_SET, nValue, 1, 1);	
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
		
		//BurstWriteRegisterEx(nID, nPageAddr, pBuf + j, 1, nSegment);
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
		//BurstReadRegister(nID, nPageAddr, pBuf + j, 1, nSegment);	
	}		
}
