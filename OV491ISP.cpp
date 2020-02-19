// COV491ISP.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "OV491ISP.h"
#include "afxdialogex.h"
#include "AutoDebugDlg.h"
#include ".\\I2C\\i2c.h"
#include ".\\OV491\\OV491_func.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

// COV491ISP dialog
unsigned int m_ov491id=00;
#define MAX_CNT (3)

IMPLEMENT_DYNAMIC(COV491ISP, CDialogEx)

COV491ISP::COV491ISP(CWnd* pParent /*=NULL*/)
	: CDialogEx(COV491ISP::IDD, pParent)
{

}

COV491ISP::~COV491ISP()
{
}

void COV491ISP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ID, m_ISP_ID);
	DDX_Control(pDX, IDC_BUTTON_ERASEFLASH, m_Eraseflash);
	DDX_Control(pDX, IDC_BUTTON_READFLASH, m_Readflash);
	DDX_Control(pDX, IDC_BUTTON_BURNFLASH, m_Burnflash);
	DDX_Control(pDX, IDC_BUTTON_SPIINIT, m_Spiinit);
	DDX_Control(pDX, IDC_CHECK_VERIFY, m_check_verify);
	DDX_Control(pDX, IDC_STATIC_ID, m_IDs);

	DDX_Control(pDX, IDC_EDIT_SVAL, m_SVal);
	DDX_Control(pDX, IDC_EDIT_SADDR, m_SAddr);
	DDX_Control(pDX, IDC_EDIT_SID, m_SId);
	DDX_Control(pDX, IDC_BUTTON_READSNR, m_ReadSNR);
	DDX_Control(pDX, IDC_BUTTON_WRITESNR, m_WriteSNR);

	DDX_Text(pDX, IDC_EDIT_SID, s_SId);
	DDX_Text(pDX, IDC_EDIT_SADDR, s_SAddr);
	DDX_Text(pDX, IDC_EDIT_SVAL, s_SVal);
	DDX_Control(pDX, IDC_CHECK_SECTORERASE, m_SectorErase);
}


BEGIN_MESSAGE_MAP(COV491ISP, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_IDCHECK, &COV491ISP::OnBnClickedButtonIdcheck)
	ON_BN_CLICKED(IDC_BUTTON_SETID, &COV491ISP::OnBnClickedButtonSetid)
	ON_BN_CLICKED(IDC_BUTTON_ISP0, &COV491ISP::OnBnClickedButtonIsp0)
	ON_BN_CLICKED(IDC_BUTTON_ISP1, &COV491ISP::OnBnClickedButtonIsp1)
	ON_BN_CLICKED(IDC_BUTTON_READSNR, &COV491ISP::OnBnClickedButtonReadsnr)
	ON_BN_CLICKED(IDC_BUTTON_WRITESNR, &COV491ISP::OnBnClickedButtonWritesnr)
	ON_BN_CLICKED(IDC_BUTTON_SPIINIT, &COV491ISP::OnBnClickedButtonSpiinit)
	ON_BN_CLICKED(IDC_BUTTON_ERASEFLASH, &COV491ISP::OnBnClickedButtonEraseflash)
	ON_BN_CLICKED(IDC_BUTTON_BURNFLASH, &COV491ISP::OnBnClickedButtonBurnflash)
	ON_BN_CLICKED(IDC_CHECK_SECTORERASE, &COV491ISP::OnBnClickedCheckSectorerase)
END_MESSAGE_MAP()


// COV495ISP message handlers
BOOL COV491ISP::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Spiinit.EnableWindow(FALSE);
	m_Eraseflash.EnableWindow(FALSE);
	m_Readflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	m_SVal.EnableWindow(FALSE);
	m_SAddr.EnableWindow(FALSE);
	m_SId.EnableWindow(FALSE);
	m_ReadSNR.EnableWindow(FALSE);
	m_WriteSNR.EnableWindow(FALSE);

	m_ISP_ID.SetWindowTextA("48");

	((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHECK_VERIFY))->SetCheck(0);

	return TRUE;
}


//void COV491ISP::ServiceMessages()
//{
//	MSG message;
//	while (PeekMessage((LPMSG)&message,NULL,NULL,NULL,PM_REMOVE))
//	{
//		TranslateMessage((LPMSG)&message);
//		DispatchMessage((LPMSG)&message);
//	}
//}

