#pragma once
#include "afxwin.h"


// COV490ISP dialog

class COV490ISP : public CDialogEx
{
	DECLARE_DYNAMIC(COV490ISP)

public:
	COV490ISP(CWnd* pParent = NULL);   // standard constructor
	virtual ~COV490ISP();

	// Dialog Data
	enum { IDD = IDD_DIALOG_OV490 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	void Flash_Burn(void);
	void Flash_Read(void);
	UINT static WorkerThread_FlashBurn( LPVOID pParam);
	UINT static WorkerThread_FlashRead( LPVOID pParam);

public:
	CDialog *m_pMianDlg;
	void set_dlg(CDialog * parnet_dlg);
	void ServiceMessages();

	afx_msg void OnBnClickedButtonB8080();
	afx_msg void OnBnClickedButtonB8020();
	afx_msg void OnBnClickedButtonB8028();
	afx_msg void OnBnClickedButtonB8029();
	afx_msg void OnBnClickedButtonB8018();
	afx_msg void OnBnClickedButtonB8019();
	afx_msg void OnBnClickedButtonSpiinit();
	afx_msg void OnBnClickedButtonB8082();
	afx_msg void OnBnClickedButtonEraseflash();
	afx_msg void OnBnClickedButtonReadflash();
	afx_msg void OnBnClickedButtonBurnflash();
	afx_msg void OnBnClickedButtonIdcheck();

	CButton m_Spiinit;
	CButton m_Eraseflash;
	CButton m_Readflash;
	CButton m_Burnflash;
	CEdit m_ISP_ID;
	afx_msg void OnBnClickedButtonSetid();
	CButton m_check_verify;
	CStatic m_IDs;
	afx_msg void OnEnChangeEditSid();
	afx_msg void OnEnChangeEditSaddr();
	afx_msg void OnEnChangeEditSval();
	afx_msg void OnBnClickedButtonReadsnr();
	afx_msg void OnBnClickedButtonWritesnr();


	CEdit m_SVal;
	CEdit m_SAddr;
	CEdit m_SId;
	CButton m_ReadSNR;
	CButton m_WriteSNR;
	CString s_SId;
	CString s_SAddr;
	CString s_SVal;
};
