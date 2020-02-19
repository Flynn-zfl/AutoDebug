// OV480ISP.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "OV480ISP.h"
#include "afxdialogex.h"

#include "AutoDebugDlg.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

#include ".\\I2C\\i2c.h"
#include ".\\OV480\\OV480sup.h"
#include ".\\OV480\\ov480.h"

#define PAGESIZE 0x100 //

//ov480 SPI falsh memory structure
struct OLYI{
	char tag[4]; //"OLYI"
	BYTE nSettings;
	BYTE nFW;
	WORD nImages;
	//DWORD *index;
};

struct OVFW{
	char tag[4]; //"OVFW"
	DWORD nSize;
	//BYTE *data;
	//DWORD link;
};

struct OLY8{
	char tag[4]; //"OLY8"
	BYTE YUV0to15[48];
	WORD nWidth;
	WORD nHeight;
	DWORD nSize;
};

unsigned char ov480_ReadSensor(unsigned char id, unsigned int addr)
{
	unsigned char RegData;
	WriteReg16(0x0a, 0x0206, 0x01);		//;REG_I2C_WIDTH = 0x01; // Bit1 = 16-bit data, Bit0 = 16-bit addr
	WriteReg16(0x0a, 0x0201, id&0xff);	//;REG_I2C_DEVID = devid;
	WriteReg16(0x0a, 0x0202, (addr>>8 & 0xff));	//;REG_I2C_ADDRH = addr>>8;
	WriteReg16(0x0a, 0x0203, (addr & 0xff));	//;REG_I2C_ADDRL = addr;
	WriteReg16(0x0a, 0x0209, 0x13);	//;REG_I2C_CTRL/ 0x13; // 0001_0011, start+address+devid
	PollLo(I2C_STAT, 0x01);	// poll busy
	WriteReg16(0x0a, 0x0209, 0x51);	//;REG_I2C_CTRL = 0x51; // 0101_0001, read+(re)start+devid	
	PollLo(I2C_STAT, 0x01);	// poll busy
	WriteReg16(0x0a, 0x0209, 0xa8);	//;REG_I2C_CTRL = 0xA8; // 1010_1000, nack+stop+data_in
	PollLo(I2C_STAT, 0x01);	// poll busy
	RegData = ReadReg16(0x0a, 0x0208);

	return RegData&0xff;

}

unsigned char ov480_WriteSensor(unsigned char id, unsigned int addr, unsigned char pdata)
{
	// for sensor
	WriteReg16(0x0a, 0x0206, 0x01);		//;REG_I2C_WIDTH = 0x01; // Bit1 = 16-bit data, Bit0 = 16-bit addr
	WriteReg16(0x0a, 0x0201, id&0xff);	//;REG_I2C_DEVID = devid;
	WriteReg16(0x0a, 0x0202, (addr>>8 & 0xff));	//;REG_I2C_ADDRH = addr>>8;
	WriteReg16(0x0a, 0x0203, (addr & 0xff));	//;REG_I2C_ADDRL = addr;
	WriteReg16(0x0a, 0x0205, pdata&0xff);	//;REG_I2C_DATOL = dato;
	WriteReg16(0x0a, 0x0209, 0x37);		//;REG_I2C_CTRL = 0x37; // 0011_0111, stop+start+data_out+address+devid
	PollLo(I2C_STAT, 0x01);	// poll busy
	return 0;
}


// COV480ISP dialog

IMPLEMENT_DYNAMIC(COV480ISP, CDialogEx)

	COV480ISP::COV480ISP(CWnd* pParent /*=NULL*/)
	: CDialogEx(COV480ISP::IDD, pParent)
{

}

COV480ISP::~COV480ISP()
{
}