void COV491ISP::OnBnClickedButtonIdcheck()
{
	// TODO: Add your control notification handler code here
	unsigned char id[4]={0x48,0x46,0x4a,0x4c};	
	unsigned int id_tmp=0;
	CString strtxt;
	
	for(int i = 0; i<4; i++)
	{
		if((ReadReg16(id[i], 0x300b)&0xff) == 0x95)
			id_tmp = id_tmp*256 + id[i];
			//id_tmp = id[i];
	}	

	strtxt.Format("%x %x %x %x",id_tmp&0xff, (id_tmp>>8)&0xff, (id_tmp>>16)&0xff, (id_tmp>>24)&0xff);
	m_IDs.SetWindowTextA(strtxt);
}


void COV491ISP::OnBnClickedButtonSetid()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;

	unsigned char mId;
	unsigned int version;

	m_ISP_ID.GetWindowTextA(strtxt);
	sscanf_s(strtxt,_T("%x"),&mId);

	WriteReg16(mId, 0x3516, 0x00); //unlock
	version=(ReadReg16(mId, 0x3001)<<8)&0xff00;
	version+=(ReadReg16(mId, 0x3000))&0xff;

	if(version==0x4951)
	{
		m_ov491id=mId&0xff; 

		m_Spiinit.EnableWindow(TRUE);
		m_SId.EnableWindow(TRUE);
		m_SVal.EnableWindow(TRUE);
		m_SAddr.EnableWindow(TRUE);
		m_ReadSNR.EnableWindow(TRUE);
		m_WriteSNR.EnableWindow(TRUE);


		strtxt.Format("ov491/493/495 is existed", version);
		parent->m_Status.SetWindowTextA(strtxt);
		strtxt.Format("BootFlag=0x%02x\r\n", ReadReg16(m_ov491id, 0x31a4));
		parent->Log(strtxt);
		strtxt.Format("FirmwareVer=%d(0x%x)\r\n", ReadReg16(m_ov491id, 0x31a2)<<8|ReadReg16(m_ov491id, 0x31a3), ReadReg16(m_ov491id, 0x31a2)<<8|ReadReg16(m_ov491id, 0x31a3));
		parent->Log(strtxt);
		strtxt.Format("SettingVer=%d(0x%x)\r\n", ReadReg16(m_ov491id, 0x418a)<<8|ReadReg16(m_ov491id, 0x418b), ReadReg16(m_ov491id, 0x418a)<<8|ReadReg16(m_ov491id, 0x418b));
		parent->Log(strtxt);
	}
	else parent->m_Status.SetWindowTextA("ov491/493/495 is not existed!");	
}


void COV491ISP::OnBnClickedButtonIsp0()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov491id, 0x3516, 0x00); //unlock
	WriteReg16(m_ov491id, 0x354a, 0x1d);
	WriteReg16(m_ov491id, 0x354d, 0x10);

	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->Log("Switch to ISP0\r\n");
}


void COV491ISP::OnBnClickedButtonIsp1()
{
	// TODO: Add your control notification handler code here
	WriteReg16(m_ov491id, 0x3516, 0x00); //unlock
	WriteReg16(m_ov491id, 0x354a, 0x1e);
	WriteReg16(m_ov491id, 0x354d, 0x00);

	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->Log("Switch to ISP1\r\n");
}


void COV491ISP::OnBnClickedButtonReadsnr()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString strtxt;

	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SId.GetWindowTextA(s_SId);
	sscanf_s(s_SId,_T("%x"),&SCCB_ID);
	m_SAddr.GetWindowTextA(s_SAddr);
	sscanf_s(s_SAddr,_T("%x"),&RegAddr);
	//RegData = ov495_ReadSensor(SCCB_ID, RegAddr);
	ReadReg_Sensor(RegAddr, RegData, 0);

	s_SVal.Format(_T("%x"),RegData);
	m_SVal.SetWindowTextA(s_SVal);
	strtxt.Format(_T("%02x %04x %02x;<hc r>\r\n"), SCCB_ID, RegAddr, RegData);
	parent->Log(strtxt);
}


void COV491ISP::OnBnClickedButtonWritesnr()
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
	//ov495_WriteSensor(SCCB_ID, RegAddr, RegData);
	WriteReg_Sensor(RegAddr, RegData, 0);

	strtxt.Format(_T("%02x %04x %02x;<hc w>\r\n"),SCCB_ID, RegAddr, RegData);	
	parent->Log(strtxt);
}


