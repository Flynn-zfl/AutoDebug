#include "..\\stdafx.h"
#include "..\\I2C\\i2c.h"
#include "X1F.h"

//extern unsigned char g_PrintProtocol;

#define HOSTCMD_STATUS_DONE     0x99

unsigned short crc16_ccitt_Chip(unsigned char *buf, int len)
{
	int counter;
	unsigned short crc = 0;
	for (counter = 0; counter < len; counter++)
		crc = (crc << 8) ^ Chip_crc16tab_Burn[((crc >> 8) ^ *buf++) & 0x00FF];
	return crc;
}

int OVChipInitRam(int nSlaveID)
{	
	int nResult = 1;
	//streaming off
	WORD addr = 0x0100;
	BYTE value =0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);
	Sleep(100);

	BYTE nFWflag[4];

	int nReturn = 0;
	addr = 0x81A8;		//check safety mode for eco1
	value = 0;
	BurstReadRegisterEx(nSlaveID, addr, nFWflag, 1, 1);
	//if not enter safety mode, enter safety mode
	if ((nFWflag[0] & 0x0f) != 0x05)
	{
		addr = 0x8057;
		value = 0x80;
		BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

		addr = 0x805f;
		BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

		addr = 0xA00D;
		value = 1;
		BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);
	}
	//exit safety mode
	addr = 0xA013;
	value = 0xff;
	for (int i = 0; i < 4; i++)
		BurstWriteRegisterEx(nSlaveID, (addr + i), &value, 1, 1);

	addr = 0x8000;
	value = 0x3D;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0x8003;
	value = 0x01;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0x8000;
	BurstReadRegisterEx(nSlaveID, addr, nFWflag, 1, 1);
	value = nFWflag[0] | 0x02;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0x8088;
	value = 0x08;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	Sleep(100);

	addr = 0xa10a;	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa11e;	value = 0x7f;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa11d;	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa110;	value = 0x80;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10f;	value = 0x18;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10e;	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10d;	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	return nResult;
}

int OVChipRunRam(int nSlaveID)
{
	//unsigned int addr = 0;
	int nResult = 0;
	BYTE value = 0;

	BYTE nFWflag[4];
	WORD addr = OVChip_SYS_BASE + OVChip_SYS_SSRR;;
	BurstReadRegisterEx(nSlaveID, addr, nFWflag, 1, 1);

	addr = 0x801b;	//watch dog disable
	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = OVChip_SYS_BASE + OVChip_SYS_SSRR;
	value = nFWflag[0] & (~0x3c);
	value = value | 0x3c;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = OVChip_SYS_BASE + OVChip_SYS_SLSRR_0;	//cpu auto boot
	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10a;
	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa11e;
	value = 0x7f;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa11d;
	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa110;
	value = 0x80;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10f;
	value = 0x18;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10e;
	value = 0x4c;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	addr = 0xa10d;
	value = 0x00;
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);

	return nResult;
}

void OVChipReboot(int nSlaveID)
{
	//step1: clear watchdog status
	WORD addr = OVChip_SYS_BASE + OVChip_SYS_CRSR;
	BYTE value = 0x00;    //clear all software status
	BurstWriteRegisterEx(nSlaveID, addr, &value, 1, 1);
	//end of step1

	//step2: enable watchdog to reboot FW from SPI flash
	//clear watchdog count register
	addr = OVChip_SYS_BASE + OVChip_SYS_WDCR;
	BYTE ndata[4] = { 0 };
	BurstWriteRegisterEx(nSlaveID, addr, ndata, 1, 4);
	
	//set watchdog time period register and enable watchdog, wait for reboot.
	int value32 = (BIT31 + BIT30 + 0x400);	
	ndata[3] = (value32 >> 24) & 0xff; ndata[2] = (value32 >> 16) & 0xff; ndata[1] = (value32 >> 8) & 0xff; ndata[0] = value32 & 0xff;
	addr = OVChip_SYS_BASE + OVChip_SYS_WDMR;
	BurstWriteRegisterEx(nSlaveID, addr, ndata, 1, 4);
	//end of step2
}

BOOL SendHostCmd_Chip(unsigned char nID, unsigned char *pBuf, unsigned int size)
{
	unsigned int   nPageAddr = 0;
	unsigned short nCrcBufOffset = 0;
	unsigned short nCrcCnt = 0;	
	unsigned short CalCrc = 0;	
	unsigned char  nValue[2];
	unsigned short j = 0;
	unsigned char nSegment = 0;	
	unsigned char nTryTime = 0;

	//send host command			
	nCrcBufOffset = 0;
	for (j = 0; j<size; j += Chip_PAGE_SIZE)
	{	
		if ((size - j) >= Chip_PAGE_SIZE)
			nSegment = Chip_PAGE_SIZE;
		else
			nSegment = size - j;		
		
		if (size == 10/* && pBuf[0] == 0x81*/)
			nPageAddr = Chip_HOSTCMD_START_ADDR + j;
		else
			nPageAddr = (0xe40b + j);

		BurstWriteRegisterEx(nID, nPageAddr, (pBuf + j), 1, nSegment);
	}

	WORD addr = 0x8160;
	BYTE value = 0x01;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	nTryTime = 0;
	nValue[0] = 0x00;
	addr = 0x8180;
	BurstReadRegisterEx(nID, addr, nValue, 1, 1);

	while(nValue[0] != HOSTCMD_STATUS_DONE)
	{
		Sleep(10);
		nValue[0] = 0x00;		
		BurstReadRegisterEx(nID, addr, nValue, 1, 1);
		nTryTime++;
		if(nTryTime >= 100)
			return FALSE;
	}
	return TRUE;
}

