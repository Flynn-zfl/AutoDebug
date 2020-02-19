#include "..\\stdafx.h"
#include "LIB_OV495.h"
#include "..\\I2C\\i2c.h"

extern bool I2C_ACCESS_READ_FLAG; //1;
extern unsigned int m_ov495id;


int val_fffd = -1;
int val_fffe = -1;


/*************************************************************************************************************
Function:
SET PLL OF OV495
SYSTEM CLOCK 150MHZ
CLOCK IN IS 24MHZ(NOTE:THIS IS IMPORTANT)
*************************************************************************************************************/
void ov495_pll0_set(
                    int ref_clk        , //OSC frequency, reserved for auto calculation
					int system_clk     , //system clock frequency,reserved for auto calculation  
					int isp_clk        , //isp clock frequency   ,reserved for auto calculation  
                    int auto_en        , //unused, enable auto caculation
					int r_pll0_bypass  ,
                    int r_pll0_prediv  ,
                    int r_pll0_dsm     ,
                    int r_pll0_loopdivp,
                    int r_pll0_loopdivs,
                    int r_pll0_div0    ,    
                    int r_pll0_div1        
				)
{
        ov495_WriteReg32(SC_BASE_ADDR+0x200, (ov495_ReadReg32(SC_BASE_ADDR+0x0200)&
                                       (!((0x7<<2)|(0x1<<5)|(0xF<<18)|(0xF<<22)|(0x1<<26)))) |
                                       ((r_pll0_prediv&0x7) << 2) |
                                       ((r_pll0_bypass &0x1)  << 5) |
                                       ((r_pll0_div0 &0xF)  <<18) |
                                       ((r_pll0_div1 &0xF)  <<22) |
                                       ((0 & 0x1)<<26)         
                                       );
        ov495_WriteReg32(SC_BASE_ADDR+0x204,(ov495_ReadReg32(SC_BASE_ADDR+0x0204) & 
                                      (!((0xFF<<20)|(0xFFFFF)|(0x1<<28)))) |
                                        (r_pll0_dsm & 0xFFFFF)        | 
                                       ((r_pll0_loopdivp & 0xFF)<<20) |
                                       ((r_pll0_loopdivs & 0x1)<<28) 
                                       );
}


/*************************************************************************************************************
Function:
WRITE ONE BYTE BY SCCB
*************************************************************************************************************/
void ov495_WriteReg8(unsigned int addr,unsigned char val)
{
	val_fffd = (addr&0xff000000)>>24;
	val_fffe = (addr&0xff0000)>>16;
	if (I2C_ACCESS_READ_FLAG)
	{
		ov495_WriteRegister(m_ov495id, 0xfffd, val_fffd, 1);
		ov495_WriteRegister(m_ov495id, 0xfffe, val_fffe, 1);
	}			

	ov495_WriteRegister(m_ov495id, (addr&0xffff), val, 1);
}
/*************************************************************************************************************
Function:
WRITE TWO BYTES BY SCCB
*************************************************************************************************************/
void ov495_WriteReg16(unsigned int addr,unsigned short val)
{
	ov495_WriteReg8(addr+1, val&0xff);
	ov495_WriteReg8(addr+0, (val&0xff00)>>8);
}

/*************************************************************************************************************
Function:
WRITE FOUR BYTES BY SCCB
*************************************************************************************************************/
void ov495_WriteReg32(unsigned int addr,unsigned int val)
{
	ov495_WriteReg8(addr+3, val&0xff);
	ov495_WriteReg8(addr+2, (val&0xff00)>>8);
	ov495_WriteReg8(addr+1, (val&0xff0000)>>16);
	ov495_WriteReg8(addr+0, (val&0xff000000)>>24);
}

/*************************************************************************************************************
Function:
READ ONE BYTES BY SCCB
*************************************************************************************************************/
unsigned char ov495_ReadReg8(unsigned int addr)
{
	val_fffd = (addr&0xff000000)>>24;
	val_fffe = (addr&0xff0000)>>16;
	if (I2C_ACCESS_READ_FLAG)
	{
		ov495_ReadRegister(m_ov495id, 0xfffd, val_fffd, 1);
		ov495_ReadRegister(m_ov495id, 0xfffe, val_fffe, 1);
	}			

	int val;
	ov495_ReadRegister(m_ov495id, (addr&0xffff), val, 1);	
	return val;
}
/*************************************************************************************************************
Function:
READ TWO BYTES BY SCCB
*************************************************************************************************************/
unsigned short ov495_ReadReg16(unsigned int addr)
{
	return (ov495_ReadReg8(addr+1)<<8)+ov495_ReadReg8(addr);
}
/*************************************************************************************************************
Function:
READ FOUR BYTES BY SCCB
*************************************************************************************************************/
unsigned int ov495_ReadReg32(unsigned int addr)
{
	BYTE v1, v2, v3, v4;
	/////////////////////
	v1 = ov495_ReadReg8(addr+0);
	v2 = ov495_ReadReg8(addr+1);
	v3 = ov495_ReadReg8(addr+2);
	v4 = ov495_ReadReg8(addr+3);
	/////////////////////
	return v1+(v2<<8)+(v3<<16)+(v4<<24);
}