// COV490ISP.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "OV490ISP.h"
#include "afxdialogex.h"
#include "AutoDebugDlg.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

#include ".\\I2C\\i2c.h"

#include ".\\OV490\\ov490_spi.h"
#include ".\\OV490\\ov490_spi_operation.h"

#define PAGESIZE (0x100)
#define HOST_CTRL_CMD_IN_PARAM (0x80195000)
#define HOST_CTRL_CMD_OUT_PARAM (0x80195000)
#define HOST_CTRL_CMD_INTR_REG (0x808000c0)
#define HOST_CTRL_CMD_STATUS_REG (0x80195ffc)
#define HOSTCMD_STATUS_DONE (0x99)
#define HOSTCMD_STATUS_DOING (0x88)
#define HOSTCMD_STATUS_ERR (0x55)
#define HOSTCMD_STATUS_CRC_ERR (0x44)
#define HOSTCMD_STATUS_RESET (0x00)

unsigned m_ov490id=0;

void ov490_WriteReg8(unsigned char SccbId, unsigned int Address, unsigned char value)
{
	unsigned short addr;
	int val;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);     
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	WriteReg16(SccbId, 0xfffd, val_fffd);
	WriteReg16(SccbId, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val=(unsigned char)(value&0xff);
	WriteReg16(SccbId, addr, val);
}

unsigned char ov490_ReadReg8(unsigned char SccbId, unsigned int Address)
{
	unsigned char val=0;
	unsigned short addr;
	unsigned char val_fffd = (unsigned char)((Address&0xff000000)>>24);     
	unsigned char val_fffe = (unsigned char)((Address&0xff0000)>>16);
	WriteReg16(SccbId, 0xfffd, val_fffd);
	WriteReg16(SccbId, 0xfffe, val_fffe);
	addr=(unsigned short)(Address&0xffff);
	val=ReadReg16(SccbId, addr);
	val=(unsigned char)(val&0xff);
	return val&0xff;
}

unsigned char ov490_ReadSensor(unsigned char id, unsigned int addr)
{
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+0, 0x01); //read cmd
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+1, (addr>>8)&0xff); //addrh
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+2, addr&0xff); //addrl
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+3, 0x00); //reserved
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+4, 0x01); //16bit addr
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_INTR_REG, 0xc1); //CMD_SNR_REG_ACCESS

	int i=10;
	while(i){
		if(ov490_ReadReg8(m_ov490id, HOST_CTRL_CMD_STATUS_REG) == 0x99) break;
		Sleep(10);
		i--;
	}
	return ov490_ReadReg8(m_ov490id, HOST_CTRL_CMD_OUT_PARAM);
}

unsigned char ov490_WriteSensor(unsigned char id, unsigned int addr, unsigned char pdata)
{
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+0, 0x00); //write cmd
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+1, (addr>>8)&0xff); //addrh
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+2, addr&0xff); //addrl
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+3, pdata&0xff); //reserved
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_IN_PARAM+4, 0x01); //16bit addr
	ov490_WriteReg8(m_ov490id, HOST_CTRL_CMD_INTR_REG, 0xc1); //CMD_SNR_REG_ACCESS

	int i=10;
	while(i){
		if(ov490_ReadReg8(m_ov490id, HOST_CTRL_CMD_STATUS_REG) == 0x99) break;
		Sleep(10);
		i--;
	}

	return 1;
}

unsigned char Read_ISP_Reg8(unsigned int Address)
{
	return ov490_ReadReg8(m_ov490id, Address);
}

void Write_ISP_Reg8(unsigned int Address,unsigned char value)
{
	ov490_WriteReg8(m_ov490id, Address, value);
}

// COV490ISP dialog

IMPLEMENT_DYNAMIC(COV490ISP, CDialogEx)

	COV490ISP::COV490ISP(CWnd* pParent /*=NULL*/)
	: CDialogEx(COV490ISP::IDD, pParent)
{

}

COV490ISP::~COV490ISP()
{

}

void COV490ISP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_SPIINIT, m_Spiinit);
	DDX_Control(pDX, IDC_BUTTON_ERASEFLASH, m_Eraseflash);
	DDX_Control(pDX, IDC_BUTTON_READFLASH, m_Readflash);
	DDX_Control(pDX, IDC_BUTTON_BURNFLASH, m_Burnflash);
	DDX_Control(pDX, IDC_EDIT_ID, m_ISP_ID);
	DDX_Control(pDX, IDC_CHECK_VERIFY, m_check_verify);
	DDX_Control(pDX, IDC_STATIC_IDS, m_IDs);
	DDX_Control(pDX, IDC_EDIT_SVAL, m_SVal);
	DDX_Control(pDX, IDC_EDIT_SADDR, m_SAddr);
	DDX_Control(pDX, IDC_EDIT_SID, m_SId);
	DDX_Control(pDX, IDC_BUTTON_READSNR, m_ReadSNR);
	DDX_Control(pDX, IDC_BUTTON_WRITESNR, m_WriteSNR);
	DDX_Text(pDX, IDC_EDIT_SID, s_SId);
	DDX_Text(pDX, IDC_EDIT_SADDR, s_SAddr);
	DDX_Text(pDX, IDC_EDIT_SVAL, s_SVal);
}

