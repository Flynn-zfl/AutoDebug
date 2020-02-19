#include "..\\stdafx.h"
#include "OV480sup.h"
#include "..\\I2C\\i2c.h"

void PollLo(UINT16 addr, BYTE bitmask) {
	BYTE in;

	do {
		in = ReadReg16(0x0a, addr);
	} while (in & bitmask);
}

void PollData(UINT16 addr, int match) {
	int in;

	do {
		in = ReadReg16(0x0a, addr);
	    //Sleep(20);
	} while (in != match);
}

void SpiPollBusy()
{
	BYTE dummy;
	BOOL bBusy;
	//
	bBusy = true;
	while (bBusy) {
		SPI_SS_ASSERT
		WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_RDSR);	//REG_SPI_DATA_OUT = SPI_RDSR;
		WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);			//REG_SPI_DATA_OUT = 0;	// dummy read
		PollData(SPI_RBUFF_NFO, 0x02);						// wait for completion
		dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);		//dummy = REG_SPI_DATA_IN;
		bBusy = ReadReg16(0x0a, REG_SPI_DATA_IN) & SPI_RDSR_BUSY; //bBusy = REG_SPI_DATA_IN&SPI_RDSR_BUSY;
		SPI_SS_RELEASE
	}
}

UINT32 SpiRDID()
{
	UINT32 rval;
	BYTE dummy;

	rval = 0;
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_RDID);		//REG_SPI_DATA_OUT = SPI_RDID;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;	// dummy read
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;	// dummy read
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;	// dummy read
	PollData(SPI_RBUFF_NFO, 0x04);							//wait for completion
	SPI_SS_RELEASE
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	rval =  ReadReg16(0x0a, REG_SPI_DATA_IN);			//rval = REG_SPI_DATA_IN;
	rval = rval << 8; rval = rval + ReadReg16(0x0a, REG_SPI_DATA_IN); //rval = rval << 8; rval = rval + REG_SPI_DATA_IN;
	rval = rval << 8; rval = rval + ReadReg16(0x0a, REG_SPI_DATA_IN); //rval = rval << 8; rval = rval + REG_SPI_DATA_IN;
	return rval;
}

void SpiUnlock()
{
	BYTE dummy;

	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_WREN);		//REG_SPI_DATA_OUT = SPI_WREN;
	PollData(SPI_RBUFF_NFO, 0x01);							// wait for completion
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	SPI_SS_RELEASE
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_WRSR);		//REG_SPI_DATA_OUT = SPI_WRSR;	// global unprotect
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;	// dummy read
	PollData(SPI_RBUFF_NFO, 0x02);							// wait for completion
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	SPI_SS_RELEASE
	SpiPollBusy();
}

void SpiSectorErase(UINT8 sector)
{
	BYTE dummy;
	UINT8 i;

	SpiUnlock();
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_WREN);		//REG_SPI_DATA_OUT = SPI_WREN;
	PollData(SPI_RBUFF_NFO, 0x01);							// wait for completion
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	SPI_SS_RELEASE
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_BE64);		//REG_SPI_DATA_OUT = SPI_BE64;	// support 64KB sectors only
	WriteReg16(0x0a, REG_SPI_DATA_OUT, sector);			//REG_SPI_DATA_OUT = sector;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;
	PollData(SPI_RBUFF_NFO, 0x04);							// wait for completion
	for (i=0;i<4;i++) {
		dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);		//dummy = REG_SPI_DATA_IN;	// clear the receive fifo
	}
	SPI_SS_RELEASE
	SpiPollBusy();											// wait until erase done
}

void SpiChipErase()
{
	BYTE dummy;

	SpiUnlock();
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_WREN);		//REG_SPI_DATA_OUT = SPI_WREN;
	PollData(SPI_RBUFF_NFO, 0x01);							// wait for completion
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	SPI_SS_RELEASE
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_CE2);			//REG_SPI_DATA_OUT = SPI_CE2;	// support bulk erase on newer chips
	PollData(SPI_RBUFF_NFO, 0x01);							// wait for completion
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	SPI_SS_RELEASE
	//SpiPollBusy();											// wait until erase done
}

