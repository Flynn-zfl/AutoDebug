#include "..\\stdafx.h"
#include "..\\I2C\\i2c.h"
#include <WinDef.h>
#include "OV491_func.h"

extern unsigned int m_ov491id;

unsigned short CRC_INITIAL_VALUE=0xffff;//define this as global var

unsigned short crc16_byte(unsigned short crc, const unsigned char data) {
	return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xffu];
}

void MID_CRC_RESET_CONTROL(unsigned char ResetCRC)
{
	if(ResetCRC)
		CRC_INITIAL_VALUE = 0xffff;
	else
		CRC_INITIAL_VALUE=CRC_INITIAL_VALUE;
}

unsigned short MID_Calculate_CRC(unsigned short address, unsigned char value)
{
	unsigned char data[3];
	BYTE i = 0;

	data[0] = (BYTE)((address >> 8) & 0xffu);
	data[1] = (BYTE)((address)& 0xffu);
	data[2] = value;

	for (i = 0; i<3; i++) {
		CRC_INITIAL_VALUE = crc16_byte(CRC_INITIAL_VALUE, data[i]);
	}
	return (CRC_INITIAL_VALUE & 0xffff);
} 
#define OUTPUT_MSG(x, y, z)\
	{if (g_mainDlg!=NULL) g_mainDlg->UpdateMSG("%02X %04X, %02X\r\n", x,y,z);}
bool HostCmd_Set(unsigned char cmd, unsigned char* param, unsigned int para_num, unsigned int timeout)
{

	MID_CRC_RESET_CONTROL(1);
	ov495_WriteRegister(m_ov491id, 0x3516, 0x00, 1); 
	ov495_WriteRegister(m_ov491id, 0x354D, 0x10, 1);
	ov495_WriteRegister(m_ov491id, 0x354A, 0x1D, 1);
	ov495_WriteRegister(m_ov491id, 0x0FFC, 0x00, 1);

	for (unsigned int i=0; i<para_num; i+=BURST_SIZE)
	{
		if(para_num-i>BURST_SIZE)
			BurstWriteRegisterEx(m_ov491id, 0x0500+i, param+i, 1, BURST_SIZE);
		else
			BurstWriteRegisterEx(m_ov491id, 0x0500+i, param+i, 1, para_num-i);
	}

	for (unsigned int i=0; i<para_num; i++)
	{
		MID_Calculate_CRC(0x0500+i, param[i]);
	}

	unsigned short crc = MID_Calculate_CRC(0x30C0, cmd);
	ov495_WriteRegister(m_ov491id, 0x3004, (para_num>>8)&0xFF, 1);
	ov495_WriteRegister(m_ov491id, 0x3005, para_num&0xFF, 1);
	ov495_WriteRegister(m_ov491id, 0x31a6, (CRC_INITIAL_VALUE & 0xff), 1);
	ov495_WriteRegister(m_ov491id, 0x31a7, (CRC_INITIAL_VALUE >> 8), 1);

	ov495_WriteRegister(m_ov491id, 0x30C0, cmd, 1);

	int status = 0;
	unsigned int timer = 0;
	// waiting for the task to be done
	while(status != TASK_STATUS_DONE)
	{
		ov495_ReadRegister(m_ov491id, 0xFFC, status, 1);

		_CHECK_CMD_CRC;
		_CHECK_CMD_ERR;
		_CHECK_CMD_TIMEOUT;		
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Following are the host commands for normal FW
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NM_SPI_Status_Get(BYTE& status)
{
	BYTE para[1];
	unsigned int para_num = 1;	
	para[0] = READ_SPI_FLASH_STATUS;	

	if (!HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num)) 
	{
		return 0;
	}
	else
	{
		int val = 0;
		ov495_ReadRegister(m_ov491id, 0x501, val, 1);
		status = (BYTE)val;
	}
	return 1;
}


bool NM_SPI_Status_Set(BYTE status)
{
	BYTE para[2];
	unsigned int para_num = 2;	
	para[0] = WRITE_SPI_FLASH_STATUS;	
	para[1] = status;
	
	return HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num);
}

bool NM_SendCmd_PageRead(BYTE* buf, DWORD pgIdx)
{
	BYTE para_num = 4;
	BYTE para[4];
	para[0] = READ_PAGE;
	para[1] = 0x00;
	para[2] = (pgIdx>>8)&0xff;
	para[3] = (pgIdx>>0)&0xff;
	
	if (HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num)==0)
		return 0;

	for (int i=0; i<SF_PAGE_SIZE; i+=BURST_SIZE)
	{
		BurstReadRegisterEx(m_ov491id, 0x0505+i, buf+i, 1, BURST_SIZE);
	}

	return 1;
}

