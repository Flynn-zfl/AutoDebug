#pragma once
#include "afxwin.h"


// COV496ISP dialog

class COV496ISP : public CDialogEx
{
	DECLARE_DYNAMIC(COV496ISP)

public:
	COV496ISP(CWnd* pParent = NULL);   // standard constructor
	virtual ~COV496ISP();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_OV496 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void Flash_Burn(void);
	UINT static WorkerThread_FlashBurn( LPVOID pParam);

public:
	CDialog *m_pMianDlg;
	afx_msg void OnOK();
	afx_msg void OnClose();
	void set_dlg(CDialog * parnet_dlg);
	void ServiceMessages();
	afx_msg BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedButtonCheckid();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonReadflash();
	afx_msg void OnBnClickedButtonBurnflash();
	CEdit m_ISP_ID;
	CStatic m_IDs;

	afx_msg BOOL WriteDataToDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode);
	afx_msg void ReadDataFromDevice(unsigned char nID, unsigned char *pBuf, unsigned int data_start, unsigned int data_size, unsigned char mode);
	CButton m_ReadFlash;
	CButton m_BurnFlash;
	CEdit m_SVal;
	CEdit m_SAddr;
	CEdit m_SId;
	CButton m_ReadSNR;
	CButton m_WriteSNR;
	afx_msg void OnBnClickedButtonReadsnr();
	afx_msg void OnBnClickedButtonWritesnr();
	CString s_SId;
	CString s_SAddr;
	CString s_SVal;

	CButton m_check_verify;
};
