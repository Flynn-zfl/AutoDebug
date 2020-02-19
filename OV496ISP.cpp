// OV496ISP.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "OV496ISP.h"
#include "afxdialogex.h"

#include "AutoDebugDlg.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include ".\\I2C\\i2c.h"

#include ".\\OV496\\OV496.h"
#include ".\\OV496\\OV496BurnFW1Line.h"

#define HOST_CTRL_CMD_IN_PARAM (0x3000)
#define HOST_CTRL_CMD_OUT_PARAM (0x3C00)
#define HOST_CTRL_CMD_INTR_REG (0x8160)
#define HOST_CTRL_CMD_STATUS_REG (0x9C14)
#define HOSTCMD_STATUS_DONE (0x99)
#define HOSTCMD_STATUS_DOING (0x88)
#define HOSTCMD_STATUS_ERR (0x55)
#define HOSTCMD_STATUS_CRC_ERR (0x44)
#define HOSTCMD_STATUS_RESET (0x00)


// COV496ISP dialog
unsigned m_ov496id=00;

void ov496_SccbLock(unsigned char id, BOOL ON)
{
	if(ON) WriteReg16(id, 0x93c0, 0x07);
	else WriteReg16(id, 0x93c0, 0x00);
}

unsigned char ov496_ReadSensor(unsigned char id, unsigned int addr)
{

	unsigned char  pCmdBuf[512];

	pCmdBuf[0] = 0xb0;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x04;
	pCmdBuf[3] = 01;
	pCmdBuf[4] = 02;
	pCmdBuf[5] = (addr>>8)&0xff;
	pCmdBuf[6] = addr&0xff;

	if(OV496SendHostCmd(m_ov496id, pCmdBuf, 0x07) == FALSE){
		return 0xff;
	}
	return ReadReg16(m_ov496id, HOST_CTRL_CMD_OUT_PARAM);
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+0, 0xff); //crc
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+1, 0xff);
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+2, 0xb0); //cmd
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+3, 0x00); 
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+4, 0x04); //param num
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+5, 0x01); //read cmd
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+6, 0x02); //single 16bit addr
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+7, (addr>>8)&0xff); //addrh
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+8, addr&0xff); //addrl
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_INTR_REG, 0x01); //enable, triggerred

	//int i=10;
	//while(i){
	//	if(ReadReg16(m_ov496id, HOST_CTRL_CMD_STATUS_REG) == 0x99) break;
	//	Sleep(10);
	//	i--;
	//}

	//return ReadReg16(m_ov496id, HOST_CTRL_CMD_OUT_PARAM);
}

BOOL ov496_WriteSensor(unsigned char id, unsigned int addr, unsigned char pdata)
{

	unsigned char  pCmdBuf[512];

	pCmdBuf[0] = 0xb0;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x05;
	pCmdBuf[3] = 00;
	pCmdBuf[4] = 02;
	pCmdBuf[5] = (addr>>8)&0xff;
	pCmdBuf[6] = addr&0xff;
	pCmdBuf[7] = pdata&0xff;

	if(OV496SendHostCmd(m_ov496id, pCmdBuf, 0x08) == FALSE){
		return FALSE;
	}

	return TRUE;
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+0, 0xff); //crc
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+1, 0xff);
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+2, 0xb0); //cmd
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+3, 0x00); 
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+4, 0x05); //param num
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+5, 0x00); //write cmd
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+6, 0x02); //single, 16bit addr
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+7, (addr>>8)&0xff); //addrh
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+8, addr&0xff); //addrl
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+9, pdata&0xff); //val
	//WriteReg16(m_ov496id, HOST_CTRL_CMD_INTR_REG, 0x01); //enable, triggerred

	//int i=10;
	//while(i){
	//	if(ReadReg16(m_ov496id, HOST_CTRL_CMD_STATUS_REG) == 0x99) break;
	//	Sleep(10);
	//	i--;
	//}

	//return 1;
}

