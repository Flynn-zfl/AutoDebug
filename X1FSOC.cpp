// X1FSOC.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "X1FSOC.h"
#include "afxdialogex.h"
#include "AutoDebugDlg.h"
#include ".\\I2C\\i2c.h"
#include ".\\X1F\\X1F.h"
#include ".\\X1F\\X1F_BurnFWPro.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <time.h>

// CX1FSOC dialog
#define SCCBS_RETRY_TIME    (4)

unsigned int m_x1f1id=00;

enum BurnToolErrorCode
{
	ERROR_CODE_INITRAM = 1001,
	ERROR_CODE_WRITE_BURN_FW,
	ERROR_CODE_RUN_RAM,
	ERROR_CODE_WRITE_FW_BIN,
	ERROR_CODE_CHECK_SPI_DATA,
	ERROR_CODE_SPI_PROTECT,
};

IMPLEMENT_DYNAMIC(CX1FSOC, CDialogEx)

CX1FSOC::CX1FSOC(CWnd* pParent /*=NULL*/)
	: CDialogEx(CX1FSOC::IDD, pParent)
{

}

CX1FSOC::~CX1FSOC()
{
}

void CX1FSOC::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BUTTON_ERASEFLASH, m_Eraseflash);
	DDX_Control(pDX, IDC_BUTTON_READFLASH, m_Readflash);
	DDX_Control(pDX, IDC_BUTTON_BURNFLASH, m_Burnflash);
	DDX_Control(pDX, IDC_BUTTON_SPIINIT, m_Spiinit);
	DDX_Control(pDX, IDC_CHECK_VERIFY, m_check_verify);
	DDX_Control(pDX, IDC_STATIC_ID, m_IDs);
	DDX_Control(pDX, IDC_CHECK_SECTORERASE, m_SectorErase);
	DDX_Control(pDX, IDC_EDIT_ID, m_ISP_ID);
}


BEGIN_MESSAGE_MAP(CX1FSOC, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_IDCHECK, &CX1FSOC::OnBnClickedButtonIdcheck)
	ON_BN_CLICKED(IDC_BUTTON_SETID, &CX1FSOC::OnBnClickedButtonSetid)
	ON_BN_CLICKED(IDC_BUTTON_SPIINIT, &CX1FSOC::OnBnClickedButtonSpiinit)
	ON_BN_CLICKED(IDC_BUTTON_ERASEFLASH, &CX1FSOC::OnBnClickedButtonEraseflash)
	ON_BN_CLICKED(IDC_BUTTON_READFLASH, &CX1FSOC::OnBnClickedButtonReadflash)
	ON_BN_CLICKED(IDC_BUTTON_BURNFLASH, &CX1FSOC::OnBnClickedButtonBurnflash)
	ON_BN_CLICKED(IDC_CHECK_SECTORERASE, &CX1FSOC::OnBnClickedCheckSectorerase)
	ON_BN_CLICKED(IDC_CHECK_VERIFY, &CX1FSOC::OnBnClickedCheckVerify)
END_MESSAGE_MAP()

BOOL CX1FSOC::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Spiinit.EnableWindow(FALSE);
	m_Eraseflash.EnableWindow(FALSE);
	m_Readflash.EnableWindow(FALSE);
	m_Burnflash.EnableWindow(FALSE);

	m_ISP_ID.SetWindowTextA("6c");

	((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHECK_VERIFY))->SetCheck(0);

	return TRUE;
}

// CX1FSOC message handlers


void CX1FSOC::OnBnClickedButtonIdcheck()
{
	// TODO: Add your control notification handler code here
	unsigned char id[1]={0x6c};	
	unsigned int id_tmp=0;
	CString strtxt;
	
	for(int i = 0; i<1; i++)
	{
		if((ReadReg16(id[i], 0x300a)&0xff) == 0x58)
			id_tmp = id_tmp*256 + id[i];
	}	

	strtxt.Format("%x",id_tmp&0xff);
	m_IDs.SetWindowTextA(strtxt);
}