void COV480ISP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_IDS, m_IDs);
	DDX_Control(pDX, IDC_BUTTON_ERASEFLASH, m_Eraseflash);
	DDX_Control(pDX, IDC_BUTTON_READFLASH, m_Readflash);
	DDX_Control(pDX, IDC_BUTTON_BURNFLASH, m_Burnflash);
	DDX_Control(pDX, IDC_CHECK_VERIFY, m_check_verify);

	DDX_Control(pDX, IDC_EDIT_SVAL, m_SVal);
	DDX_Control(pDX, IDC_EDIT_SADDR, m_SAddr);
	DDX_Control(pDX, IDC_EDIT_SID, m_SId);
	DDX_Control(pDX, IDC_BUTTON_READSNR, m_ReadSNR);
	DDX_Control(pDX, IDC_BUTTON_WRITESNR, m_WriteSNR);

	DDX_Text(pDX, IDC_EDIT_SID, s_SId);
	DDX_Text(pDX, IDC_EDIT_SADDR, s_SAddr);
	DDX_Text(pDX, IDC_EDIT_SVAL, s_SVal);
}


BEGIN_MESSAGE_MAP(COV480ISP, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_IDCHECK, &COV480ISP::OnBnClickedButtonIdcheck)
	ON_BN_CLICKED(IDC_BUTTON_ERASEFLASH, &COV480ISP::OnBnClickedButtonEraseflash)
	ON_BN_CLICKED(IDC_BUTTON_READFLASH, &COV480ISP::OnBnClickedButtonReadflash)
	ON_BN_CLICKED(IDC_BUTTON_BURNFLASH, &COV480ISP::OnBnClickedButtonBurnflash)
	ON_BN_CLICKED(IDC_CHECK_VERIFY, &COV480ISP::OnBnClickedCheckVerify)
	ON_BN_CLICKED(IDC_BUTTON_READSNR, &COV480ISP::OnBnClickedButtonReadsnr)
	ON_BN_CLICKED(IDC_BUTTON_WRITESNR, &COV480ISP::OnBnClickedButtonWritesnr)
END_MESSAGE_MAP()


// COV480ISP message handlers
void COV480ISP::OnOK()
{
	//
}

void COV480ISP::OnClose()
{
	CDialog::OnClose();	
	//CDialog::OnOK();
}


void COV480ISP::OnBnClickedButtonIdcheck()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;

	unsigned int id_tmp=0;
	id_tmp = ReadReg16(0x0a, 0x7fda)&0xff;
	if(id_tmp == 0x48)
	{

		parent->m_Status.SetWindowTextA("ov480 is existed!\r\n");
		strtxt.Format("0a");
		m_IDs.SetWindowTextA(strtxt);
		m_Eraseflash.EnableWindow(TRUE);
		m_Burnflash.EnableWindow(FALSE);
		m_Readflash.EnableWindow(TRUE);

		m_SId.EnableWindow(TRUE);
		m_SAddr.EnableWindow(TRUE);
		m_SVal.EnableWindow(TRUE);
		m_ReadSNR.EnableWindow(TRUE);
		m_WriteSNR.EnableWindow(TRUE);
	}else
	{	
		parent->m_Status.SetWindowTextA("ov480 is not existed!\r\n");
		strtxt.Format("00");
		m_IDs.SetWindowTextA(strtxt);
	}
}


void COV480ISP::OnBnClickedButtonEraseflash()
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(WorkerThread_Chiperase, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}


void COV480ISP::OnBnClickedButtonReadflash()
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(WorkerThread_Spiread, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}


void COV480ISP::OnBnClickedButtonBurnflash()
{
	// TODO: Add your control notification handler code here
	AfxBeginThread(WorkerThread_Spiburn, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}


void COV480ISP::OnBnClickedCheckVerify()
{
	// TODO: Add your control notification handler code here
}


void COV480ISP::OnBnClickedButtonReadsnr()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	RegData = ov480_ReadSensor(SCCB_ID, RegAddr);
	s_SVal.Format(_T("%x"),RegData);
	m_SVal.SetWindowTextA(s_SVal);
	strtxt.Format(_T("%04x %02x;<hc r>\r\n"), RegAddr, RegData);
	parent->Log(strtxt);
}


