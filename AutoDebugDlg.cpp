
// AutoDebugDlg.cpp : implementation file

#include "stdafx.h"
#include "AutoDebug.h"
#include "AutoDebugDlg.h"
#include "afxdialogex.h"

#include ".\\I2C\\i2c.h"
#include ".\\I2C\\PantherSDK.h"
#include ".\\I2C\\ftd2xx.h"
#include ".\\I2C\\LibFT4222.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAutoDebugDlg dialog
#define FT4222_I2C_SPEED (400)

UINT8 Device=NULL;
FT_HANDLE ftHandle = NULL;
uint16 sizeTransferred = 0;
bool Is16bitAddr = false;


#ifndef FT4222LIB
#define FT4222LIB
std::vector< FT_DEVICE_LIST_INFO_NODE > g_FT4222DevList;
inline std::string DeviceFlagToString(DWORD flags)
{
	std::string msg;
	msg += (flags & 0x1)? "DEVICE_OPEN" : "DEVICE_CLOSED";
	msg += ", ";
	msg += (flags & 0x2)? "High-speed USB" : "Full-speed USB";
	return msg;
}
void ListFtUsbDevices()
{
	FT_STATUS ftStatus = 0;

	DWORD numOfDevices = 0;
	ftStatus = FT_CreateDeviceInfoList(&numOfDevices);

	for(DWORD iDev=0; iDev<numOfDevices; ++iDev)
	{
		FT_DEVICE_LIST_INFO_NODE devInfo;
		memset(&devInfo, 0, sizeof(devInfo));

		ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type,
			&devInfo.ID, &devInfo.LocId,
			devInfo.SerialNumber,
			devInfo.Description,
			&devInfo.ftHandle);

		if (FT_OK == ftStatus)
		{
			const std::string desc = devInfo.Description;
			if(desc == "FT4222" || desc == "FT4222 A")
			{
				g_FT4222DevList.push_back(devInfo);
			}
		}
	}
}
#endif

void CAutoDebugDlg::WriteReg(int SCCB_Addr, int RegAddr,int RegData)
{
	if(Is16bitAddr) WriteReg16(SCCB_Addr, RegAddr, RegData);
	else if((RegAddr&0xffff) > 0xff) WriteReg16(SCCB_Addr, RegAddr, RegData);
	else WriteReg8(SCCB_Addr, RegAddr, RegData);
}

int CAutoDebugDlg::ReadReg(int SCCB_Addr, int RegAddr)
{
	if(Is16bitAddr) return ReadReg16(SCCB_Addr, RegAddr)&0xff;
	else if((RegAddr&0xffff) > 0xff) return ReadReg16(SCCB_Addr, RegAddr)&0xff;
	else return ReadReg8(SCCB_Addr, RegAddr)&0xff;
}

CAutoDebugDlg::CAutoDebugDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAutoDebugDlg::IDD, pParent)
	, s_Val(_T(""))
	, s_Addr(_T(""))
	, s_SccbId(_T(""))
	, s_Status(_T(""))
	, s_Log(_T(""))
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_CAR);
}

void CAutoDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SCCBID, m_SccbId);
	DDX_Control(pDX, IDC_EDIT_Addr, m_Addr);
	DDX_Control(pDX, IDC_EDIT_Val, m_Val);
	DDX_Text(pDX, IDC_EDIT_Val, s_Val);
	DDX_Text(pDX, IDC_EDIT_Addr, s_Addr);
	DDX_Text(pDX, IDC_EDIT_SCCBID, s_SccbId);

	DDX_Control(pDX, IDC_EDIT_SCCBID2, m_SccbId2);
	DDX_Control(pDX, IDC_EDIT_Addr2, m_Addr2);
	DDX_Control(pDX, IDC_EDIT_Val2, m_Val2);
	DDX_Text(pDX, IDC_EDIT_Val2, s_Val2);
	DDX_Text(pDX, IDC_EDIT_Addr2, s_Addr2);
	DDX_Text(pDX, IDC_EDIT_SCCBID2, s_SccbId2);

	DDX_Control(pDX, IDC_TAB_ISP, m_TabMain);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_Status);
	DDX_Text(pDX, IDC_STATIC_STATUS, s_Status);
	DDX_Control(pDX, IDC_LIST_Log, m_Log);
	DDX_LBString(pDX, IDC_LIST_Log, s_Log);
}

