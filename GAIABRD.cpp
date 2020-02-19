// GAIABRD.cpp : implementation file
//

#include "stdafx.h"
#include "AutoDebug.h"
#include "GAIABRD.h"
#include "afxdialogex.h"
#include "AutoDebugDlg.h"

#include <fstream>
#include <iostream>
#include <string>  

#include ".\\I2C\\i2c.h"

using namespace std;

CString m_BUTTON1;
CString m_BUTTON2;
CString m_BUTTON3;
CString m_BUTTON4;
CString m_BUTTON5;
CString m_BUTTON6;
CString m_BUTTON7;
CString m_BUTTON8;

struct RegList
{
	BYTE SccbId;
	DWORD RegAddr;
	DWORD RegVal;
	int Counter;
};
RegList Reglist1[20][2000];

#define BurstSize (32)

//std::string CString2string(CString input)
//{
//#ifdef UNICODE
//    int len = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(input), -1, NULL, 0, NULL, NULL);
//    char *str = new char[len];
//    memset(str, 0, len);
//    WideCharToMultiByte(CP_ACP, 0, LPCWSTR(input), -1, str, len, NULL, NULL);
//    std::string output(str);
//    delete[] str;
//    return output;
//#else
//    return std::string((LPCSTR)input);
//#endif // !UNICODE
//}
//
//CString string2CString(std::string input)
//{
//#ifdef UNICODE
//    int len = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, NULL, 0);
//    wchar_t *wstr = new wchar_t[len];
//    memset(wstr, 0, len*sizeof(wchar_t));
//    MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, wstr, len);
//    CStringW output = wstr;
//    delete[] wstr;
//
//    return output;
//#else
//    return CString(input.c_str());
//#endif // !UNICODE
//}

// CGAIABRD dialog

IMPLEMENT_DYNAMIC(CGAIABRD, CDialogEx)

CGAIABRD::CGAIABRD(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGAIABRD::IDD, pParent)
{

}

CGAIABRD::~CGAIABRD()
{
}

void CGAIABRD::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, c_BUTTON1);
	DDX_Control(pDX, IDC_BUTTON2, c_BUTTON2);
	DDX_Control(pDX, IDC_BUTTON3, c_BUTTON3);
	DDX_Control(pDX, IDC_BUTTON4, c_BUTTON4);
	DDX_Control(pDX, IDC_BUTTON5, c_BUTTON5);
	DDX_Control(pDX, IDC_BUTTON6, c_BUTTON6);
	DDX_Control(pDX, IDC_BUTTON7, c_BUTTON7);
	DDX_Control(pDX, IDC_BUTTON8, c_BUTTON8);
	DDX_Control(pDX, IDC_COMBO1, m_Combobox);

	DDX_Control(pDX, IDC_EDIT_DEVID, m_SccbId);
	DDX_Control(pDX, IDC_EDIT_Addr, m_Addr);
	DDX_Control(pDX, IDC_EDIT_LEN, m_Length);
	DDX_Text(pDX, IDC_EDIT_LEN, s_Length);
	DDX_Text(pDX, IDC_EDIT_Addr, s_Addr);
	DDX_Text(pDX, IDC_EDIT_DEVID, s_SccbId);
}