void GetDataFromDevice_Chip(unsigned char nID, unsigned char *pBuf, unsigned int size)
{
	unsigned int   nPageAddr = 0;
	unsigned short j = 0;
	unsigned char nSegment = 0;	

	for (j = 0; j<size; j += Chip_PAGE_SIZE){
		if ((size - j) >= Chip_PAGE_SIZE){
			nSegment = Chip_PAGE_SIZE;
		}else{
			nSegment = size - j;
		}		
		nPageAddr = HOSTCMD_SF_RD_ADDR + j;
		BurstReadRegisterEx(nID, nPageAddr, (pBuf + j), 1, nSegment);
	}
}

BOOL SendChiprase_Chip(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int   j = 0;
	unsigned short nSegment = 0;		
	unsigned char  pCmdBuf[512] = {0};

	//step1 Erase 1 sector 4k	
	pCmdBuf[0] = HOSTCMD_SF_ID;	//cmd id
	pCmdBuf[1] = HOSTCMD_SF_MODE;	//cmd mode: loop
	pCmdBuf[2] = 0x00;
	pCmdBuf[3] = 0x04;
	pCmdBuf[4] = 0x10;
	pCmdBuf[5] = (nFWoffset>>16) & 0xff;
	pCmdBuf[6] = (nFWoffset>>8) & 0xff;
	pCmdBuf[7] = (nFWoffset>>0) & 0xff;
	pCmdBuf[8] = 0x00;
	pCmdBuf[9] = 0x00; //cmd crc
	BYTE data = 0x00;
	BurstWriteRegisterEx(nID, 0x8181, &data, 1, 1);

	if (SendHostCmd_Chip( nID, pCmdBuf, 10) == FALSE)
		return FALSE;

	return TRUE;
}

BOOL SendFwSector_Chip(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int   j = 0;
	unsigned short nSegment = 0;		
	unsigned char  pCmdBuf[512] = {0};

	//step1 Erase 1 sector 4k	
	pCmdBuf[0] = HOSTCMD_SF_ID;	//cmd id
	pCmdBuf[1] = HOSTCMD_SF_MODE;	//cmd mode: loop
	pCmdBuf[2] = 0x00;
	pCmdBuf[3] = 0x04;
	pCmdBuf[4] = 0x10;
	pCmdBuf[5] = (nFWoffset>>16) & 0xff;
	pCmdBuf[6] = (nFWoffset>>8) & 0xff;
	pCmdBuf[7] = (nFWoffset>>0) & 0xff;
	pCmdBuf[8] = 0x00;
	pCmdBuf[9] = 0x00; //cmd crc
	BYTE data = 0x00;
	BurstWriteRegisterEx(nID, 0x8181, &data, 1, 1);

	if (SendHostCmd_Chip( nID, pCmdBuf, 10) == FALSE)
		return FALSE;

	//step2 write to internal 4K sram	
	for(j=0; j<nSect; j+=SF_PAGE_SIZE)
	{	
		if((nSect - j) >= SF_PAGE_SIZE)
			nSegment = SF_PAGE_SIZE;
		else
			nSegment = nSect - j;
		
		pCmdBuf[0] = HOSTCMD_SF_ID;
		pCmdBuf[1] = HOSTCMD_SF_MODE;
		pCmdBuf[2] = ((nSegment+7) >> 8) & 0xff;
		pCmdBuf[3] = ((nSegment +7) & 0xff);
		pCmdBuf[4] = 0x11;
		pCmdBuf[5] = 0x00;
		pCmdBuf[6] = ((nFWoffset+j)>>16) & 0xff;
		pCmdBuf[7] = ((nFWoffset+j)>>8) & 0xff;
		pCmdBuf[8] = ((nFWoffset+j)>>0) & 0xff;
		pCmdBuf[9] = (nSegment>>8) & 0xff;
		pCmdBuf[10] = (nSegment>>0) & 0xff;
		BurstWriteRegisterEx(nID, Chip_HOSTCMD_START_ADDR, pCmdBuf, 1, 11);

		memcpy(pCmdBuf, pBuf+j, nSegment);
		if (SendHostCmd_Chip( nID, pCmdBuf, (nSegment)) == FALSE){
			return FALSE;
		}		
	}
	return TRUE;
}

