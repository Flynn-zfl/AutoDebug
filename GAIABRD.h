#pragma once
#include "afxwin.h"


// CGAIABRD dialog

class CGAIABRD : public CDialogEx
{
	DECLARE_DYNAMIC(CGAIABRD)

public:
	CGAIABRD(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGAIABRD();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_GAIA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CDialogEx *m_pMianDlg;
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	CButton c_BUTTON1;
	CButton c_BUTTON2;
	CButton c_BUTTON3;
	CButton c_BUTTON4;
	CButton c_BUTTON5;
	CButton c_BUTTON6;
	CButton c_BUTTON7;
	CButton c_BUTTON8;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();	
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonSend();
	CComboBox m_Combobox;
	afx_msg void OnBnClickedButtonBatchread();
	afx_msg void OnBnClickedCheckIs16baddrbatch();

	CEdit m_SccbId;
	CEdit m_Addr;
	CEdit m_Length;

	CString s_Length;
	CString s_Addr;
	CString s_SccbId;

};