BOOL HC_CRC_CHECK(bool enable)
{
	unsigned char  pCmdBuf[512];

	pCmdBuf[0] = 0xb6;
	pCmdBuf[1] = 0x00;
	pCmdBuf[2] = 0x03+0x01;
	pCmdBuf[3] = enable;

	if(OV496SendHostCmd(m_ov496id, pCmdBuf, 0x04) == FALSE){
		return FALSE;
	}

	return TRUE;



	//if(enable)
	//{
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+0, 0x2b); //crc
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+1, 0x41);
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+2, 0xb6); //cmd
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+3, 0x00); 
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+4, 0x01); //param num
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+5, 0x01); // enable
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_INTR_REG, 0x01); //enable, triggerred

	//	int i=10;
	//	while(i){
	//		if(ReadReg16(m_ov496id, HOST_CTRL_CMD_STATUS_REG) == 0x99) break;
	//		Sleep(10);
	//		i--;
	//	}

	//}else
	//{
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+0, 0x5d); //crc
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+1, 0xf5);
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+2, 0xb6); //cmd
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+3, 0x00); 
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+4, 0x01); //param num
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_IN_PARAM+5, 0x00); // disable
	//	WriteReg16(m_ov496id, HOST_CTRL_CMD_INTR_REG, 0x01); //enable, triggerred

	//	int i=10;
	//	while(i){
	//		if(ReadReg16(m_ov496id, HOST_CTRL_CMD_STATUS_REG) == 0x99) break;
	//		Sleep(10);
	//		i--;
	//	}
	//}
}

// COV496ISP dialog

IMPLEMENT_DYNAMIC(COV496ISP, CDialogEx)

	COV496ISP::COV496ISP(CWnd* pParent /*=NULL*/)
	: CDialogEx(COV496ISP::IDD, pParent)
	, s_SId(_T(""))
	, s_SAddr(_T(""))
	, s_SVal(_T(""))
{

}

COV496ISP::~COV496ISP()
{
}

void COV496ISP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ISP_ID, m_ISP_ID);
	DDX_Control(pDX, IDC_STATIC_IDS, m_IDs);
	DDX_Control(pDX, IDC_BUTTON_READFLASH, m_ReadFlash);
	DDX_Control(pDX, IDC_BUTTON_BURNFLASH, m_BurnFlash);
	DDX_Control(pDX, IDC_EDIT_SVAL, m_SVal);
	DDX_Control(pDX, IDC_EDIT_SADDR, m_SAddr);
	DDX_Control(pDX, IDC_EDIT_SID, m_SId);
	DDX_Control(pDX, IDC_BUTTON_READSNR, m_ReadSNR);
	DDX_Control(pDX, IDC_BUTTON_WRITESNR, m_WriteSNR);
	DDX_Control(pDX, IDC_CHECK_VERIFY, m_check_verify);

	DDX_Text(pDX, IDC_EDIT_SID, s_SId);
	DDX_Text(pDX, IDC_EDIT_SADDR, s_SAddr);
	DDX_Text(pDX, IDC_EDIT_SVAL, s_SVal);

}


BEGIN_MESSAGE_MAP(COV496ISP, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CHECKID, &COV496ISP::OnBnClickedButtonCheckid)
	ON_BN_CLICKED(IDC_BUTTON2, &COV496ISP::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_READFLASH, &COV496ISP::OnBnClickedButtonReadflash)
	ON_BN_CLICKED(IDC_BUTTON_BURNFLASH, &COV496ISP::OnBnClickedButtonBurnflash)
	ON_BN_CLICKED(IDC_BUTTON_READSNR, &COV496ISP::OnBnClickedButtonReadsnr)
	ON_BN_CLICKED(IDC_BUTTON_WRITESNR, &COV496ISP::OnBnClickedButtonWritesnr)
END_MESSAGE_MAP()