void COV491ISP::OnBnClickedButtonSpiinit()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	m_Spiinit.EnableWindow(FALSE);

	AfxBeginThread(WorkerThread_BootLoader, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);

	parent->m_Status.SetWindowTextA("ov491/493/495 Burn Tool init ok!");
	if (((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->GetCheck())
		m_Eraseflash.EnableWindow(FALSE);
	else m_Eraseflash.EnableWindow(TRUE);
	//m_Readflash.EnableWindow(TRUE);
	//m_Burnflash.EnableWindow(TRUE);
	m_Spiinit.EnableWindow(TRUE);

	
}


void COV491ISP::OnBnClickedButtonEraseflash()
{
	// TODO: Add your control notification handler code here
	m_Eraseflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_Erase, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Eraseflash.EnableWindow(FALSE);	
}


void COV491ISP::OnBnClickedButtonBurnflash()
{
	// TODO: Add your control notification handler code here
	m_Burnflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_Burn, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Burnflash.EnableWindow(TRUE);


}


bool COV491ISP::SCCB_Download_Bootloader(BYTE* buf, DWORD size)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString msg;

	ov495_WriteRegister(m_ov491id, 0x3516, 0x00, 1);
	ov495_WriteRegister(m_ov491id, 0x3010, 0xFF, 1);  // [3]: Reset SC; [6]: reset SRAM
	Sleep(100);	
	ov495_WriteRegister(m_ov491id, 0x3010, 0x04, 1);  // [2]: Reset CPU

	/*---------------------------------------------------------------
	-- Master downloads firmware to SRAM address 0x80140000 --
	-----------------------------------------------------------------*/

	//1. switch to 8014 bank
	ov495_WriteRegister(m_ov491id, 0x3502, 0x20, 1);	
	ov495_WriteRegister(m_ov491id, 0xFFFD, 0x80, 1);
	ov495_WriteRegister(m_ov491id, 0xFFFE, 0x14, 1);

	//2. download bootloader bin file
	DWORD i=0;
	for (i=0; i<size; i+=BURST_SIZE)
	{
		BurstWriteRegister(m_ov491id, 0x00f8+i, buf+i, 1, BURST_SIZE);

		msg.Format("Send bootloader %d%%", i*100/size+1);
		parent->m_Status.SetWindowTextA(msg);
	}

	if (i>size)
		BurstWriteRegister(m_ov491id, 0x00f8+i-BURST_SIZE, buf+i-BURST_SIZE, 1, (unsigned char)(size-i+BURST_SIZE));

	//3. switch to 8020 bank
	ov495_WriteRegister(m_ov491id, 0xFFFD, 0x80, 1);	
	ov495_WriteRegister(m_ov491id, 0xFFFE, 0x20, 1);
	ov495_WriteRegister(m_ov491id, 0x3502, 0x00, 1);

	//4.0 switch PAD clk
	ov495_WriteRegister(m_ov491id, 0x3021, 0x10, 1);		//; change system clk source to PADCLK
	ov495_WriteRegister(m_ov491id, 0x3023, 0x10, 1);		 //; change isp clk source to PADCLK
	ov495_WriteRegister(m_ov491id, 0x3029, 0x10, 1);		  //; change mtx clk source to PADCLK

	//4. boot up 
	int bootmode = 0;	
	ov495_ReadRegister(m_ov491id, 0x3044, bootmode, 1);
	ov495_WriteRegister(m_ov491id, 0x3044, bootmode|0x01, 1); //boot from RAM
	ov495_WriteRegister(m_ov491id, 0x3040, 0x01, 1);	//RISC reset
	ov495_WriteRegister(m_ov491id, 0x3153, 0x00, 1); 	//Disable WC
	ov495_WriteRegister(m_ov491id, 0x3010, 0x00, 1);  // [2]: Release CPU

	//5. check whether bootloader FW boot up or not.
	int val = 0, idx = 0;
	while (val != 0x02	)
	{
		ov495_ReadRegister(m_ov491id, 0x31A4, val);
		Sleep(5);
		idx ++;
		if (idx >=100)
			break;
	}

	if (val == 0x02)
		return 1;
	else
		return 0;
}