void COV480ISP::OnBnClickedButtonWritesnr()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	m_SVal.GetWindowTextA(s_SVal);
	sscanf_s(s_SVal,_T("%x"),&RegData);
	ov480_WriteSensor(SCCB_ID, RegAddr, RegData);
	strtxt.Format(_T("%02x %04x %02x;<hc w>\r\n"),SCCB_ID, RegAddr, RegData);	
	parent->Log(strtxt);
}


void COV480ISP::Chiperase(void)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	// TODO: Add your control notification handler code here
	m_Eraseflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	CString sTMP;

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	parent->Log(_T("Erase SPI flash starting...... \r\n"));
	WriteReg16(0x0a, 0x7fc0, 0x04); // hold MCU
	WriteReg16(0x0a, 0x080d, 0x01); //OSD function disabled
	WriteReg16(0x0a, 0x0302, 0x01); //SPI divide
	WriteReg16(0x0a, 0x0306, 0x01); //SPI optimize

	SPI_FW_RELEASE
	SPI_FW_GRAB
	//WriteReg16(0x0a, 0x080d, 0x01); //OSD function disabled
	sTMP.Format(_T("SPI flash ID = 0x%06x; \r\n"),SpiRDID());
	parent->Log(sTMP);	

	SPI_FW_RELEASE
	SPI_FW_GRAB	

	SpiChipErase();

	BYTE dummy;
	BOOL bBusy;
	bBusy = true;
	while (bBusy) {
		SPI_SS_ASSERT
		WriteReg16(0x0a, REG_SPI_DATA_OUT, SPI_RDSR);	//REG_SPI_DATA_OUT = SPI_RDSR;
		WriteReg16(0x0a, REG_SPI_DATA_OUT, 0);			//REG_SPI_DATA_OUT = 0;	// dummy read
		PollData(SPI_RBUFF_NFO, 0x02);						// wait for completion
		dummy = ReadReg16(0x0a, REG_SPI_DATA_IN);		//dummy = REG_SPI_DATA_IN;
		bBusy = ReadReg16(0x0a, REG_SPI_DATA_IN) & SPI_RDSR_BUSY; //bBusy = REG_SPI_DATA_IN&SPI_RDSR_BUSY;
		SPI_SS_RELEASE

		parent->m_Status.SetWindowTextA(_T("Erasing...... \r\n"));
	}

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);

	sTMP.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(sTMP);
	parent->Log(_T("Erase SPI flash finished\r\n"));
	parent->m_Status.SetWindowTextA(_T("Erase SPI flash finished\r\n"));

	m_Eraseflash.EnableWindow(TRUE);
	m_Burnflash.EnableWindow(TRUE);
}