// COV496ISP message handlers
// COV496ISP message handlers
BOOL COV496ISP::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ReadFlash.EnableWindow(FALSE);
	m_BurnFlash.EnableWindow(FALSE);

	m_SId.EnableWindow(FALSE);
	m_SAddr.EnableWindow(FALSE);
	m_SVal.EnableWindow(FALSE);
	m_ReadSNR.EnableWindow(FALSE);
	m_WriteSNR.EnableWindow(FALSE);

	m_ISP_ID.SetWindowTextA("48");
	m_check_verify.SetCheck(0);

	return TRUE;
}

void COV496ISP::OnOK()
{
	//
}

void COV496ISP::OnClose()
{
	CDialog::OnClose();	
	//CDialog::OnOK();
}

void COV496ISP::set_dlg(CDialog * parnet_dlg)
{
	m_pMianDlg = parnet_dlg;
}
void COV496ISP::ServiceMessages()
{
	MSG message;
	while (PeekMessage((LPMSG)&message,NULL,NULL,NULL,PM_REMOVE))
	{
		TranslateMessage((LPMSG)&message);
		DispatchMessage((LPMSG)&message);
	}
}
#define SCCBS_RETRY_TIME    (4)
BOOL COV496ISP::WriteDataToDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	BOOL bReturn = 0;
	unsigned char nRetry = 0;


	unsigned int i = 0;	
	unsigned int j = 0;
	unsigned int nSect = 0;
	unsigned char nSegment = 0;		
	unsigned char nPageSize = 0;

	CString msg;

	for(i=0; i<data_size; i+=SECT_SIZE){		
		if((data_size - i) >= SECT_SIZE){
			nSect = SECT_SIZE;
		}else{
			nSect = data_size - i;
		}		

		bReturn = 0;
		nRetry = 0;
		while(bReturn == 0){			
			if(mode == MODE_ONLY_RAM){
				bReturn = OV496SendFwRam(nID, nSect, i+data_start, pBuf + i);
			}else{
				bReturn = OV496SendFwSector(nID, nSect, i+data_start, pBuf + i);
			}							
			nRetry ++;
			if(nRetry > SCCBS_RETRY_TIME){				
				return 0;
			}	
			break;
		}
		msg.Format("oax496 Downloading (%d%%)", i*100/data_size );
		parent->m_Status.SetWindowTextA(msg);
		//ServiceMessages();
	}	
	//ServiceMessages();

	return 1;
}

void COV496ISP::ReadDataFromDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	unsigned int i = 0;	
	unsigned int j = 0;
	unsigned int nSect = 0;
	unsigned char nSegment = 0;	
	unsigned char nPageSize = 0;
	CString msg;

	for(i=0; i<data_size; i+=SECT_SIZE){		
		if((data_size - i) >= SECT_SIZE){
			nSect = SECT_SIZE;
		}else{
			nSect = data_size - i;
		}

		if(mode == MODE_ONLY_RAM){
			OV496GetFwRam(nID, nSect, i+data_start,  pBuf + i);
		}else{
			OV496GetFwSector(nID, nSect, i+data_start,  pBuf + i);
		}	

		msg.Format("oax496 reading (%d%%)", i*100/data_size );
		parent->m_Status.SetWindowTextA(msg);
		//ServiceMessages();
	}	
	//ServiceMessages();
}


void COV496ISP::OnBnClickedButtonCheckid()
{
	// TODO: Add your control notification handler code here
	unsigned char id[4]={0x48,0x46,0x4a,0x4c};	
	unsigned int id_tmp=0;
	CString strtxt;

	for(int i = 0; i<4; i++)
	{
		ov496_SccbLock(id[i], 0); //unlock
		if((ReadReg16(id[i], 0x81a0)&0xff) == 0x96)
			id_tmp = id_tmp*256 + id[i];
		//id_tmp = id[i];
	}	

	strtxt.Format("%x %x %x %x",id_tmp&0xff, (id_tmp>>8)&0xff, (id_tmp>>16)&0xff, (id_tmp>>24)&0xff);
	m_IDs.SetWindowTextA(strtxt);
}