BEGIN_MESSAGE_MAP(CAutoDebugDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_Is16bAddr, &CAutoDebugDlg::OnBnClickedCheckIs16baddr)
	ON_BN_CLICKED(IDC_BUTTON_READ, &CAutoDebugDlg::OnBnClickedButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, &CAutoDebugDlg::OnBnClickedButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CAutoDebugDlg::OnBnClickedButtonSend)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_ISP, &CAutoDebugDlg::OnTcnSelchangeTabIsp)
	ON_BN_CLICKED(IDC_RADIO_Panther, &CAutoDebugDlg::OnBnClickedRadioPanther)
	ON_BN_CLICKED(IDC_RADIO_FT4222, &CAutoDebugDlg::OnBnClickedRadioFt4222)
	ON_BN_CLICKED(IDC_BUTTON_LOG, &CAutoDebugDlg::OnBnClickedButtonLog)
	ON_LBN_SELCHANGE(IDC_LIST_Log, &CAutoDebugDlg::OnLbnSelchangeListLog)
	ON_BN_CLICKED(IDC_BUTTON_CLR, &CAutoDebugDlg::OnBnClickedButtonClr)
	ON_BN_CLICKED(IDC_BUTTON_READ2, &CAutoDebugDlg::OnBnClickedButtonRead2)
	ON_BN_CLICKED(IDC_BUTTON_WRITE2, &CAutoDebugDlg::OnBnClickedButtonWrite2)
END_MESSAGE_MAP()


// CAutoDebugDlg message handlers

BOOL CAutoDebugDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_SHOWNORMAL);

	// TODO: Add extra initialization here
	((CButton*)GetDlgItem(IDC_CHECK_Is16bAddr))->SetCheck(0);
	FT4222_UnInitialize(ftHandle);
	FT_Close(ftHandle);
	OVIF_Command(CMD_OVIF_DRIVER_QUIT, 0, 0);
	//Tabctrl
	m_TabMain.InsertItem(0,_T("DEBUG"));
	m_TabMain.InsertItem(1,_T("ov490"));
	m_TabMain.InsertItem(2,_T("ov492/4/6"));
	m_TabMain.InsertItem(3,_T("ov491/3/5"));
	m_TabMain.InsertItem(4,_T("X1E/X1F"));
	m_TabMain.InsertItem(5,_T("ov491New"));
	m_TabMain.InsertItem(6,_T("oax496"));
	//m_TabMain.InsertItem(5,_T("TI"));
	//m_TabMain.InsertItem(6,_T("Maxim"));
	m_TabMain.InsertItem(7,_T("ov480"));

	

	//m_tiserdes.m_pMianDlg=this;
	//m_maximserdes.m_pMianDlg=this;
	m_ov490isp.m_pMianDlg=this;
	m_ov494isp.m_pMianDlg=this;
	m_ov495isp.m_pMianDlg=this;
	m_ov496isp.m_pMianDlg=this;
	m_gaiaboard.m_pMianDlg=this;
	m_ov480isp.m_pMianDlg=this;
	m_ov491isp.m_pMianDlg=this;
	m_x1fsoc.m_pMianDlg=this;


	//m_tiserdes.Create(IDD_DIALOG_TI_SERDES, &m_TabMain); 
	//m_maximserdes.Create(IDD_DIALOG_MAXIM_SERDES, &m_TabMain); 
	m_ov490isp.Create(IDD_DIALOG_OV490, &m_TabMain); 
	m_ov494isp.Create(IDD_DIALOG_OV494, &m_TabMain); 
	m_ov495isp.Create(IDD_DIALOG_OV495, &m_TabMain); 
	m_ov496isp.Create(IDD_DIALOG_OV496, &m_TabMain); 					
	m_gaiaboard.Create(IDD_DIALOG_GAIA, &m_TabMain); 
	m_ov480isp.Create(IDD_DIALOG_OV480, &m_TabMain); 
	m_ov491isp.Create(IDD_DIALOG_OV491, &m_TabMain);
	m_x1fsoc.Create(IDD_DIALOG_X1F, &m_TabMain); 

	CRect r;    
	m_TabMain.GetClientRect (&r);   
	m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);	
	m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	//m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	//m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAutoDebugDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAutoDebugDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAutoDebugDlg::OnOK()
{
	//
}