BOOL CGAIABRD::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(pMsg->message==WM_RBUTTONDOWN) //可以改为任何消息
	{
		POINT pt;
		::GetCursorPos(&pt);          //得到鼠标动作的位置		

		CRect BUTTON1_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON1)->GetSafeHwnd(),&BUTTON1_r);   //得到button所在的rect
		if(PtInRect(&BUTTON1_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON1 = openFileDialog.GetPathName();
				c_BUTTON1.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON2_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON2)->GetSafeHwnd(),&BUTTON2_r);   //得到button所在的rect
		if(PtInRect(&BUTTON2_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON2 = openFileDialog.GetPathName();
				c_BUTTON2.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON3_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON3)->GetSafeHwnd(),&BUTTON3_r);   //得到button所在的rect
		if(PtInRect(&BUTTON3_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON3 = openFileDialog.GetPathName();
				c_BUTTON3.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON4_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON4)->GetSafeHwnd(),&BUTTON4_r);   //得到button所在的rect
		if(PtInRect(&BUTTON4_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON4 = openFileDialog.GetPathName();
				c_BUTTON4.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON5_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON5)->GetSafeHwnd(),&BUTTON5_r);   //得到button所在的rect
		if(PtInRect(&BUTTON5_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON5 = openFileDialog.GetPathName();
				c_BUTTON5.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON6_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON6)->GetSafeHwnd(),&BUTTON6_r);   //得到button所在的rect
		if(PtInRect(&BUTTON6_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON6 = openFileDialog.GetPathName();
				c_BUTTON6.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON7_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON7)->GetSafeHwnd(),&BUTTON7_r);   //得到button所在的rect
		if(PtInRect(&BUTTON7_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON7 = openFileDialog.GetPathName();
				c_BUTTON7.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

		CRect BUTTON8_r;
		::GetWindowRect(GetDlgItem(IDC_BUTTON8)->GetSafeHwnd(),&BUTTON8_r);   //得到button所在的rect
		if(PtInRect(&BUTTON8_r,pt))   //判断鼠标动作是否在button范围之内  
		{
			CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
			if((openFileDialog.DoModal()) == IDOK)
			{
				m_BUTTON8 = openFileDialog.GetPathName();
				c_BUTTON8.SetWindowTextA(openFileDialog.GetFileName());
				return CDialog::PreTranslateMessage(pMsg);
			}
		}

	}

	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CGAIABRD, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CGAIABRD::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CGAIABRD::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CGAIABRD::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CGAIABRD::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CGAIABRD::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON2, &CGAIABRD::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON7, &CGAIABRD::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CGAIABRD::OnBnClickedButton8)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CGAIABRD::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CGAIABRD::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CGAIABRD::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_BATCHREAD, &CGAIABRD::OnBnClickedButtonBatchread)
	ON_BN_CLICKED(IDC_CHECK_Is16bAddrBatch, &CGAIABRD::OnBnClickedCheckIs16baddrbatch)
END_MESSAGE_MAP()


// CGAIABRD message handlers
BOOL CGAIABRD::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	((CButton*)GetDlgItem(IDC_CHECK_Is16bAddrBatch))->SetCheck(1);

	return TRUE;
}

void CGAIABRD::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON1;
	parent->RunTextScript();

}

void CGAIABRD::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON2;
	parent->RunTextScript();
}

void CGAIABRD::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON3;
	parent->RunTextScript();
}


void CGAIABRD::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON4;
	parent->RunTextScript();
}


void CGAIABRD::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON5;
	parent->RunTextScript();
}


void CGAIABRD::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON6;
	parent->RunTextScript();
}


void CGAIABRD::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON7;
	parent->RunTextScript();
}


void CGAIABRD::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;
	parent->m_csFilename = m_BUTTON8;
	parent->RunTextScript();
}


void CGAIABRD::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
}