void COV480ISP::Spiburn(void)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	m_Eraseflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	WriteReg16(0x0a, 0x7fc0, 0x04); // hold MCU
	WriteReg16(0x0a, 0x080d, 0x01); //OSD function disabled

	FILE *fp;
	UINT32 nOLYSize = 0;
	UINT32 count = 0;
	CString sTMP; 

	parent->Log(_T("Click ov480 SPI burn !!\r\n"));

	CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("OLY files|*.oly|所有文件 | *.*|| ")); 
	CString FileName;

	if((openFileDialog.DoModal()) != IDOK) goto returnback; // return ;	
	FileName = openFileDialog.GetPathName();
	fopen_s(&fp, FileName,"rb");
	if(fp==NULL) {
		parent->Log(_T("Load oly file ERROR!! \r\n"));
		goto returnback; // return ;
	}

	fseek(fp, 0, SEEK_END);
	nOLYSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	BYTE* buf = new BYTE [nOLYSize];
	parent->Log(_T("Load oly file successs!! \r\n"));

	fread(buf, 1, nOLYSize, fp);	
	fclose(fp);	
	/*
	for(count=0; count<nOLYSize ; count++)
	{
	if( count%16 == 0 )	{
	sTMP.Format(_T("\r\n0x%08x: %02x "), count, buf[count]);
	LOGOUT2 +=sTMP;
	}
	else {
	sTMP.Format(_T("%02x "), buf[count]);
	LOGOUT2 +=sTMP;
	}
	}
	*/

	SPI_FW_RELEASE
	SPI_FW_GRAB
	WriteReg16(0x0a, 0x080d, 0x01); //OSD function disabled
	sTMP.Format(_T("SPI flash ID = 0x%06x; \r\n"),SpiRDID());	

	//m_status = _T("chip erase firstly !!\r\n");
	//Log(_T("chip erase firstly !!\r\n"));
	//LOGOUT2 += GetSystemTime() + m_status;

	//chip erase firstly
	//OnBnClickedButtonChiperase();
	//SPI_SS_ASSERT		
	//SpiWriteCmd(SPI_PAGE_OFFSET + 0x00000000);
	//SpiPutByte(0x5f);
	//for (count=0;count<8;count++) {
	//	header[count] = SpiGetByte();	//get byte back
	//}
	//}

	UINT32 offset = 0;
	int len = 0;
	BYTE *outbuf;
	outbuf = (BYTE *)malloc(PAGESIZE);

	parent->Log(_T("start to programming......\r\n"));


	for (offset = 0; offset < nOLYSize; offset += PAGESIZE) 
	{
		if ((nOLYSize-offset)>PAGESIZE) 
		{
			len = PAGESIZE;
			memcpy(outbuf, buf+offset, len);
		} else 
		{
			len = nOLYSize-offset;
			memcpy(outbuf, buf+offset, len);
			// if granularity > 1 and size falls on a non-word boundary,
			// then must round up to the next whole word
		}
		SpiWriteCmd(offset);
		SpiWritePage(outbuf, len);
		SPI_SS_RELEASE
		SpiPollBusy();

		sTMP.Format("ov480 fw size = %dbytes, Downloading (%d%%)", nOLYSize, offset*100/nOLYSize+1 );
		parent->m_Status.SetWindowTextA(sTMP);

	}
	SPI_SS_RELEASE

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	sTMP.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(sTMP);

	parent->Log(_T("SPI burn success!!\r\n"));

	delete [] buf;
	delete [] outbuf;
	buf = NULL;



returnback: 
	m_Eraseflash.EnableWindow(TRUE);
	m_Burnflash.EnableWindow(FALSE);

}

