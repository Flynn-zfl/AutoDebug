#include "..\\stdafx.h"		//please remove this file if no need
//#include "../fw_code.h"	//please remove this file
#include "ov490_spi_operation.h"

t_libsif_cfg g_libsif_cfg_l;
static t_sf g_sf;

void spi_init()
{
	spiflash_init(&g_sf);

	// Reset CPU
	Write_ISP_Reg8(0x80800010, 0x04); 

	// enable sccb slave burst write
	Write_ISP_Reg8(0x802a6000, 0x0d);

	memset(&g_libsif_cfg_l, 0, sizeof(g_libsif_cfg_l));

	int div = PAD_CLK / (1000000*2) - 1; 
	if(div < 0 ) {	div = 0; }
	g_libsif_cfg_l.div = div;

	sccb_sif_hw_init(&g_libsif_cfg_l, &g_sf);
}

bool spi_erase(unsigned int src_addr,unsigned char erase_type)
{
	sccb_sf_erase(src_addr, erase_type);
	return 1;
}

bool spi_page_read(unsigned int src_addr, unsigned int *des_addr)
{
	sccb_sf_rd_data(src_addr, des_addr, g_sf.page_size);
	return 1;
}

bool spi_page_write(unsigned int* data, unsigned int wr_addr)
{
	unsigned char* hdr = new unsigned char [g_sf.page_size+4]; 

	hdr[0] = g_sf.cmd_page_program;
	hdr[1] = ((wr_addr) >> 16) & 0xffu;
	hdr[2] = ((wr_addr) >> 8) & 0xffu;
	hdr[3] = (wr_addr) & 0xffu;

	for(unsigned int i=0; i<g_sf.page_size/4; i++)
	{
		hdr[i*4+4]=(data[i])&0xff;
		hdr[i*4+5]=(data[i]>>8)&0xff;
		hdr[i*4+6]=(data[i]>>16)&0xff;
		hdr[i*4+7]=(data[i]>>24)&0xff;
	}

	sccb_sf_page_program((u32*)hdr, g_sf.page_size + 4, &wr_addr);

	if (hdr) delete [] hdr;

	//DELAY_LOOP(200);//delay some time
	return 1;
}