void CGAIABRD::OnBnClickedButtonOpen()
{
	// TODO: Add your control notification handler code here
	system("del *.SMP");
	m_Combobox.ResetContent();

	CFileDialog openFileDialog(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,_T("all files | *.*|| ")); 	
	if((openFileDialog.DoModal()) != IDOK) return;
	CString mPath = openFileDialog.GetPathName();

	CStdioFile fh;
	CString buf1, buf2, fmt;
	int ti, dummy, alen, dlen;
	DWORD addr, data;
	BYTE devid;
	CString ext;
	int line;
	int k;
	int i, j;
	CString strtxt;

	int ItemNum = 0;
	if (!fh.Open(mPath,CFile::modeRead)) return;
	while (fh.ReadString(buf1)) {
		if (buf1.Find(_T("@@"), 0)==0)
		{
			ItemNum++;
		}	
	}
	if(ItemNum>16)
	{
		AfxMessageBox(_T("Items too much!!"));
		system("del *.SMP");
		m_Combobox.ResetContent();
		return;
	}
	fh.Close();


	if (!fh.Open(mPath,CFile::modeRead)) return;

	i=0;
	line = 0;
	while (fh.ReadString(buf1)) {
		if (buf1.Find(_T("@@"), 0)==0)
		{
			Reglist1[i][0].Counter=j;
			m_Combobox.AddString(buf1);
			i++;
			j=0;
			continue;
		}
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
		Reglist1[i][j].SccbId=devid;
		Reglist1[i][j].RegAddr=addr;
		Reglist1[i][j].RegVal=data;
		j++;
	}
	Reglist1[i][0].Counter=j;
	fh.Close();

	FILE *fp;
	CString TmpName;
	for(int m =1; m < i+1; m++)
	{
		TmpName.Format(_T("Reg%02d.SMP"), m);
		int err = fopen_s(&fp, TmpName,"wb");
		if(err != 0)
		{
			AfxMessageBox(_T("Parse failure!!"));
			system("del *.SMP");
			m_Combobox.ResetContent();
			return;
		}
		for(int n = 0; n < Reglist1[m][0].Counter; n++)
		{
			fprintf(fp, "%02x %04x %02x;\r\n", Reglist1[m][n].SccbId, Reglist1[m][n].RegAddr, Reglist1[m][n].RegVal);
		}
		fclose(fp);
	}
	
	m_Combobox.SetCurSel(0);
}


void CGAIABRD::OnBnClickedButtonSend()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	CString TmpName;
	TmpName.Format(_T("Reg%02d.SMP"), m_Combobox.GetCurSel()+1);

	parent->m_csFilename = TmpName;
	parent->RunTextScript();

}


void CGAIABRD::OnBnClickedButtonBatchread()
{
	// TODO: Add your control notification handler code here
	CAutoDebugDlg *parent = (CAutoDebugDlg *)m_pMianDlg;

	CString strtxt;
	int SCCB_ID=0, RegAddr=0;
	DWORD Length=0;

	m_SccbId.GetWindowTextA(s_SccbId);
	sscanf_s(s_SccbId,_T("%x"),&SCCB_ID);
	m_Addr.GetWindowTextA(s_Addr);
	sscanf_s(s_Addr,_T("%x"),&RegAddr);
	m_Length.GetWindowTextA(s_Length);
	sscanf_s(s_Length,_T("%x"),&Length);
	
	int Is16bitAddr = ((CButton*)GetDlgItem(IDC_CHECK_Is16bAddrBatch))->GetCheck();

	BYTE *pBuf = new BYTE[Length];
    memset(pBuf, 0, Length); 

	for(DWORD i=0; i<Length; i+=BurstSize)
	{
		if((i+BurstSize)>Length)BurstReadRegister(SCCB_ID, RegAddr, pBuf+i, Is16bitAddr, (BYTE)(Length-i));
		else BurstReadRegister(SCCB_ID, RegAddr, pBuf+i, Is16bitAddr, BurstSize);
	}

	for(DWORD j=0; j<Length; j++)
	{
		if(Is16bitAddr)strtxt.Format(_T("%02x %04x %02x;<r>\r\n"), SCCB_ID, RegAddr+j, pBuf[j]);
		else if(RegAddr>0xff)strtxt.Format(_T("%02x %04x %02x;<r>\r\n"), SCCB_ID, RegAddr+j, pBuf[j]);
		else strtxt.Format(_T("%02x %02x %02x;<r>\r\n"), SCCB_ID, RegAddr+j, pBuf[j]);
		parent->Log(strtxt);
	}

	delete[] pBuf;

}


void CGAIABRD::OnBnClickedCheckIs16baddrbatch()
{
	// TODO: Add your control notification handler code here
}