bool COV491ISP::DownloadBootloader()
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	DWORD   size;
	BYTE* buffer = NULL;

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);


	size = sizeof(BOOT_FW);
	buffer = new BYTE [size];
	memcpy(buffer, BOOT_FW, size);
	
	bool bRet = 0;
	for (int i=0; i<MAX_CNT; i++)
	{
		bRet = SCCB_Download_Bootloader(buffer, size);
		if (bRet)
		{
			parent->Log("bootloader successfully!\r\n");
			m_Burnflash.EnableWindow(TRUE);
			break;
		}
	}

	if (!bRet)
	{
		parent->Log("bootloader failed!\r\n");
		delete [] buffer;
		return 0;
	}

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("SPI init time=%.1fsec\r\n", write_t);
	parent->Log(msg);

	delete [] buffer;
	return 1;	
}

void COV491ISP::burn_sf()
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	parent->Log("Start Burn firmware...\r\n");

	CFileDialog dlg(TRUE,//TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框 
		".bin",//默认的打开文件的类型 
		NULL,//默认打开的文件名 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,//打开只读文件 
		"USB Data(*.bin)|*.bin; *.dat|所有文件 (*.*)|*.*||");//所有可以打开的文件类型 12

	if(dlg.DoModal()!=IDOK)    return;

	CString m_FilePath = dlg.GetPathName();////////取出文件路径 

	FILE* fp;
	int err = fopen_s(&fp, m_FilePath, "rb");
	if(err != 0) return;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	BYTE* buffer = NULL;
	int size1 = 0;
	size1 = (size/256+1)*256;
	buffer = new BYTE [size1];
	memset(buffer, 0, size1);
	fread(buffer, 1, size, fp);
	fclose(fp);

	DownloadMainFW(buffer, size1);

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(msg);

	delete [] buffer;

	OV49X_Reboot();
}

void COV491ISP::OV49X_Reboot()
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	ov495_WriteRegister(m_ov491id, 0x3010, 0x04, 1); //restart
	Sleep(100);
	ov495_WriteRegister(m_ov491id, 0x3010, 0x08, 1);
	
	int val = 0;
	Sleep(1000);	
	ov495_ReadRegister(m_ov491id, 0x31A4, val, 1);
	if (val == 0xAA)
	{
		parent->Log("FW bootup successfully!\r\n");
		parent->m_Status.SetWindowTextA("FW bootup successfully!");
	}
}

bool COV491ISP::DownloadMainFW(BYTE* buf, DWORD size)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString msg;

	bool flag = 0;
	
	parent->m_Status.SetWindowTextA("FW downloading...\r\n");

	for (DWORD i=0; i<size; i+=SF_PAGE_SIZE)
	{
		if (((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->GetCheck())
		{
			if (i/0x1000*0x1000 == i) // one sector
			{	
				TRACE("e: sector %d\r\n", i/0x1000);
				if (!NM_SendCmd_SectorErase((WORD)(i/SF_PAGE_SIZE)))//erase one sector;
				{
					msg.Format("%xth sector erase error!", i/0x1000);
					parent->m_Status.SetWindowTextA(msg);
					return 0;
				}
			}
		}

		TRACE("w: page %d\r\n", i/SF_PAGE_SIZE);
		if (!(flag = NM_SendCmd_PageWrite(buf+i, i/SF_PAGE_SIZE)))
		{
			msg.Format("%xth page write error!", i/SF_PAGE_SIZE);
			parent->m_Status.SetWindowTextA(msg);
			//MessageBox(msg);
			return 0;
		}
		msg.Format("FW downloading %d%%", i*100/size+1);
		parent->m_Status.SetWindowTextA(msg);
	
	}

	parent->Log("FW download finished!\r\n");

	if (((CButton*)GetDlgItem(IDC_CHECK_VERIFY))->GetCheck())
	{
		parent->Log("FW Verifing ... \r\n");

		BYTE* buf_rd = new BYTE [size];
		memset(buf_rd, 0, size);
		for (DWORD i= 0x00; i<size; i+=SF_PAGE_SIZE)
		{		
			if (!(flag = NM_SendCmd_PageRead(buf_rd+i, i/SF_PAGE_SIZE)))
			{
				msg.Format("%xth page read error!", i/SF_PAGE_SIZE);
				parent->m_Status.SetWindowTextA(msg);

				delete [] buf_rd;
				buf_rd = NULL;
				return 0;
			}
			for(int j=0; j<SF_PAGE_SIZE;j++)
			{
				if (*(buf_rd+i+j)!=*(buf+i+j))
				{
					parent->m_Status.SetWindowTextA("Verified failed");
					
					delete [] buf_rd;
					buf_rd = NULL;
					return 0;
				}
			}

			msg.Format("Verifying %d%%", i*100/size+1);
			parent->m_Status.SetWindowTextA(msg);
			
		}

		//FILE* file = fopen("e:\\rd.bin", "wb");
		//fwrite(buf_rd, 1, size, file);
		//fclose(file);

		delete [] buf_rd;
		buf_rd = NULL;
		parent->Log("FW verified OK!\r\n");
	}	

	return 1;
}

//void COV491ISP::OnBnClickedButton_SetStatus()
//{	
//	CString tmp;
//	int status;
//	GetDlgItem(IDC_EDIT2)->GetWindowText(tmp);
//	sscanf(tmp, TEXT("%x"), &status);
//
//	if (!NM_SPI_Status_Set((BYTE)status))
//	{
//		GetDlgItem(IDC_STATIC_MSG)->SetWindowText("Set Status Failed!");	
//	}
//	else
//	{
//		GetDlgItem(IDC_STATIC_MSG)->SetWindowText("Set Status OK!");
//	}
//}
//
//void COV491ISP::OnBnClickedButton_GetStatus()
//{
//	CString msg;
//	BYTE val = 0;
//	if (!NM_SPI_Status_Get(val))
//	{
//		msg = "Get Status Failed!";		
//	}
//	else
//	{
//		msg.Format("%X", val);
//		GetDlgItem(IDC_EDIT2)->SetWindowText(msg);
//		msg = "Get Status OK!";		
//	}
//	GetDlgItem(IDC_STATIC_MSG)->SetWindowText(msg);
//}

void COV491ISP::ChipErase()
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	bool flag = 0;

	parent->Log("Chip Erasing ...\r\n");
	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);

	//chip erase
	flag = NM_SendCmd_ChipErase();
	if (!flag)
	{
		parent->m_Status.SetWindowTextA("ChipErase cmd failed!");
		return;
	}
	else
	{	
		parent->m_Status.SetWindowTextA("ChipErase cmd OK!");
	}

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("ChipErase time=%.1fsec\r\n", write_t);
	parent->Log(msg);
}