void CAutoDebugDlg::OnClose()
{

	system("del *.SMP");
	FT4222_UnInitialize(ftHandle);
	FT_Close(ftHandle);
	OVIF_Command(CMD_OVIF_DRIVER_QUIT, 0, 0);

	CDialog::OnClose();	
	//CDialog::OnOK();
}

BOOL CAutoDebugDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)    
	{
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val))
		{
			OnBnClickedButtonWrite();
		}

		if (GetFocus() == GetDlgItem(IDC_EDIT_Val2))
		{
			OnBnClickedButtonWrite2();
		}
	}
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
	{
		if (GetFocus() == GetDlgItem(IDC_EDIT_Addr))
		{
			OnBnClickedButtonRead();			  
		}
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val))
		{
			(CButton*)GetDlgItem(IDC_EDIT_Addr)->SetFocus();
			(CButton*)GetDlgItem(IDC_EDIT_Addr)->SendMessage( EM_SETSEL, 0, -1 );
			return FALSE;
		}

		if (GetFocus() == GetDlgItem(IDC_EDIT_Addr2))
		{
			OnBnClickedButtonRead2();			  
		}
		if (GetFocus() == GetDlgItem(IDC_EDIT_Val2))
		{
			(CButton*)GetDlgItem(IDC_EDIT_Addr2)->SetFocus();
			(CButton*)GetDlgItem(IDC_EDIT_Addr2)->SendMessage( EM_SETSEL, 0, -1 );
			return FALSE;
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
				OnBnClickedButtonWrite();
			}
			SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
		} 	

		if (GetFocus() == GetDlgItem(IDC_EDIT_Val2)||
			GetFocus() == GetDlgItem(IDC_EDIT_Addr2))
		{
			GetDlgItemText(GetFocus()->GetDlgCtrlID(), csTempBuf);
			sscanf_s(csTempBuf,_T("%x"),&data);

			if(GetFocus() == GetDlgItem(IDC_EDIT_Addr2))
				csTempBuf.Format(_T("%x"), data+1);
			else
			{
				csTempBuf.Format(_T("%x"), (data>=0xff)?0x0:(data+1));
				SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
				OnBnClickedButtonWrite2();
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
				OnBnClickedButtonWrite();
			}
			SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
		}

		if (GetFocus() == GetDlgItem(IDC_EDIT_Val2) ||
			GetFocus() == GetDlgItem(IDC_EDIT_Addr2))
		{
			GetDlgItemText(GetFocus()->GetDlgCtrlID(), csTempBuf);
			sscanf_s(csTempBuf,_T("%x"),&data);

			if(GetFocus() == GetDlgItem(IDC_EDIT_Addr2))
				csTempBuf.Format(_T("%x"), data-1);
			else
			{
				csTempBuf.Format(_T("%x"), (data>=0xff)?0x0:(data-1));
				SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
				OnBnClickedButtonWrite2();
			}
			SetDlgItemText(GetFocus()->GetDlgCtrlID(),csTempBuf);
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CAutoDebugDlg::ServiceMessages()
{
	MSG message;
	while (PeekMessage((LPMSG)&message,NULL,NULL,NULL,PM_REMOVE))
	{
		TranslateMessage((LPMSG)&message);
		DispatchMessage((LPMSG)&message);
	}
}

void CAutoDebugDlg::RunTextScript()
{
	// TODO: Add your control notification handler code here
	CStdioFile fh;
	CString buf1, buf2, fmt;
	int ti, dummy, alen, dlen;
	DWORD addr, data;
	BYTE devid;
	CString ext;
	int line;
	int k;
	CString strtxt;

	Log("Send setting files -start- \r\n");
	if (!fh.Open(m_csFilename,CFile::modeRead)) {
		Log("Cannot open script file \r\n");
		return;
	}
	line = 0;
	while (fh.ReadString(buf1)) {
		if (buf1.GetLength()<6) continue;
		line++;
		ti = 0;
		k = buf1.Find(_T(";"), 0);
		if (k == 0) continue;
		if (k > 0) {
			buf1 = buf1.Left(k);
		}
		buf2 = buf1.Tokenize(_T(", \t"),ti);
		if (buf2.GetLength()==0) continue; if (buf2.Left(1)==_T(';')) continue;
		sscanf_s(buf2,_T("%x"),&dummy);
		devid = dummy;
		buf2 = buf1.Tokenize(_T(", \t"),ti);
		if (buf2.GetLength()==0) continue; if (buf2.Left(1)==_T(';')) continue;
		sscanf_s(buf2,_T("%x"),&dummy);
		addr = dummy;
		alen = (buf2.GetLength()+1)/2;
		buf2 = buf1.Tokenize(_T(", \t"),ti);
		if (buf2.GetLength()==0) continue; if (buf2.Left(1)==_T(';')) continue;
		sscanf_s(buf2,_T("%x"),&dummy);
		data = dummy;
		dlen = (buf2.GetLength()+1)/2;
		if((devid==0xff)&&(addr==0xffff))
		{
			Sleep(data); //delay ms
			strtxt.Format(_T("Delay%dms\r\n"), data);
			Log(strtxt);
		}
		else 
		{
			WriteReg(devid, addr, data);
			if(Is16bitAddr)strtxt.Format(_T("%02x %04x %02x;<w>\r\n"), devid, addr, data);
			else if(addr>0xff)strtxt.Format(_T("%02x %04x %02x;<w>\r\n"), devid, addr, data);
			else strtxt.Format(_T("%02x %02x %02x;<w>\r\n"), devid, addr, data);
			Log(strtxt);		
			if ((line%500)==0) ServiceMessages();
		}
	}
	fh.Close();
	Log("send setting files -end- \r\n");
}

void CAutoDebugDlg::OnBnClickedCheckIs16baddr()
{
	// TODO: Add your control notification handler code here
	if ( BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_Is16bAddr) )
	{
		Is16bitAddr = true; // 16bit address mode
	}
	else
	{
		Is16bitAddr = false; // auto mode 
	}
}

