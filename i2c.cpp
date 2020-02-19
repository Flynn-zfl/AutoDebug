#include "stdafx.h"
#include "PantherSDK.h"
#include "ftd2xx.h"
#include "LibFT4222.h"
#include "i2c.h"

// panther
#pragma comment(lib,"PantherSDKLib.lib")
//ft42222
#pragma comment(lib,"ftd2xx.lib")
#pragma comment(lib,"LibFT4222.lib")

extern UINT8 Device;
extern FT_HANDLE ftHandle;
extern uint16 sizeTransferred;

void WriteReg16(int SCCB_Addr, int RegAddr, int RegData)
{
	if(Device==FT4222)
	{
		uint8 slaveAddr = (SCCB_Addr>>1)&0xff;
		uint8 master_data[3] = {(RegAddr>>8)&0xff, (RegAddr)&0xff, RegData&0xff};
		FT4222_I2CMaster_Write(ftHandle, slaveAddr, master_data, sizeof(master_data), &sizeTransferred);
	}else if(Device==PANTHER)
	{	
		WriteRegister(SCCB_Addr&0xff, RegAddr&0xffff, RegData&0xff, 1);
	}
}

int ReadReg16(int SCCB_Addr, int RegAddr)
{
	int val=0;
	if(Device==FT4222)
	{
		uint8 slaveAddr = (SCCB_Addr>>1)&0xff;
		uint8 master_data[2] = {(RegAddr>>8)&0xff, (RegAddr)&0xff};
		uint8 slave_data[1]; 
		FT4222_I2CMaster_Write(ftHandle, slaveAddr, master_data, sizeof(master_data), &sizeTransferred);
		FT4222_I2CMaster_Read(ftHandle, slaveAddr, slave_data, sizeof(slave_data), &sizeTransferred);
		val=slave_data[0]&0xff;	
	}else if(Device==PANTHER)
	{	
		int RegData=0;
		ReadRegister(SCCB_Addr&0xff, RegAddr&0xffff, RegData, 1);
		val=RegData&0xff;
	}
	return val&0xff;
}

void WriteReg8(int SCCB_Addr, int RegAddr,int RegData)
{
	if(Device==FT4222)
	{
		uint8 slaveAddr = (SCCB_Addr>>1)&0xff;
		uint8 master_data[2] = {(RegAddr)&0xff, RegData&0xff};
		FT4222_I2CMaster_Write(ftHandle, slaveAddr, master_data, sizeof(master_data), &sizeTransferred);
	}else if(Device==PANTHER)
	{	
		WriteRegister(SCCB_Addr&0xff, RegAddr&0xff, RegData&0xff, 0);
	}
}

int ReadReg8(int SCCB_Addr, int RegAddr)
{
	int val=0;
	if(Device==FT4222)
	{
		uint8 slaveAddr = (SCCB_Addr>>1)&0xff;
		uint8 master_data[1] = {(RegAddr)&0xff};
		uint8 slave_data[1]; 
		FT4222_I2CMaster_Write(ftHandle, slaveAddr, master_data, sizeof(master_data), &sizeTransferred);
		FT4222_I2CMaster_Read(ftHandle, slaveAddr, slave_data, sizeof(slave_data), &sizeTransferred);
		val=slave_data[0]&0xff;	
	}else if(Device==PANTHER)
	{	
		int RegData=0;
		ReadRegister(SCCB_Addr&0xff, RegAddr&0xff, RegData, 0);
		val=RegData&0xff;
	}
	return val&0xff;
}


//void ov490_WriteReg8(unsigned char SccbId, unsigned int Address, unsigned char value)
//{
//	unsigned short addr;
//	int val;
//	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);     
//	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
//	WriteReg16(SccbId, 0xfffd, val_fffd);
//	WriteReg16(SccbId, 0xfffe, val_fffe);
//	addr=(unsigned short)(Address&0xffff);
//	val=(unsigned char)(value&0xff);
//	WriteReg16(SccbId, addr, val);
//}
//
//unsigned char ov490_ReadReg8(unsigned char SccbId, unsigned int Address)
//{
//	unsigned char val=0;
//	unsigned short addr;
//	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);     
//	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
//	WriteReg16(SccbId, 0xfffd, val_fffd);
//	WriteReg16(SccbId, 0xfffe, val_fffe);
//	addr=(unsigned short)(Address&0xffff);
//	val=ReadReg16(SccbId, addr);
//	val=(unsigned char)(val&0xff);
//	return val&0xff;
//}
//
//void ov490_SNR_WriteReg8(unsigned char ISPID, unsigned char SensorID,  unsigned int Address,unsigned char value)
//{
//	WriteReg16(ISPID, 0xfffd, 0x80);
//	WriteReg16(ISPID, 0xfffe, 0x19);
//	WriteReg16(ISPID, 0x5000, 0x00);
//	WriteReg16(ISPID, 0x5001, (Address&0xff00)>>8);
//	WriteReg16(ISPID, 0x5002, Address&0xff);
//	WriteReg16(ISPID, 0x5003, value&0xff);
//	WriteReg16(ISPID, 0xfffe, 0x80);
//	WriteReg16(ISPID, 0x00c0, 0xc1);
//}
//
//unsigned char ov490_SNR_ReadReg8(unsigned char ISPID, unsigned char SensorID,  unsigned int Address)
//{
//	unsigned char val=0;
//	WriteReg16(ISPID, 0xfffd, 0x80);
//	WriteReg16(ISPID, 0xfffe, 0x19);
//	WriteReg16(ISPID, 0x5000, 0x01);
//	WriteReg16(ISPID, 0x5001, (Address&0xff00)>>8);
//	WriteReg16(ISPID, 0x5002, Address&0xff);
//	WriteReg16(ISPID, 0xfffe, 0x80);
//	WriteReg16(ISPID, 0x00c0, 0xc1);
//	WriteReg16(ISPID, 0xfffe, 0x19);
//	val=ReadReg16(ISPID, 0x5000)&0xff;
//	return val&0xff;
//}

unsigned char BurstReadRegister(unsigned char id, unsigned int addr, unsigned char *pdata, unsigned char addr_16_8, unsigned char len)
{

	if(addr_16_8==1){//16bit addr
		for(int i=0; i<len; i++)pdata[i] = ReadReg16(id, addr+i);

	}else{//8bit addr
		for(int i=0; i<len; i++)pdata[i] = ReadReg8(id, addr+i);	
	}
	return 1;
}

unsigned char BurstWriteRegister(unsigned char id, unsigned int addr, unsigned char *pdata, unsigned char addr_16_8, unsigned char len)
{
	if(addr_16_8==1){//16bit addr
		for(int i=0; i<len; i++)WriteReg16(id, addr+i, pdata[i]);

	}else{//8bit addr
		for(int i=0; i<len; i++)WriteReg8(id, addr+i, pdata[i]);
	}
	return 1;
}