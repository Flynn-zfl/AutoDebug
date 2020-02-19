#include "stdafx.h"
#include "LIB_OV495.h"

extern unsigned char m_ov495id;

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

void ov495_WriteReg8(unsigned int Address,unsigned char value)
{
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);	
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	ov495_WriteReg16(m_ov495id, 0xfffd, val_fffd);
	ov495_WriteReg16(m_ov495id, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val=(unsigned char)(value&0xff);
	ov495_WriteReg16(m_ov495id, addr, val);
}
void ov495_WriteReg16(unsigned int Address,unsigned short value)
{
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);	
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	ov495_WriteReg16(m_ov495id, 0xfffd, val_fffd);
	ov495_WriteReg16(m_ov495id, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val=(unsigned char)(value&0xff);
	ov495_WriteReg16(m_ov495id, addr, val);
	val=(unsigned char)((value>>8)&0xff);
	ov495_WriteReg16(m_ov495id, addr+1, val);
}
void ov495_WriteReg32(unsigned int Address,unsigned int value)
{
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);	
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	ov495_WriteReg16(m_ov495id, 0xfffd, val_fffd);
	ov495_WriteReg16(m_ov495id, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val=(unsigned char)(value&0xff);
	ov495_WriteReg16(m_ov495id, addr, val);
	val=(unsigned char)((value>>8)&0xff);
	ov495_WriteReg16(m_ov495id, addr+1, val);
	val=(unsigned char)((value>>16)&0xff);
	ov495_WriteReg16(m_ov495id, addr+2, val);
	val=(unsigned char)((value>>24)&0xff);
	ov495_WriteReg16(m_ov495id, addr+3, val);
}

unsigned char ov495_ReadReg8(unsigned int Address)
{
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);	
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	ov495_WriteReg16(m_ov495id, 0xfffd, val_fffd);
	ov495_WriteReg16(m_ov495id, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val = ov495_ReadReg16(m_ov495id, addr);
	return ((unsigned char)(val&0xff));
}
unsigned short ov495_ReadReg16(unsigned int Address)
{
	unsigned short val16=0;
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);	
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	ov495_WriteReg16(m_ov495id, 0xfffd, val_fffd);
	ov495_WriteReg16(m_ov495id, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val = ov495_ReadReg16(m_ov495id, addr+1);
	val16+=val&0xff;
	val16<<=8;
	val = ov495_ReadReg16(m_ov495id, addr);
	val16+=val&0xff;
	return val16;
}
unsigned int ov495_ReadReg32(unsigned int Address)
{
	unsigned int val32=0;
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);	
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);

	ov495_WriteReg16(m_ov495id, 0xfffd, val_fffd);
	ov495_WriteReg16(m_ov495id, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val = ov495_ReadReg16(m_ov495id, addr+3);
	val32+=val&0xff;
	val32<<=8;
	val = ov495_ReadReg16(m_ov495id, addr+2);
	val32+=val&0xff;
	val32<<=8;
	val = ov495_ReadReg16(m_ov495id, addr+1);
	val32+=val&0xff;
	val32<<=8;
	val = ov495_ReadReg16(m_ov495id, addr+0);
	val32+=val&0xff;
	return val32;
}