void COV496ISP::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	CString strtxt;
	unsigned char mId;
	m_ISP_ID.GetWindowTextA(strtxt);
	sscanf_s(strtxt,_T("%x"),&mId);

	//ov496_SccbLock(mId, 0); //unlock
	if((ReadReg16(mId&0xff, 0x81a0)&0xff) == 0x96)	
	{
		parent->m_Status.SetWindowTextA("oax496 is existed!\r\n");
		m_ov496id=mId&0xff; 

		//strtxt.Format("BootFlag=0x%2x\r\n", ReadReg16(m_ov496id, 0x9c17));
		//parent->Log(strtxt);
		strtxt.Format("FirmwareVer=%d\r\n", ReadReg16(m_ov496id, ReadReg16(m_ov496id, 0x9c12)<<8|ReadReg16(m_ov496id, 0x9c13)));
		parent->Log(strtxt);
		strtxt.Format("SettingVer=0x%08x\r\n", ReadReg16(m_ov496id, 0x9c20)<<24|ReadReg16(m_ov496id, 0x9c21)<<16|ReadReg16(m_ov496id, 0x9c22)<<8|ReadReg16(m_ov496id, 0x9c23));
		parent->Log(strtxt);

		m_BurnFlash.EnableWindow(TRUE);
		//m_ReadFlash.EnableWindow(TRUE);
		//m_SId.EnableWindow(TRUE);
		m_SAddr.EnableWindow(TRUE);
		m_SVal.EnableWindow(TRUE);
		m_ReadSNR.EnableWindow(TRUE);
		m_WriteSNR.EnableWindow(TRUE);

		//HC_CRC_CHECK(FALSE);

	}else parent->m_Status.SetWindowTextA("oax496 is not existed!\r\n");
}


void COV496ISP::OnBnClickedButtonReadflash()
{
	// TODO: Add your control notification handler code here
}

void COV496ISP::OnBnClickedButtonBurnflash()
{
	AfxBeginThread(WorkerThread_FlashBurn, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

void COV496ISP::Flash_Burn(void)
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	CFileDialog dlg(TRUE,".bin",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"ISP firmware(*.bin)|*.bin; *.bin|all bin files (*.*)|*.*||");

	if(dlg.DoModal()!=IDOK)    return;

	FILE *fp;
	if((fopen_s(&fp,dlg.GetPathName(),"rb+")) != 0)
	{
		parent->m_Status.SetWindowTextA("Open firmwre file wrong");
		return;
	}

	m_BurnFlash.EnableWindow(FALSE);
	//m_BurnFlash.SetWindowTextA("burning..");

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	fseek(fp,0L,SEEK_END);
	long file_len = ftell(fp);  
	fseek(fp, 0, SEEK_SET);

	BYTE* buf = new BYTE [file_len];
	BYTE* readbuf = new BYTE [file_len];
	memset(readbuf, 0, file_len);
	memset(buf, 0, file_len);
	fread(buf, 1, file_len, fp);
	fclose(fp);    

	BOOL bReturn = 0;
	unsigned char nID = m_ov496id;	
	//step1: download burnfw into pram
	/////////////////////////////////////////////////////////////////////////////	
	OV496InitRam(nID);		
	bReturn = WriteDataToDevice(nID, OV496BurnFW1Line, 0, sizeof(OV496BurnFW1Line), MODE_ONLY_RAM);  //download small burn FW into pram		
	OV496RunRam(nID);	

	//step2: download updata data to spi flash
	/////////////////////////////////////////////////////////////////////////////
	bReturn = WriteDataToDevice(nID, buf, 0, file_len, MODE_RAM_SPI);
	if(bReturn == 0){
		parent->m_Status.SetWindowTextA("Download fails, please try again!\r\n");
	}else{
		parent->m_Status.SetWindowTextA("oax496 Download finish!\r\n");
	}
	/////////////////////////////////////////////////////////////////////////////

	//step3: check update data from spi flash
	/////////////////////////////////////////////////////////////////////////////
	//just use SCCBS CRC to check
	
	unsigned int m_size;
	
	if(m_check_verify.GetCheck())
	{
		m_size = file_len;
		parent->m_Status.SetWindowTextA("oax496 firmware verifying...\r\n");
	}	
	else m_size= 0x100;	
	
	Sleep(TIMER_INTERVAL*4);	
	ReadDataFromDevice(nID, readbuf, 0, m_size, MODE_RAM_SPI);	

	unsigned int i = 0;
	for(i=0; i<m_size; i++){
		if(readbuf[i] != buf[i]){	
			break;
		}
	}
	if(i != m_size){
		parent->m_Status.SetWindowTextA("oax496 verify data dismatch \r\n");
	}else{
		parent->m_Status.SetWindowTextA("oax496 Download Succeed!\r\n");
		parent->Log("oax496 Download Succeed!\r\n");
	}

	//step4: reboot from spi flash
	/////////////////////////////////////////////////////////////////////////////
	Sleep(TIMER_INTERVAL*4);
	OV496Reboot(nID);	
	m_BurnFlash.EnableWindow(TRUE);
	/////////////////////////////////////////////////////////////////////////////

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(msg);

	delete [] buf;
	delete [] readbuf;

	buf = NULL;
	readbuf = NULL;
}

UINT COV496ISP::WorkerThread_FlashBurn(LPVOID pParam)
{
	COV496ISP* pObject = (COV496ISP*)pParam;

	if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->Flash_Burn();

	return 0;   // thread completed successfully
}


void COV496ISP::OnBnClickedButtonReadsnr()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	RegData = ov496_ReadSensor(SCCB_ID, RegAddr);
	s_SVal.Format(_T("%x"),RegData);
	m_SVal.SetWindowTextA(s_SVal);
	strtxt.Format(_T("%04x %02x;<hc r>\r\n"), RegAddr, RegData);
	parent->Log(strtxt);
}