BEGIN_MESSAGE_MAP(COV490ISP, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_B8080, &COV490ISP::OnBnClickedButtonB8080)
	ON_BN_CLICKED(IDC_BUTTON_B8020, &COV490ISP::OnBnClickedButtonB8020)
	ON_BN_CLICKED(IDC_BUTTON_B8028, &COV490ISP::OnBnClickedButtonB8028)
	ON_BN_CLICKED(IDC_BUTTON_B8029, &COV490ISP::OnBnClickedButtonB8029)
	ON_BN_CLICKED(IDC_BUTTON_B8018, &COV490ISP::OnBnClickedButtonB8018)
	ON_BN_CLICKED(IDC_BUTTON_B8019, &COV490ISP::OnBnClickedButtonB8019)
	ON_BN_CLICKED(IDC_BUTTON_SPIINIT, &COV490ISP::OnBnClickedButtonSpiinit)
	ON_BN_CLICKED(IDC_BUTTON_B8082, &COV490ISP::OnBnClickedButtonB8082)
	ON_BN_CLICKED(IDC_BUTTON_ERASEFLASH, &COV490ISP::OnBnClickedButtonEraseflash)
	ON_BN_CLICKED(IDC_BUTTON_READFLASH, &COV490ISP::OnBnClickedButtonReadflash)
	ON_BN_CLICKED(IDC_BUTTON_BURNFLASH, &COV490ISP::OnBnClickedButtonBurnflash)
	ON_BN_CLICKED(IDC_BUTTON_IDCHECK, &COV490ISP::OnBnClickedButtonIdcheck)
	ON_BN_CLICKED(IDC_BUTTON_SETID, &COV490ISP::OnBnClickedButtonSetid)
	ON_EN_CHANGE(IDC_EDIT_SID, &COV490ISP::OnEnChangeEditSid)
	ON_EN_CHANGE(IDC_EDIT_SADDR, &COV490ISP::OnEnChangeEditSaddr)
	ON_EN_CHANGE(IDC_EDIT_SVAL, &COV490ISP::OnEnChangeEditSval)
	ON_BN_CLICKED(IDC_BUTTON_READSNR, &COV490ISP::OnBnClickedButtonReadsnr)
	ON_BN_CLICKED(IDC_BUTTON_WRITESNR, &COV490ISP::OnBnClickedButtonWritesnr)
END_MESSAGE_MAP()

// COV490ISP message handlers
BOOL COV490ISP::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Spiinit.EnableWindow(FALSE);
	m_Eraseflash.EnableWindow(FALSE);
	m_Readflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	m_check_verify.SetCheck(0);
	m_ISP_ID.SetWindowTextA("48");

	return TRUE;
}

void COV490ISP::set_dlg(CDialog * parnet_dlg)
{
	m_pMianDlg = parnet_dlg;
}

void COV490ISP::ServiceMessages()
{
	MSG message;
	while (PeekMessage((LPMSG)&message,NULL,NULL,NULL,PM_REMOVE))
	{
		TranslateMessage((LPMSG)&message);
		DispatchMessage((LPMSG)&message);
	}
}

void COV490ISP::OnBnClickedButtonB8080()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x80);
}


void COV490ISP::OnBnClickedButtonB8020()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x20);
}


void COV490ISP::OnBnClickedButtonB8028()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x28);
}


void COV490ISP::OnBnClickedButtonB8029()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x29);
}


void COV490ISP::OnBnClickedButtonB8018()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x18);
}


void COV490ISP::OnBnClickedButtonB8019()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x19);
}


void COV490ISP::OnBnClickedButtonB8082()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov490id, 0xfffd, 0x80);
	WriteReg16(m_ov490id, 0xfffe, 0x82);
}

void COV490ISP::OnBnClickedButtonSpiinit()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	spi_init();
	parent->Log("ov490 SPI init finish!!\r\n");
	parent->m_Status.SetWindowTextA("ov490 SPI init finish!!");
	m_Eraseflash.EnableWindow(TRUE);
	m_Readflash.EnableWindow(TRUE);	
}

void COV490ISP::OnBnClickedButtonEraseflash()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	if (spi_erase(0, ERASE_CHIP))
	{
		parent->m_Status.SetWindowTextA("Chip Erase done!");
		parent->Log("Chip Erase done!");
	}
	ServiceMessages();
	m_Burnflash.EnableWindow(TRUE);
}