void CAutoDebugDlg::OnBnClickedButtonRead()
{
	// TODO: Add your control notification handler code here
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SccbId.GetWindowTextA(s_SccbId);
	sscanf_s(s_SccbId,_T("%x"),&SCCB_ID);
	m_Addr.GetWindowTextA(s_Addr);
	sscanf_s(s_Addr,_T("%x"),&RegAddr);
	RegData = ReadReg(SCCB_ID, RegAddr);
	s_Val.Format(_T("%x"),RegData);
	m_Val.SetWindowTextA(s_Val);
	if(Is16bitAddr)strtxt.Format(_T("%02x %04x %02x;<r>\r\n"),SCCB_ID, RegAddr, RegData);
	else if(RegAddr>0xff)strtxt.Format(_T("%02x %04x %02x;<r>\r\n"),SCCB_ID, RegAddr, RegData);
	else strtxt.Format(_T("%02x %02x %02x;<r>\r\n"),SCCB_ID, RegAddr, RegData);
	Log(strtxt);	
}

void CAutoDebugDlg::OnBnClickedButtonWrite()
{
	// TODO: Add your control notification handler code here
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SccbId.GetWindowTextA(s_SccbId);
	sscanf_s(s_SccbId,_T("%x"),&SCCB_ID);
	m_Addr.GetWindowTextA(s_Addr);
	sscanf_s(s_Addr,_T("%x"),&RegAddr);
	m_Val.GetWindowTextA(s_Val);
	sscanf_s(s_Val,_T("%x"),&RegData);
	WriteReg(SCCB_ID, RegAddr, RegData);
	if(Is16bitAddr)strtxt.Format(_T("%02x %04x %02x;<w>\r\n"),SCCB_ID, RegAddr, RegData);
	else if(RegAddr>0xff)strtxt.Format(_T("%02x %04x %02x;<w>\r\n"),SCCB_ID, RegAddr, RegData);
	else strtxt.Format(_T("%02x %02x %02x;<w>\r\n"),SCCB_ID, RegAddr, RegData);
	Log(strtxt);	
}