void COV480ISP::Spiread(void)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	// TODO: Add your control notification handler code here
	m_Eraseflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	WriteReg16(0x0a, 0x7fc0, 0x04); // hold MCU
	WriteReg16(0x0a, 0x080d, 0x01); //OSD function disabled

	FILE *fp;
	CString  sTMP;
	UINT32 count = 0;
	UINT8 header[12];
	UINT32 nOLYSize = 0;
	UINT8 nNUM = 0;
	//UINT32 nFWSize = 0;
	//UINT32 nIMGSize = 0;
	DWORD nLastFWAddr = 0;
	DWORD nLastIMGAddr = 0;
	UINT32 m_pos = 0;

	OLYI OLYI;
	OVFW OLFW;
	OLY8 OLY8;

	parent->Log(_T("Click ov480 SPI read!!\r\n"));

	SPI_FW_RELEASE
	SPI_FW_GRAB
	//WriteReg16(0x0a, 0x080d, 0x01); //OSD function disabled
	sTMP.Format(_T("SPI flash ID = 0x%06x; \r\n"),SpiRDID());
	parent->Log(sTMP);

	SPI_FW_RELEASE
	SPI_FW_GRAB
	SPI_SS_ASSERT
	SpiReadCmdFast(0x00000000);
	for (count=0;count<8;count++) {
		header[count] = SpiGetByte();	//get byte back
	}
	SPI_SS_RELEASE

	if (!CheckHeader(header, "OLYI")) {
		parent->Log(_T("FLASH is empty!!\r\n"));
		goto returnback; 		
	}
	parent->Log(_T("FLASH header\"OLYI\" is existed!!\r\n"));

	if (header[5]==0) {

		parent->Log(_T("FW isn't existing!!\r\n"));
		goto returnback; 	
	}

	parent->Log(_T("FW is existed!!\r\n"));

	OLYI.nSettings = header[4];
	OLYI.nFW = header[5];
	OLYI.nImages = header[6] | header[7]<<8; 

	if(OLYI.nImages == 0 ) //no osd
	{
		//get last FW addr
		nNUM = 0x04 + OLYI.nSettings*4 + OLYI.nFW*4;		
		SPI_FW_RELEASE
			SPI_FW_GRAB
			SPI_SS_ASSERT
			SpiReadCmdFast(nNUM);
		nLastFWAddr = SpiGetByte();
		nLastFWAddr = nLastFWAddr | (SpiGetByte()<<8);
		nLastFWAddr = nLastFWAddr | (SpiGetByte()<<16);
		SPI_SS_RELEASE

		sTMP.Format(_T("nLastFWAddr=0x%x \r\n"), nLastFWAddr);
		parent->Log(sTMP);

		// get last FW size;
		SPI_SS_ASSERT
			SpiReadCmdFast(nLastFWAddr);
		for (count=0;count<8;count++) {
			header[count] = SpiGetByte();	// get return byte
		}
		SPI_SS_RELEASE

		//get oly size
		if (!CheckHeader(header, "OVFW")) goto returnback; 	//return ;
		if (header[6]!=0x00) goto returnback; 	//return ;	// fw too big, something wrong
		if (header[7]!=0x00) goto returnback; 	//return ;	// fw too big, something wrong
		OLFW.nSize = (header[5]<<8)|header[4];
		nOLYSize = nLastFWAddr + 8 + OLFW.nSize;

		sTMP.Format(_T("nFWSize=0x%x, nOLYSize=0x%x \r\n"), OLFW.nSize, nOLYSize);
		parent->Log(sTMP);

		//read out oly
		BYTE* buf = new BYTE [nOLYSize];

		parent->Log(_T("Read SPI flash starting...... \r\n"));

		SPI_FW_RELEASE
		SPI_FW_GRAB
		SPI_SS_ASSERT
		SpiReadCmdFast(0x00000000);
		for (count=0; count<nOLYSize; count++) {
			buf[count] = SpiGetByte();		//get byte back
			if(count%0x100 ==0)
			{
				sTMP.Format("ov480 fw size = %dbytes, reading (%d%%)", nOLYSize, count*100/nOLYSize+1 );
				parent->m_Status.SetWindowTextA(sTMP);
			}
		}
		SPI_SS_RELEASE

		//save to bin file
		CFileDialog saveFileDialog(FALSE, _T("bin"), _T("readout.bin"), OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("BIN files|*.bin|all files | *.*|| ")); 
		CString FileName;
		if((saveFileDialog.DoModal()) == IDOK) 
		{
			FileName = saveFileDialog.GetPathName();

			fopen_s(&fp, FileName,"rb");
			if(fp==NULL) {
				parent->Log(_T("Save \"Read_flash \" ERROR\r\n"));
				goto returnback; 	//return ;
			}
			fwrite(buf, 1, nOLYSize, fp);     
			fclose(fp);	

			parent->Log(_T("Save \"Read_flash\" successs!! \r\n"));
		}

		delete [] buf;
		buf = NULL;
	}
	else //osd image is existed
	{
		//get last image addr
		nNUM = 0x04 + OLYI.nSettings*4 + OLYI.nFW*4 + 4 + OLYI.nImages*4;		
		SPI_FW_RELEASE
		SPI_FW_GRAB
		SPI_SS_ASSERT
		SpiReadCmdFast(nNUM);
		nLastIMGAddr = SpiGetByte();
		nLastIMGAddr = nLastIMGAddr | (SpiGetByte()<<8);
		nLastIMGAddr = nLastIMGAddr | (SpiGetByte()<<16);
		SPI_SS_RELEASE

		//get last image header;
		SPI_SS_ASSERT
		SpiReadCmdFast(nLastIMGAddr);
		for (count=0;count<8;count++) 
		{
			header[count] = SpiGetByte();		//get return byte
		}
		SPI_SS_RELEASE

		if (!CheckHeader(header, "OLY8")) goto returnback; 	//return ;
		//if (header[6]!=0x00) return ;	// fw too big, something wrong
		//if (header[7]!=0x00) return ;	// fw too big, something wrong

		//get last image size;
		SPI_SS_ASSERT
		SpiReadCmdFast(nLastIMGAddr+4+48);
		for (count=0;count<8;count++) 
		{
			header[count] = SpiGetByte();		//get return byte
		}
		SPI_SS_RELEASE

		//get oly size
		OLY8.nWidth = (header[1]<<8)|header[0];
		OLY8.nHeight = (header[3]<<8)|header[2];
		OLY8.nSize = (header[7]<<24)|(header[6]<<16)|(header[5]<<8)|header[4];
		nOLYSize = nLastIMGAddr + 60 + OLY8.nSize; //4+48+8

		//read out oly
		BYTE* buf = new BYTE [nOLYSize];

		parent->Log(_T("Read SPI flash starting...... \r\n"));

		SPI_FW_RELEASE
		SPI_FW_GRAB
		SPI_SS_ASSERT
		SpiReadCmdFast(0x00000000);
		for (count=0; count<nOLYSize; count++) 
		{
			buf[count] = SpiGetByte();	// get byte back

			if(count%0x100 ==0)
			{
				sTMP.Format("ov480 fw size = %dbytes, reading (%d%%)", nOLYSize, count*100/nOLYSize+1 );
				parent->m_Status.SetWindowTextA(sTMP);
			}
		}
		SPI_SS_RELEASE

		//save to bin file
		CFileDialog saveFileDialog(FALSE, _T("bin"), _T("readout.bin"), OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("BIN files|*.bin|所有文件 | *.*|| ")); 
		CString FileName;
		if((saveFileDialog.DoModal()) == IDOK) 
		{
			FileName = saveFileDialog.GetPathName();

			fopen_s(&fp, FileName,"rb");
			if(fp==NULL) {	
				parent->Log(_T("Save \"Read_flash 0X100\" ERROR\r\n"));
				goto returnback; 	//return ;
			}
			fwrite(buf, 1, nOLYSize, fp);     
			fclose(fp);	
			parent->Log(_T("Save \"Read_flash\" successs!! \r\n"));
		}

		delete [] buf;
		buf = NULL;	
	}	

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	sTMP.Format("read time=%.1fsec\r\n", write_t);
	parent->Log(sTMP);

returnback: 
	m_Eraseflash.EnableWindow(TRUE);
	m_Burnflash.EnableWindow(TRUE);

	return ;
}

UINT COV480ISP::WorkerThread_Chiperase(LPVOID pParam)
{
	COV480ISP* pObject = (COV480ISP*)pParam;

	if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->Chiperase();

	return 0;   // thread completed successfully
}

UINT COV480ISP::WorkerThread_Spiburn(LPVOID pParam)
{
	COV480ISP* pObject = (COV480ISP*)pParam;

	if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->Spiburn();

	return 0;   // thread completed successfully
}

UINT COV480ISP::WorkerThread_Spiread(LPVOID pParam)
{
	COV480ISP* pObject = (COV480ISP*)pParam;

	if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->Spiread();

	return 0;   // thread completed successfully
}