void COV490ISP::OnBnClickedButtonReadflash()
{
	m_Readflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_FlashRead, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Readflash.EnableWindow(TRUE);
}

void COV490ISP::Flash_Read(void)
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->Log("ov490 SPI read flash...\r\n");

	FILE *fp;
	CFileDialog dlg(FALSE, _T("bin"), NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("Bin files|*.bin|all files | *.*|| "));

	time_t	st_t, end_t;
	double	read_t;
	st_t=time(0);

	CString msg;
	BYTE buf[PAGESIZE];
	memset(buf, 0, PAGESIZE);
	spi_page_read(0x00, (unsigned int*)buf);

	unsigned int total_size = 0;
	unsigned int fw_size = 0;
	unsigned int startAddress = 0;
	unsigned int iAddr = 0; 


	startAddress = (buf[8]<<24)+(buf[9]<<16)+(buf[10]<<8)+(buf[11]);
	fw_size = (buf[0x12]<<8)+(buf[0x13]);
	total_size = startAddress + fw_size;	

	unsigned int header = (buf[0]<<24)+(buf[1]<<16)+(buf[2]<<8)+(buf[3]);
	if(header!=0x4f565449)
	{
		parent->m_Status.SetWindowTextA("Flash is empty!!");
		return;
	}

	msg.Format("total_size=%dbytes\r\n", total_size);
	parent->Log(msg);

	unsigned int fullsize = (total_size/256+1)*256;

	BYTE* readbuf = new BYTE [fullsize];
	memset(readbuf, 0, fullsize);
	for (iAddr=0; iAddr<fullsize; iAddr+=PAGESIZE)
	{
		spi_page_read(iAddr, (unsigned int*)(readbuf+iAddr));
		//ServiceMessages();              
		msg.Format("ov490 fw size=%dbytes, Reading (%d%%)", total_size, iAddr*100/fullsize);
		parent->m_Status.SetWindowTextA(msg);
	}

	end_t	=	time(0);
	read_t	=	difftime(end_t, st_t);
	msg.Format("Read time=%.1fsec\r\n", read_t);
	parent->Log(msg);

	if((dlg.DoModal()) != IDOK)return;	
	int err = fopen_s(&fp, dlg.GetPathName(),"wb+");
	if(err!=0)
	{
		parent->m_Status.SetWindowTextA("Save firmwre file wrong");
		return;
	}
	fwrite(readbuf, 1, total_size, fp);
	//fwrite(buf, 1, total_size, fp);
	fclose(fp);  
	parent->m_Status.SetWindowTextA("ov490 Read Flash OK!");

	delete [] readbuf;	
	readbuf = NULL;
}

void COV490ISP::OnBnClickedButtonBurnflash()
{
	m_Burnflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_FlashBurn, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Burnflash.EnableWindow(TRUE);
}

void COV490ISP::Flash_Burn(void)
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->Log("ov490 SPI burn flash...\r\n");	

	CFileDialog dlg(TRUE,".bin",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"ISP firmware(*.bin)|*.bin; *.bin|all bin files (*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)    return;

	FILE *fp;
	int err = fopen_s(&fp, dlg.GetPathName(),"rb+");
	if(err!=0)
	{
		parent->m_Status.SetWindowTextA("Open firmwre file wrong");
		return;
	}

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	fseek(fp,0L,SEEK_END);
	long file_len = ftell(fp);  
	fseek(fp, 0, SEEK_SET);

	BYTE* buf = new BYTE [file_len];	
	memset(buf, 0, file_len);
	fread(buf, 1, file_len, fp);
	fclose(fp); 

	unsigned int header = (buf[0]<<24)+(buf[1]<<16)+(buf[2]<<8)+(buf[3]);
	if(header!=0x4f565449)
	{
		parent->m_Status.SetWindowTextA("This firmware is not with header!");
		return;
	}

	//erase chip first
	//OnBnClickedButtonErase();
	//unsigned int startAddress = (buf[8]<<24)+(buf[9]<<16)+(buf[10]<<8)+(buf[11]);
	//unsigned int fw_size = (buf[16]<<24)+(buf[17]<<16)+(buf[18]<<8)+(buf[19]);
	//unsigned int total_size = startAddress + fw_size;
	unsigned int iAddr = 0; 
	unsigned int total_size = file_len;
	unsigned int fullsize = (total_size/256+1)*256;
	BYTE* readbuf = new BYTE [fullsize];
	memset(readbuf, 0, fullsize);

	CString msg;
	msg.Format("total_size=%dbytes\r\n", total_size);
	parent->Log(msg);

	for (iAddr=0; iAddr<total_size; iAddr+=PAGESIZE)
	{
		spi_page_write((unsigned int*)(buf+iAddr), iAddr);
		//ServiceMessages();              
		msg.Format("ov490 fw size = %dbytes, Downloading (%d%%)", total_size, iAddr*100/total_size );
		parent->m_Status.SetWindowTextA(msg);
	}
	parent->m_Status.SetWindowTextA("ov490 Download OK!");
	parent->Log("ov490 Download OK!\r\n");

	if(m_check_verify.GetCheck())
	{
		parent->Log("ov490 start verifying\r\n");

		unsigned int i=0;
		for (iAddr=0; iAddr<fullsize; iAddr+=PAGESIZE)
		{
			spi_page_read(iAddr, (unsigned int*)(readbuf+iAddr));
			if(iAddr+PAGESIZE>total_size)
			{
				for(i=iAddr; i<total_size; i++)
				{
					if(buf[i]!=readbuf[i])
					{
						parent->m_Status.SetWindowTextA("ov490 verify error!");
						return;
					}
				}
			}else
			{
				for(i=iAddr; i<(iAddr+PAGESIZE); i++)
				{
					if(buf[i]!=readbuf[i])
					{
						parent->m_Status.SetWindowTextA("ov490 verify error!");
						return;
					}
				}
			}
			msg.Format("verifying (%d%%)", iAddr*100/fullsize);
			parent->m_Status.SetWindowTextA(msg);
		}

		parent->m_Status.SetWindowTextA("ov490 verify OK!");
		parent->Log("ov490 verify OK!\r\n");
	}

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	msg.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(msg);

	delete [] buf;
	delete [] readbuf;

	buf = NULL;
	readbuf = NULL;

	m_Burnflash.EnableWindow(FALSE);
}