//void COV491ISP::BlockErase()
//{
//		
//	bool flag = 0;
//	//chip erase
//	flag =NM_SendCmd_BlockErase(m_sectorBlockIndex);	
//	if (!flag)
//	{
//		MessageBox("BlockErase cmd failed!");
//		GetDlgItem(IDC_STATIC_MSG)->SetWindowText("BlockErase cmd failed!");
//		return;
//	}
//	else
//	{	
//		GetDlgItem(IDC_STATIC_MSG)->SetWindowText("BlockErase cmd OK!");
//	}
//}
//
//void COV491ISP::SectorErase()
//{
//	UpdateData(1);
//	bool flag = 0;
//	flag = NM_SendCmd_SectorErase(m_sectorBlockIndex);
//	if (!flag)
//	{
//		MessageBox("SectorErase cmd failed!");
//		GetDlgItem(IDC_STATIC_MSG)->SetWindowText("SectorErase cmd failed!");
//		return;
//	}
//	else
//	{	
//		GetDlgItem(IDC_STATIC_MSG)->SetWindowText("SectorErase cmd OK!");
//	}
//}

UINT COV491ISP::WorkerThread_Burn(LPVOID pParam)
{
	COV491ISP* pObject = (COV491ISP*)pParam;

	

	if (pObject == NULL)
	return 1;   // if pObject is not valid		

	pObject->burn_sf();

    return 0;   // thread completed successfully
}

UINT COV491ISP::WorkerThread_Erase( LPVOID pParam)
{
	COV491ISP* pObject = (COV491ISP*)pParam;

    if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->ChipErase();

    return 0;   // thread completed successfully
}

UINT COV491ISP::WorkerThread_BootLoader( LPVOID pParam)
{
	COV491ISP* pObject = (COV491ISP*)pParam;

    if (pObject == NULL)
		return 1;   // if pObject is not valid	
	pObject->DownloadBootloader();

    return 0;   // thread completed successfully
}

void COV491ISP::OnBnClickedCheckSectorerase()
{
	// TODO: Add your control notification handler code here
	if (((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->GetCheck())
		m_Eraseflash.EnableWindow(FALSE);
	else m_Eraseflash.EnableWindow(TRUE);
}