void CAutoDebugDlg::OnBnClickedButtonSend()
{
	// TODO: Add your control notification handler code here
	CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
	if((openFileDialog.DoModal()) == IDOK)
	{
		m_csFilename = openFileDialog.GetPathName();
		RunTextScript();
	}
}

void CAutoDebugDlg::OnTcnSelchangeTabIsp(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	CRect r;    
	m_TabMain.GetClientRect(&r);
	if((m_TabMain.GetCurSel()==0))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}
	else if((m_TabMain.GetCurSel()==1))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}
	else if((m_TabMain.GetCurSel()==2))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}
	else if((m_TabMain.GetCurSel()==3))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}
	else if((m_TabMain.GetCurSel()==4))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
	}
	else if((m_TabMain.GetCurSel()==5))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}
	else if((m_TabMain.GetCurSel()==6))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}
	else if((m_TabMain.GetCurSel()==7))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		/*m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);*/
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}/*else if((m_TabMain.GetCurSel()==8))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}else if((m_TabMain.GetCurSel()==9))
	{
		m_gaiaboard.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);	
		m_ov490isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov494isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov495isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov496isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_tiserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_maximserdes.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov480isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
		m_ov491isp.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_SHOWWINDOW);
		m_x1fsoc.SetWindowPos(NULL,0,20,r.right,r.bottom, SWP_HIDEWINDOW);
	}*/

	*pResult = 0;
}

void CAutoDebugDlg::OnBnClickedRadioPanther()
{
	// TODO: Add your control notification handler code here
	OVIF_Command(CMD_OVIF_DRIVER_QUIT, 0, 0);
	FT4222_UnInitialize(ftHandle);
	FT_Close(ftHandle);

	int nRet = OVIF_Command(CMD_OVIF_DRIVER_INIT, 0, 0);
	if (nRet == -1)
	{		
		m_Status.SetWindowTextA("Unable to find Panther device!\r\n");
		Log("Unable to find Panther device!\r\n");
	}
	else
	{
		Device=PANTHER;
		SetWindowText("AutoDebug(Panther)");
		m_Status.SetWindowTextA("Find Panther device!\r\n");
		Log("Find Panther device!\r\n");
	}
}

void CAutoDebugDlg::OnBnClickedRadioFt4222()
{
	// TODO: Add your control notification handler code here
	FT4222_UnInitialize(ftHandle);
	FT_Close(ftHandle);
	OVIF_Command(CMD_OVIF_DRIVER_QUIT, 0, 0);

	ListFtUsbDevices();
	if(g_FT4222DevList.empty()) {
		m_Status.SetWindowTextA("No FT4222 is found!\r\n");
		Log("No FT4222 is found!\r\n");
		//m_SccbId.EnableWindow(FALSE);
		//m_Addr.EnableWindow(FALSE);
		//m_Val.EnableWindow(FALSE);
		return ;
	}

	const FT_DEVICE_LIST_INFO_NODE& devInfo = g_FT4222DevList[0];

	FT_STATUS ftStatus;
	ftStatus = FT_OpenEx((PVOID)g_FT4222DevList[0].LocId, FT_OPEN_BY_LOCATION, &ftHandle);
	if (FT_OK != ftStatus)
	{
		m_Status.SetWindowTextA("Open a FT4222 device failed!\r\n");
		Log("Open a FT4222 device failed!\r\n");
		return ;
	}
	m_Status.SetWindowTextA("init FT4222 as I2C master!\r\n");
	Log("init FT4222 as I2C master!\r\n");

	ftStatus = FT4222_I2CMaster_Init(ftHandle, FT4222_I2C_SPEED);
	if (FT_OK != ftStatus)
	{
		m_Status.SetWindowTextA("Init FT4222 as I2C master device failed!\r\n");
		Log("Init FT4222 as I2C master device failed!\r\n");
		return ;
	}
	Device=FT4222;
	SetWindowText("AutoDebug(FT4222)");
	m_Status.SetWindowTextA("FT4222 device connected!\r\n");
	Log("FT4222 device connected!\r\n");
}