BOOL SendExternFwSector_Chip(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int   j = 0;
	unsigned short nSegment = 0;
	unsigned char  pCmdBuf[512] = { 0 };

	//step1 Erase 1 sector 4k	
	pCmdBuf[0] = HOSTCMD_SF_ID;	//cmd id
	pCmdBuf[1] = HOSTCMD_SF_MODE;	//cmd mode: loop
	pCmdBuf[2] = 0x00;
	pCmdBuf[3] = 0x04;
	pCmdBuf[4] = 0x10;
	pCmdBuf[5] = (nFWoffset >> 16) & 0xff;
	pCmdBuf[6] = (nFWoffset >> 8) & 0xff;
	pCmdBuf[7] = (nFWoffset >> 0) & 0xff;
	pCmdBuf[8] = 0x00;
	pCmdBuf[9] = 0x00; //cmd crc
	BYTE data = 0x00;
	BurstWriteRegisterEx(nID, 0x8181, &data, 1, 1);

	if (SendHostCmd_Chip( nID, pCmdBuf, 10) == FALSE)
		return FALSE;

	//step2 write to internal 4K sram	
	for (j = 0; j<nSect; j += SF_PAGE_SIZE)
	{
		if ((nSect - j) >= SF_PAGE_SIZE)
			nSegment = SF_PAGE_SIZE;
		else
			nSegment = nSect - j;

		pCmdBuf[0] = HOSTCMD_SF_ID;
		pCmdBuf[1] = HOSTCMD_SF_MODE;
		pCmdBuf[2] = ((nSegment + 7) >> 8) & 0xff;
		pCmdBuf[3] = ((nSegment + 7) & 0xff);
		pCmdBuf[4] = 0x11;
		pCmdBuf[5] = 0x01;
		pCmdBuf[6] = ((nFWoffset + j) >> 16) & 0xff;
		pCmdBuf[7] = ((nFWoffset + j) >> 8) & 0xff;
		pCmdBuf[8] = ((nFWoffset + j) >> 0) & 0xff;
		pCmdBuf[9] = (nSegment >> 8) & 0xff;
		pCmdBuf[10] = (nSegment >> 0) & 0xff;
		BurstWriteRegisterEx(nID, Chip_HOSTCMD_START_ADDR, pCmdBuf, 1, 11);
		memcpy(pCmdBuf, pBuf + j, nSegment);
		if (SendHostCmd_Chip( nID, pCmdBuf, (nSegment)) == FALSE){
			return FALSE;
		}
	}
	return TRUE;
}

BOOL GetFwSector_Chip(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
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
		pCmdBuf[1] = HOSTCMD_SF_MODE;
		pCmdBuf[2] = 0x00;
		pCmdBuf[3] = 0x06;		
		pCmdBuf[4] = 0x12;
		pCmdBuf[5] = ((nFWoffset+j)>>16) & 0xff;
		pCmdBuf[6] = ((nFWoffset+j)>>8) & 0xff;
		pCmdBuf[7] = ((nFWoffset+j)>>0) & 0xff;
		pCmdBuf[8] = (nSegment>>8) & 0xff;
		pCmdBuf[9] = (nSegment>>0) & 0xff;
		
		BYTE data = 0x00;
		BurstWriteRegisterEx(nID, 0x8181, &data, 1, 1);

		if (SendHostCmd_Chip( nID, pCmdBuf, (0x0a)) == FALSE){
			return FALSE;
		}
		else{
			GetDataFromDevice_Chip( nID, pBuf + j, nSegment);
		}
	}
	return TRUE;
}

BOOL SendFwRam_Chip(unsigned m_nSlaveID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned char nRamBlock = 0;
	unsigned int nPageAddr = 0;
	unsigned int nCrcBufOffset = 0;
	unsigned int nCrcCnt = 0;
	unsigned short SccbCrc = 0;
	unsigned short CalCrc = 0;

	//write to internal 4K sram
	nCrcBufOffset = 0;
	for (j = 0; j<nSect; j += Chip_PAGE_SIZE)
	{	
		if ((nSect - j) >= Chip_PAGE_SIZE)
			nSegment = Chip_PAGE_SIZE;
		else
			nSegment = nSect - j;	

		//for burn FW larger 16K
		nPageAddr = Chip_Start_SCCB_ADDR + nFWoffset + j;

		BurstWriteRegisterEx(m_nSlaveID, nPageAddr, (pBuf + j), 1, nSegment);
	}
	return 1;
}

void GetFwRam_Chip(unsigned nID, unsigned int nSect, unsigned int nFWoffset, unsigned char *pBuf)
{
	unsigned int j = 0;
	unsigned char nSegment = 0;		
	unsigned int nPageAddr = 0;
	
}

int OVChipInitReadSF(int nID)
{
	int nResult = 0;

	WORD addr = 0x8088;
	BYTE value = 0x08;
	int nReturn = 0;

	addr = 0xa10a;	value = 0x00;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	addr = 0xa11e;	value = 0x7f;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	addr = 0xa11d;	value = 0x00;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	addr = 0xa110;	value = 0x80;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	addr = 0xa10f;	value = 0x18;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	addr = 0xa10e;	value = 0x4c;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	addr = 0xa10d;	value = 0x00;
	BurstWriteRegisterEx(nID, addr, &value, 1, 1);

	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	return nResult;
}