#include "stdio.h"


typedef enum {
	PANTHER=0,
	FT4222	
}ISPType;

typedef enum {
	OV490_ISP=0,
	OV495_ISP,
	OV494_ISP
}DeviceType;

void WriteReg16(int SCCB_Addr, int RegAddr, int RegData);
int ReadReg16(int SCCB_Addr, int RegAddr);

void WriteReg8(int SCCB_Addr, int RegAddr,int RegData);
int ReadReg8(int SCCB_Addr, int RegAddr);

unsigned char BurstReadRegister(unsigned char id, unsigned int addr, unsigned char *pdata, unsigned char addr_16_8, unsigned char len);
unsigned char BurstWriteRegister(unsigned char id, unsigned int addr, unsigned char *pdata, unsigned char addr_16_8, unsigned char len);