void CX1FSOC::OnBnClickedButtonSetid()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	CString strtxt;

	unsigned char mId;
	unsigned int version;

	m_ISP_ID.GetWindowTextA(strtxt);
	sscanf_s(strtxt,_T("%x"),&mId);

	version=ReadReg16(mId, 0x300a);

	if(version==0x58)
	{
		m_x1f1id=mId&0xff; 

		m_Spiinit.EnableWindow(TRUE);

		strtxt.Format("X1E/X1F is existed", version);
		parent->m_Status.SetWindowTextA(strtxt);
	}
	else parent->m_Status.SetWindowTextA("X1E/X1F is not existed!");	
}

UINT CX1FSOC::WorkerThread_Burn(LPVOID pParam)
{
	CX1FSOC* pObject = (CX1FSOC*)pParam;
	
	if (pObject == NULL)
	return 1;   // if pObject is not valid		

	pObject->burn_sf();

    return 0;   // thread completed successfully
}

UINT CX1FSOC::WorkerThread_Erase( LPVOID pParam)
{
	CX1FSOC* pObject = (CX1FSOC*)pParam;

    if (pObject == NULL)
		return 1;   // if pObject is not valid	

	pObject->ChipErase();

    return 0;   // thread completed successfully
}

UINT CX1FSOC::WorkerThread_BootLoader( LPVOID pParam)
{
	CX1FSOC* pObject = (CX1FSOC*)pParam;

    if (pObject == NULL)
		return 1;   // if pObject is not valid	
	pObject->init_sf();

    return 0;   // thread completed successfully
}