bool NM_SendCmd_PageWrite(BYTE* buf, DWORD pgIdx)
{
	unsigned int para_num = SF_PAGE_SIZE+5;
	BYTE para[SF_PAGE_SIZE+5];

	para[0] = WRITE_PAGE;
	para[1] = 0x00u;
	para[2] = (pgIdx>>8)&0xff;
	para[3] = (pgIdx>>0)&0xff;
	para[4] = 0x00u;//reserved
	memcpy(para+5, buf, SF_PAGE_SIZE);
	return HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num, 500 );
}

bool NM_SendCmd_BlockErase(WORD pageIdx)
{
	BYTE para_num = 4;
	BYTE para[4];
	para[0] = ERASE_BLOCK;
	para[1] = 0x00u;
	para[2] = (BYTE)((pageIdx>>8)&0xff);
	para[3] = (BYTE)((pageIdx>>0)&0xff);	
	return HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num, 500);
}

bool NM_SendCmd_SectorErase(WORD pageIdx)
{
	BYTE para_num = 4;
	BYTE para[4];
	para[0] = ERASE_SECTOR;
	para[1] = 0x00u;
	para[2] = (BYTE)((pageIdx>>8)&0xff); 
	para[3] = (BYTE)((pageIdx>>0)&0xff);	
	return HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num, 500);
}

bool NM_SendCmd_ChipErase()
{
	BYTE para_num = 1;
	BYTE para[1];
	para[0] = ERASE_CHIP;	

	return HostCmd_Set(HOSTCMD_SPI_OPER, para, para_num, 10000);
}


//#define BURST_SIZE (32)
//bool OV495_HostCmd_Set(unsigned char cmd, unsigned char* param, unsigned int para_num)
//{
//	ov495_WriteRegister(m_ov491id, 0x3516, 0x00, 1);
//	MID_CRC_RESET_CONTROL(1);
//	ov495_WriteRegister(m_ov491id, 0x354D, 0x10, 1);
//	ov495_WriteRegister(m_ov491id, 0x354a, 0x1D, 1);
//
//	for (int i=0; i<para_num; i+=BURST_SIZE)
//	{
//		if(para_num-i>BURST_SIZE)
//			BurstWriteRegisterEx(m_ov491id, 0x0500+i, param+i, 1, BURST_SIZE);
//		else
//			BurstWriteRegisterEx(m_ov491id, 0x0500+i, param+i, 1, para_num-i);
//	}
//
//	for (int i=0; i<para_num; i++)
//	{
//		MID_Calculate_CRC(0x0500+i, param[i]);
//	}
//
//	unsigned short crc = MID_Calculate_CRC(0x30C0, cmd);
//	ov495_WriteRegister(m_ov491id, 0x3004, (para_num>>8)&0xFF, 1);
//	ov495_WriteRegister(m_ov491id, 0x3005, para_num&0xFF, 1);
//	ov495_WriteRegister(m_ov491id, 0x31a6, (CRC_INITIAL_VALUE & 0xff), 1);
//	ov495_WriteRegister(m_ov491id, 0x31a7, (CRC_INITIAL_VALUE >> 8), 1);
//
//	ov495_WriteRegister(m_ov491id, 0x30C0, cmd, 1);
//
//	int status = 0;
//	int timer = 0;
//	// waiting for the task to be done
//	while(status != TASK_STATUS_DONE_OV495)
//	{
//		ov495_ReadRegister(m_ov491id, 0xFFC, status, 1);
//
//		_CHECK_CMD_ERR_OV495;
//
//		Sleep(10);
//		timer += 1;
//		if (timer > 100)
//		{
//			AfxMessageBox("failed!");
//			return 0;
//		}
//	}
//
//	return 1;
//}
//
//
BYTE WriteReg_Sensor(int iAddr, int iVal, bool rightsensor)
{
	BYTE para[5];
	para[0] = 0; // write
	para[1] = (iAddr >> 8);
	para[2] = (iAddr & 0xff);
	para[3] = iVal;
	para[4] = (rightsensor?0x81:0x01);

	return HostCmd_Set(HOSTCMD_SNR_ACCESS_ID, para, 5);
}

BYTE ReadReg_Sensor(int iAddr, int& iVal, bool rightsensor)
{	
	BYTE para[5];
	para[0] = 1; // read
	para[1] = (iAddr>>8);
	para[2] = (iAddr&0xff);
	para[3] = 0;
	para[4] = (rightsensor?0x81:0x01);

	if (!HostCmd_Set(HOSTCMD_SNR_ACCESS_ID, para, 5))
		return 0;
	
	ov495_ReadRegister(m_ov491id, 0x0500, iVal, 1);
	return 1;
}