void CAutoDebugDlg::OnBnClickedButtonLog()
{
	// TODO: Add your control notification handler code here
	FILE *fp;

	CFileDialog saveFileDialog(FALSE, _T("log"), NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("log files|*.log|all files | *.*|| ")); 
	CString FileName;
	if(m_Log.GetCount()==0) return; 
	if((saveFileDialog.DoModal()) == IDOK)
	{
		FileName = saveFileDialog.GetPathName();

		int err=fopen_s(&fp, FileName,"w+");
		if(err) return;
		CString strText;
		int i = 0;
		do
		{
			m_Log.GetText(i, strText);
			fprintf(fp,"%s", strText);
			i++;
		}while( i<m_Log.GetCount() );
		fclose(fp);
	}
}

void CAutoDebugDlg::Log(LPCTSTR text)
{
	m_Log.SetTopIndex(m_Log.AddString(text));
	m_Log.SetCurSel(m_Log.GetCount()-1);
}

void CAutoDebugDlg::OnLbnSelchangeListLog()
{
	// TODO: Add your control notification handler code here
}

void CAutoDebugDlg::OnBnClickedButtonClr()
{
	// TODO: Add your control notification handler code here
	m_Log.ResetContent();
	system("del *.SMP");

}


void CAutoDebugDlg::OnBnClickedButtonRead2()
{
	// TODO: Add your control notification handler code here
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SccbId2.GetWindowTextA(s_SccbId2);
	sscanf_s(s_SccbId2,_T("%x"),&SCCB_ID);
	m_Addr2.GetWindowTextA(s_Addr2);
	sscanf_s(s_Addr2,_T("%x"),&RegAddr);
	RegData = ReadReg(SCCB_ID, RegAddr);
	s_Val2.Format(_T("%x"),RegData);
	m_Val2.SetWindowTextA(s_Val2);
	if(Is16bitAddr)strtxt.Format(_T("%02x %04x %02x;<r>\r\n"),SCCB_ID, RegAddr, RegData);
	else if(RegAddr>0xff)strtxt.Format(_T("%02x %04x %02x;<r>\r\n"),SCCB_ID, RegAddr, RegData);
	else strtxt.Format(_T("%02x %02x %02x;<r>\r\n"),SCCB_ID, RegAddr, RegData);
	Log(strtxt);	
}

void CAutoDebugDlg::OnBnClickedButtonWrite2()
{
	// TODO: Add your control notification handler code here	
	CString strtxt;
	int SCCB_ID=0, RegData=0, RegAddr=0;
	m_SccbId2.GetWindowTextA(s_SccbId2);
	sscanf_s(s_SccbId2,_T("%x"),&SCCB_ID);
	m_Addr2.GetWindowTextA(s_Addr2);
	sscanf_s(s_Addr2,_T("%x"),&RegAddr);
	m_Val2.GetWindowTextA(s_Val2);
	sscanf_s(s_Val2,_T("%x"),&RegData);
	WriteReg(SCCB_ID, RegAddr, RegData);
	if(Is16bitAddr)strtxt.Format(_T("%02x %04x %02x;<w>\r\n"),SCCB_ID, RegAddr, RegData);
	else if(RegAddr>0xff)strtxt.Format(_T("%02x %04x %02x;<w>\r\n"),SCCB_ID, RegAddr, RegData);
	else strtxt.Format(_T("%02x %02x %02x;<w>\r\n"),SCCB_ID, RegAddr, RegData);
	Log(strtxt);
}