void CX1FSOC::OnBnClickedButtonSpiinit()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	m_Spiinit.EnableWindow(FALSE);

	AfxBeginThread(WorkerThread_BootLoader, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);

	//parent->m_Status.SetWindowTextA("X1E/X1F SPI init");
	if (((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->GetCheck())
		m_Eraseflash.EnableWindow(FALSE);
	else m_Eraseflash.EnableWindow(TRUE);
	//m_Readflash.EnableWindow(TRUE);
	//m_Burnflash.EnableWindow(TRUE);
	m_Spiinit.EnableWindow(TRUE);
}


void CX1FSOC::OnBnClickedButtonEraseflash()
{
	// TODO: Add your control notification handler code here
	m_Eraseflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_Erase, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Eraseflash.EnableWindow(FALSE);
}


void CX1FSOC::OnBnClickedButtonReadflash()
{
	// TODO: Add your control notification handler code here
}


void CX1FSOC::OnBnClickedButtonBurnflash()
{
	// TODO: Add your control notification handler code here
	m_Burnflash.EnableWindow(FALSE);
	AfxBeginThread(WorkerThread_Burn, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	m_Burnflash.EnableWindow(TRUE);
}


void CX1FSOC::OnBnClickedCheckSectorerase()
{
	// TODO: Add your control notification handler code here
	if (((CButton*)GetDlgItem(IDC_CHECK_SECTORERASE))->GetCheck())
		m_Eraseflash.EnableWindow(FALSE);
	else m_Eraseflash.EnableWindow(TRUE);
}
void CX1FSOC::init_sf()
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	unsigned char nID = m_x1f1id;
	//Step 1: Boot FW
	OVChipInitRam(nID);
	int nResult = WriteDataToDevice(nID, OVChipBurnFW, 0, sizeof(OVChipBurnFW), MODE_ONLY_RAM);  //download small burn FW into pram	
	if (nResult > 0)
	{
		parent->m_Status.SetWindowTextA("X1E/X1F Bootloader sending failed!");
		return;
	}

	nResult = OVChipRunRam(nID);
	if (nResult > 0)
	{
		parent->m_Status.SetWindowTextA("X1E/X1F Bootloader runing failed!");
		return;
	}
	parent->m_Status.SetWindowTextA("X1E/X1F Bootloader Ready!\r\n");
	parent->Log("X1E/X1F Bootloader Ready!\r\n");
	m_Burnflash.EnableWindow(TRUE);
}
void CX1FSOC::ChipErase()
{
	// TODO: Add your control notification handler code here

}
void CX1FSOC::burn_sf()
{
	// TODO: Add your control notification handler code here	
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	

	CFileDialog dlg(TRUE,
		".bin",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"USB Data(*.bin)|*.bin|all files(*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
	{
		parent->Log("open bin file error!!\r\n");
		return;
	}

	CString m_FilePath = dlg.GetPathName();

	FILE* fp;
	int err = fopen_s(&fp, m_FilePath, "rb");
	if(err != 0) return;

	fseek(fp,0L,SEEK_END);
	long file_len = ftell(fp);  
	fseek(fp, 0, SEEK_SET);

	BYTE* buf = new BYTE [file_len];
	BYTE* readbuf = new BYTE [file_len];
	memset(readbuf, 0, file_len);
	memset(buf, 0, file_len);
	fread(buf, 1, file_len, fp);
	fclose(fp);  

	time_t	st_t, end_t;
	double	write_t;
	st_t=time(0);
	parent->Log("Start Burn firmware...\r\n");

	unsigned char nID = m_x1f1id;
	//step2: download bin file data to spi flash
	int nResult = WriteDataToDevice(nID, buf, 0, file_len, MODE_RAM_SPI);  //download small burn FW into pram	
	if (nResult > 0)
	{
		return;
	}

	//step3: check update data from spi flash
	long m_CheckSize=0;
	if (((CButton*)GetDlgItem(IDC_CHECK_VERIFY))->GetCheck())
		m_CheckSize = file_len;
	else m_CheckSize= 0x1000;

	ReadDataFromDevice(nID, readbuf, 0, m_CheckSize, MODE_RAM_SPI);

	long i = 0;
	for (i = 0; i<m_CheckSize; i++)
	{
		if (readbuf[i] != buf[i])
		{
			break;
		}
	}

	if(i != m_CheckSize){
		parent->m_Status.SetWindowTextA("SCCB CRC check data dismatch!\r\n");
	}else{
		parent->m_Status.SetWindowTextA("X1E/X1F Download Succeed!\r\n");
		parent->Log("X1E/X1F Download Succeed!\r\n");
	}

	end_t	=	time(0);
	write_t	=	difftime(end_t, st_t);
	CString msg;
	msg.Format("Burn time=%.1fsec\r\n", write_t);
	parent->Log(msg);

	//step4: reboot
	Sleep(400);
	OVChipReboot(nID);

	delete [] buf;
	delete [] readbuf;

	buf = NULL;
	readbuf = NULL;
}
void CX1FSOC::OnBnClickedCheckVerify()
{
	// TODO: Add your control notification handler code here
}
int CX1FSOC::WriteDataToDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString msg;

	unsigned int nResult = 0;
	unsigned int nSect = 0;
	for (unsigned int i = 0; i<data_size; i += SECT_SIZE)
	{
		if ((data_size - i) >= SECT_SIZE)
			nSect = SECT_SIZE;
		else
			nSect = data_size - i;

		BOOL bReturn = 0;
		int nRetry = 0;
		while (bReturn == 0)
		{
			nResult = 0;
			if (mode == MODE_ONLY_RAM)
			{
				bReturn = SendFwRam_Chip(nID, nSect, i + data_start, pBuf + i);
				if (!bReturn)
					nResult = ERROR_CODE_WRITE_BURN_FW;
			}
			else
			{
				bReturn = SendFwSector_Chip(nID, nSect, i + data_start, pBuf + i);
				if (!bReturn)
					nResult = ERROR_CODE_WRITE_FW_BIN;
			}
			nRetry++;
			if (nRetry > SCCBS_RETRY_TIME)
				return nResult;
		}
		msg.Format("X1E/X1F Downloading (%d%%)", i*100/data_size );
		parent->m_Status.SetWindowTextA(msg);
	}
	return nResult;
}

void CX1FSOC::ReadDataFromDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode)
{
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	CString msg;

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int nSect = 0;
	unsigned char nSegment = 0;
	unsigned char nPageSize = 0;
	OVChipInitReadSF(nID);
	for (i = 0; i<data_size; i += SECT_SIZE)
	{
		if ((data_size - i) >= SECT_SIZE)
			nSect = SECT_SIZE;
		else
			nSect = data_size - i;

		if (mode == MODE_ONLY_RAM)
			GetFwRam_Chip(nID, nSect, i + data_start, (pBuf + i));
		else
			GetFwSector_Chip(nID, nSect, i + data_start, (pBuf + i));

		msg.Format("ov494 reading (%d%%)", i*100/data_size );
		parent->m_Status.SetWindowTextA(msg);
	}
}