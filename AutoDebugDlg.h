
// AutoDebugDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "GAIABRD.h"
#include "OV490ISP.h"
#include "OV494ISP.h"
#include "OV495ISP.h"
#include "OV496ISP.h"
#include "TISERDES.h"
#include "MAXIMSERDES.h"
#include "OV480ISP.h"
#include "OV491ISP.h"
#include "X1FSOC.h"

// CAutoDebugDlg dialog
class CAutoDebugDlg : public CDialogEx
{
	// Construction
public:
	CAutoDebugDlg(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data
	enum { IDD = IDD_AUTODEBUG_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CDialogEx *m_pMianDlg;
	//keyboard
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnOK();
	afx_msg void OnClose();
	afx_msg void RunTextScript();
	afx_msg void ServiceMessages();
	afx_msg void Log(LPCTSTR text);
	afx_msg void WriteReg(int SCCB_Addr, int RegAddr,int RegData);
	afx_msg int ReadReg(int SCCB_Addr, int RegAddr);
	COV490ISP m_ov490isp;
	COV494ISP m_ov494isp;
	COV495ISP m_ov495isp;
	COV496ISP m_ov496isp;
	CTISERDES m_tiserdes;
	CMAXIMSERDES m_maximserdes;
	CGAIABRD m_gaiaboard;
	COV480ISP m_ov480isp;
	COV491ISP m_ov491isp;
	CX1FSOC m_x1fsoc;

	afx_msg void OnBnClickedCheckIs16baddr();
	afx_msg void OnBnClickedButtonRead();
	afx_msg void OnBnClickedButtonWrite();
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnTcnSelchangeTabIsp(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_SccbId;
	CEdit m_Addr;
	CEdit m_Val;
	CString s_Val;
	CString s_Addr;
	CString s_SccbId;

	CEdit m_SccbId2;
	CEdit m_Addr2;
	CEdit m_Val2;
	CString s_Val2;
	CString s_Addr2;
	CString s_SccbId2;

	CTabCtrl m_TabMain;
	afx_msg void OnBnClickedRadioPanther();
	afx_msg void OnBnClickedRadioFt4222();
	CStatic m_Status;
	CString s_Status;
	CListBox m_Log;
	CString s_Log;
	afx_msg void OnBnClickedButtonLog();
	afx_msg void OnLbnSelchangeListLog();
	CString m_csFilename;
	afx_msg void OnBnClickedButtonClr();
	afx_msg void OnBnClickedButtonRead2();
	afx_msg void OnBnClickedButtonWrite2();
};