void COV496ISP::OnBnClickedButtonWritesnr()
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
	ov496_WriteSensor(SCCB_ID, RegAddr, RegData);
	strtxt.Format(_T("%04x %02x;<hc w>\r\n"), RegAddr, RegData);	
	parent->Log(strtxt);
}


BOOL COV496ISP::PreTranslateMessage(MSG* pMsg)
{
#if 0
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)    
	{
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val))
		{
			OnBnClickedButtonWritesnr();
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
	{
		if (GetFocus() == GetDlgItem(IDC_EDIT_Addr))
		{
			OnBnClickedButtonReadsnr();			  
		}
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val))
		{
			//(CButton*)GetDlgItem(IDC_EDIT_Addr)->SetFocus();
			//return FALSE;
		}
	}
	else if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_UP)     
	{
		DWORD data;
		CString csTempBuf;
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val)||
			GetFocus() == GetDlgItem(IDC_EDIT_Addr))
		{
			GetDlgItemText(GetFocus()->GetDlgCtrlID(), csTempBuf);
			sscanf_s(csTempBuf,_T("%x"),&data);

			if(GetFocus() == GetDlgItem(IDC_EDIT_Addr))
				csTempBuf.Format(_T("%x"), data+1);
			else
			{
				csTempBuf.Format(_T("%x"), (data>=0xff)?0x0:(data+1));
				SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
				//OnBnClickedButtonWritesnr();
			}
			SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
		} 	
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DOWN)
	{
		DWORD data;
		CString csTempBuf;
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val) ||
			GetFocus() == GetDlgItem(IDC_EDIT_Addr))
		{
			GetDlgItemText(GetFocus()->GetDlgCtrlID(), csTempBuf);
			sscanf_s(csTempBuf,_T("%x"),&data);

			if(GetFocus() == GetDlgItem(IDC_EDIT_Addr))
				csTempBuf.Format(_T("%x"), data-1);
			else
			{
				csTempBuf.Format(_T("%x"), (data>=0xff)?0x0:(data-1));
				SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
				//OnBnClickedButtonWritesnr();
			}
			SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
		}
	}
#endif 	

	return CDialog::PreTranslateMessage(pMsg);


}