void COV490ISP::OnBnClickedButtonIdcheck()
{
	// TODO: Add your control notification handler code here
	unsigned char id[4]={0x48,0x46,0x4a,0x4c};	
	unsigned int id_tmp=0;
	CString strtxt;
	OnBnClickedButtonB8080();
	
	for(int i = 0; i<4; i++)
	{
		if((ReadReg16(id[i], 0x300b)&0xff) == 0x90)
			id_tmp = id_tmp*256 + id[i];
		//id_tmp = id[i];
	}	
	strtxt.Format("%x %x %x %x",id_tmp&0xff, (id_tmp>>8)&0xff, (id_tmp>>16)&0xff, (id_tmp>>24)&0xff);
	m_IDs.SetWindowTextA(strtxt);
}


void COV490ISP::OnBnClickedButtonSetid()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	CString strtxt;
	unsigned char mId;
	m_ISP_ID.GetWindowTextA(strtxt);
	sscanf_s(strtxt,_T("%x"),&mId);	

	OnBnClickedButtonB8080();
	if((ReadReg16(mId&0xff, 0x300b)&0xff) == 0x90)	
	{
		m_ov490id=mId&0xff; 
		m_Spiinit.EnableWindow(TRUE);
		parent->m_Status.SetWindowTextA("ov490 is existed!");

		strtxt.Format("BootFlag=0x%02x\r\n", ReadReg16(m_ov490id, 0x0120));
		parent->Log(strtxt);
		strtxt.Format("FirmwareVer=%d\r\n", ReadReg16(m_ov490id, 0x0102)<<8|ReadReg16(m_ov490id, 0x0103));
		parent->Log(strtxt);
		strtxt.Format("SettingVer=0x%04x\r\n", ReadReg16(m_ov490id, 0x0133)<<8|ReadReg16(m_ov490id, 0x0134));
		parent->Log(strtxt);

	}else parent->m_Status.SetWindowTextA("ov490 is not existed!");
}

UINT COV490ISP::WorkerThread_FlashRead(LPVOID pParam)
{
	COV490ISP* pObject = (COV490ISP*)pParam;

	if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->Flash_Read();

	return 0;   // thread completed successfully
}

UINT COV490ISP::WorkerThread_FlashBurn(LPVOID pParam)
{
	COV490ISP* pObject = (COV490ISP*)pParam;

	if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->Flash_Burn();

	return 0;   // thread completed successfully
}


void COV490ISP::OnEnChangeEditSid()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void COV490ISP::OnEnChangeEditSaddr()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void COV490ISP::OnEnChangeEditSval()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void COV490ISP::OnBnClickedButtonReadsnr()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	RegData = ov490_ReadSensor(SCCB_ID, RegAddr);
	s_SVal.Format(_T("%x"),RegData);
	m_SVal.SetWindowTextA(s_SVal);
	strtxt.Format(_T("%04x %02x;<hc r>\r\n"), RegAddr, RegData);
	parent->Log(strtxt);
}


void COV490ISP::OnBnClickedButtonWritesnr()
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
	ov490_WriteSensor(SCCB_ID, RegAddr, RegData);
	strtxt.Format(_T("%04x %02x;<hc w>\r\n"), RegAddr, RegData);	
	parent->Log(strtxt);
}