void SpiWriteCmd(UINT32 addr)
{
	UINT8 i;
	BYTE dummy;

	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_WREN);		//REG_SPI_DATA_OUT = SPI_WREN;
	PollData(SPI_RBUFF_NFO, 0x01);							// wait for completion
	dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;
	SPI_SS_RELEASE
	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_BP);			//REG_SPI_DATA_OUT = SPI_BP;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, (addr>>16)&0xFF); //REG_SPI_DATA_OUT = (addr>>16)&0xFF;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, (addr>>8)&0xFF);	//REG_SPI_DATA_OUT = (addr>>8)&0xFF;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr&0xFF);		//REG_SPI_DATA_OUT = addr&0xFF;
	PollData(SPI_RBUFF_NFO, 0x04);							// wait for completion
	for (i=0;i<4;i++) {
		dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);		//dummy = REG_SPI_DATA_IN;	// clear the receive fifo
	}
}

void SpiReadCmdSlow(UINT32 addr)
{
	BYTE dummy;
	UINT8 count;

	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_READ);		//REG_SPI_DATA_OUT = SPI_READ;	//0x03
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr>>16);		//REG_SPI_DATA_OUT = addr>>16;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr>>8);			//REG_SPI_DATA_OUT = addr>>8;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr);			//REG_SPI_DATA_OUT = addr;	// LSB addr
	PollData(SPI_RBUFF_NFO, 0x04);							// wait for completion
	for (count=0;count<4;count++) {
		dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);		//dummy = REG_SPI_DATA_IN;	// clear the receive fifo
	}
}

void SpiReadCmdFast(UINT32 addr)
{
	BYTE dummy;
	UINT8 count;

	SPI_SS_ASSERT
	WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_FREAD);		//REG_SPI_DATA_OUT = SPI_FREAD;	//0x0B
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr>>16);		//REG_SPI_DATA_OUT = addr>>16;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr>>8);			//REG_SPI_DATA_OUT = addr>>8;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, addr);			//REG_SPI_DATA_OUT = addr;	// LSB addr
	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0;	// dummy read
	PollData(SPI_RBUFF_NFO, 0x05);							// wait for completion
	for (count=0;count<5;count++) {
		dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);		//dummy = REG_SPI_DATA_IN;	// clear the receive fifo
	}
}

BYTE SpiGetByte()
{
	BYTE rval;

	WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);				//REG_SPI_DATA_OUT = 0x00;
	PollData(SPI_RBUFF_NFO, 0x01);						// wait for completion
	rval =  ReadReg16(0x0a, REG_SPI_DATA_IN);			//rval = REG_SPI_DATA_IN;
	return rval;
}

void SpiPutByte(BYTE dat)
{
	BYTE dummy;
	WriteReg16(0x0a, REG_SPI_DATA_OUT, dat);				//REG_SPI_DATA_OUT = dat;
	PollData(SPI_RBUFF_NFO, 0x01);						// wait for completion
	dummy =  ReadReg16(0x0a, REG_SPI_DATA_IN);			//dummy = REG_SPI_DATA_IN;	// clear the receive fifo
}

bool CheckHeader(UINT8 *header, char *text)
{
	if (header[0]!=(UINT8)text[0]) return false;
	if (header[1]!=(UINT8)text[1]) return false;
	if (header[2]!=(UINT8)text[2]) return false;
	if (header[3]!=(UINT8)text[3]) return false;
	return true;
}
#if 0

void SpiReadPage(UINT16 addr, UINT16 len)
{
	UINT16 count;

	for (count = 0; count < len; count++) {
		WriteReg16(0x0a, REG_SPI_DATA_OUT, 0x00);		//REG_SPI_DATA_OUT = 0x00;
		PollData(SPI_RBUFF_NFO, 0x01);						// wait for completion
		XMem(addr) = ReadReg16(0x0a, REG_SPI_DATA_IN);   //REG_SPI_DATA_IN;
		addr++;
	}
}
#endif

void SpiWritePage(BYTE* data, UINT32 len)
{
	BYTE dummy;
	UINT16 i, j, count;

	for (i = 0; i < len; ) {
		count = (len-i>32)?32:(len-i);
		for (j=0; j<count; j++)
			WriteReg16(0x0a, REG_SPI_DATA_OUT, data[i+j]);		
		PollData(SPI_RBUFF_NFO, count);	// wait for completion
		for (j=0; j<count; j++)
			dummy = ReadReg16(0x0a, REG_SPI_DATA_IN); //dummy = REG_SPI_DATA_IN;	// clear the receive fifo
		i+=count;
	}
}







